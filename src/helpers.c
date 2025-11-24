#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "externs.h"

char *get_cmd(char **tokens)
{
    return tokens[CMD_IDX];
}

char *get_arg(char **tokens, int index)
{
    return tokens[CMD_IDX + ARG1_IDX + index];
}

int check_cmd(const char *cmd, int verbose)
{
    const char *path = getenv("PATH");
    if (!path)
    {
        return 1;
    }

    char path_buf[512];
    strncpy(path_buf, path, sizeof(path_buf));
    path_buf[sizeof(path_buf) - 1] = '\0';

    char *it = path_buf;
    char *dir;

    while ((dir = strsep(&it, ":")) != NULL)
    {
        if (*dir == '\0')
            continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);

        if (access(full_path, X_OK) != 0)
            continue;

        if (verbose == 1)
        {
            printf("%s is %s\n", cmd, full_path);
        }
        return 0;
    }

    return 1;
}

int exec_cmd(char *cmd, data_pointers args[], int args_count)
{
    pid_t pid = fork();
    int arg_index;

    if (pid == 0)
    {
        /* subprocess */
        int arg_index = 0;
        char *argv[args_count + 2];

        argv[args_count + 1] = NULL; /* NULL terminatd array */

        argv[arg_index] = cmd;
        for (arg_index = 1; arg_index < args_count + 2; arg_index++)
        {
            argv[arg_index] = args[arg_index - 1].s_pointer;
        }

        execvp(cmd, argv);
        _exit(1);
    }

    /* wait for execute */
    int status;
    waitpid(pid, &status, 0);

    return status;
}

int get_input(char *command_buffer, int buffer_size)
{
    /* get command */
    if (fgets(command_buffer, buffer_size, stdin) == NULL)
    {
        if (feof(stdin))
        {
            /* ctrl + d – standard interruption */
            shouldExit = 1;
            return 0;
        }

        if (ferror(stdin))
        {
            shouldExit = 1;
            return 1;
        }
    }

    /* Remove the trailing newline */
    command_buffer[strcspn(command_buffer, "\n")] = '\0';

    return 0;
}

void get_args(data_pointers dps[], char **tokens, int arg_count)
{
    int arg_index;
    for (arg_index = 0; arg_index < arg_count; arg_index++)
    {
        dps[arg_index].s_pointer = get_arg(tokens, arg_index);
    }
}
