#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/types.h>

typedef struct {
    char **argv;
    int argc;
} Command;

typedef struct {
    Command *commands;
    int command_count;
} Pipeline;

void free_command(Command *cmd) {
    if (!cmd || !cmd->argv) return;
    for (int i = 0; i < cmd->argc; i++) {
        free(cmd->argv[i]);
    }
    free(cmd->argv);
    cmd->argv = NULL;
    cmd->argc = 0;
}

void free_pipeline(Pipeline *pipeline) {
    if (!pipeline || !pipeline->commands) return;
    for (int i = 0; i < pipeline->command_count; i++) {
        free_command(&pipeline->commands[i]);
    }
    free(pipeline->commands);
    pipeline->commands = NULL;
    pipeline->command_count = 0;
}

int parse_command(const char *cmd_str, Command *cmd) {
    cmd->argv = NULL;
    cmd->argc = 0;

    if (!cmd_str || *cmd_str == '\0') {
        return 0;
    }

    char *copy = strdup(cmd_str);
    if (!copy) return -1;

    int capacity = 4;
    cmd->argv = malloc(sizeof(char *) * capacity);
    if (!cmd->argv) {
        free(copy);
        return -1;
    }

    const char *delim = " \t\n\r";
    char *saveptr;
    char *token = strtok_r(copy, delim, &saveptr);

    while (token != NULL) {
        if (cmd->argc + 1 >= capacity) {
            capacity *= 2;
            char **new_argv = realloc(cmd->argv, sizeof(char *) * capacity);
            if (!new_argv) {
                free(copy);
                free_command(cmd);
                return -1;
            }
            cmd->argv = new_argv;
        }

        cmd->argv[cmd->argc] = strdup(token);
        cmd->argc++;

        token = strtok_r(NULL, delim, &saveptr);
    }

    cmd->argv[cmd->argc] = NULL;
    free(copy);

    return 0;
}

int parse_pipeline(const char *input, Pipeline *pipeline) {
    pipeline->commands = NULL;
    pipeline->command_count = 0;

    if (!input || *input == '\0') {
        return 0;
    }

    char *copy = strdup(input);
    if (!copy) return -1;

    int capacity = 2;
    pipeline->commands = malloc(sizeof(Command) * capacity);
    if (!pipeline->commands) {
        free(copy);
        return -1;
    }

    char *saveptr;
    char *segment = strtok_r(copy, "|", &saveptr);

    while (segment != NULL) {
        if (pipeline->command_count >= capacity) {
            capacity *= 2;
            Command *new_cmds = realloc(pipeline->commands, sizeof(Command) * capacity);
            if (!new_cmds) {
                free(copy);
                free_pipeline(pipeline);
                return -1;
            }
            pipeline->commands = new_cmds;
        }

        if (parse_command(segment, &pipeline->commands[pipeline->command_count]) != 0) {
            free(copy);
            free_pipeline(pipeline);
            return -1;
        }

        pipeline->command_count++;
        segment = strtok_r(NULL, "|", &saveptr);
    }

    free(copy);
    return 0;
}

void execute_pipeline(Pipeline *pipeline) {
    if (!pipeline || pipeline->command_count == 0) {
        return;
    }

    int count = pipeline->command_count;
    pid_t *pids = malloc(sizeof(pid_t) * count);
    if (!pids) {
        perror("malloc");
        return;
    }

    int prev_read = -1;

    for (int i = 0; i < count; i++) {
        int pipefd[2];
        int next_read = -1;
        int next_write = -1;

        if (i < count - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                free(pids);
                return;
            }
            next_read = pipefd[0];
            next_write = pipefd[1];
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(pids);
            return;
        }

        if (pid == 0) {

            if (prev_read != -1) {
                if (dup2(prev_read, 0) < 0) {
                    perror("dup2 prev_read");
                    exit(1);
                }
                close(prev_read);
            }

            if (i < count - 1) {
                if (dup2(next_write, 1) < 0) {
                    perror("dup2 next_write");
                    exit(1);
                }
                close(next_write);
                close(next_read);
            }

            execvp(pipeline->commands[i].argv[0], pipeline->commands[i].argv);

            perror("execvp");
            exit(1);
        }

        pids[i] = pid;

        if (prev_read != -1) {
            close(prev_read);
        }

        if (i < count - 1) {
            close(next_write);
            prev_read = next_read;
        }
    }

    if (prev_read != -1) {
        close(prev_read);
    }

    for (int i = 0; i < count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    free(pids);
}

char *read_line(void) {
    char *line = NULL;
    size_t bufsize = 0;
    ssize_t characters_read;

    characters_read = getline(&line, &bufsize, stdin);

    if (characters_read == -1) {
        free(line);
        return NULL;
    }

    if (characters_read > 0 && line[characters_read - 1] == '\n') {
        line[characters_read - 1] = '\0';
    }

    return line;
}

void shell_loop(void) {
    char *line = NULL;
    Pipeline pipeline;

    while (1) {
        printf("$ ");
        //fflush(stdout);

        line = read_line();

        if (line == NULL) {
            printf("\n");
            break;
        }

        if (line[0] == '\0') {
            free(line);
            continue;
        }

        if (strcmp(line, "myexit") == 0) {
            free(line);
            break;
        }

        if (parse_pipeline(line, &pipeline) == 0) {
            if (pipeline.command_count > 0) {
                execute_pipeline(&pipeline);
            }
            free_pipeline(&pipeline);
        } else {
            fprintf(stderr, "myshell: syntax error\n");
        }

        free(line);
    }
}

int main(void) {
    shell_loop();
    return 0;
}
