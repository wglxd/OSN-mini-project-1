#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>
#include "prompt.h"
#include "lexer.h"
#include "parser.h"
#include "command.h"


// Command initializer helper function only.
static Command *new_command(void){
    Command *head = (Command*)(malloc(sizeof(Command)));
    head->argc = 0;
    head->argv = (char**)(malloc(256*sizeof(char*)));
    head->argv[0] = NULL;
    head->n_infiles = 0;
    head->n_outfiles = 0;
    head->piped_to_next = 0;
    head->background = 0;
    head->next = NULL;
    return head;
}

Command *build_commands(int index){
    if(tokens[index] == NULL){  // The line we got was completely empty
        return NULL;
    }
    Command *head = new_command();
    int i = index;
    int append_mode = 0;
    int pending = 0;
    while(tokens[i] != NULL){
        if(tok_is_op[i]==0){  // If the token is not an operator, then we must add it to arguments of the command
            if(pending == 1){
                head->infiles[head->n_infiles] = strdup(tokens[i]);
                head->n_infiles++;
                pending = 0;
            }else if(pending == 2){
                head->outfiles[head->n_outfiles] = strdup(tokens[i]);
                head->outappend[head->n_outfiles] = append_mode;
                head->n_outfiles++;
                pending = 0;
            }else{
                head->argv[head->argc] = strdup(tokens[i]);
                head->argc++;
            }
            i++;
        }else{
            if(strcmp(tokens[i], "|")==0){
                head->piped_to_next=1;
                head->next = build_commands(++i);
                break;
            }else if(strcmp(tokens[i], ">")==0){
                pending = 2;
                append_mode = 0;
                i++;
            }else if(strcmp(tokens[i], ">>")==0){
                pending = 2;
                append_mode = 1;
                i++;
            }else if(strcmp(tokens[i], "&")==0){
                head->background = 1;
                head->next = build_commands(i+1);
                break;
            }else if(strcmp(tokens[i], ";")==0){
                head->next = build_commands(i+1);
                break;
            }else if(strcmp(tokens[i], "<")==0){
                pending = 1;
                i++;
            }
        }
    }
    head->argv[head->argc] = NULL;
    return head;
}

// Free the command linked list
void free_commands(Command *head){
    if(head==NULL) return;

    Command *temp = head;
    while(head->next != NULL){
        head = head->next;
        for(int i = 0;i<temp->argc;i++){
            free(temp->argv[i]);
        }
        free(temp->argv);
        for(int i = 0;i<temp->n_infiles;i++){
            free(temp->infiles[i]);
        }
        for(int i = 0;i<temp->n_outfiles;i++){
            free(temp->outfiles[i]);
        }
        free(temp);
        temp = head;
    }

    for(int i = 0;i<head->argc;i++){
        free(head->argv[i]);
    }
    free(head->argv);
        for(int i = 0;i<temp->n_infiles;i++){
            free(temp->infiles[i]);
        }
        for(int i = 0;i<temp->n_outfiles;i++){
            free(temp->outfiles[i]);
        }
    free(head);
    return;
}