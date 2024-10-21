#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define MAX_ARGS 32

int main() {
  char cmd_buffer[BUF_SIZE];
  char *args[MAX_ARGS];
  char *tok;

  int fd1[2], fd2[2];
  int pid1, pid2, pid3;

  while (1) {
    // show prompt
    printf("$ ");

    // read stdin, removing any newlines
    fgets(cmd_buffer, BUF_SIZE, stdin);
    cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';

    // tokenize by spaces
    tok = strtok(cmd_buffer, " ");

    int i = 0;
    while (tok != NULL && i < MAX_ARGS) {
      // handle exit
      if (strcmp(tok, "exit") == 0)
        exit(EXIT_SUCCESS);
  
      // add arg
      args[i] = (char *)malloc(strlen(tok));
      strcpy(args[i], tok);

      // move on
      tok = strtok(NULL, " ");
      i++;
    }

    // null-terminate the args array
    args[i] = (char *)NULL;

    // if used cd, change working dir to passed one
    if (strcmp(args[0], "cd") == 0) {
      if (chdir(args[1]) == -1) {
        perror("chdir");
      }
    }

    else {
      // run process
      pid1 = fork();
      if (pid1 == 0) {
        if (execvp(args[0], args) == -1) {
          perror("execvp");
          exit(EXIT_FAILURE);
        }
      }

      // await completion
      wait(NULL);
    }

    // free args
    i = 0;
    while (args[i] != NULL) {
      free(args[i++]);
    }
  }
}
