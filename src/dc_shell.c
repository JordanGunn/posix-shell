#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdint-gcc.h>
#include "../include/dc_shell.h"

int main() {
    printf("Hello, World!\n");
    return 0;
}


int parse_input(char * cmd, char * params[])
{
    char in[512];
    uint8_t byte_count;
    char * array[100];
    char * str;

    read_n_bytes(STDIN_FILENO, in, sizeof(in));


    return 0;
}

//
//void prompt();
//void _exit();
//void run();



ssize_t read_n_bytes(int file_des, void * pBuffer, size_t num_bytes)
{
    size_t bytes_left;
    ssize_t bytes_read;
    char * buffer;

    buffer = pBuffer;
    bytes_left = num_bytes;

    while (bytes_left > 0)
    {
        // if system call fails
        if ( (bytes_read = read(file_des, buffer, bytes_left)) < 0 )
        {
            // if system
            // call was interrupted - we need to try again
            if ( errno == EINTR )     { bytes_left = 0; }
                // any other error - something went wrong
            else    { return -1; }
        }
        // all bytes read
        else if ( bytes_read == 0 )   { break; }

        // decrement bytes and move pointer
        bytes_left -= (unsigned long) bytes_read;
        buffer += bytes_read;
    }

    return ( (ssize_t) (num_bytes - bytes_left) );
}


