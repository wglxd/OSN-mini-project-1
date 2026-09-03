#ifndef EXEC_H
#define EXEC_H

#include "command.h"

int run_command(Command *cmd);
int run_pipeline(Command *head);

#endif /* EXEC_H */