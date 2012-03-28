#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "env.h"

#define CHUNK 16

/* definitons */

extern void init_env(Environment *env, size_t initial_size)
{
	env->used_size = 0;
	env->size      = initial_size > 0 ? initial_size : CHUNK;
	env->symbols   = (EnvSymbol *)malloc(env->size * sizeof(EnvSymbol));
	env->parent    = NULL;
}

extern void free_env(Environment *env)
{
	free(env->symbols);
}

extern TypedValue *set_var(Environment *env, char *name, TypedValue *value)
{
	int i;
	
	/* if evn not empty search for name : */
	for (i = 0; i < env->used_size; i++)
		if (strcmp(name, env->symbols[i].name) == 0)
			return env->symbols[i].value = value; /* if I could I would make it even more compact :(  */
	
	/* no name found : */

	/* get more memmory if we are running out of free space : */
	if (env->size <= env->used_size)
	{
		env->size   += CHUNK; /* TODO: hey, what about some allocation result handling? */
		env->symbols = (EnvSymbol *) realloc(env->symbols,  env->size * sizeof(EnvSymbol));
	}

	/* set new variable : */
	i = strlen(name);
	env->symbols[env->used_size].name = (char *)malloc(i * sizeof(char));
	strcpy(env->symbols[env->used_size].name, name);
	env->symbols[env->used_size].value = value; /* TODO: consider realloctaion */

	env->used_size++;

	return value;
}

extern TypedValue *get_var(Environment *env, char *name)
{
	int i;
	for (i = 0; i < env->used_size; i++)
		if (strcmp(name, env->symbols[i].name) == 0)
			return env->symbols[i].value;
	/* value not found, try in higher scope */
	return env->parent == NULL ? NULL : get_var(env->parent, name);
}
