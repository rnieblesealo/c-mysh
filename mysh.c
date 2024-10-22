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
  char *cmd_tokens[MAX_ARGS];
  
  while (1) {
    printf("$ ");

    // get whole command
    fgets(cmd_buffer, BUF_SIZE, stdin);
    cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';

    // tokenize by space
    int i;
    char *tok = strtok(cmd_buffer, " ");
    for (i = 0; i < MAX_ARGS && tok != NULL; ++i) {
      cmd_tokens[i] = (char*)malloc(strlen(tok) * sizeof(char));
      strcpy(cmd_tokens[i], tok);
      tok = strtok(NULL, " ");
    }
    
    i = 0; 
  }

  exit(EXIT_SUCCESS);
}
