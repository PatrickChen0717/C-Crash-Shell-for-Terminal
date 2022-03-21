#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXLINE 1024
#define MAXJOBS 1024

typedef struct {
    int arrayindex;
    int pid;
    char *status;
    char *commandname;
}jobdata;

char **environ;
jobdata array[MAXJOBS];
// TODO: you will need some data structure(s) to keep track of jobs

void handle_sigchld(int sig) {
    // TODO
}

void handle_sigtstp(int sig) {
    // TODO
}

void handle_sigint(int sig) {
    // TODO
}

void handle_sigquit(int sig) {
    // TODO
}

void install_signal_handlers() {
    // TODO
}

void spawn(const char **toks, bool bg) { // bg is true iff command ended with &
    //printf("entered spawn\n");
    int newprocess;
    char buffer[20];
    if(bg==true){//background
        int p = fork();
        
        if(p==0){ //it is a child
            
            newprocess = execvp(toks[0],toks,environ);
            if(newprocess==-1){
                printf("ERROR: cannot run %s\n",toks[0]); 
                exit(0);
            }
        }
        else{
            //int p2=waitpid(p,WNOHANG,0);
            
            
            for(int i=0;i<MAXJOBS;i++){
                if(array[i].arrayindex==NULL&&array[i].pid==NULL){
                    array[i].arrayindex=i+1;
                    array[i].pid=p;
                    array[i].status=malloc(7);
                    memcpy(array[i].status,"running",7);
                    array[i].commandname=malloc(strlen(toks[0]));
                    memcpy(array[i].commandname, toks[0], strlen(toks[0]));
                    printf("[%d] (%d)  %s\n",array[i].arrayindex,array[i].pid,array[i].commandname);
                    break;
                }
            }   
              
        }
    }
    else{//forground
        int fd[2];
        pipe(fd);//pipeline
        int p = fork();

        if(p==0){ //it is a child
            close(fd[0]);
            newprocess = execvp(toks[0],toks,environ);
            if(newprocess==-1){
                printf("ERROR: cannot run %s\n",toks[0]);
                char *message = "Error forground";
                int length = strlen( message );
                write( fd[1], message, length+1 );
                exit(0);
            }
        }
        else{
            close(fd[1]);
            int p2=waitpid(p,NULL,0);
            int count = read( fd[0], buffer, 20 );
            buffer[count] = '\0';
            //printf( "read message: %s\n", buffer );
            if ( count <= 0 ) {
                //perror( "read" );
                for(int i=0;i<MAXJOBS;i++){
                    if(array[i].arrayindex==NULL&&array[i].pid==NULL){
                        array[i].arrayindex=i+1;
                        array[i].pid=p;
                        array[i].status=malloc(0);
                        memcpy(array[i].status,"running",7);
                        array[i].commandname=malloc(strlen(toks[0]));
                        memcpy(array[i].commandname, toks[0], strlen(toks[0]));
                        printf("[%d] (%d)  %s  %s\n",array[i].arrayindex,array[i].pid,array[i].status,array[i].commandname);
                        break;
                    }
                }   
            }       
        }
    }
}


 
void cmd_jobs(const char **toks) {
    //printf("Enter jobs");
    if(toks[1]!=NULL){
        fprintf(stderr, "ERROR: job takes no arguments\n");
    }
    else{
        int i=0;
        while(array[i].arrayindex!=NULL&&array[i].pid!=NULL){
            if(strlen(array[i].status)==0){
                printf("[%d] (%d)  %s\n",array[i].arrayindex,array[i].pid,array[i].commandname);
            }
            else{
                printf("[%d] (%d)  %s  %s\n",array[i].arrayindex,array[i].pid,array[i].status,array[i].commandname);
            }
            i++;
        }
    } 
}

void cmd_fg(const char **toks) {
    // TODO
}

void cmd_bg(const char **toks) {
    // TODO
}

void cmd_slay(const char **toks) {
    int ret = kill(**toks);
    if (ret == -1) {
        int jobid=0;
        for(int i=0;i<MAXJOBS;i++){
            if(array[i].status=="killed"){
                jobid=i;
                break;
            }
        }
        printf("[%d] %d  killed  %c\n",jobid,getpid(),*toks[0]);
        exit(EXIT_FAILURE);
    }
}

void cmd_quit(const char **toks) {
    if (toks[1] != NULL) {
        fprintf(stderr, "ERROR: quit takes no arguments\n");
    } else {
        exit(0);
    }
}

void eval(const char **toks, bool bg) { // bg is true iff command ended with &
    assert(toks);
    if (*toks == NULL) return;
    else if(bg==true){
        spawn(toks, bg);
    }
    else if (strcmp(toks[0], "quit") == 0) {
        cmd_quit(toks);
    }
    else if (strcmp(toks[0], "jobs") == 0) {
        cmd_jobs(toks);
    }
    else if (strcmp(toks[0], "fg") == 0) {
        cmd_fg(toks);
    }
    else if (strcmp(toks[0], "bg") == 0) {
        cmd_bg(toks);
    }
    else if (strcmp(toks[0], "slay") == 0) {
        cmd_slay(toks);
    }
    else {
        spawn(toks, bg);
    }
}

// you don't need to touch this unless you are submitting the bonus task
void parse_and_eval(char *s) {
    assert(s);
    const char *toks[MAXLINE+1];
    
    while (*s != '\0') {
        bool end = false;
        bool bg = false;
        int t = 0;

        while (*s != '\0' && !end) {
            while (*s == ' ' || *s == '\t' || *s == '\n') ++s;
            if (*s != '&' && *s != ';' && *s != '\0') toks[t++] = s;
            while (strchr("&; \t\n", *s) == NULL) ++s;
            switch (*s) {
            case '&':
                bg = true;
                end = true;
                break;
            case ';':
                end = true;
                break;
            }
            if (*s) *s++ = '\0';
        }
        toks[t] = NULL;
        eval(toks, bg);
    }
}

// you don't need to touch this unless you are submitting the bonus task
void prompt() {
    printf("crash> ");
    fflush(stdout);
}

// you don't need to touch this unless you are submitting the bonus task
int repl() {
    char *line = NULL;
    size_t len = 0;
    while (prompt(), getline(&line, &len, stdin) != -1) {
        parse_and_eval(line);
    }

    if (line != NULL) free(line);
    if (ferror(stdin)) {
        perror("ERROR");
        return 1;
    }
    return 0;
}

// you don't need to touch this unless you want to add debugging options
int main(int argc, char **argv) {
    install_signal_handlers();
    return repl();
}
