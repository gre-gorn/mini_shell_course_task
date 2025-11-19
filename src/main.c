#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDEX_OF_CMD 0
#define INDEX_OF_ARGV1 1

typedef union
{
  int *i_pointer;
  char *s_pointer;
} data_pointers;

typedef int (*cmd_fn)(data_pointers[]);

typedef enum
{
  CMD_EXIT = 0
} command_type;

typedef struct
{
  char *command;
  cmd_fn fn;
  int argc;
  command_type type;
} command;

int cmd_exit(data_pointers args[]);

int shouldExit = 0;

command commands[] = {
    {"exit", cmd_exit, 1, CMD_EXIT}};

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
      int commands_count = sizeof(commands) / sizeof(command);

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
          tokens = realloc(tokens, tokens_cap * sizeof(char *));
        }

        tokens[token_index] = token;
        token_index++;
      }

      tokens[token_index] = NULL;

      for (i = 0; i < commands_count; i++)
      {
        if (strcmp(tokens[INDEX_OF_CMD], commands[i].command))
          continue;

        if (token_index - 1 == commands[i].argc)
        {
          switch (commands[i].type)
          {
          case CMD_EXIT:
            int value = atoi(tokens[INDEX_OF_CMD + INDEX_OF_ARGV1]);
            data_pointers dp[1];
            dp[0].i_pointer = &value;
            exit_code = commands[i].fn(dp);
            break;

          default:
            break;
          }
        }
        else
        {
          printf("%s: invalid number of arguments. Given %d, expected: %d\n", commands[i].command, (token_index - 1), commands[i].argc);
        }

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
  return *args[0].i_pointer;
}
