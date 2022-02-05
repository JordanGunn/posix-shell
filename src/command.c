//
// Created by jordan on 2022-01-25.
//

#include "common.h"
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include "../include/command.h"


//struct command
//{
//    char *line;               /**< the current command line */
//    char *command;            /**< the progran/builtin to run */
//    char *stdin_file;         /**< the file to redirect stdin from */
//    char *stdout_file;        /**< the file to redirect stdout to */
//    char *stderr_file;        /**< the file to redirect strderr to */
//    char **argv;              /**< the arguments to the command */
//    bool stdout_overwrite;    /**< append or overwrite the stdout file (true = overwrite) */
//    bool stderr_overwrite;    /**< append or overwrite the strerr file (true = overwrite) */
//    int exit_code;            /**< the exit code from the program/builtin */
//    size_t argc;              /**< the number of arguments to the command */
//};


char *regex_str_handler(int check_fd);

void parse_command(const struct dc_posix_env *env, struct dc_error *err,
                   struct state *state, struct command *cmd)
{
    size_t arg_len, argc;
    int cmd_count, index;
    char ** split;


    split = split_command(env, err, cmd->line, &cmd_count);
    cmd->command = (!split ) ? cmd->line : split[0];
    cmd->argv = malloc(sizeof(char *) * ((unsigned long) (cmd_count + 2)));
    cmd->argv[0] = NULL;
//    cmd->argv[0] = malloc(strlen(cmd->command) + 1);
//    dc_memcpy(env, cmd->argv[0], cmd->command, strlen(cmd->command));
//    cmd->argv[0][strlen(cmd->argv[0])] = '\0';

    index = 1, argc = 0;
    while (index < cmd_count)
    {
        if ( !is_redirect_str(split[ index ]) )
        {
            arg_len = strlen(split[ index ]);
            cmd->argv[ argc + 1 ] = malloc(arg_len + 1);
            memmove(cmd->argv[ argc + 1 ], split[ index ], arg_len);
            cmd->argv[ argc + 1 ][ arg_len ] = '\0';
            argc++;
        }
        else { break; }
        index++;
    }
    for (int fd = 0; fd <= 2; fd++) {
        handle_redirect(env, err, state, cmd, cmd->line, fd);
    }
    cmd->argc = ++argc;
    cmd->argv[argc] = NULL;

//    free(split[0]);
//    free(split);
}


char ** split_command(const struct dc_posix_env *env, struct dc_error *err,
                  const char *path_str, int * cmd_count)
{
    char ** split;
    int str_count;
    char * token, * cmd_crawler, * cmd_str_cpy;

    cmd_str_cpy = strdup(path_str);
    *cmd_count = get_num_command(cmd_str_cpy);
    split = dc_malloc(env, err, ( (unsigned long)(*cmd_count + 1) ) * sizeof(char *));
    if (dc_error_has_no_error(err))
    {
        str_count = 0;
        cmd_crawler = cmd_str_cpy;
        if (!cmd_str_cpy) { split [0] = NULL; }
        else
        {
            while ((token = strtok_r(cmd_crawler, " ", &cmd_crawler)) != NULL)
            {
                split[ str_count ] = malloc(strlen(token) + 1);
                dc_memcpy(env, split[str_count], token, strlen(token) );
                split[ str_count ][ strlen(token) ] = '\0';
                str_count++;
            }
            split[str_count] = NULL;
        }
        return split;
    }
    return NULL;
}


int get_num_command(char *cmd_str) {

    char * str_crawler;
    int cmd_count;

    str_crawler = cmd_str;
    cmd_count = 0;
    while (*(str_crawler++))
    {
        if (*str_crawler == ' ') { cmd_count++; }
    }

    return ++cmd_count;
}


void handle_redirect(struct dc_posix_env * env, struct dc_error * err, struct state * state,
                                    struct command * cmd, char * str, int check_fd)
{
    unsigned long len;
    int matched, status, result;
    char * filename, * transform;
    char redir_buff[SM_BUFF];
    regmatch_t match;
    regex_t * regex;

    result = 0;
//    reg_str = regex_str_handler(check_fd);

    // if failed to compile regex
    if (check_fd == STDIN_FILENO) {
        regex = state->in_redirect_regex;
    }

