#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

#define S_LINE 0
#define S_ARG  1
#define S_CMD  2
#define S_TGT  3
#define S_BG   4


static int is_word(int k){
    if(tok_is_op[k]==0){
        return 1;
    }else{
        return 0;
    }
}

static int op_is(int k, const char* sym){
    if(tok_is_op[k]==1 && strcmp(tokens[k], sym)==0){
        return 1;
    }else{
        return 0;
    }
}

int parse(void){
    int n = 0;
    while(tokens[n]!=NULL){
        n++;
    }
    if(n==0){
        return 1;
    }
    int state = S_LINE;
    int i = 0;
    while(1){
        if(state==S_LINE){
            if(i>=n){
                return 1;
            }
            if(is_word(i)==1){
                i++;
                state = S_ARG;
                continue;
            }
            fprintf(stderr, "cshell: invalid syntax\n");
            return 0;
        }
        if(state==S_ARG){
            if(i>=n){
                return 1;
            }
            if(is_word(i)==1){
                i++;
                state = S_ARG;
                continue;
            }
            if(op_is(i,"<")==1 || op_is(i,">")==1 || op_is(i,">>")==1){
                i++;
                state = S_TGT;
                continue;
            }
            if(op_is(i, "|")==1 || op_is(i, ";")==1){
                i++;
                state = S_CMD;
                continue;
            }
            if(op_is(i, "&")==1){
                i++;
                state = S_BG;
                continue;
            }
            fprintf(stderr, "cshell: invalid syntax\n");
            return 0;
        }
        if(state == S_CMD || state == S_TGT){
            if(i>=n||(!is_word(i))==1){
                fprintf(stderr, "cshell: invalid syntax\n");
                return 0;
            }
            i++;
            state = S_ARG;
            continue;
        }
        if(state==S_BG){
            if(i>=n){
                return 1;
            }
            if(is_word(i)==1){
                i++;
                state = S_ARG;
                continue;
            }
            fprintf(stderr, "cshell: invalid syntax\n");
            return 0;
        }
        fprintf(stderr, "cshell: invalid syntax\n");
        return 0;
    }
}