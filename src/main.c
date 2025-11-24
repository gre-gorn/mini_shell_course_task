#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "types.h"
#include "externs.h"

#define ARGS_VARIABLE -1

const int CMD_IDX = 0;
const int ARG1_IDX = 1;

/* string constant definitions */
const char *not_found_msg = "not found";
const char *numeric_arg_req_msg = "numeric argument required";
const char *no_such_file_org_directory_msg = "No such file or directory";
const char built_in_msg[] = "is a shell builtin"; /* [] needed for initialization while compilation time */

/* minishell commands definitions */
const command commands[LAST_CMD] = {
    {"exit", cmd_exit, ARGS_VARIABLE, CMD_EXIT, built_in_msg},
    {"echo", cmd_echo, ARGS_VARIABLE, CMD_ECHO, built_in_msg},
    {"type", cmd_type, 1, CMD_TYPE, built_in_msg},
    {"pwd", cmd_pwd, 0, CMD_PWD, built_in_msg},
    {"cd", cmd_cd, 1, CMD_CD, built_in_msg},
};

const int commands_count = sizeof(commands) / sizeof(command);

int shouldExit = 0; /* TODO: consider using signal to exit */

int main(int argc, char *argv[])
{
  int exit_code = 0;

  /* stores users input */
  char command_buffer[1024];

  /* Flush after every printf */
  setbuf(stdout, NULL);

  while (shouldExit == 0)
  {
    /* prompt */
    printf("$ ");

    /* get command */
    if (!fgets(command_buffer, sizeof(command_buffer), stdin))
    {
      if (feof(stdin))
      {
        /* ctrl + d – standard interruption */
        shouldExit = 1;
        continue;
      }

      if (ferror(stdin))
      {
        exit_code = 1;
        shouldExit = 1;
        continue;
      }
    }

    /* Remove the trailing newline */
    command_buffer[strcspn(command_buffer, "\n")] = '\0';

    if (!*command_buffer)
      continue;

    int i;
    int found = 0;
    char *buffer_copy = strdup(command_buffer);
    char *p = buffer_copy;
    char *token;
    int tokens_cap = 2;
    char **tokens = malloc(tokens_cap * sizeof(char *));
    if (!tokens)
    {
      shouldExit = 1;
      continue;
    }

    int token_index = 0;

    while ((token = strsep(&p, " ")) != NULL)
    {
      if (*token == '\0')
        continue;

      if (token_index >= tokens_cap)
      {
        tokens_cap *= 2;
        char **tmp = realloc(tokens, tokens_cap * sizeof(char *));
        if (!tmp)
        {
          shouldExit = 1;
          break;
        }
        tokens = tmp;
      }

      tokens[token_index] = token;
      token_index++;
    }

    tokens[token_index] = NULL;
    int arg_count = token_index - 1;

    data_pointers *dps = malloc((arg_count + 1) * sizeof(data_pointers));
    if (!dps)
    {
      shouldExit = 1;
      continue;
    }

    memset(dps, 0, (arg_count + 1) * sizeof(data_pointers));

    for (i = 0; i < commands_count; i++)
    {
      if (strcmp(get_cmd(tokens), commands[i].command) != 0)
        continue;

      if (commands[i].argc == ARGS_VARIABLE || arg_count == commands[i].argc)
      {
        switch (commands[i].type)
        {
        case CMD_EXIT:
        {
          if (arg_count == 1)
          {
            char *end;
            long value = strtol(get_arg(tokens, 0), &end, 10);

            if (*end != '\0')
            {
              printf("%s: %s\n", commands[i].command, numeric_arg_req_msg);
              value = 1;
            }

            dps[0].i_var = (int)value;
          }
        }
        break;
        case CMD_ECHO:
        {
          int arg_index;
          for (arg_index = 0; arg_index < arg_count; arg_index++)
          {
            dps[arg_index].s_pointer = get_arg(tokens, arg_index);
          }
        }
        break;
        case CMD_TYPE:
        case CMD_CD:
        {
          dps[0].s_pointer = get_arg(tokens, 0);
        }
        break;
        default:
          break;
        }

        dps[arg_count].s_pointer = NULL;
        dps[arg_count].i_var = 0;

        exit_code = commands[i].fn(dps);
      }

      found = 1;
      break;
    }

    if (found == 0)
    {
      /* TODO: implement function data_pointers[] get_args(char **tokens) */
      char *cmd = get_cmd(tokens);
      if (check_cmd(cmd, 0) == 0)
      {
        int arg_index;
        for (arg_index = 0; arg_index < arg_count; arg_index++)
        {
          dps[arg_index].s_pointer = get_arg(tokens, arg_index);
        }
        exec_cmd(cmd, dps, arg_count);
      }
      else
      {
        printf("%s: command not found\n", cmd);
      }
    }

    if (dps != NULL)
      free(dps);

    if (tokens != NULL)
      free(tokens);

    if (buffer_copy != NULL)
      free(buffer_copy);
  }

  exit(exit_code);
}

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
