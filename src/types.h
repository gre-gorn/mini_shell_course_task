#ifndef TYPES_H
#define TYPES_H

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
    CMD_ECHO,
    CMD_TYPE,
    CMD_PWD,
    CMD_CD,
    LAST_CMD
} command_type;

typedef struct
{
    char *command;
    cmd_fn fn;
    int argc;
    command_type type;
    const char *desc;
} command;

#endif
