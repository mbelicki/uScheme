#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "env.h"

#define CHUNK 16

/* definitons */

extern int init_env(Environment *env)
{
	env->used_size = 0;
	env->size  = CHUNK;
	env->symbols = (EnvSymbol *)malloc(CHUNK * sizeof(EnvSymbol));
}

extern int set_var(Environment *env, char *name, LispList *value)
{
	int i;
	
	/* if evn not empty search for name : */
	for (i = 0; i < env->used_size; i++)
		if (strcmp(name, env->symbols[i].name) == 0)
			return (int)(env->symbols[i].value = value); /* if I could I would make it even more compact :(  */
	
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

	return (int)(value);
}

extern LispList *get_var(Environment *env, char *name)
{
	int i;
	for (i = 0; i < env->used_size; i++)
		if (strcmp(name, env->symbols[i].name) == 0)
			return env->symbols[i].value;
	/* value not found, sorry */
	return NULL;
}
