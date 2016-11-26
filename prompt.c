#include<stdio.h>
#include<stdlib.h>
#include "mpc.h"

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

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy= mpc_new("lispy");
	mpc_result_t r;

	// Initialize grammar
	mpca_lang(MPCA_LANG_DEFAULT,
			"					\
			number : /-?[0-9]+/ ;			\
			operator : '+' | '-' | '/' | '*' ;	\
			expr : <number> | '(' <operator> <expr>+ ')' ;\
			lispy : /^/ <operator> <expr>+ /$/ ;	\
			",
			Number, Operator, Expr, Lispy);

	// Version prompt
	puts("Lispy version 0.0.1");
	puts("Press Ctrl+c to exit\n");

	while(TRUE){
		char * input = readline("lispy>");
		add_history(input);
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			mpc_ast_print(r.output);
			mpc_ast_delete(r.output);
		} else{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}

	mpc_cleanup(4, Number, Operator, Expr, Lispy);
	return 0;
}
