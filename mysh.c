#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define MAX_ARGS 32
#define MAX_PIPES 2

#define PIPE_SYMBOL "|"
#define SPACE " "

// collect the input into 3 string arrays

void fail(int id) {
  printf("Reached %d!\n", id);
  exit(EXIT_FAILURE);
}

int main() {
  char **args[MAX_PIPES + 1];
  char cmd_buffer[BUF_SIZE];

  while (1) {
    printf("$ ");

    fgets(cmd_buffer, BUF_SIZE, stdin);
    cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';

    int i, j;
    char *temp[MAX_PIPES + 1];
    char *tok;

    // tokenize by pipe, copying each string
    tok = strtok(cmd_buffer, PIPE_SYMBOL);
    for (i = 0; i < MAX_PIPES + 1 && tok != NULL; ++i) {
      temp[i] = (char *)malloc(strlen(tok) * sizeof(char));
      strcpy(temp[i], tok);

      tok = strtok(NULL, PIPE_SYMBOL);
    }

    // null-terminate if less args than we have space for
    if (i < MAX_PIPES + 1) {
      temp[i] = NULL;
      args[i] = NULL;
    }

    for (i = 0; i < MAX_PIPES + 1 && temp[i] != NULL; ++i) {
      // store each set of args as an array of 32 strings max
      args[i] = (char **)malloc(MAX_ARGS * sizeof(char *));

      // tokenize by space, copying to the above
      tok = strtok(temp[i], SPACE);
      for (j = 0; tok != NULL; ++j) {
        args[i][j] = (char *)malloc(strlen(tok) * sizeof(char));
        strcpy(args[i][j], tok);

        tok = strtok(NULL, SPACE);
      }

      // terminate each string array in NULL, as mandated by exec
      args[i][j] = NULL;

      // don't need this anymore
      free(temp[i]);
    }

    // show commands
    /*
    for (i = 0; args[i] != NULL; ++i){
      for (j = 0; args[i][j] != NULL; ++j)
        printf("%s ", args[i][j]);
      putchar('\n');
    }
    */

    int pipefd1[2], pipefd2[2];
    pid_t pid1, pid2, pid3;

    pipe(pipefd1);

    pid1 = fork();
    if (pid1 == 0) {
      if (dup2(pipefd1[1], STDOUT_FILENO) == -1)
        perror("dup2");

      if (close(pipefd1[0]) == -1)
        perror("close");
      if (close(pipefd1[1]) == -1)
        perror("close");

      execvp(args[0][0], args[0]);

      perror("exec");
      exit(EXIT_FAILURE);
    }

    pipe(pipefd2);

    pid2 = fork();
    if (pid2 == 0) {
      if (dup2(pipefd1[0], STDIN_FILENO) == -1)
        perror("dup2");

      if (dup2(pipefd2[1], STDOUT_FILENO) == -1)
        perror("dup2");

      if (close(pipefd1[0]) == -1)
        perror("close");
      if (close(pipefd1[1]) == -1)
        perror("close");

      if (close(pipefd2[0]) == -1)
        perror("close");
      if (close(pipefd2[1]) == -1)
        perror("close");

      execvp(args[1][0], args[1]);

      perror("exec");
      exit(EXIT_FAILURE);
    }

    if (close(pipefd1[0]) == -1)
      perror("close");
    if (close(pipefd1[1]) == -1)
      perror("close");

    pid3 = fork();
    if (pid3 == 0) {
      if (dup2(pipefd2[0], STDIN_FILENO) == -1)
        perror("dup2");

      if (close(pipefd2[0]) == -1)
        perror("close");
      if (close(pipefd2[1]) == -1)
        perror("close");

      execvp(args[2][0], args[2]);

      perror("exec");
      exit(EXIT_FAILURE);
    }

    // close remaining descriptors
    if (close(pipefd2[0]) == -1)
      perror("close");
    if (close(pipefd2[1]) == -1)
      perror("close");

    // wait for process completion to move on
    do {
      while (wait(NULL) > 0)
        ;
    } while (errno != ECHILD);

    // clear input storage for next use
    for (i = 0; args[i] != NULL; ++i) {
      for (j = 0; args[i][j] != NULL; ++j)
        free(args[i][j]);
      free(args[i]);
    }
  }
}
