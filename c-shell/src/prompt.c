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
#include "exec.h"


char prompt[10000];
char home_dir[PATH_MAX];
char prev_dir[PATH_MAX];

int main(){
    char username[256];
    char host[256];

    if(getlogin_r(username, sizeof(username)) != 0){
        char *e = getenv("USER");
        if(e != NULL){
            snprintf(username, sizeof(username), "%s", e);
        }else{
            snprintf(username, sizeof(username), "user");
        }
    }
    if(gethostname(host, sizeof(host)) != 0){
        snprintf(host, sizeof(host), "host");
    }

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
            printf("<%s@%s:~%s> ", username, host, new_pwd);
        }else{
            printf("<%s@%s:%s> ", username, host, pwd);
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
                        }else if(cmds->argc >= 1 && strcmp(cmds->argv[0], "peek")==0){
                            builtin_peek(cmds);
                        }else{
                            run_pipeline(cmds);
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