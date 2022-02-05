//
// Created by jordan on 2022-01-25.
//


#include <../include/util.h>
#include <string.h>
#include <unistd.h>


char *get_prompt(const struct dc_posix_env *env, struct dc_error *err)
{
    char * prompt_env, * temp;
    if ( (prompt_env = dc_getenv(env, "PS1")) == NULL ) {
        dc_setenv(env, err, "PS1", "$ ", true);
    }
    temp = dc_getenv(env, "PS1");
    prompt_env = malloc(strlen(temp) + 1);
    memcpy(prompt_env, temp, strlen(temp));
    prompt_env[strlen(temp)] = '\0';

    return prompt_env;
}


char *get_path(const struct dc_posix_env *env, struct dc_error *err)
{
    char * path_env, * temp;
    if ( (path_env = dc_getenv(env, "PATH")) == NULL ) { return NULL; }
    temp = dc_getenv(env, "PATH");
    path_env = malloc(strlen(temp) + 1);
    memcpy(path_env, temp, strlen(temp));
    path_env[strlen(temp)] = '\0';

    return path_env;
}


char **parse_path(const struct dc_posix_env *env, struct dc_error *err,
                  const char *path_str)
{
    char ** parsed;
    char * token, * path_crawler, * path_str_cpy;
    unsigned long path_count;
    int str_count;


    path_str_cpy = strdup(path_str);
    path_count = (unsigned long) get_num_env_paths(path_str_cpy);
    parsed = dc_malloc( env, err, (path_count + 1) * sizeof(char *));

    if (dc_error_has_no_error(err))
    {
        str_count = 0;
        path_crawler = path_str_cpy;

        if (!path_str_cpy) {
            parsed [0] = NULL;
        } else {
            while ((token = strtok_r(path_crawler, ":", &path_crawler)) != NULL)
            {
                parsed[ str_count ] = malloc(strlen(token) + 1);
                dc_memcpy( env, parsed[str_count], token, strlen(token) );
                parsed[ str_count ][ strlen(token) ] = '\0';

                str_count++;
            }
            parsed[str_count] = NULL;
        }
        return parsed;
    }
    return NULL;
}


int get_num_env_paths(char *path_str) {

    char * str_crawler;
    int path_count;

    str_crawler = path_str;
    path_count = 0;
    while (*(str_crawler++))
    {
        if (*str_crawler == ':') { path_count++; }
    }

    return ++path_count;
}


void do_reset_state(const struct dc_posix_env *env, struct dc_error *err, struct state *state)
{
    // free pointers
    if (state->current_line)
    {
        free(state->current_line), state->current_line = NULL;
    }

    if (state->command)
    {
        free(state->command), state->command = NULL;
    }

    // set file pointers to NULL
    state->stdin = stdin;
    state->stdout = stdout;
    state->stderr = stderr;

    // Set numerics and booleans to 0
    state->fatal_error          = 0;
    state->current_line_length  = 0;

    if (state->command)
    {
        destroy_command(env, state->command);
    }

    dc_error_reset(err);
}


void do_destroy_state(const struct dc_posix_env *env, struct dc_error *err, struct state *state)
{
//     free pointers
    if (state->in_redirect_regex)
    {
        regfree(state->in_redirect_regex);
        free(state->in_redirect_regex); state->in_redirect_regex  = NULL;
    }

    if (state->out_redirect_regex)
    {
        regfree(state->out_redirect_regex);
        free(state->out_redirect_regex); state->out_redirect_regex = NULL;
    }

    if (state->err_redirect_regex)
    {
        regfree(state->err_redirect_regex);
        free(state->err_redirect_regex); state->err_redirect_regex = NULL;
    }

    if (state->current_line)
    {
        free(state->current_line), state->current_line = NULL;
    }

    if (state->prompt)
    {
        free(state->prompt), state->prompt = NULL;
    }

    // set file pointers to NULL
    state->stdin = stdin;
    state->stdout = stdout;
    state->stderr = stderr;

    // Set numerics and booleans to 0
    state->fatal_error          = 0;
    state->max_line_length      = 0;
    state->current_line_length  = 0;

    // free array of strings
    if (state->path) {
        while ( *(state->path) )
        {
            if (*(state->path) )
            {
                free( *(state->path) );
                *(state->path) = NULL;
                (state->path)++;
            }
        }
        state->path = NULL; // (Path is innocent)
    }

    if (state->command)
    {
        destroy_command(env, state->command);
    }

    dc_error_reset(err);
}


void display_state(const struct dc_posix_env *env, const struct state *state, FILE *stream)
{
    printf("PATH: ");
    print_paths(state->path);
    fprintf(stream, "PROMPT:\t\t%s\n", state->prompt);
    fprintf(stream, "MAX LINE LENGTH:\t%zu\n", state->max_line_length);
    fprintf(stream, "CURRENT LINE:\t\t%s\n", state->current_line);
    fprintf(stream, "CURRENT LINE LENGTH:\t%zu\n", state->current_line_length);

    fclose(stream);
}


void print_paths(char ** path)
{
    int num_paths, index;
    char * env_path;

    env_path = getenv("PATH");
    num_paths = get_num_env_paths( env_path );

    if (num_paths != 0 && env_path != NULL)
    {
        printf("[ ");
        for (index = 0; index < num_paths - 1; index++)
        {
            printf("%s, ", path[index]);
        }
        printf("%s ]\n", path[num_paths - 1]);
    }
}


char *state_to_string(const struct dc_posix_env *env,  struct dc_error *err, const struct state *state)
{
    char buff[LG_BUFF];
    char * to_string;

    memset(buff, 0, LG_BUFF);
    sprintf(
        buff,(state->current_line)                           // format
        ? "current_line = \"%s\", fatal_error = %d"                 // format
        : "current_line = %s, fatal_error = %d",                    // format
            (state->current_line) ? state->current_line : "NULL",   // insert
            state->fatal_error                                      // insert
    );

    to_string = dc_malloc(env, err, strlen(buff));
    dc_memcpy(env, to_string, buff, strlen(buff) + 1);

    return to_string;
}
