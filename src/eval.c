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

static int to_int(LispList *expr);
static int to_bool(LispList *expr);

static LispList *reduce_int(LispList *expr, int base, int (*func)(int, int));

static LispList *if_form(LispList *expr);
static LispList *set_form(LispList *expr);

static LispList *begin_form(LispList *expr);

/* exec enviorment : */

static int set_var(char *name, LispList *value);
static LispList *get_var(char *name);

static unsigned long env_used_size;
static unsigned long env_size;

static char **env_atoms;
static LispList **env_values;

/* definitions : */

static int set_var(char *name, LispList *value)
{
	int i;
	
	/* if evn not empty search for name : */
	for (i = 0; i < env_used_size; i++)
		if (strcmp(name, env_atoms[i]) == 0)
			return (int)(env_values[i] = value); /* if I could I would make it even more compact :(  */
	
	/* no name found : */

	/* get more memmory if we are running out of free space : */
	if (env_size <= env_used_size)
	{
		env_size += CHUNK; /* TODO: hey, what about some allocation result handling? */
		env_atoms = (char **)realloc(env_atoms, env_size * sizeof(char *));
		env_values = (LispList **)realloc(env_values, env_size * sizeof(LispList *));
	}

	/* set new variable : */
	i = strlen(name);
	env_atoms[env_used_size] = (char *)malloc(i * sizeof(char));
	strcpy(env_atoms[env_used_size], name);
	env_values[env_used_size] = value; /* TODO: consider realloctaion */

	env_used_size++;

	return (int)(value);
}

static LispList *get_var(char *name)
{
	int i;
	/* if evn not empty search for name : */
	for (i = 0; i < env_used_size; i++)
		if (strcmp(name, env_atoms[i]) == 0)
			return env_values[i];
	/* value not found, sorry */
	return NULL;
}

extern LispList *eval(LispList *root)
{
	if (root->type == LIST)
	{
		LispList *evaled = eval(root->here.list);
		evaled->tail = root->tail;
		root = evaled;
	}
	if (root->type == ATOM)
	{
		if      (strcmp(root->here.atom, "+") == 0)
			return reduce_int(root->tail, 0, intadd);
		else if (strcmp(root->here.atom, "-") == 0)
			return reduce_int(root->tail->tail, root->tail->here.integer, intsub);
		else if (strcmp(root->here.atom, "*") == 0)
			return reduce_int(root->tail, 1, intmul);
		else if (strcmp(root->here.atom, "/") == 0)
			return reduce_int(root->tail->tail, root->tail->here.integer, intdiv);
		else if (strcmp(root->here.atom, "modulo") == 0)
			return reduce_int(root->tail->tail, root->tail->here.integer, intmod);
		
		else if (strcmp(root->here.atom, "if") == 0)
			return if_form(root->tail);
		else if (strcmp(root->here.atom, "set!") == 0)
			return set_form(root->tail);
		else if (strcmp(root->here.atom, "begin") == 0)
			return begin_form(root->tail);
		else if (strcmp(root->here.atom, "display") == 0)
			return display(root->tail);
		else
			return get_var(root->here.atom);
	}
	else return root;
}

static LispList *set_form(LispList *expr)
{
	LispList *atom = expr; /* TODO: check count! */
	LispList *value = expr->tail;

	if (value->tail->type != END_OF_LIST)
		fprintf(stderr, "WARN -> 'set!' form contains aditional arguments (more than 3).");

	if (atom->type != ATOM)
	{
		fprintf(stderr, "ERRO -> First argument must be of atom type.");
		return NULL;
	}

	set_var(atom->here.atom, eval(value)); /* TODO: check if not NULL, NULL means no setting ;/ */

	return value;
}

static LispList *begin_form(LispList *expr)
{
	LispList *result;

	while (expr->type != END_OF_LIST)
	{
		result = eval(expr);
		expr = expr->tail;
	}

	return result;
}

