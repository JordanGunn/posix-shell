//
// Created by jordan on 2022-01-23.
//

#ifndef SHELL_SHELL_H
#define SHELL_SHELL_H

void prompt();
int parse_input(char * cmd, char * params[]);
void _exit();
void run();
ssize_t read_n_bytes(int file_des, void * pBuffer, size_t num_bytes);

#endif //SHELL_SHELL_H
