#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int get_command(char **out, char* in, size_t out_len, size_t in_len, size_t start_idx) {
  char *start = &in[start_idx]; 

  // Convert special characters between quotations into control characters
  quote_special_to_ctrl(start, in_len - start_idx);

  // Get the end position for the current command
  int pipe_pos = findpipe(start, in_len - start_idx);
  int end_pos = (pipe_pos < 0) ? strlen(start) - 1 : pipe_pos;

  // Copy the current command to its own buffer
  char cmd[end_pos + 1];
  memcpy(cmd, start, end_pos * sizeof(char));
  cmd[end_pos] = 0; 

  // Tokenize the command on whitespace
  int i = 0;
  char* token = strtok(cmd, " \t\n\r\"");
  while (token != NULL) {
    if (i >= out_len) {
      free(out[i - 1]);
      out[i - 1] = NULL;
      return -1;
    }

    out[i] = strdup(token);
    i++;

    token = strtok(NULL, " \t\n\r\"");
  }

  if (i >= out_len) {
    free(out[i - 1]);
    out[i - 1] = NULL;
    return -1;
  }
  out[i] = NULL;

  // Convert control characters in tokens back into their special characters
  for (int j = 0; j < i; j++) {
    quote_ctrl_to_special(out[j], strlen(out[j]));
  }

  // Get the position of the next command, if there is a pipe
  int cmd_pos = -1;
  if (pipe_pos >= 0) {
    for (int i = pipe_pos + 1; i < in_len - start_idx; i++) {
      if (start[i] != ' ') {
        cmd_pos = i;
        break;
      }
    }
  }

  return cmd_pos;
}

int findpipe(const char* inputbuffer, size_t bufferlen){
  for (int i = 0; i < bufferlen; i++) {
    if (inputbuffer[i] == '|') {
      return i;
    }
  }

  return -1;
}

int get_command_path(char *out, const char *cmd, size_t out_len) {
  if (out == NULL || cmd == NULL) return -1;
  if (strlen(cmd) >= out_len) return -1;

  // Default for error message
  strcpy(out, cmd);

  // Don't do anything if the command is already fully-qualified
  if (cmd[0] == '/') {
    return 0;
  }

  int ret = -1;

  const char *path = getenv("PATH");
  if (path == NULL) return -1;

  char *cwd = getcwd(NULL, 0);

  // Prepend current working directory to PATH
  char *new_path;
  asprintf(&new_path, "%s:%s", cwd, path);

  // Tokenize path on directory delimiters
  char *token = strtok(new_path, ":");
  while (token != NULL) {
    size_t full_path_len = strlen(token) + strlen(cmd) + 1;

    char *full_path;
    // Construct the full path for a given PATH directory
    asprintf(&full_path, "%s/%s", token, cmd);

    // Check if the command is found
    if (access(full_path, F_OK) == 0) {
      if (full_path_len < out_len) {
        strcpy(out, full_path);
        ret = 0;
      }

      free(full_path);
      break;
    }

    free(full_path);

    token = strtok(NULL, ":");
  }

  free(new_path);

  return ret;
}

void quote_special_to_ctrl(char* in, size_t in_len) {
  bool in_quotes = false;

  for (int i = 0; i < in_len; i++) {
    if (in[i] == '"') {
      in_quotes = !in_quotes;
    }

    if (in_quotes) {
      switch (in[i]) {
        case ' ':
          in[i] = CTRL_SPACE;
          break;

        case '|':
          in[i] = CTRL_PIPE;
          break;

        default:
          break;
      }
    }
  }
}

void quote_ctrl_to_special(char* in, size_t in_len) {
  for (int i = 0; i < in_len; i++) {
    switch (in[i]) {
      case CTRL_SPACE:
        in[i] = ' ';
        break;

      case CTRL_PIPE:
        in[i] = '|';
        break;

      default:
        break;
    }
  }
}
