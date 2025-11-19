#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDEX_OF_CMD 0
#define INDEX_OF_ARGV1 1
#define ARGS_VARIABLE -1

typedef union
{
  int i_var;
  char *s_pointer;
} data_pointers;

typedef int (*cmd_fn)(data_pointers[]);

typedef enum
{
  CMD_EXIT = 0,
  CMD_ECHO = 1,
  CMD_TYPE = 2,
  LAST_CMD = 3
} command_type;

typedef struct
{
  const char *str;
} message;

typedef struct
{
  char *command;
  cmd_fn fn;
  int argc;
  command_type type;
  const char *desc;
} command;

int cmd_exit(data_pointers args[]);
int cmd_echo(data_pointers args[]);
int cmd_type(data_pointers args[]);

int shouldExit = 0;

static const char built_in_msg[] = "is a shell builtin";
static const char not_found_msg[] = "not found";

command commands[LAST_CMD] = {
    {"exit", cmd_exit, 1, CMD_EXIT, built_in_msg},
    {"echo", cmd_echo, ARGS_VARIABLE, CMD_ECHO, built_in_msg},
    {"type", cmd_type, 1, CMD_TYPE, built_in_msg}};

static const int commands_count = sizeof(commands) / sizeof(command);

int main(int argc, char *argv[])
{
  int exit_code = 0;

  // stores users input
  char command_buffer[1024];

  // Flush after every printf
  setbuf(stdout, NULL);

  while (!shouldExit)
  {
    // prompt
    printf("$ ");

    // get command
    fgets(command_buffer, sizeof(command_buffer), stdin);

    // Remove the trailing newline
    command_buffer[strcspn(command_buffer, "\n")] = '\0';

    if (strlen(command_buffer) >= 0)
    {
      int i;
      int found = 0;
      // int commands_count = sizeof(commands) / sizeof(command);

      char *buffer_copy = strdup(command_buffer);
      char *p = buffer_copy;
      char *token;
      int tokens_cap = 2;
      char **tokens = malloc(tokens_cap * sizeof(char *));
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
            free(tokens);
            free(buffer_copy);
            return 1;
          }
          tokens = tmp;
        }

        tokens[token_index] = token;
        token_index++;
      }

      tokens[token_index] = NULL;

      for (i = 0; i < commands_count; i++)
      {
        if (strcmp(tokens[INDEX_OF_CMD], commands[i].command))
          continue;

        int argc = token_index - 1;

        data_pointers *dps = malloc((argc + 1) * sizeof(data_pointers));
        if (!dps)
        {
          free(tokens);
          free(buffer_copy);
          return 1;
        }
        memset(dps, 0, (argc + 1) * sizeof(data_pointers));

        if (commands[i].argc == ARGS_VARIABLE || argc == commands[i].argc)
        {
          switch (commands[i].type)
          {
          case CMD_EXIT:
          {
            int value = atoi(tokens[INDEX_OF_CMD + INDEX_OF_ARGV1]);
            dps[0].i_var = value;
          }
          break;
          case CMD_ECHO:
          {
            for (int arg_index = 0; arg_index < argc; arg_index++)
            {
              dps[arg_index].s_pointer = tokens[INDEX_OF_CMD + INDEX_OF_ARGV1 + arg_index];
            }
          }
          break;
          case CMD_TYPE:
          {
            dps[0].s_pointer = tokens[INDEX_OF_CMD + INDEX_OF_ARGV1];
          }
          break;
          default:
            break;
          }
          exit_code = commands[i].fn(dps);
        }
        else
        {
          printf("%s: invalid number of arguments. Given %d, expected: %d\n", commands[i].command, (token_index - 1), commands[i].argc);
        }

        free(dps);
        found = 1;
        break;
      }

      if (!found)
      {
        // Prints the "<command>: command not found" message
        printf("%s: command not found\n", command_buffer);
      }

      free(tokens);
      free(buffer_copy);
    }
  }

  return exit_code;
}

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
  int found = 0;

  for (int i = 0; i < commands_count; i++)
  {
    if (strcmp(args[0].s_pointer, commands[i].command))
      continue;

    found = 1;
    printf("%s %s", commands[i].command, commands[i].desc);
  }

  if (!found)
  {
    printf("%s %s", args[0].s_pointer, not_found_msg);
  }

  printf("\n");

  return 0;
}
