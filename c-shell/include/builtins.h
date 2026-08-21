#ifndef BUILTINS_H
#define BUILTINS_H


#include "command.h"

int builtin_locate(Command *cmd);
int builtin_hop(Command *cmd);
void frec_load(void);
int builtin_reveal(Command *cmd);

#endif

