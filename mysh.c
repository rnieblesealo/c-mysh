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

    // (use...)
    for (i = 0; args[i] != NULL; ++i) {
      for (j = 0; args[i][j] != NULL; ++j)
        printf("%s ", args[i][j]);
      putchar('\n');
    }  
    
    // clear for next use
    for (i = 0; args[i] != NULL; ++i) {
      for (j = 0; args[i][j] != NULL; ++j)
        free(args[i][j]);
      free(args[i]);
    }
  }
}
