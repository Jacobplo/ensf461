#ifndef __PARSER_H
#define __PARSER_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>


#define CTRL_SPACE 0x01
#define CTRL_PIPE  0x02


/**
 * Accepts a string (in) that represents a full command to be executed, and
 * tokenizes it into (out). It accepts a start index to only tokenize from that
 * point on.
 *
 * Returns the index of the next command to be run, if there is a pipe.
 * Otherwise returns -1.
 */

int get_command(char **out, char* in, size_t out_len, size_t in_len, size_t start_idx);

/**
 * Returns to first index of a pipe character
 */
int findpipe(const char* inputbuffer, size_t bufferlen);

/**
 * Searches PATH for the provided command, and stores the resulting
 * fully-qualified path in out.
 *
 * Returns 0 on success, or -1 on failure or if the command is not found in PATH
 */
int get_command_path(char *out, const char *cmd, size_t out_len);

/**
 * Replaces special characters within quotations with unused sentinel values.
 *
 * For example, spaces and pipes within quotations should not be used during
 * tokenization.
 */
void quote_special_to_ctrl(char* in, size_t in_len);

/**
 * Replaces the sentinel characters in a string to their special character
 * equivalent.
 */
void quote_ctrl_to_special(char* in, size_t in_len);

#endif
