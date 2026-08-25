#ifndef EXEC_H
#define EXEC_H

#include "command.h"

void run_command(Command *cmd);
void run_pipeline(Command *head);

#endif /* EXEC_H */