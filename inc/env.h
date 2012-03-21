#pragma once
#include "types.h"

/* types */
typedef struct EnvSymbol_
{
	char *name;
	LispList *value;
	char flags;
}
EnvSymbol;

#define ENVVAR_MUTABLE 0x01

typedef struct Environment_
{
	EnvSymbol *symbols;
	unsigned int used_size;
	unsigned int size;
}
Environment;

/* declarations */

extern int init_env(Environment *env);

extern int set_var(Environment *env, char *name, LispList *value);

extern LispList *get_var(Environment *env, char *name);

