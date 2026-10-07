#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"

#define BUFLEN 1024
#define ARGS_LEN 16

int main() {
  char buffer[BUFLEN] = { 0 };
  char* args[ARGS_LEN] = { 0 };

  printf("Welcome to the GroupXX shell! Enter commands, enter 'quit' to exit\n");
  while (1) {
    // Print the terminal prompt and get input
    printf("$ ");
    char *input = fgets(buffer, sizeof(buffer), stdin);
    if(!input) {
      fprintf(stderr, "Error reading input\n");
      return -1;
    }

    // Get a single tokenized command, before separated at pipes
    int cmd_pos = get_command(args, input, ARGS_LEN, BUFLEN, 0); 
    
    if (args[0] != NULL) {
      if (strcmp(args[0], "quit") == 0) {
        printf("Bye!!\n");
        return 0;
      }
    }

    int pipe_fd[2];
    pipe(pipe_fd);

    int rc = fork();
    if (rc == 0) {
      if (cmd_pos > 0) {
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
      }
      execvp(args[0], args);
      perror("execvp");
      _exit(127);
    }

    int rc2 = -1;
    if (cmd_pos > 0) {
      rc2 = fork();
      if (rc2 == 0) {
        cmd_pos = get_command(args, input, ARGS_LEN, BUFLEN, cmd_pos);
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        execvp(args[0], args);
        perror("execvp");
        _exit(127);
      }
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);

    waitpid(rc, NULL, 0);
    if (rc2 != -1) waitpid(rc2, NULL, 0);

    // Free memory allocated by get_command()
    int i = 0;
    char *cur = args[i];
    while (cur != NULL) {
      free(cur);
      cur = args[++i];
    }
  };

  return 0;
}
