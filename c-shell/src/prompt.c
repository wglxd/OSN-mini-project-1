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
#include "builtins.h"


char prompt[10000];
char home_dir[PATH_MAX];
char prev_dir[PATH_MAX];

int main(){
    char *buf1 = "<tanush@iiit:";

    if(getcwd(home_dir, sizeof(home_dir))==NULL){
        printf("Failed to fetch the home directory!\n");
        return 1;
    }
    frec_load();
    prev_dir[0] = '\0';

    while(1){
        char pwd[PATH_MAX];
        if(getcwd(pwd, sizeof(pwd)) == NULL){
            printf("Failed to fetch the present working directory!\n");
            return 1;
        }

        int hlen = (int)strlen(home_dir);
        if(strncmp(pwd, home_dir, hlen) == 0 && (pwd[hlen] == '/' || pwd[hlen] == '\0')){
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

                    if(cmds!=NULL){
                        if(cmds->argc >= 1 && strcmp(cmds->argv[0], "locate")==0){
                            builtin_locate(cmds);
                        }else if(cmds->argc >= 1 && strcmp(cmds->argv[0], "hop")==0){
                            builtin_hop(cmds);
                        }else if(cmds->argc >= 1 && strcmp(cmds->argv[0], "reveal")==0){
                            builtin_reveal(cmds);
                        }
                    }
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

