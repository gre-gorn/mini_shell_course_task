#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "externs.h"

int tokenize(char ***tokens, char *buffer_copy, int *arg_count)
{
    int errno = 0;

    char *p = buffer_copy;
    char *token;
    int tokens_cap = 2;

    *tokens = malloc(tokens_cap * sizeof(char *));

    if (*tokens == NULL)
    {
        printf("Here!");
        errno = 1;
    }
    else
    {
        int token_index = 0;

        while ((token = strsep(&p, " ")) != NULL)
        {
            if (*token == '\0')
                continue;

            if (token_index >= tokens_cap)
            {
                tokens_cap *= 2;
                char **tmp = realloc(*tokens, tokens_cap * sizeof(char *));
                if (!tmp)
                {
                    printf("And here!");
                    errno = 1;
                    break;
                }
                *tokens = tmp;
            }

            (*tokens)[token_index] = token;
            token_index++;
        }

        (*tokens)[token_index] = NULL;
        *arg_count = token_index - 1;
    }

    return errno;
}

int prepare_datapointers(data_pointers **dps, int arg_count)
{
    int errno = 0;
    *dps = malloc((arg_count + 1) * sizeof(data_pointers));
    if (*dps != NULL)
    {
        memset(*dps, 0, (arg_count + 1) * sizeof(data_pointers));
    }
    else
    {
        errno = 1;
    }

    return errno;
}
