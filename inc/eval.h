#include "types.h"
#include "env.h"

extern void clear_env(void);

extern LispList *eval(LispList *root, Environment *env);

extern LispList *display(LispList *expr, Environment *env);
