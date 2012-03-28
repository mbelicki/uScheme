/* types.h */
#pragma once

/* forward declarations : */
union RawValue_;
struct TypedValue_;
struct LispProcedure_;
struct LispList_;

typedef enum ValueType_
{
	ATOM, LIST, INTEGER, STRING, BOOLEAN, PROCEDURE, END_OF_LIST
}
ValueType;

typedef union RawValue_ 
{
	char                  *atom;   /* string of extended alfanumeric characters */
	struct LispList_      *list;   /* enclosed by ( and ) */
	int                    integer; /* string of digits */
	char                  *string; /* string enclosed by " */
	int                    boolean; /* #t or #f */
	struct LispProcedure_ *procedure;
} 
RawValue;

typedef struct LispProcedure_
{
	struct LispList_ *formals;
	struct LispList_ *expression;
}
LispProcedure;

typedef struct TypedValue_
{
	union RawValue_ raw;
	enum ValueType_	type;
}
TypedValue;

typedef struct LispList_
{
	struct TypedValue_  here;
	struct LispList_   *tail;
}
LispList;
