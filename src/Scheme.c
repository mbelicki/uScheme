// Scheme.cpp : Defines the entry point for the console application.
//
/*
#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "parser.h"
#include "eval.h"

int main(int argc, char **argv)
{
	LispList *prog;
	LispList *result;

	//atexit(clear_env); /* I just love this feature */

	//char code[512] = "(begin (define \r\n sq \t (lambda (x) (* x x)))(sq 3))";
	//char code[512] = "(+ 2 (+ 1 1) 4 (* 4 2 3) (- 2 6) (/ 8 2))";
	char code[512] = "(display \"hello, it's scheme\")";

	
	while (strcmp(code, ":q") != 0)
	{
		prog = parse(code);
		result = eval(prog);
		
		printf("===> ");
		display(result);
		
		gets(code);
	}

	exit(0);
}
