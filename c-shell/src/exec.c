#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "command.h"
#include "exec.h"
#include <fcntl.h>
#include <signal.h>

static int is_executable(const char *path){
    struct stat st;
    if(stat(path, &st) != 0){
        return 0;
    }
    if(S_ISREG(st.st_mode) == 0){
        return 0;
    }
    if(access(path, X_OK)==0){
        return 1;
    }else{
        return 0;
    }    
}


static int resolve_command(const char *name, char *out, size_t n){
    if(strchr(name, '/') != NULL){
        if(is_executable(name)){
            snprintf(out, n, "%s", name);
            return 1;
        }
        return 0;
    }
    const char *real = name;
    int skip_cwd = 0;
    if(name[0] == '%'){
        real = name + 1;
        skip_cwd = 1;
    }

    if(skip_cwd == 0){
        char cwd[PATH_MAX];
        if(getcwd(cwd, sizeof(cwd))!=NULL){
            snprintf(out, n, "%s/%s", cwd, real);
            if(is_executable(out)){
                return 1;
            }
        }
    }
    char *p = getenv("PATH");
    if(p != NULL){
        char *copy = strdup(p);
        char *dir = strtok(copy, ":");
        while(dir != NULL){
            snprintf(out, n,"%s/%s",dir, real);
            if(is_executable(out)){
                free(copy);
                return 1;
            }
            dir = strtok(NULL,":");
        }
        free(copy);
    }

    return 0;
}


static int setup_input(Command *cmd, int *infd){
    if(cmd->n_infiles == 0){
        *infd = -1;
        return 1;
    }

    int fds[64];
    int nfds = 0;

    for(int i = 0; i < cmd->n_infiles; i++){
        int fd = open(cmd->infiles[i], O_RDONLY);
        if(fd < 0){
            for(int j = 0; j < nfds; j++){
                close(fds[j]);
            }
            printf("cshell: no such file or directory\n");
            return 0;
        }
        fds[nfds] = fd;
        nfds++;
    }

    int p[2];
    if(pipe(p) < 0){
        for(int j = 0; j < nfds; j++){
            close(fds[j]);
        }
        return 0;
    }

    pid_t pid = fork();
    if(pid == 0){
        close(p[0]);
        char buf[4096];
        for(int i = 0; i < nfds; i++){
            ssize_t r;
            while((r = read(fds[i], buf, sizeof(buf))) > 0){
                write(p[1], buf, r);
            }
            close(fds[i]);
        }
        close(p[1]);
        _exit(0);
    }

    close(p[1]);
    for(int j = 0; j < nfds; j++){
        close(fds[j]);
    }
    *infd = p[0];
    return 1;
}

static int setup_output(Command *cmd, int *outfd){
    if(cmd->n_outfiles == 0){
        *outfd = -1;
        return 1;
    }

    int fds[64];
    int nfds = 0;

    for(int i = 0;i<=cmd->n_outfiles-1; i++){
        int flags = O_WRONLY | O_CREAT;
        if(cmd->outappend[i]==1){
            flags = flags | O_APPEND;
        }else{
            flags = flags | O_TRUNC;
        }
        int fd = open(cmd->outfiles[i], flags, 0644);
        if(fd<0){
            for(int j = 0; j < nfds; j++){
                close(fds[j]);
            }
            printf("cshell: unable to create file for writing\n");
            return 0;
        }
        fds[nfds] = fd;
        nfds++;
    }

    int p[2];
    if(pipe(p)<0){
        for(int j = 0;j<=nfds-1; j++){
            close(fds[j]);
        }
        return 0;
    }

    pid_t pid = fork();
    if(pid == 0){
        close(p[1]);
        char buf[4096];
        ssize_t r;
        while((r = read(p[0], buf, sizeof(buf)))>=1){
            for(int i = 0;i<=nfds-1;i++){
                write(fds[i], buf, r);
            }
        }
        close(p[0]);
        for(int i = 0;i<nfds;i++){
            close(fds[i]);
        }
        _exit(0);
    }

    close(p[0]);
    for(int j = 0;j<=nfds-1; j++){
        close(fds[j]);
    }
    *outfd = p[1];
    return 1;
}


