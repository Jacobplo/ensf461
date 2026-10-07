#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//Function to trim whitespace and ASCII control characters from buffer
//[Input] char* inputbuffer - input string to trim
//[Input] size_t bufferlen - size of input and output string buffers
//[Output] char* outputbuffer - output string after trimming 
//[Return] size_t - size of output string after trimming
size_t trimstring(char* outputbuffer, const char* inputbuffer, size_t bufferlen)
{   
    memcpy(outputbuffer, inputbuffer, bufferlen*sizeof(char));

    for(size_t ii = strlen(outputbuffer)-1; ii >=0; ii--){
        if(outputbuffer[ii] < '!') //In ASCII '!' is the first printable (non-control) character
        {
            outputbuffer[ii] = 0;
        }else{
            break;
        }    
    }

    return strlen(outputbuffer);
}

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
      return -1;
    }

    out[i] = strdup(token);
    i++;

    token = strtok(NULL, " \t\n\r\"");
  }

  if (i >= out_len) {
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

//Function to trim the input command to just be the first word
//[Input] char* inputbuffer - input string to trim
//[Input] size_t bufferlen - size of input and output string buffers
//[Output] char* outputbuffer - output string after trimming 
//[Return] size_t - size of output string after trimming
size_t firstword(char* outputbuffer, const char* inputbuffer, size_t bufferlen)
{
    //TO DO: Implement this function
    return 0;
}

//Function to test that string only contains valid ascii characters (non-control and not extended)
//[Input] char* inputbuffer - input string to test
//[Input] size_t bufferlen - size of input buffer
//[Return] bool - true if no invalid ASCII characters present
bool isvalidascii(const char* inputbuffer, size_t bufferlen)
{
    //TO DO: Correct this function so that the second test string fails
    size_t testlen = bufferlen;
    size_t stringlength = strlen(inputbuffer);
    if(strlen(inputbuffer) < bufferlen){
        testlen = stringlength;
    }

    bool isValid = true;
    for(size_t ii = 0; ii < testlen; ii++)
    {
        isValid &= ((unsigned char) inputbuffer[ii] <= '~'); //In (lower) ASCII '~' is the last printable character
    }

    return isValid;
}

//Function to find location of pipe character in input string
//[Input] char* inputbuffer - input string to test
//[Input] size_t bufferlen - size of input buffer
//[Return] int - location in the string of the pipe character, or -1 pipe character not found
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

  if (cmd[0] == '/') {
    strcpy(out, cmd);
    return 0;
  }

  int ret = -1;
  char *full_path = NULL;

  const char *path = getenv("PATH");
  if (path == NULL) return -1;

  char *path_copy = strdup(path);
  if (path_copy == NULL) return -1;

  char *token = strtok(path_copy, ":");
  while (token != NULL) {
    size_t full_path_len = strlen(token) + strlen(cmd) + 1;

    char *full_path = malloc(full_path_len + 1);
    if (full_path == NULL) break;

    snprintf(full_path, full_path_len + 1, "%s/%s", token, cmd);

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

  free(path_copy);

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
