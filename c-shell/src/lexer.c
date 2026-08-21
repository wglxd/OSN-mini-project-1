#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <string.h>
#include "prompt.h"
#include "lexer.h"

char* tokens[10001];
int tok_is_op[10001];

int func(int string_length){
    for(int k = 0; tokens[k] != NULL; k++){
        free(tokens[k]);
        tokens[k] = NULL;
    }
    int j = 0;
    char word_buf[10000];
    int i = 0;
    int word_len = 0;

    while(i < string_length){
        char c = prompt[i];
        if(c == ' ' || c== '\t' || c == '\n' || c == '\r'){
            i++;
            continue;
        }
        if(c== '|' || c == '<' || c == '&' || c==';'){
            char s[2];
            s[0] = c;
            s[1] = '\0';
            tok_is_op[j] = 1;
            tokens[j++] = strdup(s);
            i++;
            continue;
        }
        if(c=='>'){
            if(i+1 < string_length && prompt[i+1] == '>'){
                char s[3];
                s[0] = '>';
                s[1] = '>';
                s[2] = '\0';
                tok_is_op[j] = 1;
                tokens[j++] = strdup(s);
                i+=2;
                continue;
            }else{
                char s[2];
                s[0] = '>';
                s[1] = '\0';
                tok_is_op[j] = 1;
                tokens[j++] = strdup(s);
                i++;
                continue;
            }
        }

        word_len = 0;

        while(i < string_length){
            c = prompt[i];
            if(c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
               c == '|' || c == '&' || c == ';' || c == '<' || c == '>') {
                break;
            }
            if(c == '\\'){
                if(i+1 >= string_length){
                    tokens[j] = NULL;
                    fprintf(stderr, "cshell: invalid syntax\n");
                    return 0;
                }
                word_buf[word_len++] = prompt[i+1];
                i+=2;
                continue;
            }
            if(c == '"' || c == '\''){
                char quote = c;
                i++;
                int closed = 0;
                while(i < string_length){
                    c = prompt[i];
                    if(c == quote){
                        closed = 1;
                        i++;
                        break;
                    }
                    if(quote == '"' && c == '\\'){
                        if(i+1 >= string_length){
                            tokens[j] = NULL;
                            fprintf(stderr, "cshell: invalid syntax\n");
                            return 0;
                        }
                        char next = prompt[i + 1];
                        if(next == '"' || next == '\\'){
                            word_buf[word_len++] = next;
                        }else{
                            word_buf[word_len++] = '\\';
                            word_buf[word_len++] = next;
                        }
                        i += 2;
                        continue;
                    }
                    word_buf[word_len++] = c;
                    i++;
                }
                if(closed==0){
                    tokens[j] = NULL;
                    fprintf(stderr, "cshell: invalid syntax\n");
                    return 0;
                }
                continue;
            }
            word_buf[word_len++]=c;
            i++;
        }
        word_buf[word_len++] = '\0';
        tok_is_op[j] = 0;
        tokens[j++]=strdup(word_buf);
    }
    tokens[j] = NULL;

    return 1;
}