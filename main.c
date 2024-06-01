// TODO implement the way to concatenate error message
// TODO make entire code use dynamic array instead of static
//
//  TODO make options subcommands
//  TODO add complete subcommand
//  TODO add uncomplete subcommand
//  TODO add delete subcommand
//  TODO add edit subcommand
//  TODO add tag subcommand
//  TODO add sub-todo to todo

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define TASK_CAPACITY 128  // 2 ^ 7
#define MAX_TASK_NAME_LENGTH 32  // 2 ^ 5
#define MAX_TASK_FILE_LENGTH 128 * 32  // 2 ^ 12
#define DATA_FILE_NAME "todo.bin"  // TODO Define file-name as a variable
                                   // Instead of macro to pass the file-name as an argument outside of function.

enum Err {
    ERR_OK,
    ERR_FAILED, // TODO Add more Errors e.g. ERR_TOO_LONG_TASKNAME, ERR_FAILED_TO_READ_FILE ...
};

typedef struct {
    char name[MAX_TASK_NAME_LENGTH];
    int is_completed;  // 0 = not completed, 1 = completed
} Todo;

typedef struct {
    Todo data[TASK_CAPACITY];
    size_t count;
} Todo_List;

char *shift_arg(int *argc, char** *argv);
void usage(const char *program_size);
enum Err add_todo(const char *todo_name);
enum Err list_todos();
enum Err load_todos(Todo_List *todo_list);
enum Err save_todos(const Todo_List todo_list);


char *shift_arg(int *argc, char** *argv)
{
    if (*argc <= 0) {
        fprintf(stderr, "No more arguments to shift\n");
        return NULL;
    }
    char *result = **argv;
    *argc -= 1;
    *argv += 1;
    return result;
}

void usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s [-a | --add] | [-l | --list]\n", program_name);
}

enum Err load_todos(Todo_List *todo_list)
{
    FILE *file = fopen(DATA_FILE_NAME, "rb");
    if (file == NULL) {
        if (errno == ENOENT) {
            // File does not exist, treat as empty list
            todo_list->count = 0;
            return ERR_OK;
        }
        fprintf(stderr, "Failed to open todo file\n");  // TODO Make this line need not to write here
        return ERR_FAILED;
    }

    size_t read_count = fread(todo_list->data, sizeof(Todo), TASK_CAPACITY, file);
    if (ferror(file)) {
        fprintf(stderr, "Failed to read todo file\n");
        return ERR_FAILED;
    }

    todo_list->count = read_count;
    fclose(file);
    return ERR_OK;
}

enum Err save_todos(const Todo_List todo_list)
{
    FILE *file = fopen(DATA_FILE_NAME, "wb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open todo file\n");
        return ERR_FAILED;
    }

    fwrite(todo_list.data, sizeof(Todo), todo_list.count, file);

    if (ferror(file)) {
        fprintf(stderr, "Failed to write to todo file\n");
        fclose(file);
        return ERR_FAILED;
    }

    fclose(file);
    return ERR_OK;
}

enum Err add_todo(const char *todo_name)  // TODO make `add_todo` don't add todo if todo_name is dupulicated.
{

    Todo_List todos = { 0 };

    if (load_todos(&todos) != ERR_OK) {
        return ERR_FAILED;
    }

    if (todos.count >= TASK_CAPACITY) {
        fprintf(stderr, "Todo capacity reached\n");
        return ERR_FAILED;
    }

    strncpy(todos.data[todos.count].name, todo_name, MAX_TASK_NAME_LENGTH - 1);
    todos.data[todos.count].name[MAX_TASK_NAME_LENGTH - 1] = '\0';  // Ensure null-termination
    todos.data[todos.count].is_completed = 0;
    todos.count++;

    if (save_todos(todos) != ERR_OK) {
        return ERR_FAILED;
    }

    return ERR_OK;
}

enum Err list_todos() // 48
{
    Todo_List todos = { 0 };
    if (load_todos(&todos) != ERR_OK) {
        return ERR_FAILED;
    }

    for (size_t i=0; i < todos.count; i++) {
        Todo todo = todos.data[i];
        char complete_mark = todo.is_completed ? 'x' : ' ';
        printf("%zu: %s [%c]\n", i+1, todo.name, complete_mark); 
    }

    return ERR_OK;
}

int main(int argc, char** argv)
{
    const char *program_name = shift_arg(&argc, &argv);
    if (argc == 0) {
        usage(program_name);
        fprintf(stderr, "ERROR: arguments were not provided\n");
        return 1;
    }
    
    while (argc > 0) {
        char *flag = shift_arg(&argc, &argv);
        if (strcmp(flag, "-l") == 0 || strcmp(flag, "--list") == 0) {
            if (list_todos() != ERR_OK) return 1;
        } else if (strcmp(flag, "-a") == 0 || strcmp(flag, "--add") == 0) {
            if (argc == 0) {
                usage(program_name);
                fprintf(stderr, "ERROR: todo name is not provided\n");
                return 1;
            }
            const char *todo_name = shift_arg(&argc, &argv);
            if (add_todo(todo_name) != ERR_OK) return 1;
        } else {
            usage(program_name);
            fprintf(stderr, "ERROR: unknown flag `%s`\n", flag);
            return 1;
        }
    }

    return 0;
}

