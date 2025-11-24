#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  int buffer_size = sizeof(command_buffer);

  /* Flush after every printf */
  setbuf(stdout, NULL);

  while (shouldExit == 0)
  {
    /* prompt */
    printf("$ ");
    shouldExit = 0;
    exit_code = 0;

    exit_code = get_input(command_buffer, buffer_size);

    if (shouldExit == 1)
      break;

    if (command_buffer[0] == '\0') /* if buffer is empty continue */
      continue;

    char **tokens = NULL;
    int arg_count = 0;
    char *buffer_copy = strdup(command_buffer);

    shouldExit = tokenize(&tokens, buffer_copy, &arg_count);

    if (shouldExit == 1)
      break;

    data_pointers *dps;
    shouldExit = prepare_datapointers(&dps, arg_count);

    if (shouldExit == 1)
      break;

    int found = 0;

    int i;
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
          get_args(dps, tokens, arg_count);
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
        get_args(dps, tokens, arg_count);
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
