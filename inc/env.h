#pragma once
#include "types.h"

/* types */
typedef struct EnvSymbol_
{
	char *name;
	TypedValue *value;
	char flags;
}
EnvSymbol;

#define ENVVAR_MUTABLE 0x01

typedef struct Environment_
{
	EnvSymbol           *symbols;
	struct Environment_ *parent;
	unsigned int         used_size;
	unsigned int         size;
}
Environment;

/* declarations */

extern void init_env(Environment *env, size_t initial_size);
extern void free_env(Environment *env);

extern TypedValue *set_var(Environment *env, char *name, TypedValue *value);
extern TypedValue *get_var(Environment *env, char *name);

