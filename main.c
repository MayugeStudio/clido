/*
 * TODO add delete subcommand
 * TODO add edit subcommand
 * TODO add tag subcommand
 * TODO add sub-todo to todo
 * TODO add marking todo as working in progress subcommand
 * TODO implement the way to concatenate error message
 * TODO make entire code use dynamic array instead of static
*/

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define TASK_CAPACITY 128  // 2 ^ 7
#define MAX_TASK_NAME_LENGTH 32  // 2 ^ 5
#define MAX_TASK_FILE_LENGTH 128 * 32  // 2 ^ 12

enum Error
{
    ERROR_OK,
    ERROR_FAILED, // TODO Add more Errors e.g. ERR_TOO_LONG_TASKNAME, ERR_FAILED_TO_READ_FILE ...
};

typedef struct
{
    char name[MAX_TASK_NAME_LENGTH];
    int is_completed;  // 0 = not completed, 1 = completed
} Todo;

typedef struct
{
    Todo data[TASK_CAPACITY];
    size_t count;
} Todo_List;

char *shift_arg(int *argc, char** *argv);
void usage(const char *program_size);
enum Error add_todo(const char *filename, const char *todo_name);
enum Error delete_todo(const char *filename, const char *todo_name);
enum Error list_todos(const char *filename);
enum Error complete_todo(const char* filename, const char *todo_name);
enum Error uncomplete_todo(const char* filename, const char *todo_name);
enum Error load_todos(const char *filename, Todo_List *todo_list);
enum Error save_todos(const char *filename, const Todo_List todo_list);


char *shift_arg(int *argc, char** *argv)
{
    if (*argc <= 0)
    {
        fprintf(stderr, "No more arguments to shift\n");  // TODO Make this line need not to write here
        return NULL;
    }

    char *result = **argv;
    *argc -= 1;
    *argv += 1;
    return result;
}

void usage(const char *program_name)
{
    printf("Usage: %s <subcommand> <args> <options>\n", program_name);
    printf("        add          Add todo\n");
    printf("        list         List todos\n");
    printf("        complete     Mark todo as complete\n");
    printf("        uncomplete   Mark todo as uncomplete\n");
}

enum Error load_todos(const char *filename, Todo_List *todo_list)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        if (errno == ENOENT)
        {
            // File does not exist, treat as empty list
            todo_list->count = 0;
            return ERROR_OK;
        }

        fprintf(stderr, "Failed to open todo file\n");  // TODO Make this line need not to write here
        return ERROR_FAILED;
    }

    size_t read_count = fread(todo_list->data, sizeof(Todo), TASK_CAPACITY, file);
    if (ferror(file))
    {
        fprintf(stderr, "Failed to read todo file\n");  // TODO Make this line need not to write here
        fclose(file);
        return ERROR_FAILED;
    }

    todo_list->count = read_count;
    fclose(file);
    return ERROR_OK;
}

enum Error save_todos(const char *filename, const Todo_List todo_list)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Failed to open todo file\n");  // TODO Make this line need not to write here
        return ERROR_FAILED;
    }

    fwrite(todo_list.data, sizeof(Todo), todo_list.count, file);

    if (ferror(file)) {
        fprintf(stderr, "Failed to write to todo file\n");  // TODO Make this line need not to write here
        fclose(file);
        return ERROR_FAILED;
    }

    fclose(file);
    return ERROR_OK;
}

enum Error add_todo(const char *filename, const char *todo_name)  // TODO make `add_todo` don't add todo if todo_name is dupulicated.
{

    Todo_List todos = { 0 };

    if (load_todos(filename, &todos) != ERROR_OK) return ERROR_FAILED;

    if (todos.count >= TASK_CAPACITY)
    {
        fprintf(stderr, "Todo capacity reached\n");  // TODO Make this line need not to write here
        return ERROR_FAILED;
    }

    strncpy(todos.data[todos.count].name, todo_name, MAX_TASK_NAME_LENGTH - 1);
    todos.data[todos.count].name[MAX_TASK_NAME_LENGTH - 1] = '\0';  // Ensure null-termination
    todos.data[todos.count].is_completed = 0;
    todos.count++;

    if (save_todos(filename, todos) != ERROR_OK)
    {
        return ERROR_FAILED;
    }

