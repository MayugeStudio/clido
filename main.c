/*
 * TODO add tag subcommand
 * TODO add search todo function
 * TODO add sub-todo to todo
 * TODO add marking todo as working in progress subcommand
 * TODO implement error collector
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

typedef enum {
    ERROR_OK,
    ERROR_FAILED, // TODO Add more Errors e.g. ERR_TOO_LONG_TASKNAME, ERR_FAILED_TO_READ_FILE ...
} Error;

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
Error add_todo(const char *filename, const char *todo_name);
Error delete_todo(const char *filename, const char *todo_name);
Error edit_todo(const char *filename, const char *old_name, const char *new_name);
Error list_todo(const char *filename);
Error complete_todo(const char* filename, const char *todo_name);
Error uncomplete_todo(const char* filename, const char *todo_name);
Error load_todo_list(const char *filename, Todo_List *todo_list);
Error save_todo_list(const char *filename, const Todo_List todo_list);


char *shift_arg(int *argc, char** *argv)
{
    if (*argc <= 0) {
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
    printf("        add         <todo-name>             Add todo\n");
    printf("        delete      <todo-name>             Delete todo\n");
    printf("        edit        <old-name> <new-name>   Edit todo name\n");
    printf("        list                                List todo\n");
    printf("        complete    <todo-name>             Mark todo as complete\n");
    printf("        uncomplete  <todo-name>             Mark todo as uncomplete\n");
}

Error load_todo_list(const char *filename, Todo_List *todo_list)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        if (errno == ENOENT) {
            // File does not exist, treat as empty list
            todo_list->count = 0;
            return ERROR_OK;
        }

        fprintf(stderr, "Failed to open todo file\n");  // TODO Make this line need not to write here
        return ERROR_FAILED;
    }

    size_t read_count = fread(todo_list->data, sizeof(Todo), TASK_CAPACITY, file);
    if (ferror(file)) {
        fprintf(stderr, "Failed to read todo file\n");  // TODO Make this line need not to write here
        fclose(file);
        return ERROR_FAILED;
    }

    todo_list->count = read_count;
    fclose(file);
    return ERROR_OK;
}

Error save_todo_list(const char *filename, const Todo_List todo_list)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
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

Error add_todo(const char *filename, const char *todo_name)  // TODO make `add_todo` don't add todo if todo_name is dupulicated.
{
    if (strlen(todo_name) >= MAX_TASK_NAME_LENGTH-1) { return ERROR_FAILED; }

    Todo_List todo_list = { 0 };

    if (load_todo_list(filename, &todo_list) != ERROR_OK) return ERROR_FAILED;

    if (todo_list.count >= TASK_CAPACITY) {
        fprintf(stderr, "Todo capacity reached\n");  // TODO Make this line need not to write here
        return ERROR_FAILED;
    }

    strncpy(todo_list.data[todo_list.count].name, todo_name, MAX_TASK_NAME_LENGTH - 1);
    todo_list.data[todo_list.count].name[MAX_TASK_NAME_LENGTH - 1] = '\0';  // Ensure null-termination
    todo_list.data[todo_list.count].is_completed = 0;
    todo_list.count++;

    if (save_todo_list(filename, todo_list) != ERROR_OK) {
        return ERROR_FAILED;
    }

    return ERROR_OK;
}

Error delete_todo(const char *filename, const char *todo_name)
{
    Todo_List todo_list = { 0 };

    if (load_todo_list(filename, &todo_list) != ERROR_OK) return ERROR_FAILED;

    for (size_t i=0; i<todo_list.count; i++) {
        if (strcmp(todo_list.data[i].name, todo_name) == 0) {
            for (size_t j=i; j<todo_list.count-1; j++) {
                todo_list.data[j] = todo_list.data[j + 1];
            }
            todo_list.count--;
            if (save_todo_list(filename, todo_list) != ERROR_OK) return ERROR_FAILED;
            return ERROR_OK;
        }
    }

    fprintf(stderr, "ERROR: The task named `%s` doesn't exist", todo_name);
    return ERROR_FAILED;
}

Error edit_todo(const char *filename, const char *old_name, const char *new_name)
{
    if (strlen(new_name) >= MAX_TASK_NAME_LENGTH - 1) { return ERROR_FAILED; }

    Todo_List todo_list = { 0 };

    if (load_todo_list(filename, &todo_list) != ERROR_OK) return ERROR_FAILED;

    for (size_t i=0; i<todo_list.count; i++) {
        if (strcmp(todo_list.data[i].name, old_name) == 0) {
            memset(todo_list.data[i].name, '\0', MAX_TASK_NAME_LENGTH);
            strncpy(todo_list.data[i].name, new_name, MAX_TASK_NAME_LENGTH - 1);
            todo_list.data[i].name[MAX_TASK_NAME_LENGTH-1] = '\0';  // ensure null-termination
            if (save_todo_list(filename, todo_list) != ERROR_OK) return ERROR_FAILED;
            return ERROR_OK;
        }
    }

    fprintf(stderr, "ERROR: The task named `%s` doesn't exist\n", old_name);
    return ERROR_FAILED;
}

Error list_todo(const char* filename)
{
    Todo_List todo_list = { 0 };

    if (load_todo_list(filename, &todo_list) != ERROR_OK) return ERROR_FAILED;

    for (size_t i=0; i < todo_list.count; i++) {
        Todo todo = todo_list.data[i];
        char complete_mark = todo.is_completed ? 'x' : ' ';
        printf("%zu: %s [%c]\n", i+1, todo.name, complete_mark); 
    }

    return ERROR_OK;
}

Error complete_todo(const char *filename, const char *todo_name) {
    Todo_List todo_list = { 0 };
    if (load_todo_list(filename, &todo_list) != ERROR_OK) return ERROR_FAILED;

    for (size_t i=0; i < todo_list.count; i++) {
        if (strcmp(todo_list.data[i].name, todo_name) == 0) {
            todo_list.data[i].is_completed = 1;

            if (save_todo_list(filename, todo_list) != ERROR_OK) return ERROR_FAILED;
            return ERROR_OK;
        }
    }

    fprintf(stderr, "ERROR: The task named `%s` doesn't exist", todo_name);
    return ERROR_FAILED;
}

Error uncomplete_todo(const char* filename, const char *todo_name)
{
    Todo_List todo_list = { 0 };
    if (load_todo_list(filename, &todo_list) != ERROR_OK) return ERROR_FAILED;

    for (size_t i=0; i<todo_list.count; i++) {
        if (strcmp(todo_list.data[i].name, todo_name) == 0) {
            todo_list.data[i].is_completed = 0;

            if (save_todo_list(filename, todo_list) != ERROR_OK) return ERROR_FAILED;
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
    if (argc == 0) {
        usage(program_name);
        fprintf(stderr, "ERROR: No arguments specified.\n");  // TODO Make this line need not to write here
        return 1;
    }
    
    char *subcommand_name = shift_arg(&argc, &argv);
    if (strcmp(subcommand_name, "--help") == 0 || strcmp(subcommand_name, "-h") == 0) {
        usage(program_name);
        return 0;
    }

    if (strcmp(subcommand_name, "add") == 0) {
        if (argc == 0) {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");  // TODO Make this line need not to write here
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (add_todo(filename, todo_name) != ERROR_OK) return 1;
    } else if (strcmp(subcommand_name, "delete") == 0) {
        if (argc == 0) {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (delete_todo(filename, todo_name) != ERROR_OK) return 1;
    } else if (strcmp(subcommand_name, "list") == 0) {
        if (list_todo(filename) != ERROR_OK) return 1;
    } else if (strcmp(subcommand_name, "complete") == 0) {
        if (argc == 0) {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (complete_todo(filename, todo_name) != ERROR_OK) return 1;
    } else if (strcmp(subcommand_name, "uncomplete") == 0) {
        if (argc == 0) {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (uncomplete_todo(filename, todo_name) != ERROR_OK) return 1;
    } else if (strcmp(subcommand_name, "edit") == 0) {
        if (argc == 0) {
            usage(program_name);
            fprintf(stderr, "ERROR: old todo name doesn't specified\n");
            return 1;
        }

        const char *old_name = shift_arg(&argc, &argv);
        if (argc == 0) {
            usage(program_name);
            fprintf(stderr, "ERROR: new todo name doesn't specified\n");
            return 1;
        }
        const char *new_name = shift_arg(&argc, &argv);

        if (edit_todo(filename, old_name, new_name) != ERROR_OK) { return ERROR_FAILED; }
    } else {
        usage(program_name);
        fprintf(stderr, "ERROR: unknown subcommand `%s`\n", subcommand_name);  // TODO Make this line need not to write here
        return 1;
    }

    return 0;
}

