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

char prompt[10000];

int main(){
    char *buf1 = "<tanush@iiit:";
    char hd[PATH_MAX];

    if(getcwd(hd, sizeof(hd))==NULL){
        printf("Failed to fetch the home directory!\n");
        return 1;
    }

    while(1){
        char pwd[PATH_MAX];
        if(getcwd(pwd, sizeof(pwd)) == NULL){
            printf("Failed to fetch the present working directory!\n");
            return 1;
        }

        int hlen = (int)strlen(hd);
        if(strncmp(pwd, hd, hlen) == 0 && (pwd[hlen] == '/' || pwd[hlen] == '\0')){
            char *new_pwd = pwd + hlen;
            printf("%s~", buf1);
            printf("%s> ", new_pwd);
        }else{
            printf("%s", buf1);
            printf("%s> ", pwd);
        }
        fflush(stdout);

        
        if(fgets(prompt, sizeof(prompt), stdin) != NULL){
            prompt[strcspn(prompt, "\n")] = '\0';
            int length = strlen(prompt);
            if(func(length)){
                if(parse()==1){
                    Command *cmds = build_commands(0);
                    free_commands(cmds);
                }
            }
        }else{
            printf("\n");
            break;
        }
    }


    return 0;
}