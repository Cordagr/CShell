#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGUMENTS 20

void execute_command(char *command) {
    char *args[MAX_ARGUMENTS + 1];
    char *token;
    int arg_count = 0;
    int redirect_input = 0, redirect_output = 0;
    char *input_file, *output_file;

    token = strtok(command, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            redirect_input = 1;
            token = strtok(NULL, " ");
            input_file = token;
        } else if (strcmp(token, ">") == 0) {
            redirect_output = 1;
            token = strtok(NULL, " ");
            output_file = token;
        } else {
            args[arg_count++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    int pid = fork();
    if (pid < 0) {
        printf("Fork failed.\n");
        exit(1);
    } else if (pid == 0) {
        if (redirect_input) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                printf("Error opening input file.\n");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (redirect_output) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd < 0) {
                printf("Error opening output file.\n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execvp(args[0], args);
        printf("Command execution failed.\n");
        exit(1);
    } else {
        wait(NULL);
    }
}

void change_directory(const char *path) {
    if (path == NULL) {
        // No argument provided, change to HOME directory
        struct passwd *pw = getpwuid(getuid());
        if (pw == NULL) {
            printf("Error: HOME directory not found.\n");
            return;
        }
        path = pw->pw_dir;
    }

    if (chdir(path) != 0) {
        printf("Error: Failed to change directory to %s\n", path);
    }
}

int main(int argc, char *argv[]) {
    char input[MAX_COMMAND_LENGTH];

    // Interactive mode
    if (argc == 1) {
        while (1) {
            printf("$ ");
            fgets(input, MAX_COMMAND_LENGTH, stdin);
            input[strcspn(input, "\n")] = '\0'; // Remove trailing newline

            if (strcmp(input, "exit") == 0) {
                break;
            } else {
                char *token = strtok(input, ";");
                while (token != NULL) {
                    execute_command(token);
                    token = strtok(NULL, ";");
                }
            }
        }
    }
    // Batch mode
    else if (argc == 2) {
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Error opening file.\n");
            exit(1);
        }

        while (fgets(input, MAX_COMMAND_LENGTH, file) != NULL) {
            input[strcspn(input, "\n")] = '\0'; // Remove trailing newline
            char *token = strtok(input, ";");
            while (token != NULL) {
                execute_command(token);
                token = strtok(NULL, ";");
            }
        }

        fclose(file);
    } else {
        printf("Usage: %s [batch_file]\n", argv[0]);
        exit(1);
    }

    return 0;
}
