#include<stdio.h>
#include<stdlib.h>

#ifdef _WIN32
#include<string.h>

#define BUFFER_SIZE 2048
static char buffer[BUFFER_SIZE];

char * readline(const char * prompt){
	fputs(prompt);
	fgets(buffer, BUFFER_SIZE, stdin);
	char * ret = malloc(strlen(buffer) + 1);
	if (ret == NULL){
		printf("\nOut of memory error!\n");
		exit(1);
	}
	else {
		strcpy(ret, buffer);
		return ret;
	} 
}

void add_history(const char * unused){}

#else
#include<editline/readline.h>
#include<editline/history.h>
#endif

#define TRUE 1
#define FALSE 0


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
