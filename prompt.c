#include<stdio.h>
#include<stdlib.h>
#include "mpc.h"

typedef struct {
	int type;
	long num;
	char* err;
	char* sym;
	int count;
	struct lval** cell;
} lval;

enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};
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
//Create functions
lval* lval_num(long x){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}

lval* lval_err(char* m){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m)+1);
	strcpy(v->err, m);
	return v;
}

lval* lval_sym(char* s){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->err = malloc(strlen(s)+1);
	strcpy(v->sym, m);
	return v;
}


lval* lval_sexpr(void){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;	
	return v;
}

// Delete functions
void lval_del(lval* v) {
	
	switch(v->type){
		case LVAL_NUM: break;
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;
		case LVAL_SEXPR :
			for(int i =0 ; i<= v->count; i++){
				lval_del(v->cell[i]);
				}
			free(v->cell);
			break;
		}
	free(v);
}

//Read functions
lval* lval_read_num(mpc_ast_t* t){
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno !=ERANGE ? lval_num(x) : lval_err("Invalid Number");
}
lval* lval_read(mpc_ast_t* t){
	
	if (strstr(t->tag, "number")) { return lval_read_num(t);}
	if (strstr(t->tag, "symbol")) { return lval_sym(t->contents);}

	lval* x = NULL;
	
	if (strcmp(t->tag, "<")) {  x =lval_sexpr();}
	if (strcmp(t->tag, "sexpr")) {x = lval_sexpr();}

	for(int i = 0; i<t->children_num; i++){
		
		if(!strcmp(t->children[i]->contents, "(")) {continue;}
		if(!strcmp(t->children[i]->contents, ")")) {continue;}
		if(!strcmp(t->children[i]->contents, "regex")) {continue;}
		x = lval_add(x, lval_read(t->children[i]));
		}
	}

lval* lval_add(lval* v, lval* x){
	v->count++;
	v->cell= realloc(v->cell, sizeof(lval *)* v->count);
	v->cell[v->count - 1] = x;
	return v;
	}

void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close){
	putchar(open);
	
	for(int i =0; i< v->count; i++){
		lval_print(v->cell[i]);
	}
	
	if(i !=(v->count -1)){
		putchar(' ');
	}
	
	putchar(close);

}

void lval_print(lval* v){
	switch(v->type){
		case LVAL_NUM: printf("%li", v->num);break;
		case LVAL_ERR: printf("Error : %s", v->err); break;
		case LVAL_SYM: printf("%s", v->sym); break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break; 
		default: printf("Lval type invalid!!!");
		}
}

void lval_println(lval v){
	lval_print(v); 
	putchar('\n');
}

lval* lval_pop(lval* v, int i){
	lval* x = v->cell[i];
	memmove(&v->cell[i], v->cell[i+1], sizeof(lval*)*(v->count-i-1));
	v->count--;
	v->cell = realloc(v->cell, sizeof(lval*)*v->count);
	return x;
	}

lval* lval_take(lval* v, int i){
	lval* x = lval_pop(v, i);
	lval_del(v);
	return x;
	}


lval* builtin_op(lval* v, char* op){
	
	lval* x = lval_pop(v, 0);
	if(!strcmp(op, "-") && v->count ==0 )
		if(v->type==LVAL_NUM){
			x->num = -x->num;
		}
		else{
			return lval_err("Can't operate on a non-number");
		}
	while(v->count >0){
		lval* y = lval_pop(v,0);
		if (!strcmp(op, "+")) x->num += y->num;
		if (!strcmp(op, "-")) x->num -= y->num;
		if (!strcmp(op, "*")) x->num *= y->num;
		if (!strcmp(op, "/")) {
			if (y->num == 0) {
				lval_del(x);
				lval_del(y);
				x = lval_err("Division by zero undefined");
				break;
				}
			else{
				x->num /=y->num;
				}
			}
		lval_del(y);
		}
	lval_del(v);
	return x;
	
	}

lval* lval_eval_sexpr(lval* v){
	for ( int i = 0; i<v->count; i++){
		v->cell[i] = lval_eval(v->cell[i]);
		}
	for(int i = 0; i<v->count; i++){
		if(v->cell[i]->type = LVAL_ERR){
			return lval_take(v,i);
			}
		}
	if(v->count == 0){
		return v;
		}
	
	if(v->count == 1){
		return lval_take(v, 0);
		}

	lval* f  = lval_pop(v, 0);
	if (f->type != LVAL_SYM){
		lval_del(f);
		lval_del(v);
		return lval_err("S-expr doesn't start with symbol");
		}
	lval* result = builtin_op(v, f->sym);
	lval_del(f);
	return result;

	}


int main(int argv, char **argc){

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_result_t r;

	// Initialize grammar
	mpca_lang(MPCA_LANG_DEFAULT,
			"					\
			number : /-?[0-9]+/ ;			\
			symbol : '+' | '-' | '/' | '*' ;	\
			sexpr : '(' <expr?* ')' ;		\
			expr : <number> | <symbol> | <sexpr> ; 	\ 
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

	mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
	return 0;
}

