#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include "command.h"
#include "builtins.h"
#include "prompt.h"
#include <time.h>
#include <dirent.h>


#define MAX_ENTRIES 1000

typedef struct{
    char path[PATH_MAX];
    int count;
    time_t last;
}frecentry;

static frecentry entries[MAX_ENTRIES];
static int entry_count = 0;

static void frec_file(char *buf, size_t n){
    snprintf(buf, n, "%s/.cshell_frecency", home_dir);
}

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

void frec_load(void){
    char fpath[PATH_MAX*2];
    frec_file(fpath, sizeof(fpath));
    FILE *f = fopen(fpath, "r");
    if(f==NULL){
        return;
    }
    entry_count = 0;
    char line[PATH_MAX*2];
    while(fgets(line, sizeof(line), f) != NULL && entry_count < MAX_ENTRIES){
        int c;
        long ts;
        char p[PATH_MAX];
        if(sscanf(line, "%d %ld %[^\n]", &c, &ts, p) == 3){
            strcpy(entries[entry_count].path, p);
            entries[entry_count].count = c;
            entries[entry_count].last = (time_t)ts;
            entry_count++;
        }
    }
    fclose(f);
}

static void frec_save(void){
    char fpath[PATH_MAX*2];
    frec_file(fpath, sizeof(fpath));
    FILE *f = fopen(fpath, "w");
    if(f == NULL){
        return;
    }

    for(int i = 0; i < entry_count; i++){
        fprintf(f, "%d %ld %s\n", entries[i].count, (long)entries[i].last, entries[i].path);
    }
    fclose(f);
}

static void frec_update(const char *path){
    for(int i = 0;i<entry_count;i++){
        if(strcmp(entries[i].path, path) == 0){
            entries[i].count++;
            entries[i].last = time(NULL);
            frec_save();
            return;
        }
    }
    if(entry_count<MAX_ENTRIES){
        strcpy(entries[entry_count].path, path);
        entries[entry_count].count = 1;
        entries[entry_count].last = time(NULL);
        entry_count++;
        frec_save();
    }
}

static double frec_score(int count, time_t last){
    double age = (double)(time(NULL) - last);
    if(age < 3600){
        return count*4.0;
    }
    if(age < 86400){
        return count*2.0;
    }
    if(age < 604800){
        return count*0.5;
    }
    return count*0.25;
}
static int frec_lookup(const char *name){
    int index[MAX_ENTRIES];
    int n = 0;
    for(int i = 0;i<entry_count;i++){
        if(strstr(entries[i].path, name)!=NULL){
            index[n] = i;
            n++;
        }
    }
    for(int i = 0;i<n;i++){
        int best = i;
        for(int j = i+1;j<n;j++){
            if(frec_score(entries[index[j]].count, entries[index[j]].last) >
               frec_score(entries[index[best]].count, entries[index[best]].last)){
                best = j;
            }
        }
        int t = index[i];
        index[i] = index[best];
        index[best] = t;
    }
    for(int i = 0;i<n;i++){
        if(chdir(entries[index[i]].path)==0){
            return 1;
        }
    }
    return 0;
}
int builtin_hop(Command *cmd){
    char temp[PATH_MAX];
    if(cmd->argc == 1){
        getcwd(temp, sizeof(temp));
        if(chdir(home_dir) == 0){
            strcpy(prev_dir, temp);
            char now[PATH_MAX];
            if(getcwd(now, sizeof(now)) != NULL){
                frec_update(now);
            }
        }
        return 0;
    }

    for(int k = 1;k<cmd->argc;k++){
        char *target = cmd->argv[k];
        if(strcmp(target, ".")==0){
            continue;
        }

        char *destination;
        if(strcmp(target, "~")==0){
            destination = home_dir;
        }else if(strcmp(target, "-")==0){
            if(prev_dir[0]=='\0'){
                continue;
            }
            destination = prev_dir;
        }else{
            destination = target;
        }
        getcwd(temp, sizeof(temp));
        if(chdir(destination)==0){
            strcpy(prev_dir, temp);
            char now[PATH_MAX];
            if(getcwd(now, sizeof(now)) != NULL){
                frec_update(now);
            }
        }else{
            if(frec_lookup(target)==1){
                strcpy(prev_dir, temp);
                char now[PATH_MAX];
                if(getcwd(now, sizeof(now)) != NULL){
                    frec_update(now);
                }
            }else{
                printf("hop: no such directory\n");
            }
        }
    }

    return 0;
}


#define MAX_FILES 4096

static int cmpstr(const void *a, const void *b){
    return strcmp(*(const char **)a, *(const char **)b);
}

static int is_dir(const char *path){
    struct stat st;
    if(stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
}

static void list_dir(const char *path, const char *prefix, int show_hidden, int recursive){
    DIR *d = opendir(path);
    if(d == NULL){
        return;
    }

    char *names[MAX_FILES];
    int n = 0;
    struct dirent *e;
    e = readdir(d);
    while(e!=NULL && n<MAX_FILES){
        if(strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0){
            e= readdir(d);
            continue;
        }
        if(show_hidden == 0 && e->d_name[0] == '.'){
            e= readdir(d);
            continue;
        }
        names[n] = strdup(e->d_name);
        e= readdir(d);
        n++;
    }
    closedir(d);
    qsort(names, n, sizeof(char*), cmpstr);

    for(int i = 0;i<n;i++){
        char full[PATH_MAX*2];
        snprintf(full, sizeof(full), "%s/%s", path, names[i]);

        if(prefix[0] == '\0'){
            printf("%s\n", names[i]);
        }else{
            printf("%s/%s\n", prefix, names[i]);
        }

        free(names[i]);
    }
}

int builtin_reveal(Command *cmd){
    int show_hidden = 0;
    int recursive = 0;
    char *target = NULL;
    int targets = 0;

    for(int k=1;k<=cmd->argc-1; k++){
        char *arg = cmd->argv[k];
        if(strcmp(arg, "-") == 0){
            target = arg;
            targets++;
        }else if(arg[0] == '-'){
            for(int c = 1; arg[c] != '\0'; c++){
                if(arg[c] == 'a'){
                    show_hidden = 1;
                }else if(arg[c] == 't'){
                    recursive = 1;
                }else{
                    printf("reveal: invalid syntax\n");
                    return 0;
                }
            }
        }else{
            target = arg;
            targets++;
        }
        if(targets >= 2){
            printf("reveal: invalid syntax\n");
            return 0;
        }
    }
    char resolved[PATH_MAX];
    if(target == NULL){
        strcpy(resolved, ".");
    }else if(strcmp(target, "~") == 0){
        strcpy(resolved, home_dir);
    }else if(strcmp(target, "-") == 0){
        if(prev_dir[0] == '\0'){
            printf("reveal: no such directory\n");
            return 0;
        }
        strcpy(resolved, prev_dir);
    }else{
        strcpy(resolved, target);
    }

    if(is_dir(resolved) == 0){
        printf("reveal: no such directory\n");
        return 0;
    }

    list_dir(resolved, "", show_hidden, recursive);
    return 0;
    return 0;
}