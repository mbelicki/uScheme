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
	env->atoms  = (char **)     malloc(CHUNK * sizeof(char *));
	env->values = (LispList **) malloc(CHUNK * sizeof(LispList *)); 
}

extern int set_var(Environment *env, char *name, LispList *value)
{
	int i;
	
	/* if evn not empty search for name : */
	for (i = 0; i < env->used_size; i++)
		if (strcmp(name, env->atoms[i]) == 0)
			return (int)(env->values[i] = value); /* if I could I would make it even more compact :(  */
	
	/* no name found : */

	/* get more memmory if we are running out of free space : */
	if (env->size <= env->used_size)
	{
		env->size  += CHUNK; /* TODO: hey, what about some allocation result handling? */
		env->atoms  = (char **)    realloc(env->atoms,  env->size * sizeof(char *));
		env->values = (LispList **)realloc(env->values, env->size * sizeof(LispList *));
	}

	/* set new variable : */
	i = strlen(name);
	env->atoms[env->used_size] = (char *)malloc(i * sizeof(char));
	strcpy(env->atoms[env->used_size], name);
	env->values[env->used_size] = value; /* TODO: consider realloctaion */

	env->used_size++;

	return (int)(value);
}

extern LispList *get_var(Environment *env, char *name)
{
	int i;
	for (i = 0; i < env->used_size; i++)
		if (strcmp(name, env->atoms[i]) == 0)
			return env->values[i];
	/* value not found, sorry */
	return NULL;
}
