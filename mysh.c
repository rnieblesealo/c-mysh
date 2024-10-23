#include <ctype.h>
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

#define CHANGE_DIR_CMD "cd"
#define EXIT_CMD "exit"

void wait4procs() {
  // wait for process completion to move on
  do {
    while (wait(NULL) > 0)
      ;
  } while (errno != ECHILD);
}

void freeargs(char **args[]) {
  // just null-term'd 2d char array clearing
  for (int i = 0; args[i] != NULL; ++i) {
    for (int j = 0; args[i][j] != NULL; ++j)
      free(args[i][j]);
    free(args[i]);
  }
}

int main() {
  char **args[MAX_PIPES + 1];
  char cmd_buffer[BUF_SIZE];

  while (1) {
    printf("$ ");

    fgets(cmd_buffer, BUF_SIZE, stdin);

    cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';

    // handle input that is either just an enter or all spaces 
    int all_spaces = 1;
    for (int i = 0; cmd_buffer[i] != '\0'; ++i) {
      if (cmd_buffer[i] != ' ')
        all_spaces = 0;
    }

    if (all_spaces)
      continue;

    // the part above works, but if there's only spaces, it breaks :/

    int cmd_count;
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
    cmd_count = i;
    if (cmd_count < MAX_PIPES + 1) {
      temp[cmd_count] = NULL;
      args[cmd_count] = NULL;
    }

    for (i = 0; i < MAX_PIPES + 1 && temp[i] != NULL; ++i) {
      // store each set of args as an array of 32 strings max
      args[i] = (char **)malloc(MAX_ARGS * sizeof(char *));

      // tokenize by space, copying to the above
      tok = strtok(temp[i], SPACE);
      for (j = 0; j < MAX_ARGS && tok != NULL; ++j) {
        args[i][j] = (char *)malloc(strlen(tok) * sizeof(char));
        strcpy(args[i][j], tok);

        tok = strtok(NULL, SPACE);
      }

      // terminate each string array in NULL, as mandated by exec
      args[i][j] = NULL;

      // don't need this anymore
      free(temp[i]);
    }

    int pipefd1[2], pipefd2[2];
    pid_t pid1, pid2, pid3;

    // handle exit
    if (strcmp(args[0][0], EXIT_CMD) == 0) {
      freeargs(args);
      exit(EXIT_SUCCESS);
    }

    // handle cd
    if (strcmp(args[0][0], CHANGE_DIR_CMD) == 0) {
      if (chdir(args[0][1]) == -1)
        perror("chdir");

      // doing the below assumes cd will always be the first comman
      freeargs(args);
      wait4procs();
      continue;
    }

    if (cmd_count > 1)
      if (pipe(pipefd1) == -1)
        perror("pipe");

    pid1 = fork();
    if (pid1 == 0) {
      if (cmd_count > 1) {
        if (dup2(pipefd1[1], STDOUT_FILENO) == -1)
          perror("dup2");

        if (close(pipefd1[0]) == -1)
          perror("close");
        if (close(pipefd1[1]) == -1)
          perror("close");
      }

      execvp(args[0][0], args[0]);

      perror("exec");
      exit(EXIT_FAILURE);
    }

    if (cmd_count <= 1) {
      freeargs(args);
      wait4procs();
      continue;
    }

    if (cmd_count > 2)
      if (pipe(pipefd2) == -1)
        perror("pipe");

    pid2 = fork();
    if (pid2 == 0) {
      if (dup2(pipefd1[0], STDIN_FILENO) == -1)
        perror("dup2");

      if (cmd_count > 2) {
        if (dup2(pipefd2[1], STDOUT_FILENO) == -1)
          perror("dup2");

        if (close(pipefd2[0]) == -1)
          perror("close");
        if (close(pipefd2[1]) == -1)
          perror("close");
      }

      if (close(pipefd1[0]) == -1)
        perror("close");

      if (close(pipefd1[1]) == -1)
        perror("close");

      execvp(args[1][0], args[1]);

      perror("exec");
      exit(EXIT_FAILURE);
    }

    if (close(pipefd1[0]) == -1)
      perror("close");
    if (close(pipefd1[1]) == -1)
      perror("close");

    if (cmd_count <= 2) {
      freeargs(args);
      wait4procs();
      continue;
    }

    // anything past here runs all 3 commands with all 2 pipes

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

    // move on!
    freeargs(args);
    wait4procs();
  }
}
