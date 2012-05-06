#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "parser.h"
#include "eval.h"
#include "env.h"
#include "error.h"

static void interactive();
static void execute_file();

int main(int argc, char **argv)
{
	//if (argc > 1)
	//	execute_file(argv[1]);
	//else
	//	interactive();
	execute_file("samples\\add.scm");
}

static void interactive()
{
	LispList *prog;
	LispList *result;
	Environment env;

	char code[512] = "(display \"hello, schematic world\")";

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

static void execute_file(char *filename)
{
	LispList *prog;
	LispList *result;
	Environment env;

	char code[2048];

	FILE *sourcefile;

	sourcefile = fopen(filename, "r");
	if (sourcefile == NULL)
		exit(EXIT_FAILURE);
	fread(code, 1, 2048, sourcefile);
	fclose(sourcefile);

	init_env(&env, 0);

	prog = parse(code);
	result = eval(prog, &env);

	system("pause");
	exit(0);
}
