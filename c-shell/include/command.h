#ifndef COMMAND_H
#define COMMAND_H


typedef struct Command{
    char **argv;  
    int argc;
    char *infile;  // For redirection purposes
    char *outfile;
    int append; // Distinguishes > from >>
    int background;  
    int piped_to_next;  // For piping processes
    struct Command *next;
}Command;

Command* build_commands(int index);  // For building commands.
void free_commands(Command *head); // For freeing commands.




#endif /*COMMAND_H*/