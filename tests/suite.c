#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "parser.h"
#include "eval.h"

static char code[512];

/* returns 1 if passed 0 otherwise */
int assert_eval(char *s_expr, RawValue asserted_result)
{
	LispList *prog;
	LispList *result;
	Environment env;
	
	strcpy(code, s_expr);
	
	prog = parse(code);
	result = eval(prog, &env);

	/* TODO: comparing values might be not enough for atoms, strings and lists,
	 * all of which happen to be represented by pointers... */
	if (asserted_result.integer == result->here.raw.integer)
	{
		printf("OK  : %s\n", s_expr);
		return 1;
	}
	else
	{
		printf("BAD : %s\n      returned: %d; expected: %d\n", 
			s_expr, result->here.raw.integer, asserted_result.integer);
		return 0;
	}
}

int main(int argc, char **argv)
{
	int passed = 0;
	int all    = 0;
	RawValue result;

	printf("\n--- 2. numeric functions ---\n\n");
	
	result.integer = 5;
	passed += assert_eval("(+ 3 4 -2)", result); all++;
	
	result.integer = 7;
	passed += assert_eval("(+ 3 4)", result); all++;

	result.integer = 3;
	passed += assert_eval("(+ 3)", result); all++;

	result.integer = 0;
	passed += assert_eval("(+)", result); all++;

	result.integer = -24;
	passed += assert_eval("(* 3 4 -2)", result); all++;

	result.integer = 12;
	passed += assert_eval("(* 3 4)", result); all++;

	result.integer = 4;
	passed += assert_eval("(* 4)", result); all++;

	result.integer = 1;
	passed += assert_eval("(*)", result); all++;

	result.integer = -2;
	passed += assert_eval("(- 2 4)", result); all++;

	result.integer = -3;
	passed += assert_eval("(- 3)", result); all++;

	result.integer = 1;
	passed += assert_eval("(- 8 3 4)", result); all++;

	result.integer = 4;
	passed += assert_eval("(/ 8 2)", result); all++;
	
	result.integer = 0;
	passed += assert_eval("(/ 2)", result); all++;

	result.integer = 2;
	passed += assert_eval("(/ 8 2 2)", result); all++;
	
	printf("\n--- summary ---\n\n");
	printf("runned tests: %d\n", all);
	printf("passed tests: %d (%3.2f%)\n", passed, passed * 100.0f / all);	

	exit(0);
}
