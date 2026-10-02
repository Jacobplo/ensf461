#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"

#define BUFLEN 1024

//To Do: This base file has been provided to help you start the lab, you'll need to heavily modify it to implement all of the features

int main() {
  char buffer[1024];
  char* parsedinput;
  char* args[16] = { 0 };
  char newline;

  printf("Welcome to the GroupXX shell! Enter commands, enter 'quit' to exit\n");
  do {
    //Print the terminal prompt and get input
    printf("$ ");
    char *input = fgets(buffer, sizeof(buffer), stdin);
    if(!input) {
      fprintf(stderr, "Error reading input\n");
      return -1;
    }

    //Clean and parse the input string
    //parsedinput = (char*) malloc(BUFLEN * sizeof(char));
    //size_t parselength = trimstring(parsedinput, input, BUFLEN);
 
    int cmd_pos = -1;
    int pipe_fd[2];
    do {
      // Get a single tokenized command, before separated at pipes
      cmd_pos = tokenize_string(args, &input[cmd_pos + 1], 16, BUFLEN - cmd_pos);

      pipe(pipe_fd);

      if (strcmp(args[0], "quit") == 0) {
        printf("Bye!!\n");
        return 0;
      }
      else {
        int rc = fork();
        if (rc == 0) {
          execvp(args[0], args);
        }

        else {
          wait(NULL);

          // Free memory allocated by tokenize_string()
          int i = 0;
          char *cur = args[i];
          while (cur != NULL) {
            free(cur);
            i++;
            cur = args[++i];
          }
        }
      }
    } while(cmd_pos >= 0);

    //Remember to free any memory you allocate!
    //free(parsedinput);
  } while (1);

  return 0;
}
