#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200

void parse_command(char *input, char **args) {
    int i = 0;
    char *ptr = input;
    while (*ptr) {
        while (*ptr == ' ') ptr++;
        if (*ptr == '"') {
            ptr++;
            args[i++] = ptr;
            while (*ptr && *ptr != '"') ptr++;
            if (*ptr) {
                *ptr = '\0';
                ptr++;
            }
        } else {
            args[i++] = ptr;
            while (*ptr && *ptr != ' ') ptr++;
            if (*ptr) {
                *ptr = '\0';
                ptr++;
            }
        }
    }
    args[i] = NULL;
}

char *trim(char *str) {
    while (*str == ' ') str++;
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }
    return str;
}

int main() {
    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) {
        printf("Shell> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        char *token = strtok(command, "|");
        while (token != NULL) {
            commands[command_count++] = trim(token);
            token = strtok(NULL, "|");
        }

        int pipefd[2];
        int in_fd = 0;
        pid_t pid;

        for (int i = 0; i < command_count; i++) {
            if (i < command_count - 1)
                pipe(pipefd);

            pid = fork();
            if (pid == 0) {
                if (in_fd != 0) {
                    dup2(in_fd, 0);
                    close(in_fd);
                }
                if (i < command_count - 1) {
                    dup2(pipefd[1], 1);
                    close(pipefd[0]);
                    close(pipefd[1]);
                }

                char *args[64];
                parse_command(commands[i], args);
                execvp(args[0], args);
                perror("execvp failed");
                exit(1);
            } else if (pid < 0) {
                perror("fork failed");
                exit(1);
            } else {
                if (in_fd != 0)
                    close(in_fd);
                if (i < command_count - 1) {
                    close(pipefd[1]);
                    in_fd = pipefd[0];
                }
            }
        }

        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }

        command_count = 0;
    }

    return 0;
}

