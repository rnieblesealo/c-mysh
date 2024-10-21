#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define MAX_ARGS 32
#define MAX_PIPES 3

void free_args(char *(*args)[MAX_ARGS]) {
  printf("Freeing args...\n");

  int i = 0, j = 0;
  while (args[i][0] != NULL) {
    while (args[i][j] != NULL) {
      printf("Freeing %d %d...\n", i, j);

      free(args[i][j]);
      j++;
    }

    i = i + 1;
    j = 0;
  }
}

int main() {
  char cmd_buffer[BUF_SIZE];
  char *(*args)[MAX_ARGS];
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

    int i = 0; // command
    int j = 0; // individual arg

    while (tok != NULL && j < MAX_ARGS) {
      // handle exit
      if (strcmp(tok, "exit") == 0)
        break;

      // if hit pipe token, null-terminate this args array and move to next
      if (strcmp(tok, "|") == 0) {
        args[i][j] = NULL;

        i = i + 1;
        j = 0;
      }

      else {
        // add arg
        printf("Allocing %d %d...\n", i, j);

        args[i][j] = (char *)malloc(strlen(tok));
        strncpy(args[i][j], tok, strlen(tok));

        // move to next slot
        j++;
      }

      // move token 
      tok = strtok(NULL, " ");
    }

    // nullterm last arg
    args[i][j] = NULL;

    i++;

    // first unused arg slot begins in null
    args[i][0] = NULL;

    // if used cd, change working dir to passed one
    if (strcmp(args[0][0], "cd") == 0) {
      if (chdir(args[0][1]) == -1) {
        perror("chdir");
      }
    }

    else {
      // run process
      pid1 = fork();
      if (pid1 == 0) {
        if (execvp(args[0][0], args[0]) == -1) {
          perror("execvp");
        }
      }

      else {
        // if parent, await completion
        wait(NULL);
        
        // once done with everything, this run, free args
        free_args(args);
      }
    }
  }

  exit(EXIT_SUCCESS);
}
