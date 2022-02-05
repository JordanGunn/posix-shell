#ifndef DC_SHELL_COMMAND_H
#define DC_SHELL_COMMAND_H

/*
 * This file is part of dc_shell.
 *
 *  dc_shell is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with dc_shell.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "state.h"
#include "builtins.h"
#include <regex.h>
#include <dc_posix/dc_posix_env.h>

/*! \struct command
    \brief The commands to enter, currently there is only one.

    The state passed around to the FSM functions.
*/
struct command
{
  char *line;               /**< the current command line */
  char *command;            /**< the progran/builtin to run */
  char *stdin_file;         /**< the file to redirect stdin from */
  char *stdout_file;        /**< the file to redirect stdout to */
  char *stderr_file;        /**< the file to redirect strderr to */
  char **argv;              /**< the arguments to the command */
  bool stdout_overwrite;    /**< append or overwrite the stdout file (true = overwrite) */
  bool stderr_overwrite;    /**< append or overwrite the strerr file (true = overwrite) */
  int exit_code;            /**< the exit code from the program/builtin */
  size_t argc;              /**< the number of arguments to the command */
};


/**
 * Parse the cmd. Take the cmd->line and use it to fill in all of the fields.
 *
 * @param env the posix environment.
 * @param err the error object.
 * @param state the current state, to set the fatal_error and access the cmd line and regex for redirection.
 * @param cmd the cmd to parse.
 */
void parse_command(const struct dc_posix_env *env, struct dc_error *err,
                   struct state *state, struct command *cmd);


/**
 * Reset the cmd for the next read, freeing any dynamically allocated memory.
 *
 * @param env the posix environment.
 * @param err the error object
 */
void destroy_command(const struct dc_posix_env *env, struct command *cmd);


/**
 * Get number of tokens in command string.
 *
 * @param cmd_str   A string read from stdin.
 * @return          Number of tokens in cmd string
 */
int get_num_command(char * cmd_str);


/**
 * @param env       The posix environment.
 * @param err       The error object.
 * @param cmd_str   Cmd string read from command line.
 * @param cmd_count  Pointer to int (to be updated with num strings)
 * @return          Tokenized cmd string as 2D array.
 */
char ** split_command(const struct dc_posix_env *env, struct dc_error *err,
                      const char *path_str, int * cmd_count);

/**
 * Determine if string is redirect cmd.
 *
 * Returns -1 if failure, 0 if success.
 *
 * @param str   Input string.
 * @return      result
 */
void handle_redirect(struct dc_posix_env * env, struct dc_error * err, struct state * state,
                     struct command * cmd, char * str, int check_fd);


/**
 * Determine if character is in the set
 * {'A'-'Z' | 'a'-'z' | '0'-'9' | '-' | '_' | '.' | '/'}
 *
 * @param character
 * @return
 */
bool is_file_char(char character);


/**
 * Use regex match positions to parse out filename.
 *
 * @param reg_str           String regex was performed on.
 * @param reg_start_pos     Start position from regexec().
 * @return                  Pointer to start of filename.
 */
char * get_redirect_file(char * reg_str, int reg_start_pos);


/**
 * Iterate through chars and check for '<' or '>'.
 *
 * @param str   A string.
 * @return      true or false.
 */
bool is_redirect_str(const char * str);


/**
 * Determine overwrite status of redirection.
 *
 * @param str a string.
 * @return    true or false.
 */
bool is_overwrite(char * str);








#endif // DC_SHELL_COMMAND_H
