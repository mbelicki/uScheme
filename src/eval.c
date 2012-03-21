#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "eval.h"

#define CHUNK 16

/* forward declarations : */

/* inlined versions: */
/*
static int sum_i(LispList *list);
static int sub_i(LispList *list);
static int mul_i(LispList *list);
*/

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

/* definitions : */
extern LispList *eval(LispList *root, Environment *env)
{
	if (root->type == LIST)
	{
		LispList *evaled = eval(root->here.list, env);
		evaled->tail = root->tail;
		root = evaled;
	}
	if (root->type == ATOM)
	{
		if      (strcmp(root->here.atom, "+") == 0)
			return reduce_int(root->tail, 0, intadd, env);
		else if (strcmp(root->here.atom, "-") == 0)
			return reduce_int(root->tail->tail, root->tail->here.integer, intsub, env);
		else if (strcmp(root->here.atom, "*") == 0)
			return reduce_int(root->tail, 1, intmul, env);
		else if (strcmp(root->here.atom, "/") == 0)
			return reduce_int(root->tail->tail, root->tail->here.integer, intdiv, env);
		else if (strcmp(root->here.atom, "modulo") == 0)
			return reduce_int(root->tail->tail, root->tail->here.integer, intmod, env);
		
		else if (strcmp(root->here.atom, "if") == 0)
			return if_form(root->tail, env);
		else if (strcmp(root->here.atom, "set!") == 0)
			return set_form(root->tail, env);
		else if (strcmp(root->here.atom, "begin") == 0)
			return begin_form(root->tail, env);
		else if (strcmp(root->here.atom, "display") == 0)
			return display(root->tail, env);
		else
			return get_var(env, root->here.atom);
	}
	else return root;
}

static LispList *set_form(LispList *expr, Environment *env)
{
	LispList *atom = expr; /* TODO: check count! */
	LispList *value = expr->tail;

	if (value->tail->type != END_OF_LIST)
		fprintf(stderr, "WARN -> 'set!' form contains aditional arguments (more than 3).\n        additional arguments ignored\n");

	if (atom->type != ATOM)
	{
		fprintf(stderr, "ERRO -> First argument must be of atom type.");
		return NULL;
	}

	set_var(env, atom->here.atom, eval(value, env)); /* TODO: check if not NULL, NULL means no setting ;/ */

	return value;
}

static LispList *begin_form(LispList *expr, Environment *env)
{
	LispList *result;

	while (expr->type != END_OF_LIST)
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

	if (negative->tail->type != END_OF_LIST)
		fprintf(stderr, "WARN -> 'if' form contains additional arguments (more than 3).\n        additional arguments ignored\n");

	result = (LispList *)malloc(sizeof(LispList));
	result->tail = (LispList *)malloc(sizeof(LispList));
	result->tail->type = END_OF_LIST;

	if (to_bool(pred, env))
	{
		result->here = positive->here;
		result->type = positive->type;
	}
	else
	{
		result->here = negative->here;
		result->type = negative->type;
	}

	return result;
}

extern LispList *display(LispList *expr, Environment *env)
{
	/* TODO: NULL handling*/
	LispList *result;
	
	switch(expr->type)
	{
		case ATOM:        //fprintf(stderr, "ERRO -> Sorry, varialbe: %s not found :(.\n", expr->here.atom); break;
		case LIST:        display(eval(expr, env), env); break;
		case INTEGER:     printf("%d\n", expr->here.integer); break;
		case STRING:      printf("%s\n", expr->here.string); break;
		case BOOLEAN:     expr->here.boolean ? printf("#t\n") : printf("#f\n"); break;
		case END_OF_LIST: fprintf(stderr, "ERRO -> 'display' argument missing!\n"); break;
	}

	if (expr->tail->type != END_OF_LIST)
		display(expr->tail, env);
		//fprintf(stderr, "WARN -> Only one argument 'display' variant supported.\n");

	result = (LispList *)malloc(sizeof(LispList));
	result->here.integer = 1;
	result->type = INTEGER;
	result->tail = (LispList *)malloc(sizeof(LispList));
	result->tail->type = END_OF_LIST;

	return result;
}

static LispList *reduce_int(LispList *expr, int base, int (*func)(int, int), Environment *env)
{
	int acc = base;
	LispList *result;

	while (expr->type != END_OF_LIST)
	{
		int next = to_int(expr, env);
		acc = func(acc, next);
		expr = expr->tail;
	}

	result = (LispList *)malloc(sizeof(LispList));
	result->here.integer = acc;
	result->type = INTEGER;
	result->tail = (LispList *)malloc(sizeof(LispList));
	result->tail->type = END_OF_LIST;

	return result;
}

static int to_bool(LispList *expr, Environment *env)
{
	switch (expr->type)
	{
		case INTEGER: /* fall thorugh */
		case BOOLEAN: return expr->here.boolean ? 1 : 0;
		case ATOM:
		case LIST:    return to_bool(eval(expr, expr), expr);
	}

	return 0;
}

static int to_int(LispList *expr, Environment *env)
{
	/* TODO: check complience with R5RS */
	switch (expr->type)
	{
		case INTEGER: return expr->here.integer;
		case STRING:  return atoi(expr->here.string);
		case ATOM:
		case LIST:    return to_int(eval(expr, env), env);
		case BOOLEAN: return expr->here.boolean ? 1 : 0; 
	}
	return 0;
}

