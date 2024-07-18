#include "syscalls.h"

void print_prompt() {
    printf("\033[0;32m%s\033[0m> ", current_path);
}

void parse_command(char *command) {
    char *token = strtok(command, " ");

    if (strcmp(token, "ls") == 0) {
        list_directory();
    } else if (strcmp(token, "touch") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            create_file(token);
        } else {
            fprintf(stderr, "Usage: touch <filename>\n");
        }
    } else if (strcmp(token, "mkdir") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            make_directory(token);
        } else {
            fprintf(stderr, "Usage: mkdir <dirname>\n");
        }
    } else if (strcmp(token, "cd") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            change_directory(token);
        } else {
            fprintf(stderr, "Usage: cd <dirname>\n");
        }
    } else {
        fprintf(stderr, "Unknown command: %s\n", token);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <disk_image_file>\n", argv[0]);
        return 1;
    }
    master_boot_record(argv[1]);

    printf("Disk was created successfully\n");

    while (true) {
        print_prompt();

        char command[100];
        fgets(command, sizeof(command), stdin);
        command[strlen(command) - 1] = '\0'; // Remove '\n'

        parse_command(command);
    }
    
}