int run_command(Command *cmd){
    char path[PATH_MAX*2];

    if(resolve_command(cmd->argv[0], path, sizeof(path))==0){
        const char *shown = cmd->argv[0];
        if(shown[0] == '%'){
            shown++;
        }
        printf("cshell: command not found (%s)\n", shown);
        return 0;
    }
        int infd = 0;
    if(setup_input(cmd, &infd) == 0){
        return 0;
    }

    int outfd = 0;
    if(setup_output(cmd, &outfd) == 0){
        if(infd >= 0){
            close(infd);
        }
        return 0;
    }

    pid_t pid = fork();
    if(pid == 0){
        if(infd >= 0){
            dup2(infd, STDIN_FILENO);
            close(infd);
        }
        if(outfd >= 0){
            dup2(outfd, STDOUT_FILENO);
            close(outfd);
        }
        execv(path, cmd->argv);
        _exit(1);
    }else if(pid > 0){
        if(infd >= 0){
            close(infd);
        }
        if(outfd >= 0){
            close(outfd);
        }
        int status;
        waitpid(pid, &status, 0);
    }
    return 1;
}

int bg_run_command(Command *cmd){
    char path[PATH_MAX*2];

    if(resolve_command(cmd->argv[0], path, sizeof(path))==0){
        const char *shown = cmd->argv[0];
        if(shown[0] == '%'){
            shown++;
        }
        printf("cshell: command not found (%s)\n", shown);
        return 0;
    }
    int infd = 0;
    if(setup_input(cmd, &infd) == 0){
        return 0;
    }

    int outfd = 0;
    if(setup_output(cmd, &outfd) == 0){
        if(infd >= 0){
            close(infd);
        }
        return 0;
    }

    pid_t pid = fork();
    if(pid == 0){
        if(infd >= 0){
            dup2(infd, STDIN_FILENO);
            close(infd);
        }
        if(outfd >= 0){
            dup2(outfd, STDOUT_FILENO);
            close(outfd);
        }
        execv(path, cmd->argv);
        _exit(1);
    }else if(pid > 0){
        if(infd >= 0){
            close(infd);
        }
        if(outfd >= 0){
            close(outfd);
        }
        int status;
        
    }
    return 1;
}






int run_pipeline(Command *head){
    int count = 1;
    Command *c = head;
    while(c->piped_to_next==1 && c->next!=NULL){
        count++;
        c = c->next;
    }

    if(count == 1){
        return run_command(head);
    }

    int pipes[count-1][2];
    for(int i = 0;i<count-1;i++){
        if(pipe(pipes[i]) < 0){
            return 0;
        }
    }

    pid_t pids[count];
    c = head;

    for(int i = 0; i<count;i++){
        pid_t pid = fork();

        if(pid == 0){
            if(i>0){
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            if(i<count-1){
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            for(int j = 0;j<count-1;j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            int infd = 0;
            if(setup_input(c, &infd) == 0){
                _exit(1);
            }
            if(infd >= 0){
                dup2(infd, STDIN_FILENO);
                close(infd);
            }
            int outfd = 0;
            if(setup_output(c, &outfd) == 0){
                _exit(1);
            }
            if(outfd >= 0){
                dup2(outfd, STDOUT_FILENO);
                close(outfd);
            }
            char path[PATH_MAX*2];
            if(resolve_command(c->argv[0], path, sizeof(path)) == 0){
                const char *shown = c->argv[0];
                if(shown[0] == '%') shown = shown + 1;
                printf("cshell: command not found (%s)\n", shown);
                _exit(1);
            }
            execv(path, c->argv);
            _exit(1);
        }

        pids[i] = pid;
        c = c->next;
    }

    for(int j = 0;j<count-1;j++){
        close(pipes[j][0]);
        close(pipes[j][1]);
    }

    int ok = 1;
    for(int i = 0;i<count;i++){
        int status = 0;
        waitpid(pids[i], &status, 0);
        if(WIFEXITED(status) == 0 || WEXITSTATUS(status)!=0){
            ok = 0;
        }
    }
    return ok;
}