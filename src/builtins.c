//
// Created by jordan on 2022-01-27.
//

#include <dc_posix/dc_stdlib.h>
#include "builtins.h"
#include "common.h"


void handle_file_error(const struct dc_error *err, FILE *errstream, const char *path);

/**
 * Change the working directory.
 * ~ is converted to the users home directory.
 * - no arguments is converted to the users home directory.
 * The command->exit_code is set to 0 on success or error->errno_code on failure.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param command the command information
 */
void builtin_cd(const struct dc_posix_env *env, struct dc_error *err, struct command *command, FILE * errstream)
{
    char * path;
    int result;
    char * home = dc_getenv(env, "HOME");

    if (dc_strcmp(env, command->command, "cd") == 0)
    {
        if (command->argc == 2)
        {
            path = ((command->argv)[1][0] == '~')
                ? tilde_to_home(env, err, command->argv[1])
                : command->argv[1];
        }

        if (command->argc == 1)
        {
            path = home;
        }

        if ( (result = dc_chdir(env, err, path)) == 0 )
        {
            command->exit_code = 0;
        }
        else
        {
            handle_file_error(err, errstream, path);
        }
    }
}

void handle_file_error(const struct dc_error *err, FILE *errstream, const char *path) {
    if (err->err_code == ENODATA)
    {
        fprintf(errstream, "%s: does not exist", path);
    }

    if (err->err_code == ENOTDIR)
    {
        fprintf(errstream, "%s: is not a directory\n", path);
    }

    if (err->err_code == ENOENT)
    {
        fprintf(errstream, "%s: does not exist\n", path);
    }
}

char * tilde_to_home(const struct dc_posix_env *env, struct dc_error *err, char * str)
{
    char * home_path, * str_crawler;
    const char * home;
    int byte_offset;

    byte_offset = 12;
    str_crawler = strdup(str);
    home = dc_getenv(env, "HOME");
    while (*str_crawler)
    {
        if (*str_crawler == '~') {
            home_path = dc_calloc(env, err, strlen(str) + (unsigned long) byte_offset, sizeof(char));// 12 bytes needed to go from "~/" to "home/jordan"
            if (dc_error_has_no_error(err))
            {
                // skip tilde in first byte pos, leave space to insert "home"
                dc_memcpy(env, home_path + byte_offset, str + 1, strlen(str + 1));
                dc_memcpy(env, home_path, home, strlen(home));
                home_path[strlen(home_path)] = '\0';
                return home_path;
            }

        }
        str_crawler++;
    }

    return NULL;
}


/**
 * Change the working directory.
 * ~ is converted to the users home directory.
 * - no arguments is converted to the users home directory.
 * The command->exit_code is set to 0 on success or error->errno_code on failure.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param command the command information
 */
void builtin_exit(const struct dc_posix_env *env, struct dc_error *err,
                struct command *command)
{
    if (dc_strcmp(env, command->command, "exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
}

