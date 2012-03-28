#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "parser.h"
#include "eval.h"
#include "env.h"
#include "error.h"

int main(int argc, char **argv)
{
	LispList *prog;
	LispList *result;
	Environment env;

	char code[512] = "(display \"hello, it's scheme\")";

	init_env(&env, 0);
	
	while (strcmp(code, ":q\n") != 0)
	{
		prog = parse(code);
		result = eval(prog, &env);
	
		if (!err_try())
		{
			printf("===> ");
			display(result, &env);
		}
		err_catch();
		
		if (fgets(code, 512, stdin) == NULL)
			break;
	}

	exit(0);
}
