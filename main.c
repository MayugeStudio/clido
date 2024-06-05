/*
 * TODO add tag subcommand
 * TODO add command struct
 * TODO delete one line code 
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

struct Todo {
    char name[MAX_TASK_NAME_LENGTH];
    int is_completed;  // 0 = not completed, 1 = completed
};

struct Todo_List {
    struct Todo data[TASK_CAPACITY];
    size_t count;
};

// CLI Utility Functions
char *shift_arg(int *argc, char** *argv);
void usage(const char *program_size);

// Command Functions
Error add_todo(const char *filename, const char *todo_name);
Error delete_todo(const char *filename, const char *todo_name);
Error edit_todo(const char *filename, const char *old_name, const char *new_name);
Error list_todo(const char *filename);
Error change_todo_status(const char *filename, const char *todo_name, int status);

// File IO Utility Functions
Error load_todo_list(const char *filename, struct Todo_List *todo_list);
Error save_todo_list(const char *filename, const struct Todo_List todo_list);

// Todo Utility Functions
struct Todo *find_todo_by_name(struct Todo_List *todo_list, const char *todo_name);
int find_todo_index_by_name(struct Todo_List *todo_list, const char *todo_name);

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

Error load_todo_list(const char *filename, struct Todo_List *todo_list)
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

    size_t read_count = fread(todo_list->data, sizeof(struct Todo), TASK_CAPACITY, file);
    if (ferror(file)) {
        fprintf(stderr, "Failed to read todo file\n");  // TODO Make this line need not to write here
        fclose(file);
        return ERROR_FAILED;
    }

    todo_list->count = read_count;
    fclose(file);
    return ERROR_OK;
}

Error save_todo_list(const char *filename, const struct Todo_List todo_list)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open todo file\n");  // TODO Make this line need not to write here
        return ERROR_FAILED;
    }

    fwrite(todo_list.data, sizeof(struct Todo), todo_list.count, file);

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
    if (strlen(todo_name) >= MAX_TASK_NAME_LENGTH-1) {
        return ERROR_FAILED;
    }

    struct Todo_List todo_list = { 0 };

    if (load_todo_list(filename, &todo_list) != ERROR_OK) {
        return ERROR_FAILED;
    }

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
    struct Todo_List todo_list = { 0 };

    if (load_todo_list(filename, &todo_list) != ERROR_OK) {
        return ERROR_FAILED;
    }

    int target_todo_index = find_todo_index_by_name(&todo_list, todo_name);
    if (target_todo_index == -1) {
        fprintf(stderr, "ERROR: The task named `%s` doesn't exist", todo_name);
        return ERROR_FAILED;
    }

    // shift all todo right except target_todo
    for (size_t i=target_todo_index; i<todo_list.count-1; i++) {
        todo_list.data[i] = todo_list.data[i+1];
    }

    todo_list.count--;
    if (save_todo_list(filename, todo_list) != ERROR_OK) {
        return ERROR_FAILED;
    }

    return ERROR_OK;
}

Error edit_todo(const char *filename, const char *old_name, const char *new_name)
{
    if (strlen(new_name) >= MAX_TASK_NAME_LENGTH - 1) {
        return ERROR_FAILED;
    }

    struct Todo_List todo_list = { 0 };

    if (load_todo_list(filename, &todo_list) != ERROR_OK) {
        return ERROR_FAILED;
    }

    struct Todo *target_todo = find_todo_by_name(&todo_list, old_name);
    if (target_todo == NULL) {
        fprintf(stderr, "ERROR: The task named `%s` doesn't exist\n", old_name);
    }

    memset(target_todo, '\0', MAX_TASK_NAME_LENGTH);
    strncpy(target_todo->name, new_name, MAX_TASK_NAME_LENGTH - 1);
    target_todo->name[MAX_TASK_NAME_LENGTH-1] = '\0';  // ensure null-termination

    if (save_todo_list(filename, todo_list) != ERROR_OK) {
        return ERROR_FAILED;
    }

    return ERROR_OK;
}

Error list_todo(const char* filename)
{
    struct Todo_List todo_list = { 0 };

    if (load_todo_list(filename, &todo_list) != ERROR_OK) {
        return ERROR_FAILED;
    }

    for (size_t i=0; i < todo_list.count; i++) {
        struct Todo todo = todo_list.data[i];
        char complete_mark = todo.is_completed ? 'x' : ' ';
        printf("%zu: %s [%c]\n", i+1, todo.name, complete_mark); 
    }

    return ERROR_OK;
}

Error change_todo_status(const char *filename, const char *todo_name, int status)
{
    assert(status == 0 || status == 1);

    struct Todo_List todo_list = { 0 };
    if (load_todo_list(filename, &todo_list) != ERROR_OK) {
        return ERROR_FAILED;
    }

    struct Todo *target_todo = find_todo_by_name(&todo_list, todo_name);
    if (target_todo == NULL) {
        fprintf(stderr, "ERROR: The task named `%s` doesn't exist", todo_name);
    }

    target_todo->is_completed = status;

    if (save_todo_list(filename, todo_list) != ERROR_OK) { 
        return ERROR_FAILED;
    }

    return ERROR_OK;
}


struct Todo *find_todo_by_name(struct Todo_List *todo_list, const char *todo_name)
{
    for (size_t i=0; i<todo_list->count; i++) {
        if (strcmp(todo_list->data[i].name, todo_name) == 0) {
            return &todo_list->data[i];
        }
    }
    return NULL;
}

int find_todo_index_by_name(struct Todo_List *todo_list, const char *todo_name)
{
    for (size_t i=0; i<todo_list->count; i++) {
        if (strcmp(todo_list->data[i].name, todo_name) == 0) {
            return i;
        }
    }
    return -1;
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
        if (add_todo(filename, todo_name) != ERROR_OK) {
            return 1;
        }
    } else if (strcmp(subcommand_name, "delete") == 0) {
        if (argc == 0) {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (delete_todo(filename, todo_name) != ERROR_OK) {
            return 1;
        }

    } else if (strcmp(subcommand_name, "list") == 0) {
        if (list_todo(filename) != ERROR_OK) {
            return 1;
        }

    } else if (strcmp(subcommand_name, "complete") == 0) {
        if (argc == 0) {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (change_todo_status(filename, todo_name, 1) != ERROR_OK) {
            return 1;
        }

    } else if (strcmp(subcommand_name, "uncomplete") == 0) {
        if (argc == 0) {
            usage(program_name);
            fprintf(stderr, "ERROR: todo name doesn't specified\n");
            return 1;
        }

        const char *todo_name = shift_arg(&argc, &argv);
        if (change_todo_status(filename, todo_name, 0) != ERROR_OK) {
            return 1;
        }

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

        if (edit_todo(filename, old_name, new_name) != ERROR_OK) {
            return 1;
        }

    } else {
        usage(program_name);
        fprintf(stderr, "ERROR: unknown subcommand `%s`\n", subcommand_name);  // TODO Make this line need not to write here
        return 1;
    }

    return 0;
}

