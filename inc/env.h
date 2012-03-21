#pragma once
#include "types.h"

/* types */

typedef struct Environment_
{
	char **atoms;
	LispList **values;
	unsigned int used_size;
	unsigned int size;
}
Environment;

/* declarations */

extern int init_env(Environment *env);

extern int set_var(Environment *env, char *name, LispList *value);

extern LispList *get_var(Environment *env, char *name);

