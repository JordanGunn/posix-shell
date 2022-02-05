//
// Created by jordan on 2022-01-30.
//

#include <sys/wait.h>
#include "execute.h"

#define EOTHER 125
#define ENOENT_ME 127

/**
 * Create a child process, exec the command with any redirection, set the exit code.
 * If there is an error executing the command print an error message.
 * If the command cannot be found set the command->exit_code to 127.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param command the command to execute
 * @param path the directories to search for the command
 */
void execute(const struct dc_posix_env *env, struct dc_error *err, struct command *command, char **path)
{
    int pid, exit_status;

    pid = dc_fork(env, err);
    if (pid == 0)   // child process
    {
        do_redirect(env, err, command);
        if (dc_error_has_error(err))
        {
            dc_exit(env, 126);
        }
        else
        {
            exec_loop(env, err, command, path);
            exit_status = handle_run_error(err);
            if (exit_status == 127)
            {
                printf("command: %s not found\n", command->line);
            }
            exit(exit_status);
        }
    }
    else
    {
        waitpid(pid, &exit_status, 0);
        if (WIFEXITED(exit_status))
        {
            command->exit_code = WEXITSTATUS(exit_status);
        }
    }
}


void exec_loop(const struct dc_posix_env * env, struct dc_error * err, struct command * command, char ** path) {

    char buff[SM_BUFF] = {0};

    if (is_file(command->command))
    {
        command->argv[0] = command->command;
        dc_execv(env, err, command->command, command->argv);
    }
    else
    {
        if (!path[0])
        {
            DC_ERROR_RAISE_ERRNO(err, ENOENT);
        }
        else
        {
            while ( *path )
            {
                dc_memcpy(env, buff, *path, strlen(*path));
                dc_strcat(env, buff, "/");
                dc_strncat(env, buff + 1, command->command, strlen(command->command));
                command->argv[0] = buff;
                dc_execv(env, err, buff, command->argv);
                if (!dc_error_is_errno(err, ENOENT)) { break; }
                dc_memset(env, buff, 0, sizeof(buff));
                command->argv[0] = NULL;
                path++;
            }
        }
    }
}


int do_redirect(struct dc_posix_env * env, struct dc_error * err, struct command * cmd)
{
    if (cmd->stdin_file)
    {
        stdin_redirect(env, err, cmd);
    }

    if (cmd->stdout_file)
    {
        stdout_redirect(env, err, cmd);
    }

    if (cmd->stderr_file)
    {
        stderr_redirect(env, err, cmd);
    }

    return 0;
}


int stderr_redirect(const struct dc_posix_env *env, struct dc_error *err, const struct command *cmd) {
    int fd;
    unsigned int flag;
    flag = (cmd->stderr_overwrite) ? O_TRUNC : O_APPEND;
    fd = dc_open(env, err, cmd->stderr_file, flag | O_CREAT | O_WRONLY, S_IRWXU);
    if(dc_error_has_no_error(err))
    {
        dc_dup2(env, err, fd, STDERR_FILENO);
    }
    dc_close(env, err, fd);
    return 0;
}


int stdout_redirect(const struct dc_posix_env *env, struct dc_error *err, const struct command *cmd) {
    int fd;
    unsigned int flag;
    flag = (cmd->stdout_overwrite) ? O_TRUNC : O_APPEND;
    fd = dc_open(env, err, cmd->stdout_file, flag | O_CREAT | O_WRONLY, S_IRWXU);
    if(dc_error_has_no_error(err))
    {
        dc_dup2(env, err, fd, STDOUT_FILENO);
    }
    dc_close(env, err, fd);
    return 0;
}


int stdin_redirect(const struct dc_posix_env *env, struct dc_error *err, const struct command *cmd) {
    int fd;
    fd = dc_open(env, err, cmd->stdin_file, O_RDONLY, NULL);
    if (dc_error_has_no_error(err))
    {
        dc_dup2(env, err, fd, STDIN_FILENO);
    }
    dc_close(env, err, fd);
    return 0;
}


int handle_run_error(struct dc_error * err)
{

    if (dc_error_is_errno(err, E2BIG))              { return 1;   }
    else if (dc_error_is_errno(err, EACCES))       { return 2;   }
    else if (dc_error_is_errno(err, EINVAL))        { return 3;   }
    else if (dc_error_is_errno(err, ELOOP))         { return 4;   }
    else if (dc_error_is_errno(err, ENAMETOOLONG))  { return 5;   }
    else if (dc_error_is_errno(err, ENOENT))        { return 127; }
    else if (dc_error_is_errno(err, ENOTDIR))       { return 6;   }
    else if (dc_error_is_errno(err, ENOEXEC))       { return 7;   }
    else if (dc_error_is_errno(err, ENOMEM))        { return 8;   }
    else if (dc_error_is_errno(err, ETXTBSY))       { return 9;   }
    else { return EOTHER; }
}


bool is_file(char * cmd)
{
    while (*cmd)
    {
        if ( *cmd == '/' )
        {
            return true;
        }
        cmd++;
    }
    return false;
}




