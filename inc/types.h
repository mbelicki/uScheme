/* types.h */

#pragma once

/* forward declarations : */
union LispValue_;
struct LispList_;
enum ValueType_;

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
} 
LispValue;

typedef enum ValueType_
{
	ATOM, LIST, INTEGER, STRING, BOOLEAN, END_OF_LIST
}
ValueType;

typedef struct LispList_
{
	union LispValue_ 	here;
	enum ValueType_		type;
	struct LispList_  	*tail;
}
LispList;
