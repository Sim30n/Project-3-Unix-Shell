// Petteri Särkkä 23.7.2020
// Project 3: Unix Shell
// LÄHTEET:
// https://brennan.io/2015/01/16/write-a-shell-in-c/
// https://www.geeksforgeeks.org/making-linux-shell-c/
// C ohjelmoinnin perusteet Kurssimateriaali

#define _GNU_SOURCE
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WISH_TOK_DELIM " \t\r\n\a"

// vakio error viesti
char error_message[30] = "An error has occurred\n";

// sisäänrakennettu cd komento
void wish_cd(char **args){
  if (args[1] == NULL) {
    fprintf(stderr, error_message, strlen(error_message));
  } else {
    if (chdir(args[1]) != 0) {
      fprintf(stderr, error_message, strlen(error_message));
    }
  }
}

// sisäänrakennettu path komento
void wish_path(char **args){
  if (args[1] == NULL) {
    fprintf(stderr, error_message, strlen(error_message));
  } else {
    if (setenv("PATH", args[1], 1) != 0) {
      fprintf(stderr, error_message, strlen(error_message));
    }
  }
}

// lukee komentorivin funktio
char *wish_read_line(void){
  char *line = NULL;
  size_t len = 0;
  if (getline(&line, &len, stdin) == -1){
    if (feof(stdin)) {
      exit(0);
    } else  {
      fprintf(stderr, error_message, strlen(error_message));
      exit(0);
    }
  }
  return line;
}

// parsitaan komentorivin komennot
// lähde: https://brennan.io/2015/01/16/write-a-shell-in-c/
char **wish_split_line(char *line){
  int bufsize = 1024;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;
  if (!tokens) {
    fprintf(stderr, error_message, strlen(error_message));
    exit(1);
  }
  token = strtok(line, WISH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;
    if (position >= bufsize) {
      bufsize += 1024;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, error_message, strlen(error_message));
        exit(1);
      }
    }
    token = strtok(NULL, WISH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

// lähde: https://www.geeksforgeeks.org/making-linux-shell-c/
// suoritetaan komentorivin komennot
void execArgs(char** args){
  char *builtin_str[] = {"cd","exit","path"};
  int i;
  for (i = 0; i < 2; i++) {
    if (strcmp(args[0], builtin_str[0]) == 0) {
      wish_cd(args);
      return;
    } else if (strcmp(args[0], builtin_str[1]) == 0){
      exit(0); // sisäänrakennettu exit komento
      return;
    } else if (strcmp(args[0], builtin_str[2]) == 0){
      wish_path(args);
      return;
    }
  }
  // Fork child
  pid_t pid = fork();
  if (pid == -1) {
      fprintf(stderr, error_message, strlen(error_message));
      return;
  } else if (pid == 0) {
      if (execvp(args[0], args) < 0) {
          fprintf(stderr, error_message, strlen(error_message));
      }
      exit(0);
  } else {
      // odotetaan child loppuun
      wait(NULL);
        return;
    }
}

// pääohjelman looppi
void wish_loop(char *argu){
  char *line;
  char **args;
  do {
    if(argu != NULL){
      line = argu;
      args = wish_split_line(line);
      execArgs(args);
      free(args);
      break;
    } else {
    printf("wish> ");
    line = wish_read_line();
    args = wish_split_line(line);
    execArgs(args);
    free(line);
    free(args);
   }
 } while (1);
}

// pääohjelma
int main(int argc, char *argv[]){
  char str[80] = "";
  char txt[80] = ".txt";
  FILE* tiedosto;
  char rivi[80];
  int i;
  if(argc>1){
    if(strstr(argv[1], txt) != NULL){
      if ((tiedosto = fopen(argv[1], "r")) == NULL){
        fprintf(stderr, error_message, strlen(error_message));
        exit(1);
      }
      while (fgets(rivi, 80, tiedosto) !=NULL){
        strcat(str, rivi);
      }
      fclose(tiedosto);
    } else {
      for(i=1; i<argc; i++){
        strcat(str, argv[i]);
        strcat(str, " ");
      }
    }
    wish_loop(str);
  } else {
    wish_loop(NULL);
  }
  return 0;
}
