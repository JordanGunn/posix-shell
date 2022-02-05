#ifndef DC_SHELL_UTIL_H
#define DC_SHELL_UTIL_H

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

#include "shell.h"
#include "state.h"
#include <dc_posix/dc_posix_env.h>
#include <dc_util/strings.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdio.h>
#include <dc_posix/dc_fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "command.h"
#include "common.h"

/**
 * Get the prompt to use.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return value of the PS1 environ var or "$ " if PS1 not set.
 */
char *get_prompt(const struct dc_posix_env *env, struct dc_error *err);

/**
 * Get the PATH environ var.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return the PATH environ var
 */
char *get_path(const struct dc_posix_env *env, struct dc_error *err);

/**
 * Separate a path (eg. PATH environ var) into separate directories.
 * Directories are separated with a ':' character.
 * Any directories with ~ are converted to the users home directory.
 *
 * @param env the posix environment.
 * @param err the error object.
 * @param path_str the string to separate.
 * @return The directories that make up the path.
 */
char **parse_path(const struct dc_posix_env *env, struct dc_error *err,
                  const char *path_str);

/**
 * Reset the state for the next read, freeing any dynamically allocated memory.
 *
 * @param env the posix environment.
 * @param err the error object
 */
void do_reset_state(const struct dc_posix_env *env, struct dc_error *err, struct state *state);

/**
 * Display the state values to the given stream.
 *
 * @param env the posix environment.
 * @param state the state to display.
 * @param stream the stream to display the state on,
 */
void display_state(const struct dc_posix_env *env, const struct state *state, FILE *stream);

/**
 * Display the state values to the given stream.
 *
 * @param env the posix environment.
 * @param state the state to display.
 * @param stream the stream to display the state on,
 */
char *state_to_string(const struct dc_posix_env *env,  struct dc_error *err, const struct state *state);


/**
 * Get number of paths in PATH env. var.
 *
 * @pre     Argument must be a string of colon (:) separated
 *          paths obtained from call to getenv("PATH").
 * @post    Will return number of distinct paths in PATH env.
 *
 * @param path_str  - A string of colon seperated paths.
 * @return          - Number of paths in PATH.
 */
int get_num_env_paths(char *path_str);


/**
 * Pretty-print all paths in PATH env variable.
 *
 * Prints all paths in PATH in the format
 * "[ path1, path2, path ]"
 *
 * @param path - Char sequence returned from getenv("PATH")
 */
void print_paths(char ** path);

/**
 * Free all dynamically allocated memory in state.
 *
 * @param env the posix environment.
 * @param err the error object.
 * @param state state to pass around
 */
void do_destroy_state(const struct dc_posix_env *env, struct dc_error *err, struct state *state);

#endif // DC_SHELL_UTIL_H
