/* types.h */

#pragma once

/* forward declarations : */
union LispValue_;
struct LispProcedure_;
struct LispList_;

typedef enum ValueType_
{
	ATOM, LIST, INTEGER, STRING, BOOLEAN, PROCEDURE, END_OF_LIST
}
ValueType;

/*
	either :
		atom
		list
		dottedlist
		number
		string
		boolean
*/
typedef union LispValue_ 
{
	char			*atom;   /* string of extended alfanumeric characters */
	struct LispList_	*list;   /* enclosed by ( and ) */
	int			integer; /* string of digits */
	char			*string; /* string enclosed by " */
	int			boolean; /* #t or #f */
	struct LispProcedure_	*procedure;
} 
LispValue;

typedef struct LispProcedure_
{
	struct LispList_	*formals;
	struct LispList_	*expression;
}
LispProcedure;

typedef struct LispList_
{
	union LispValue_ 	here;
	enum ValueType_		type;
	struct LispList_  	*tail;
}
LispList;
