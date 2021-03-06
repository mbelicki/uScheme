#pragma once

#define ILLEGAL_ARGS_TYPE      1
#define ILLEGAL_ARGS_COUNT     2
#define UNDEFINED_VARRIABLE    3
#define ILLEGAL_PROCEDURE_CALL 4

/* sets error status and printf informations to stderr */
extern void err_throw(int errcode, char* message, char* explanation);
/* returns status code (true) or 0 if no error has been thrown */
extern int err_try(void);
/* sets error status to 0 (hadnled) */
extern void err_catch(void);


extern void warn_throw(char* message, char* explanation);
