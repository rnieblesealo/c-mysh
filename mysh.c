#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

int main(){  
  char cmd_buffer[BUF_SIZE]; 
  char *cmd_tok;

  int running = 1;
  while (running){
    // show prompt
    printf("$ ");

    // read stdin, removing any newlines
    fgets(cmd_buffer, BUF_SIZE, stdin);
    cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';
    
    // tokenize by spaces 
    cmd_tok = strtok(cmd_buffer, " ");

    while (cmd_tok != NULL){
      // handle exit 
      if (strcmp(cmd_tok, "exit") == 0)
        exit(EXIT_SUCCESS);

      // advance
      cmd_tok = strtok(NULL, " ");
    }
  }


  return 0;
}
