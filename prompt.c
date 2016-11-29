#include<stdio.h>
#include<stdlib.h>
#include "mpc.h"

typedef struct {
	int type;
	long num;
	int err;
} lval;

enum {LVAL_NUM, LVAL_ERR};
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};


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

lval lval_num(long x){
	lval v;
	v.type = LVAL_NUM;
	v.num = x;
	v.err = -1;
	return v;
}

lval lval_err(int x){
	lval v;
	v.type = LVAL_ERR;
	v.err = x;
	v.num = 0;
	return v;
}

void lval_print(lval v){
	switch(v.type){
		case LVAL_NUM: printf("%li", v.num);break;
		case LVAL_ERR:
		switch(v.err){
			case LERR_DIV_ZERO	: printf("Error: Division by Zero!"); break;
			case LERR_BAD_OP	: printf("Error: Invalid operator!"); break;
			case LERR_BAD_NUM	: printf("Error: Invalid Number!"); break;
			default 		: printf("Unknown Error!"); break;
			}
			break;
		default: printf("Lval type invalid!!!");
		}
}

void lval_println(lval v){
	lval_print(v); 
	putchar('\n');
}

lval eval_operator(const lval x, const lval y, const char * op){
	
	if(x.type == LVAL_ERR){return x;}
	if(y.type == LVAL_ERR){return y;}
	long a1 = x.num;
	long a2 = y.num;
	if(!strcmp(op, "+")) {return lval_num(a1+a2);}
	if(!strcmp(op, "-")) {return lval_num(a1-a2);}
	if(!strcmp(op, "*")) {return lval_num(a1*a2);}
	if(!strcmp(op, "/")) {
		if (a2 == 0 )
			return lval_err(LERR_DIV_ZERO);
		else
			return lval_num(a1/a2);
		}
	return lval_err(LERR_BAD_OP);;
}

lval eval(mpc_ast_t* input){

	if (strstr(input->tag, "number")) {
		errno = 0;
		long x = strtol(input->contents, NULL, 10);
		return (errno != ERANGE) ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}
	lval ret = eval(input->children[2]);
	int i = 3;
	char * operator = input->children[1]->contents;
	while(strstr(input->children[i]->tag, "expr")) {
		ret = eval_operator(ret, eval(input->children[i]), operator);
		i++;
	}
	return ret;	
}	

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
	puts("Lispy version 0.0.2");
	puts("Press Ctrl+c to exit\n");

	while(TRUE){
		char * input = readline("lispy>");
		add_history(input);
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			lval result = eval(r.output);
			lval_println(result);
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

