#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parser.h"


#define BUFLEN 1024
#define ARGS_LEN 16

extern char **environ;

int main() {
  char input_buf[BUFLEN] = { 0 };
  char full_path[BUFLEN];
  char *args[ARGS_LEN] = { 0 };
  int ret;

  printf("Welcome to the GroupXX shell! Enter commands, enter 'quit' to exit\n");
  while (1) {
    // Print the terminal prompt and get input
    printf("$ ");
    char *input = fgets(input_buf, sizeof(input_buf), stdin);
    if(!input) {
      fprintf(stderr, "Error reading input\n");
      return -1;
    }

    // Get a single tokenized command, before separated at pipes
    int cmd_pos = get_command(args, input, ARGS_LEN, BUFLEN, 0); 
    if (args[0] == NULL) continue;

    // Exit shell if requested
    if (strcmp(args[0], "quit") == 0) {
      printf("Bye!!\n");
      return 0;
    }

    // Get the fully-qualified path of the command
    ret = get_command_path(full_path, args[0], BUFLEN); 
    if (ret < 0) {
      printf("shell: Unknown command: %s\n", full_path);
      continue;
    }

    free(args[0]);
    args[0] = strdup(full_path);

    // Create a kernel pipe if the command contains a pipe
    int pipe_fd[2];
    if (cmd_pos >= 0) { 
      pipe(pipe_fd);
    } 

    // Fork and execute the new command
    int rc = fork();
    if (rc < 0) {
      perror("fork");
      _exit(127);
    }
    else if (rc == 0) {
      // Write the the pipe, if applicable
      if (cmd_pos > 0) {
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
      }
      execve(args[0], args, environ);
      perror("execve");
      _exit(127);
    }

    // Get and run the second command if there is a pipe
    int rc2 = -1;
    if (cmd_pos > 0) {
      rc2 = fork();
      if (rc2 < 0) {
        perror("fork");
        _exit(127);
      }
      else if (rc2 == 0) {
        // Get the command
        cmd_pos = get_command(args, input, ARGS_LEN, BUFLEN, cmd_pos);
        if (args[0] == NULL) continue;

        // Get the command path
        ret = get_command_path(full_path, args[0], BUFLEN); 
        if (ret < 0) {
          printf("shell: Unknown command: %s\n", full_path);
          _exit(127);
        }

        free(args[0]);
        args[0] = strdup(full_path);

        // Receive from the pipe
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        execve(args[0], args, environ);
        perror("execve");
        _exit(127);
      }
    }

    // Close pipes, if applicable
    if (cmd_pos >= 0) {
      close(pipe_fd[0]);
      close(pipe_fd[1]);
    }
   
    // Wait on child processes
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