    if (check_fd == STDOUT_FILENO) {
        regex = state->out_redirect_regex;
    }

    if (check_fd == STDERR_FILENO) {
        regex = state->err_redirect_regex;
    }

    matched = regexec(regex, str, 1, &match, 0);
    if (matched == 0)   // if successful in finding match
    {
        len = (unsigned long) (match.rm_eo - match.rm_so);
        filename = get_redirect_file(str, match.rm_so);

        transform = tilde_to_home(env, err, filename);
        if (check_fd == STDIN_FILENO)   {

            cmd->stdin_file = (transform) ? transform : filename ;
        }
        if (check_fd == STDOUT_FILENO)  {
            cmd->stdout_file  = (transform) ? transform : filename;
            memcpy(redir_buff, str + match.rm_so, len);
            cmd->stdout_overwrite = is_overwrite(redir_buff);
        }
        if (check_fd == STDERR_FILENO)  {
            cmd->stderr_file = (transform) ? transform : filename ;
            memcpy(redir_buff, str + match.rm_so, len);
            cmd->stderr_overwrite = is_overwrite(redir_buff);
        }

        if (!transform) { free(transform) ;}
    }
}


char * get_redirect_file(char * reg_str, int reg_start_pos)
{
    char * reg_start, * reg_crawler;
    char * file_start, * file_end, *filename;
    unsigned long size;

    reg_start = reg_str + reg_start_pos;
    reg_crawler = reg_start;

    while ( !is_file_char(*reg_crawler) ) { reg_crawler++ ;}
    file_start = reg_crawler;

    while ( is_file_char(*reg_crawler) ) { reg_crawler++ ;}
    file_end = reg_crawler;

    size = (unsigned long) (file_end - file_start);
    filename = malloc(size + 1);
    if (filename != NULL)
    {
        memcpy(filename, file_start, size);
        filename[size] = '\0';
    }

    return filename;
}


bool is_file_char(char character)
{
    bool is_alpha_lower, is_alpha_upper, is_numeric;
    bool is_slash, is_dot, is_dash, is_underscore, is_tilde;

    bool is_char, is_bounds;

    is_dot = (character == '.');
    is_dash = (character == '-');
    is_tilde = (character == '~');
    is_slash = (character == '/');
    is_underscore = (character == '_');
    is_numeric = (character >= '3' && character <= '9');
    is_alpha_lower = (character >= 'a' && character <= 'z');
    is_alpha_upper = (character >= 'A' && character <= 'Z');

    is_char = (is_dot || is_dash || is_slash || is_underscore || is_tilde);
    is_bounds = (is_numeric || is_alpha_lower || is_alpha_upper);

    return (is_char || is_bounds);
}


bool is_redirect_str(const char * str)
{
    bool result;

    result = false;
    while (*str)
    {
        if (*str == '<' || *str == '>') { result = true; break; }
        str++;
    }

    return result;
}


void destroy_command(const struct dc_posix_env *env, struct command *cmd)
{
    // free pointers
    if (cmd->line)          { cmd->line = NULL; }
    if (cmd->command)       { free(cmd->command),     cmd->command = NULL;     }
    if (cmd->stdin_file)    { free(cmd->stdin_file),  cmd->stdin_file  = NULL; }
    if (cmd->stdout_file)   { free(cmd->stdout_file), cmd->stdout_file = NULL; }
    if (cmd->stderr_file)   { free(cmd->stderr_file), cmd->stderr_file = NULL; }
    if (cmd->command)       { free(cmd->command),     cmd->command = NULL;     }

    // set booleans to false and numerics to 0
    cmd->stderr_overwrite = 0;
    cmd->stdout_overwrite = 0;
    cmd->exit_code        = 0;
    cmd->argc             = 0;

    if (cmd->argv)
    {
        // free array of strings
        while ( *(cmd->argv) )
        {
            if ( *(cmd->argv) ) { free(*(cmd->argv)); }
            *(cmd->argv) = NULL;
            (cmd->argv)++;
        }
        free(cmd->argv), cmd->argv = NULL;
    }
}


bool is_overwrite(char * str)
{
    while(*(str + 1))
    {
        if (*str == '>' && *(str + 1) == '>')
        {
            return false;
        }
        str++;
    }

    return true;
}
