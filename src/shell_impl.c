//
// Created by jordan on 2022-01-30.
//

#include "shell_impl.h"

/**
 * Set up the initial state:
 *  - in_redirect_regex  "[ \t\f\v]<.*"
 *  - out_redirect_regex "[ \t\f\v][1^2]?>[>]?.*"
 *  - err_redirect_regex "[ \t\f\v]2>[>]?.*"
 *  - path the PATH environ var separated into directories
 *  - prompt the PS1 environ var or "$" if PS1 not set
 *  - max_line_length the value of _SC_ARG_MAX (see sysconf)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return READ_COMMANDS or INIT_ERROR
 */
int init_state(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    struct state * state = (struct state *) arg;

    int result, reg_status;
    const char * in_reg, * out_reg, * err_reg;
    char * prompt, * env_path_str;
    regex_t in, out, error;

    state->stdin = stdin;
    state->stdout = stdout;
    state->stderr = stderr;
    state->fatal_error = 0;

    state->max_line_length = (size_t) sysconf(_SC_ARG_MAX);
    state->current_line_length = 0;
    state->current_line = NULL;
    state->command = NULL;
    state->path = NULL;


    out_reg = "[ \\t\\f\\v][1]?>[>]?[ ]*[a-zA-Z0-9\\-\\._/~]+";
    in_reg = "[ \\t\\f\\v]<[ ]*[a-zA-Z0-9\\-\\._/~]+";
    err_reg = "[ \\t\\f\\v]2>[>]?[ ]*[a-zA-Z0-9\\-\\._/~]+";

    if ((reg_status = regcomp(&in, in_reg, REG_EXTENDED)) == 0)
    {
        state->in_redirect_regex = dc_calloc(env, err, 1, sizeof(in));
        dc_memcpy(env, state->in_redirect_regex, &in, sizeof(in));
    }

    if ((reg_status = regcomp(&out, out_reg, REG_EXTENDED)) == 0)
    {
        state->out_redirect_regex = dc_calloc(env, err, 1, sizeof(out));
        dc_memcpy(env, state->out_redirect_regex, &out, sizeof(out));
    }

    if ((reg_status = regcomp(&error, err_reg, REG_EXTENDED)) == 0)
    {
        state->err_redirect_regex = dc_calloc(env, err, 1, sizeof(error));
        dc_memcpy(env, state->err_redirect_regex, &error, sizeof(error));
    }

    env_path_str = get_path(env, err);
    if (dc_error_has_no_error(err))
    {
        state->path = parse_path(env, err, env_path_str);
    }

    prompt = get_prompt(env, err);
    if (dc_error_has_no_error(err))
    {
        state->prompt = prompt;
    }


    if (dc_error_has_no_error(err))
    {
        return READ_COMMANDS;
    }
    else
    {
        state->fatal_error = true;
        return ERROR;
    }
}




/**
 * Free any dynamically allocated memory in the state and sets variables to NULL, 0 or false.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return DC_FSM_EXIT
 */
int destroy_state(const struct dc_posix_env *env, struct dc_error *err,
                  void *arg)
{
    do_destroy_state(env, err, (struct state *) arg);
    return DC_FSM_EXIT;
}

/**
 * Reset the state for the next read (see do_reset_state).
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return READ_COMMANDS
 */
int reset_state(const struct dc_posix_env *env, struct dc_error *err,
                void *arg)
{
    do_reset_state(env, err, (struct state *) arg);

    return READ_COMMANDS;
}

/**
 * Prompt the user and read the command line (see read_command_line).
 * Sets the state->current_line and current_line_length.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return SEPARATE_COMMANDS
 */
int read_commands(const struct dc_posix_env *env, struct dc_error *err,
                  void *arg)
{
    char * line;
    char buff[SM_BUFF];
    struct state * state = (struct state *) arg;
    state->current_line_length = SM_BUFF;

    dc_getcwd(env, err, buff, SM_BUFF);
    if (dc_error_has_error(err))
    {
        state->fatal_error = true;
        return ERROR;
    }
    if (dc_error_has_no_error(err))
    {
        printf("%s %s", buff, state->prompt);
        line = read_command_line(env, err, stdin, &state->current_line_length);
        if (dc_error_has_error(err))
        {
            state->fatal_error = true;
            return ERROR;
        }

        if (line && strlen(line) != 0)
        {
            state->current_line = line;
            return SEPARATE_COMMANDS;
        }
    }

    return RESET_STATE;
}

/**
 * Separate the commands. In the current implementation there is only one command.
 * Sets the state->command.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return PARSE_COMMANDS or SEPARATE_ERROR
 */
int separate_commands(const struct dc_posix_env *env, struct dc_error *err,
                      void *arg)
{
    struct state * state = (struct state *) arg;
    state->command = dc_malloc(env, err, sizeof(struct command *));
    state->command->line = calloc(strlen(state->current_line), sizeof(char));
    state->command->line = strdup(state->current_line);
    state->command->stdin_file = NULL;
    state->command->stdout_file = NULL;
    state->command->stderr_file = NULL;

    if (dc_error_has_error(err))
    {
        return ERROR;
    }

    else
    {
        return PARSE_COMMANDS;
    }
}

/**
 * Parse the commands (see parse_command)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return EXECUTE_COMMANDS or PARSE_ERROR
 */
int parse_commands(const struct dc_posix_env *env, struct dc_error *err,
                   void *arg)
{
    struct state * state = (struct state *) arg;
    parse_command(env, err, state, state->command);

    if (dc_error_has_error(err))
    {
        return ERROR;
    }
    else
    {
        return EXECUTE_COMMANDS;
    }
}


/**
 * Run the command (see execute).
 * If the command->command is cd run builtin_cd
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return EXIT (if command->command is exit), RESET_STATE or EXECUTE_ERROR
 */
int execute_commands(const struct dc_posix_env *env, struct dc_error *err,
                     void *arg)
{
    struct state * state = (struct state *) arg;
    if ( dc_strcmp(env, state->command->command, "cd") == 0 )
    {
        builtin_cd(env, err, state->command, stderr);
    }
    else if ( dc_strcmp(env, state->command->command, "exit") == 0 )
    {
        return EXIT;
    }
    else
    {
        execute(env, err, state->command, state->path);
    }


    if (state->fatal_error)
    {
        return ERROR;
    }

    return RESET_STATE;
}



/**
 * Handle the exit command (see do_reset_state)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return DESTROY_STATE
 */
int do_exit(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    return DESTROY_STATE;
}

/**
 * Print the error->message to stderr and reset the error (see dc_err_reset).
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return RESET_STATE or DESTROY_STATE (if state->fatal_error is true)
 */
int handle_error(const struct dc_posix_env *env, struct dc_error *err,
                 void *arg)
{
    struct state * state = (struct state *) arg;
    if (state->fatal_error)
    {
        return DESTROY_STATE;
    }
    else
    {
        return RESET_STATE;
    }
}
