//
// Created by jordan on 2022-01-27.
//

#include "input.h"
#include "common.h"

/**
 * Read the command line from the user.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param stream The stream to read from (eg. stdin)
 * @param line_size the maximum characters to read.
 * @return The command line that the user entered.
 */
char * read_command_line(const struct dc_posix_env *env, struct dc_error *err, FILE * stream, size_t * line_size)
{
    char * line;
    char buff[*line_size];

    memset(buff, 0, *line_size);
    if ( (fgets(buff, (int)*line_size + 1, stream) != NULL) )
    {
        trim(buff);
        line = dc_malloc(env, err, strlen(buff) + 1);
        if (dc_error_has_no_error(err))
        {
            memcpy(line, buff, strlen(buff));
            line[strlen(buff)] = '\0';
            *line_size = strlen(line);
        }
    } else
    {
        line =  dc_malloc(env, err, sizeof(char));
        if (dc_error_has_no_error(err)) {
            line[0] = '\0';
        }
        *line_size = 0;
    }

    return line;
}


void trim(char * str)
{

    char * copy, * start;
    int pos;

    copy = strdup(str);
    start = copy;
    memset(str, 0, strlen(str));

    // trim leading (move pointer while char is fluff)
    pos = 0;
    while (is_fluff(*copy))
    {
        copy++;
        pos++;
    }

    // move pointer to end of string
    copy += strlen(copy) - 1;

    while (is_fluff(*copy))
    {
        *copy = '\0';
        copy--;
    }

    memcpy(str, start + pos, strlen(start + pos) + 1);
}


bool is_fluff(char character)
{
    char whitespace, newline, vert, tab, f_esc;
    bool is_whitespace, is_newline, is_tab, is_vert, is_esc;

    whitespace = ' ';
    newline = '\n';
    f_esc = '\f';
    vert = '\v';
    tab = '\t';

    is_whitespace   = (character == whitespace);
    is_newline      = (character == newline);
    is_tab          = (character == tab);
    is_vert         = (character == vert);
    is_esc          = (character == f_esc);

    return (is_whitespace || is_newline || is_tab || is_esc || is_vert);
}