static LispList *if_form(LispList *expr)
{
	LispList *result;
	
	/* TODO: check if not too few arguments */
	LispList *pred = expr;
	LispList *positive = pred->tail;
	LispList *negative = positive->tail; /* TODO: false is optional! */

	if (negative->tail->type != END_OF_LIST)
		fprintf(stderr, "WARN -> 'if' form contains aditional arguments (more than 3).");

	result = (LispList *)malloc(sizeof(LispList));
	result->tail = (LispList *)malloc(sizeof(LispList));
	result->tail->type = END_OF_LIST;

	if (to_bool(pred))
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

extern LispList *display(LispList *expr)
{
	/* TODO: NULL handling*/
	LispList *result;
	
	switch(expr->type)
	{
	case ATOM:			//fprintf(stderr, "ERRO -> Sorry, varialbe: %s not found :(.\n", expr->here.atom); break;
	case LIST:			display(eval(expr)); break;
	case INTEGER:		printf("%d\n", expr->here.integer); break;
	case STRING:		printf("%s\n", expr->here.string); break;
	case BOOLEAN:		expr->here.boolean ? printf("#t\n") : printf("#f\n"); break;
	case END_OF_LIST:	fprintf(stderr, "ERRO -> 'display' argument missing!\n"); break;
	}

	if (expr->tail->type != END_OF_LIST)
		fprintf(stderr, "WARN -> Only one argument 'display' variant supported.\n");

	result = (LispList *)malloc(sizeof(LispList));
	result->here.integer = 1;
	result->type = INTEGER;
	result->tail = (LispList *)malloc(sizeof(LispList));
	result->tail->type = END_OF_LIST;

	return result;
}

static LispList *reduce_int(LispList *expr, int base, int (*func)(int, int))
{
	int acc = base;
	LispList *result;

	while (expr->type != END_OF_LIST)
	{
		int next = to_int(expr);
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

static int to_bool(LispList *expr)
{
	switch (expr->type)
	{
	case INTEGER: /* fall thorugh */
	case BOOLEAN: return expr->here.boolean ? 1 : 0;
	case ATOM:
	case LIST:	  return to_bool(eval(expr));
	}

	return 0;
}

static int to_int(LispList *expr)
{
	switch (expr->type)
	{
	case INTEGER: return expr->here.integer;
	case STRING:  return atoi(expr->here.string);
	case ATOM:
	case LIST:	  return to_int(eval(expr));
	case BOOLEAN: return expr->here.boolean ? 1 : 0; /* chek what R5RS says */
	}

	return 0;
}

/* TODO: benchmark! */
/*
static int sum_i(LispList *expr)
{
	int acc = 0;

	while (expr->type != END_OF_LIST)
	{
		switch(expr->type)
		{
		case INTEGER:
			acc += expr->here.integer;
			break;

		case LIST:
			acc += eval(expr->here.list);
			break;
		}

		expr = expr->tail;
	}

	return acc;
}

static int sub_i(LispList *expr)
{
	int acc = expr->here.integer;
	expr = expr->tail;

	while (expr->type != END_OF_LIST)
	{
		switch(expr->type)
		{
		case INTEGER:
			acc -= expr->here.integer;
			break;

		case LIST:
			acc -= eval(expr->here.list);
			break;
		}

		expr = expr->tail;
	}

	return acc;
}

static int mul_i(LispList *expr)
{
	int acc = 1;

	while (expr->type != END_OF_LIST)
	{
		switch(expr->type)
		{
		case INTEGER:
			acc *= expr->here.integer;
			break;

		case LIST:
			acc *= eval(expr->here.list);
			break;
		}

		expr = expr->tail;
	}

	return acc;
}
*/
/* old recursive versions: */
/*static int sum(LispList *list)
{
	if (list->type == END_OF_LIST)
		return 0;
	else if (list->type == LIST)
		return eval(list->here.list) + sum(list->tail);
	else if (list->type == INTEGER)
		return list->here.integer + sum(list->tail);
}*/

/*static int mul(LispList *list)
{
	if (list->type == END_OF_LIST)
		return 1;
	else if (list->type == LIST)
		return eval(list->here.list) * mul(list->tail);
	else if (list->type == INTEGER)
		return list->here.integer * mul(list->tail);
}*/