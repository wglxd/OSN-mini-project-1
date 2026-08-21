#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include "command.h"
#include "builtins.h"


static int is_executable(const char *path){
    struct stat st;
    if(stat(path, &st) != 0){
        return 0;
    }
    if(S_ISREG(st.st_mode)==0){
        return 0;
    }
    if(access(path, X_OK) == 0){
        return 1;
    }else{
        return 0;
    }
}
int builtin_locate(Command *cmd){
    if(cmd->argc==1){
        printf("locate: invalid syntax\n");
        return 0;
    }

    char cwd[PATH_MAX];
    char path[PATH_MAX*2];
    for(int k = 1; k<=(cmd->argc-1); k++){
        char *name = cmd->argv[k];
        int found = 0;

        if(getcwd(cwd, sizeof(cwd)) != NULL){
            snprintf(path, sizeof(path), "%s/%s", cwd, name);
            if(is_executable(path)==1){
                printf("%s\n", path);
                found = 1;
            }
        }
        char *p = getenv("PATH");
        if(p!=NULL){
            char *copy = strdup(p);
            char *dir = strtok(copy, ":");
            while(dir!=NULL){
                snprintf(path, sizeof(path),"%s/%s",dir,name);
                if(is_executable(path)==1){
                    printf("%s\n", path);
                    found = 1;
                }
                dir = strtok(NULL,":");
            }
            free(copy);
        }
        if(found==0){
            printf("locate: command not found (%s)\n", name);
        }
    }
     return 0;
}