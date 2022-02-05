//
// Created by jordan on 2022-01-30.
//

#include "shell.h"

/**
 * Run the shell FSM.
 *
 * @param env the posix environment.
 * @param error the error object
 * @param in the keyboard (stdin) file
 * @param out the keyboard (stdout) file
 * @param err the keyboard (stderr) file
 *
 * @return the exit code from the shell.
 */


static void quit_handler ( int signum );


int run_shell(const struct dc_posix_env *env, struct dc_error *error, FILE *in, FILE *out, FILE *err)
{

    dc_error_reporter reporter;

    dc_signal(env, error, SIGINT, quit_handler);

    struct state state;

    int ret_val;
    struct dc_fsm_info *fsm_info;

    static struct dc_fsm_transition transitions[] = {

            {DC_FSM_INIT, INIT_STATE, init_state},
            {INIT_STATE, ERROR, handle_error},
            {INIT_STATE, READ_COMMANDS, read_commands},
            {READ_COMMANDS, SEPARATE_COMMANDS, separate_commands},
            {READ_COMMANDS, ERROR, handle_error},
            {READ_COMMANDS, RESET_STATE, reset_state},
            {SEPARATE_COMMANDS, ERROR, handle_error},
            {SEPARATE_COMMANDS, PARSE_COMMANDS, parse_commands},
            {PARSE_COMMANDS, ERROR, handle_error},
            {PARSE_COMMANDS, EXECUTE_COMMANDS, execute_commands},
            {EXECUTE_COMMANDS, ERROR, handle_error},
            {EXECUTE_COMMANDS, EXIT, do_exit},
            {EXECUTE_COMMANDS, RESET_STATE, reset_state},
            {RESET_STATE, READ_COMMANDS, read_commands},
            {EXIT, DESTROY_STATE, destroy_state},
            {ERROR, DESTROY_STATE, destroy_state},
            {ERROR, RESET_STATE, reset_state},
            {DESTROY_STATE, DC_FSM_EXIT, NULL}
    };

    fsm_info = dc_fsm_info_create(env, error, "shell");

    if(dc_error_has_no_error(error))
    {
        ret_val = EXIT_SUCCESS;
        int from_state;
        int to_state;

        ret_val = dc_fsm_run(env, error, fsm_info, &from_state, &to_state, &state, transitions);
        dc_fsm_info_destroy(env, &fsm_info);
    }
    else
    {
        ret_val = 1;
    }

    return ret_val;
}


static void quit_handler ( int signum ) {

    signum = 1;
}
