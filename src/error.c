#include <stdio.h>
#include <stdlib.h>

#include "error.h"

static int last_error;

extern void err_throw(int errcode, char* message, char* explanation)
{
	last_error = errcode;
	fprintf(stderr, "ERROR    : %s\n", message);
	fprintf(stderr, "           %s\n", explanation);
}

extern int err_try(void)
{
	return last_error;
}

extern void err_catch(void)
{
	last_error = 0;
}


extern void warn_throw(char* message, char* explanation)
{
	fprintf(stderr, "WARNNING : %s\n", message);
	fprintf(stderr, "           %s\n", explanation);
}
