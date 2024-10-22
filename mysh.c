#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define MAX_ARGS 32
#define MAX_PIPES 3

int main() {
  char cmd_buffer[BUF_SIZE];
  char **tokens;
  int token_c;
  int i, j;

  while (1) {
    printf("$ ");

    // get whole command, clean newlines
    fgets(cmd_buffer, BUF_SIZE, stdin);
    cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';

    // tokenize by pipe
    char *tok = strtok(cmd_buffer, "|");
    for (i = 0; tok != NULL; ++i) {
      tokens[i] = (char *)malloc(strlen(tok) * sizeof(char));
      strcpy(tokens[i], tok);

      tok = strtok(NULL, "|");
    }

    token_c = i;

    char **args;
    int arg_c = 0;
    
    for (i = 0; i < token_c - 1; ++i) {
      // wipe
      for (j = 0; j < arg_c; ++j)
        free(args[j]);
      arg_c = 0;

      // tokenize by space
      char *tok = strtok(tokens[i], " ");
      for (j = 0; tok != NULL && j < MAX_ARGS; ++j) {
        args[j] = (char *)malloc(strlen(tok) * sizeof(char));
        strcpy(args[j], tok);

        tok = strtok(NULL, " ");
      }

      arg_c = j;

      printf("running: ");
      for (j = 0; j < arg_c; ++j)
        printf("%s ", args[j]);
      putchar('\n');
      
      int pipefd[2];
      pipe(pipefd);
      
      int pid = fork();
      if (pid == 0){
        dup2(pipefd[1], STDOUT_FILENO);
        
        execvp(args[0], args);

        perror("exec");
        exit(EXIT_FAILURE);
      }

      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[1]);
    }
   
    execvp(args[0], args);

    perror("exec");
    exit(EXIT_FAILURE);
  }
}
