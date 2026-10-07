#ifndef __PARSER_H
#define __PARSER_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>

//ToDo: Here are some suggested parsing commands, you may want to expand this list, 
//Please feel free to change modify the commands below to suit your needs

#define CTRL_SPACE 0x01
#define CTRL_PIPE  0x02

size_t trimstring(char* outputbuffer, const char* inputbuffer, size_t bufferlen);
size_t firstword(char* outputbuffer, const char* inputbuffer, size_t bufferlen);
bool isvalidascii(const char* inputbuffer, size_t bufferlen);

int get_command(char **out, char* in, size_t out_len, size_t in_len, size_t start_idx);
int get_command_path(char *out, const char *cmd, size_t out_len);
void quote_special_to_ctrl(char* in, size_t in_len);
void quote_ctrl_to_special(char* in, size_t in_len);
int findpipe(const char* inputbuffer, size_t bufferlen);

#endif