    return ERROR_OK;
}

enum Error delete_todo(const char *filename, const char *todo_name)
{
    Todo_List todos = { 0 };
    if (load_todos(filename, &todos) != ERROR_OK) return ERROR_FAILED;

    for (size_t i=0; i<todos.count; i++)
    {
        if (strcmp(todos.data[i].name, todo_name) == 0)
        {
            for (size_t j=i; j<todos.count-1; j++)
            {
                todos.data[j] = todos.data[j + 1];
            }
            todos.count--;
            if (save_todos(filename, todos) != ERROR_OK) return ERROR_FAILED;
            return ERROR_OK;
        }
    }

    fprintf(stderr, "ERROR: The task named `%s` doesn't exist", todo_name);
    return ERROR_FAILED;
}

enum Error list_todos(const char* filename)
{
    Todo_List todos = { 0 };
    if (load_todos(filename, &todos) != ERROR_OK) return ERROR_FAILED;

    for (size_t i=0; i < todos.count; i++)
    {
        Todo todo = todos.data[i];
        char complete_mark = todo.is_completed ? 'x' : ' ';
        printf("%zu: %s [%c]\n", i+1, todo.name, complete_mark); 
    }

    return ERROR_OK;
}

enum Error complete_todo(const char *filename, const char *todo_name)
{
    Todo_List todos = { 0 };
    if (load_todos(filename, &todos) != ERROR_OK) return ERROR_FAILED;

    for (size_t i=0; i < todos.count; i++)
    {
        if (strcmp(todos.data[i].name, todo_name) == 0)
        {
            todos.data[i].is_completed = 1;

            if (save_todos(filename, todos) != ERROR_OK) return ERROR_FAILED;
            return ERROR_OK;
        }
    }

    fprintf(stderr, "ERROR: The task named `%s` doesn't exist", todo_name);
    return ERROR_FAILED;
}

enum Error uncomplete_todo(const char* filename, const char *todo_name)
{
    Todo_List todos = { 0 };
    if (load_todos(filename, &todos) != ERROR_OK) return ERROR_FAILED;

    for (size_t i=0; i<todos.count; i++)
    {
        if (strcmp(todos.data[i].name, todo_name) == 0)
        {
            todos.data[i].is_completed = 0;

            if (save_todos(filename, todos) != ERROR_OK) return ERROR_FAILED;
            return ERROR_OK;
        }
    }

    fprintf(stderr, "ERROR: The task named `%s` doesn't exist", todo_name);
    return ERROR_FAILED;
}

int main(int argc, char** argv)
{
    const char *filename = "todo.bin";
    const char *program_name = shift_arg(&argc, &argv);
    if (argc == 0)
    {
        usage(program_name);
        fprintf(stderr, "ERROR: No arguments specified.\n");  // TODO Make this line need not to write here
        return 1;
    }
    
    char *subcommand_name = shift_arg(&argc, &argv);
    if (strcmp(subcommand_name, "--help") == 0 || strcmp(subcommand_name, "-h") == 0)
    {
        usage(program_name);
        return 0;
    }

    if (strcmp(subcommand_name, "add") == 0)
    {
        if (argc == 0)
        {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");  // TODO Make this line need not to write here
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (add_todo(filename, todo_name) != ERROR_OK) return 1;
    }
    else if (strcmp(subcommand_name, "delete") == 0)
    {
        if (argc == 0)
        {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (delete_todo(filename, todo_name) != ERROR_OK) return 1;
    }
    else if (strcmp(subcommand_name, "list") == 0)
    {
        if (list_todos(filename) != ERROR_OK) return 1;
    }
    else if (strcmp(subcommand_name, "complete") == 0)
    {
        if (argc == 0)
        {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (complete_todo(filename, todo_name) != ERROR_OK) return 1;
    }
    else if (strcmp(subcommand_name, "uncomplete") == 0)
    {
        if (argc == 0)
        {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (uncomplete_todo(filename, todo_name) != ERROR_OK) return 1;
    }
    else
    {
        usage(program_name);
        fprintf(stderr, "ERROR: unknown subcommand `%s`\n", subcommand_name);  // TODO Make this line need not to write here
        return 1;
    }

    return 0;
}

