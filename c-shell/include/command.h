#ifndef COMMAND_H
#define COMMAND_H


typedef struct Command{
    char **argv;  
    int argc;
    char *infiles[64];  // For redirection purposes
    char *outfiles[64];
    int outappend[64]; // Distinguishes < from <<
    int n_infiles;
    int n_outfiles;
    int background;  
    int piped_to_next;  // For piping processes
    struct Command *next;
}Command;

Command* build_commands(int index);  // For building commands.
void free_commands(Command *head); // For freeing commands.




#endif /*COMMAND_H*/