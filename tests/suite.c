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
	init_env(&env, 4);
	
	strcpy(code, s_expr);
	
	prog = parse(code);
	result = eval(prog, &env);

	free_env(&env);

	/* TODO: comparing values might be not enough for atoms, strings and lists,
	 * all of which happen to be represented by pointers... */
	if (asserted_result.integer == result->here.raw.integer)
	{
		printf("\033[01;32m OK \033[0m  | %s\n", s_expr);
		return 1;
	}
	else
	{
		printf("\033[01;31m BAD \033[0m | %s\n      ^ returned: %d; expected: %d\n", 
			s_expr, result->here.raw.integer, asserted_result.integer);
		return 0;
	}
}

int main(int argc, char **argv)
{
	int passed = 0;
	int all    = 0;
	RawValue result;

	printf("\n--- 0. Language structures ---\n\n");

	result.integer = 9;
	passed += assert_eval("(if #t 9 11)", result); all++;

	result.integer = 11;
	passed += assert_eval("(if #f 9 11)", result); all++;

	result.integer = 5;
	passed += assert_eval("((if #t + *) 2 3)", result); all++;

	result.integer = 15;
	passed += assert_eval("(if #t 15)", result); all++;

	result.integer = 0;
	passed += assert_eval("(if #f 15)", result); all++;

	result.integer = 15;
	passed += assert_eval("(+ 2 (if \"non empty string\" 8 0) 2 3)", result); all++;

	result.integer = 2;
	passed += assert_eval("(begin (+ 2 2) (+ 1 1))", result); all++;
	
	result.integer = 5;
	passed += assert_eval("(begin (set! x 2) (+ x 3))", result); all++;

	result.integer = 5;
	passed += assert_eval("(begin (set! x 2) (set! x 3) (set! x 5) (+ x))", result); all++;
	
	/* TODO: investigate closely: */
	//result.integer = 3;
	//passed += assert_eval("(begin (set! plus +) (plus 1 2))", result); all++;

	result.integer = 33;
	passed += assert_eval("(begin (set! x 2) (set! y 3) (set! z 5) (set! w 6) (set! v 9) (set! u 8) (+ x y z w v u))", result); all++;

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
	printf("passed tests: %d ( %3.2f% )\n", passed, passed * 100.0f / all);	

	exit(0);
}
