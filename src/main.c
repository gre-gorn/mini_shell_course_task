#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARGS_VARIABLE -1

static const int CMD_IDX = 0;
static const int ARG1_IDX = 1;

/* string constant definitions */
static const char *not_found_msg = "not found";
static const char *numeric_arg_req_msg = "numeric argument required";

static const char built_in_msg[] = "is a shell builtin"; /* [] needed for initialization while compilation time */

/* type definitions */
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
  char *command;
  cmd_fn fn;
  int argc;
  command_type type;
  const char *desc;
} command;

/* mini shell commands declarations */
int cmd_exit(data_pointers args[]);
int cmd_echo(data_pointers args[]);
int cmd_type(data_pointers args[]);

int shouldExit = 0; /* TODO: consider using signal to exit */

/* minishell commands definitions */
command commands[LAST_CMD] = {
    {"exit", cmd_exit, 1, CMD_EXIT, built_in_msg},
    {"echo", cmd_echo, ARGS_VARIABLE, CMD_ECHO, built_in_msg},
    {"type", cmd_type, 1, CMD_TYPE, built_in_msg}};

static const int commands_count = sizeof(commands) / sizeof(command);

/* helper functions declaratios */
static char *get_arg(char **tokens, int index);
static int check_cmd(const char *cmd);

int main(int argc, char *argv[])
{
  int exit_code = 0;

  /* stores users input */
  char command_buffer[1024];

  /* Flush after every printf */
  setbuf(stdout, NULL);

  while (!shouldExit)
  {
    /* prompt */
    printf("$ ");

    /* get command */
    if (!fgets(command_buffer, sizeof(command_buffer), stdin))
    {
      if (feof(stdin))
      {
        /* ctrl + d – standard interruption */
        break;
      }

      if (ferror(stdin))
      {
        exit_code = 1;
        break;
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
      free(buffer_copy);
      return 1;
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
      if (strcmp(tokens[CMD_IDX], commands[i].command) != 0)
        continue;

      int arg_count = token_index - 1;

      data_pointers *dps = malloc((arg_count + 1) * sizeof(data_pointers));
      if (!dps)
      {
        free(tokens);
        free(buffer_copy);
        return 1;
      }
      memset(dps, 0, (arg_count + 1) * sizeof(data_pointers));

      if (commands[i].argc == ARGS_VARIABLE || arg_count == commands[i].argc)
      {
        switch (commands[i].type)
        {
        case CMD_EXIT:
        {
          char *end;
          long value = strtol(get_arg(tokens, 0), &end, 10); /* strtol(tokens[CMD_IDX + ARG1_IDX], &end, 10); */

          if (*end != '\0')
          {
            printf("%s: %s\n", commands[i].command, numeric_arg_req_msg);
            value = 1;
          }

          dps[0].i_var = (int)value;
        }
        break;
        case CMD_ECHO:
        {
          int arg_index;
          for (arg_index = 0; arg_index < arg_count; arg_index++)
          {
            dps[arg_index].s_pointer = get_arg(tokens, arg_index); /* [CMD_IDX + ARG1_IDX + arg_index]; */
          }
        }
        break;
        case CMD_TYPE:
        {
          dps[0].s_pointer = get_arg(tokens, 0); /* tokens[CMD_IDX + ARG1_IDX]; */
        }
        break;
        default:
          break;
        }

        dps[arg_count].s_pointer = NULL;
        dps[arg_count].i_var = 0;

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
      printf("%s: command not found\n", command_buffer);
    }

    free(tokens);
    free(buffer_copy);
  }

  return exit_code;
}

static char *get_arg(char **tokens, int index)
{
  return tokens[CMD_IDX + ARG1_IDX + index];
}

static int check_cmd(const char *cmd)
{
  const char *path = getenv("PATH");
  if (!path)
    return 1;

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

    printf("%s is %s\n", cmd, full_path);
    return 0;
  }

  return 1;
}

/* COMMANDS IMPLEMENTATION */

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
    if (check_cmd(args[0].s_pointer) != 0)
    {
      printf("%s %s\n", args[0].s_pointer, not_found_msg);
    }
  }

  return 0;
}
