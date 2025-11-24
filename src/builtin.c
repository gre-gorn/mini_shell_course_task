#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "types.h"
#include "externs.h"

/* SHELL INTERNAL COMMANDS IMPLEMENTATION */

int cmd_exit(data_pointers args[])
{
    shouldExit = 1;
    return args[0].i_var;
}

int cmd_echo(data_pointers args[])
{
    data_pointers *p = args;
    while (p->s_pointer != NULL)
    {
        printf("%s ", p->s_pointer);
        p++;
    }

    printf("\n");

    return 0;
}

int cmd_type(data_pointers args[])
{
    int i;
    int found = 0;

    for (i = 0; i < commands_count; i++)
    {
        if (strcmp(args[0].s_pointer, commands[i].command) != 0)
            continue;

        found = 1;
        printf("%s %s\n", commands[i].command, commands[i].desc);
        break;
    }

    if (!found)
    {
        if (check_cmd(args[0].s_pointer, 1) != 0)
        {
            printf("%s %s\n", args[0].s_pointer, not_found_msg);
        }
    }

    return 0;
}

int cmd_pwd(data_pointers args[])
{
    char cwd[512];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);
        return 0;
    }

    return 1;
}

int cmd_cd(data_pointers args[])
{
    const char *dir = args[0].s_pointer;
    if (chdir(dir) == 0)
    {
        return 0;
    }

    printf("cd: %s: %s\n", dir, no_such_file_org_directory_msg);
    return 1;
}