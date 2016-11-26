#include<stdio.h>
#define BUFFER_SIZE 2048
#define TRUE 1
#define FALSE 0

static char input[BUFFER_SIZE];

int main(int argv, char **argc){
	puts("Lispy version 0.0.1");
	puts("Press Ctrl+c to exit");

	while(TRUE){
		fputs("lispy> ", stdout);
		fgets(input, BUFFER_SIZE, stdin);
		printf("You entered %s", input);
	}	

}
