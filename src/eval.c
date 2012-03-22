#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "eval.h"
#include "error.h"

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

static LispList *eval_proc(LispList *func, LispList *args, Environment *env)
{
	LispList *formals = func->here.raw.procedure->formals;

	Environment *local_env = (Environment *)malloc(sizeof(Environment));
	init_env(local_env, 8);
	local_env->parent = env;

	while (formals->here.type != END_OF_LIST)
	{
		set_var(local_env, formals->here.raw.atom, args);
		formals = formals->tail;
		args = args->tail;
	}
			
	LispList *result = eval(func->here.raw.procedure->expression, local_env);
	free_env(local_env);
	free(local_env);
	return result;
}

static LispList *lambda_form(LispList *expr, Environment *env)
{
	LispList *formals;
	LispList *expression;
	/* first argument presence and type: */
	formals = expr;
	if (formals == NULL || formals->here.type == END_OF_LIST)
	{
		err_throw(ILLEGAL_ARGS_COUNT, 
			  "Mising first argument of `lambda'.", 
			  "First argument of `lambda' is mandatory, expected list of atoms.");
		return NULL;
	}
	if (formals->here.type != LIST)
	{
		err_throw(ILLEGAL_ARGS_TYPE, 
			  "Illegal first argument of `lambda'.", 
			  "Expected list of atoms.");
		return NULL;
	}
	/* presence of second argument */
	expression = formals->tail;
	if (expression == NULL || expression->here.type == END_OF_LIST)
	{
		err_throw(ILLEGAL_ARGS_COUNT, 
			  "Mising second argument of `lambda'.", 
			  "Second argument of `lambda' is mandatory.");
		return NULL;
	}
	/* allocate result */	
	LispList* result    = (LispList *)malloc(sizeof(LispList));
	LispProcedure* proc = (LispProcedure *)malloc(sizeof(LispProcedure)); 
	
	result->here.type = PROCEDURE;
	result->tail = NULL;
	result->here.raw.procedure = proc;
	
	proc->formals = expr->here.raw.list;
	proc->expression = expr->tail;

	return result;
}

static LispList *set_form(LispList *expr, Environment *env)
{
	LispList *atom;
	LispList *value;
	/* check arguments count, and type of first argument */
	atom = expr;
	if (atom == NULL || atom->here.type == END_OF_LIST)
	{
		err_throw(ILLEGAL_ARGS_COUNT, 
			  "Mising first argument of `set!'.", 
			  "First argument of `set!' is mandatory, expected atom.");
		return NULL;
	}
	if (atom->here.type != ATOM)
	{
		err_throw(ILLEGAL_ARGS_TYPE, 
			  "Illegal first argument of `set!'.", 
			  "Expected atom.");
		return NULL;
	}
	/* check presence of second argument */
	value = atom->tail;
	if (value == NULL || value->here.type == END_OF_LIST)
	{
		err_throw(ILLEGAL_ARGS_COUNT, 
			  "Mising second argument of `set!'.", 
			  "second argument of `set!' is mandatory.");
		return NULL;

	}
	/* are there any bonus arguments? */
	if (value->tail != NULL || value->tail->here.type != END_OF_LIST)
	{
		warn_throw("`set!' form contains additional arguments.", 
		 	   "Arguments following second argument will be ignored.");
	}
	value = eval(value, env);
	if (err_try())
		return NULL;	
	set_var(env, atom->here.raw.atom, value);
	return value;
}

static LispList *begin_form(LispList *expr, Environment *env)
{
	LispList *result;

	while (expr != NULL && expr->here.type != END_OF_LIST)
	{
		result = eval(expr, env);
		if (err_try())
			return NULL;
		expr = expr->tail;
	}
	return result;
}

static LispList *if_form(LispList *expr, Environment *env)
{
	LispList *result;
	LispList *pred;
	LispList *positive;
	LispList *negative;
	int pred_val = 0;
	/* check arguments count, first argument: */
	pred = expr;
	if (pred == NULL || pred->here.type == END_OF_LIST)
	{
		err_throw(ILLEGAL_ARGS_COUNT, 
			  "Missing first argument of `if'", 
			  "First argument of `if' is mandatory");
		return NULL;
	}
	/* second argument: */
	positive = pred->tail;
	if (positive == NULL || positive->here.type == END_OF_LIST)
	{
		err_throw(ILLEGAL_ARGS_COUNT, 
			  "Missing second argument of `if'", 
			  "Second argument of `if' is mandatory");
		return NULL;
	}
	/* third argument: */
	negative = positive->tail;
	if (negative != NULL && negative->here.type == END_OF_LIST)
		negative = NULL;
	/* check for aditional arguments */
	if (negative != NULL && negative->tail->here.type != END_OF_LIST)
		warn_throw("`if' form has additional arguments", 
			   "Arguments following third argument will be ignored.");
	/* evaluate predicate */
	pred_val = to_bool(pred, env);
	if (err_try())
		return NULL;
	/* allocate result */
	result = (LispList *)malloc(sizeof(LispList));
	result->tail = (LispList *)malloc(sizeof(LispList));
	result->tail->here.type = END_OF_LIST;
	
	if ( pred_val )
	{
		result->here = positive->here;
	}
	else
	{
		if (negative == NULL)
		{
			result->here.type = BOOLEAN;
			result->here.raw.boolean = 0;
		}
		else
		{
			result->here = negative->here;
		}
	}
	return result;
}

extern LispList *display(LispList *expr, Environment *env)
{
	LispList *result;

	switch(expr->here.type)
	{
		case ATOM:
		case LIST:        
			display(eval(expr, env), env); 
			break;
		case INTEGER:     
			printf("%d\n", expr->here.raw.integer); 
			break;
		case STRING:      
			printf("%s\n", expr->here.raw.string); 
			break;
		case BOOLEAN:     
			expr->here.raw.boolean ? printf("#t\n") : printf("#f\n"); 
			break;
		case PROCEDURE:   
			printf("value is a procedure\n");
			break;
		case END_OF_LIST: 
			err_throw(ILLEGAL_ARGS_COUNT, 
			          "Missing arguments of `display'.", 
				  "Expected at least one argument.");
			break;
	}
	/* check if there was error in any of evaluation above */
	if (err_try())
		return NULL;
	/* continue checking if there was no error */
	if (expr->tail->here.type != END_OF_LIST)
		display(expr->tail, env);
	/* check if there was error in recursive display */
	if (err_try())
		return NULL;
	/* return 1 if display succeeded */
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
		/* exit if there was an error during conversion */
		if (err_try())
			return NULL;
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
	LispList *temp;
	/* TODO: check complience with R5RS */
	switch (expr->here.type)
	{
		case INTEGER:
		case BOOLEAN: 
			return expr->here.raw.boolean ? 1 : 0;
		case ATOM:
		case LIST:    
			temp = eval(expr, env);
			if (err_try())
				return 0;
			return to_bool(temp, env);
		default:
			return 1;
	}
}

static int to_int(LispList *expr, Environment *env)
{
	LispList *temp;
	switch (expr->here.type)
	{
		case BOOLEAN:
			return expr->here.raw.boolean ? 1 : 0; 
		case INTEGER: 
			return expr->here.raw.integer;
		case STRING:  
			return atoi(expr->here.raw.string);
		case ATOM:
		case LIST:    
			temp = eval(expr, env);
			if (err_try())
				return 0;
			return to_int(temp, env);
		default:
			return 0;
	}
}

