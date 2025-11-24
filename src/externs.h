#ifndef EXTERNS_H
#define EXTERNS_H

#include "types.h"

extern int shouldExit;
extern const command commands[];
extern const int commands_count;

extern const char *not_found_msg;
extern const char *numeric_arg_req_msg;
extern const char *no_such_file_org_directory_msg;
extern const char built_in_msg[];

extern int cmd_exit(data_pointers args[]);
extern int cmd_echo(data_pointers args[]);
extern int cmd_type(data_pointers args[]);
extern int cmd_pwd(data_pointers args[]);
extern int cmd_cd(data_pointers args[]);

extern char *get_cmd(char **tokens);
extern char *get_arg(char **tokens, int index);
extern int check_cmd(const char *cmd, int verbose);
extern int exec_cmd(char *cmd, data_pointers args[], int args_count);

#endif
