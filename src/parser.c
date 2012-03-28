#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "parser.h"

#define DELIMITERS " \n\t\r"
#define CHUNK 16

static char **tokenize(char *source, int *count);
static char *handle_parentheses(char *source);
static size_t count_chars(char *str, char c);
static LispList *interprete_tokens(char **tokens, int *index);
static int is_an_int(char *str);


extern LispList* parse(char* source)
{
	char **tokens;
	char *temp;
	int index = 0;
	int tcount = 0;
	LispList *parsed;

	temp = handle_parentheses(source);
	assert(temp != NULL);
	
	tokens = tokenize(temp, &tcount);
	assert(temp != NULL);
	
	parsed = interprete_tokens(tokens, &index);

	free(tokens);
	//free(temp); omg :/

	return parsed;
}

static LispList *interprete_tokens(char **tokens, int *index)
{
	LispList *root;
	LispList *top;

	root = (LispList *)malloc(sizeof(LispList));
	top = root;

	while (tokens[*index] != NULL && tokens[*index][0] != ')')
	{
		if (tokens[*index][0] == '(') /* list */
		{
			LispList *expr;
			(*index)++;

			expr = interprete_tokens(tokens, index);
			
			top->here.raw.list = expr;
			top->here.type = LIST;
		}
		else if (is_an_int(tokens[*index])) /* number */
		{
			top->here.raw.integer = atoi(tokens[*index]);
			top->here.type = INTEGER;
			(*index)++;
		}
		else if (tokens[*index][0] == '"') /* string */
		{
			size_t stlen;
			char *string;
			
			stlen = strlen(tokens[*index]) - 1; /* don't include " in len */
			string = (char *)malloc(stlen * sizeof(char));
			tokens[*index][stlen] = '\0';
			strcpy(string, tokens[*index] + 1);

			top->here.raw.string = string;
			top->here.type = STRING;
			(*index)++;
		}
		else if (strcmp(tokens[*index], "#t") == 0) /* true */
		{
			top->here.raw.boolean = 1;
			top->here.type = BOOLEAN;
			(*index)++;
		}
		else if (strcmp(tokens[*index], "#f") == 0) /* false */
		{
			top->here.raw.boolean = 0;
			top->here.type = BOOLEAN;
			(*index)++;
		}
		else /* it must be an atom then! */
		{
			size_t atlen;
			char *atom;

			atlen = strlen(tokens[*index]);
			atom = (char *)malloc(atlen * sizeof(char));
			strcpy(atom, tokens[*index]);

			top->here.raw.atom = atom;
			top->here.type = ATOM;
			(*index)++;
		}

		top->tail = (LispList *)malloc(sizeof(LispList));
		top = top->tail;
		top->here.type = END_OF_LIST;
		top->tail = NULL;
	}
	(*index)++;

	return root;
}

static int is_a_digit(char c)
{
	return c >= '0' && c <= '9';
}

static int is_an_int(char *str)
{
	char *current;
	if (*str == '-' && is_a_digit(*(str + 1))) str+=2; /* first character might be - */
	for (current = str; *current != '\0'; current++)
		if (!is_a_digit(*current))
			return 0;
	
	return 1;
}

static size_t count_chars(char *str, char c)
{
	size_t count = 0;
	char *current;

	for (current = str; *current != '\0'; current++)
		if (*current == c)
			count++;

	return count; 
}

static char *handle_parentheses(char *source)
{
	size_t newlen = 0;
	int i, j;
	char *newstr;

	newlen += 2 * (count_chars(source, '(') + count_chars(source, ')'));
	newlen += strlen(source);

	newstr = (char *)malloc(newlen * sizeof(char));
	
	for (i = 0, j = 0; source[i] != '\0'; i++, j++)
	{
		newstr[j] = source[i];
		if (newstr[j] == '(' || newstr[j] == ')')
		{
			newstr[j]   = ' '; 
			newstr[++j] = source[i];
			newstr[++j] = ' ';
		}
	}
	newstr[j] = '\0';

	return newstr;	
}

static char **tokenize(char* source, int *count)
{	
	char *token;
	char **tokens;
	int tokcount = CHUNK;
	int i = 0;

	/* TODO: alloc handling */
	tokens = (char **)malloc(tokcount * sizeof(char *));
	token = strtok(source, DELIMITERS);
	if (token == NULL)
		return;

	do 
	{
		tokens[i++] = token;

		/* handle strings !*/ /* TODO: \" handling*/
		if (token[0] == '"') /* if token begins with " it must be a string */
		{ 
			for (token++; *token != '"'; token++) /* interate through characters until you find matching " */ 
				if (*token == '\0') /* on your way changes all \0 characters set by strtok back to spaces */
					*token = ' ';
			*(token + 1) = ' ';
			token = strtok(token, DELIMITERS); /* reset tokenizer state */
		}

		if (i >= tokcount)
		{
			tokcount += CHUNK;
			tokens = (char **)realloc(tokens, tokcount * sizeof(char *));
		}
		token = strtok(NULL, DELIMITERS);
	}
	while (token != NULL);
	tokens[i] = NULL;
	
	*count = i;
	return tokens;
}

