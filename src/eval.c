#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "eval.h"

#define CHUNK 16

/* forward declarations : */

int intadd(int a, int b) {return a + b;}
int intsub(int a, int b) {return a - b;}
int intmul(int a, int b) {return a * b;}
int intdiv(int a, int b) {return a / b;}
int intmod(int a, int b) {return a % b;}

static int to_int(LispList *expr, Environment *env);
static int to_bool(LispList *expr, Environment *env);

static LispList *reduce_int(LispList *expr, int base, int (*func)(int, int), Environment *env);

static LispList *if_form(LispList *expr, Environment *env);
static LispList *set_form(LispList *expr, Environment *env);

static LispList *begin_form(LispList *expr, Environment *env);

static LispList *lambda_form(LispList *expr, Environment *env);
static LispList *eval_proc(LispList *procedure, LispList *args, Environment *env);

/* definitions : */
extern LispList *eval(LispList *root, Environment *env)
{
	if (root->here.type == LIST)
	{
		LispList *evaled = eval(root->here.raw.list, env);
		evaled->tail = root->tail;
		root = evaled;
	}
	if (root->here.type == ATOM)
	{
		char *atom = root->here.raw.atom;
		if      (strcmp(atom, "+") == 0)
			return reduce_int(root->tail, 0, intadd, env);
		else if (strcmp(atom, "-") == 0)
			return reduce_int(root->tail->tail, root->tail->here.raw.integer, intsub, env);
		else if (strcmp(atom, "*") == 0)
			return reduce_int(root->tail, 1, intmul, env);
		else if (strcmp(atom, "/") == 0)
			return reduce_int(root->tail->tail, root->tail->here.raw.integer, intdiv, env);
		else if (strcmp(atom, "modulo") == 0)
			return reduce_int(root->tail->tail, root->tail->here.raw.integer, intmod, env);
		
		else if (strcmp(atom, "if") == 0)
			return if_form(root->tail, env);
		else if (strcmp(atom, "set!") == 0)
			return set_form(root->tail, env);
		else if (strcmp(atom, "begin") == 0)
			return begin_form(root->tail, env);
		else if (strcmp(atom, "display") == 0)
			return display(root->tail, env);
		else if (strcmp(atom, "lambda") == 0)
			return lambda_form(root->tail, env);
		else
		{
			LispList* var = get_var(env, atom);
			if (var == NULL)
				return root;
			if (var->here.type == PROCEDURE)
				var = eval_proc(var, root->tail, env);
			return var;
		}
	}
	else return root;
}

static LispList *eval_proc(LispList *procedure, LispList *args, Environment *env)
{
	LispList *formals = procedure->here.raw.procedure->formals;

	while (formals->here.type != END_OF_LIST)
	{
		set_var(env, formals->here.raw.atom, args);
		formals = formals->tail;
		args = args->tail;
	}
			
	return eval(procedure->here.raw.procedure->expression, env); 
}

static LispList *lambda_form(LispList *expr, Environment *env)
{
	LispList* result    = (LispList *)malloc(sizeof(LispList));
	LispProcedure* proc = (LispProcedure *)malloc(sizeof(LispProcedure)); 
	
	result->here.type = PROCEDURE;
	result->tail = NULL; /* are you sure? */
	result->here.raw.procedure = proc;
	
	proc->formals = expr->here.raw.list;
	proc->expression = expr->tail;

	return result;
}

static LispList *set_form(LispList *expr, Environment *env)
{
	LispList *atom = expr; /* TODO: check count! */
	LispList *value = expr->tail;

	if (value->tail->here.type != END_OF_LIST)
		fprintf(stderr, "WARN -> 'set!' form contains aditional arguments (more than 3).\n        additional arguments ignored\n");

	if (atom->here.type != ATOM)
	{
		fprintf(stderr, "ERRO -> First argument must be of atom type.");
		return NULL;
	}

	set_var(env, atom->here.raw.atom, eval(value, env)); /* TODO: check if not NULL, NULL means no setting ;/ */

	return value;
}

static LispList *begin_form(LispList *expr, Environment *env)
{
	LispList *result;

	while (expr->here.type != END_OF_LIST)
	{
		result = eval(expr, env);
		expr = expr->tail;
	}

	return result;
}

static LispList *if_form(LispList *expr, Environment *env)
{
	LispList *result;
	
	/* TODO: check if not too few arguments */
	LispList *pred = expr;
	LispList *positive = pred->tail;
	LispList *negative = positive->tail; /* TODO: false is optional! */

	if (negative->tail->here.type != END_OF_LIST)
		fprintf(stderr, "WARN -> 'if' form contains additional arguments (more than 3).\n        additional arguments ignored\n");

	result = (LispList *)malloc(sizeof(LispList));
	result->tail = (LispList *)malloc(sizeof(LispList));
	result->tail->here.type = END_OF_LIST;

	if (to_bool(pred, env))
	{
		result->here = positive->here;
		//result->type = positive->type;
	}
	else
	{
		result->here = negative->here;
		//result->type = negative->type;
	}

	return result;
}

extern LispList *display(LispList *expr, Environment *env)
{
	/* TODO: NULL handling*/
	LispList *result;
	
	switch(expr->here.type)
	{
		case ATOM:        printf("%s\n", expr->here.raw.atom); break;
		case LIST:        display(eval(expr, env), env); break;
		case INTEGER:     printf("%d\n", expr->here.raw.integer); break;
		case STRING:      printf("%s\n", expr->here.raw.string); break;
		case BOOLEAN:     expr->here.raw.boolean ? printf("#t\n") : printf("#f\n"); break;
		case PROCEDURE:   printf("value is a procedure\n");break;
		case END_OF_LIST: fprintf(stderr, "ERRO -> 'display' argument missing!\n"); break;
	}

	if (expr->tail->here.type != END_OF_LIST)
		display(expr->tail, env);

	result = (LispList *)malloc(sizeof(LispList));
	result->here.raw.integer = 1;
	result->here.type = INTEGER;
	result->tail = (LispList *)malloc(sizeof(LispList));
	result->tail->here.type = END_OF_LIST;

	return result;
}

static LispList *reduce_int(LispList *expr, int base, int (*func)(int, int), Environment *env)
{
	int acc = base;
	LispList *result;

	while (expr->here.type != END_OF_LIST)
	{
		int next = to_int(expr, env);
		acc = func(acc, next);
		expr = expr->tail;
	}

	result = (LispList *)malloc(sizeof(LispList));
	result->here.raw.integer = acc;
	result->here.type = INTEGER;
	result->tail = (LispList *)malloc(sizeof(LispList));
	result->tail->here.type = END_OF_LIST;

	return result;
}

static int to_bool(LispList *expr, Environment *env)
{
	/* TODO: check complience with R5RS */
	switch (expr->here.type)
	{
		case INTEGER: /* fall thorugh */
		case BOOLEAN: return expr->here.raw.boolean ? 1 : 0;
		case ATOM:
		case LIST:    return to_bool(eval(expr, env), env);
	}

	return 1;
}

static int to_int(LispList *expr, Environment *env)
{
	switch (expr->here.type)
	{
		case INTEGER: return expr->here.raw.integer;
		case STRING:  return atoi(expr->here.raw.string);
		case ATOM:
		case LIST:    return to_int(eval(expr, env), env);
		case BOOLEAN: return expr->here.raw.boolean ? 1 : 0; 
	}
	return 0;
}

