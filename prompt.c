#include<stdio.h>
#include<stdlib.h>

#include<editline/readline.h>
#include<editline/history.h>

#define BUFFER_SIZE 2048
#define TRUE 1
#define FALSE 0

static char input[BUFFER_SIZE];

int main(int argv, char **argc){
	puts("Lispy version 0.0.1");
	puts("Press Ctrl+c to exit\n");

	while(TRUE){
		char * input = readline("lispy>");
		add_history(input);
		printf("You entered %s\n", input);
		free(input);
	}	
	return 0;
}
