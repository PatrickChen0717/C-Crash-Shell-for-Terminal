#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 1024
#define MAXJOBS 1024

typedef struct {
    int arrayindex;
    int pid;
    char *status;
    char *commandname;
    bool killed;
}jobdata;
char **environ;
jobdata fg;
jobdata array[MAXJOBS];//default background process array
// TODO: you will need some data structure(s) to keep track of jobs
struct timespec remtime, reqtime = {0, 10000000};

void handle_sigchld(int sig) {
    int pid;
    int exitcode;
    while((pid=waitpid(-1,&exitcode,WNOHANG))>0){
        if(WIFEXITED(exitcode)==true){
            for(int i=0;i<MAXJOBS;i++){
                if(array[i].pid==pid){
                    //write(STDOUT_FILENO,"child died  ",13);
                    if(fg.pid==pid){
                        fg.arrayindex=NULL;
                        fg.pid=NULL;
                        fg.status=malloc(0);
                        memcpy(fg.status,"",0);
                        fg.commandname=malloc(0);
                        memcpy(fg.commandname,"",0);
                    }
                    char temp[1024];                            
                    int Length = sprintf(temp, "%d", array[i].pid);
                    //write(1,temp,Length);
                    array[i].killed=true;
                    break;
                }
            }
        }
        else{
            //write(STDOUT_FILENO,"child suspended  ",18);
            break;
        }
        
    }
}

void handle_sigtstp(int sig) {
    
    //for(int i=0;i<MAXJOBS;i++){
        if(fg.pid!=NULL){
            int result=kill(fg.pid, SIGSTOP);
            //printf("%d result: %d\n",fg.pid, result);
            int index;
            for(int i=0;i<MAXJOBS;i++){
                if(fg.pid==array[i].pid){
                    //printf("suspended: %d\n",i);
                    index=i;
                    break;
                }
            }
            array[index].status=malloc(9);
            memcpy(array[index].status,"suspended",9);
            char temp[1024];                            
            int Length = sprintf(temp, "%d", fg.arrayindex);
            write(STDOUT_FILENO,"[",2);
            write(STDOUT_FILENO,temp,Length);
            write(STDOUT_FILENO,"] ",3);
            write(STDOUT_FILENO,"(",2);
            char temp2[1024];
            Length = sprintf(temp2, "%d", fg.pid);
            write(STDOUT_FILENO,temp2,Length);;
            write(STDOUT_FILENO,")  suspended  ",15);
            write(STDOUT_FILENO,fg.commandname,strlen(fg.commandname));
            write(STDOUT_FILENO,"\n",2);

            fg.arrayindex=NULL;
            fg.pid=NULL;
            fg.status=malloc(0);
            memcpy(fg.status,"",0);
            fg.commandname=malloc(0);
            memcpy(fg.commandname,"",0);
        }
    //}
}

void handle_sigint(int sig) {
    //write(STDOUT_FILENO,"check",6);
    //check if there's fg process running
    for(int i=0;i<MAXJOBS;i++){
        if(fg.pid!=NULL){
            int result=kill(fg.pid, SIGTERM);
            for(int i=0;i<MAXJOBS;i++){
                if(fg.pid==array[i].pid){
                    //printf("killed: %d\n",i);
                    array[i].killed=true;
                    break;
                }
            }
            //printf("result: %d\n", result);
            //printf("[%d] (%d)  %s  %s\n",fgarray[i].arrayindex,fgarray[i].pid,"killed",fgarray[i].commandname);
            write(STDOUT_FILENO,"[",2);
            char temp[1024];                            
            int Length = sprintf(temp, "%d", fg.arrayindex);
            write(STDOUT_FILENO,temp,Length);
            write(STDOUT_FILENO,"] ",3);
            write(STDOUT_FILENO,"(",2);
            char temp2[1024];
            Length = sprintf(temp2, "%d", fg.pid);
            write(STDOUT_FILENO,temp2,Length);;
            write(STDOUT_FILENO,")  killed  ",12);
            write(STDOUT_FILENO,fg.commandname,strlen(fg.commandname));
            write(STDOUT_FILENO,"\n",2);

            fg.arrayindex=NULL;
            fg.pid=NULL;
            fg.status=malloc(0);
            memcpy(fg.status,"",0);
            fg.commandname=malloc(0);
            memcpy(fg.commandname,"",0);
            break;
        }
    }
}

void handle_sigquit(int sig) {
    bool running=false;
    int count;
    //check if there's fg process running
    //for(int i=0;i<MAXJOBS;i++){
        //printf("i: %d\n",i);
    if(fg.pid!=NULL){
        //count=i;
        running=true;
        //break;
    }
    //}
    if(running == false){
        //kill all background jobs
        for(int i=0;i<MAXJOBS;i++){   
            if(array[i].arrayindex!=NULL && array[i].pid!=NULL && array[i].killed==false){
                int p=kill(array[i].pid,SIGTERM);  
                //printf("result: %d\n",i);
            }
        }
        exit(0);
    }
    else{
        int result=kill(fg.pid, SIGTERM);
        //printf("result: %d\n", result);
        //printf("[%d] (%d)  %s  %s\n",fg.arrayindex,fg.pid,"killed",fg.commandname);
        write(STDOUT_FILENO,"[",2);
        char temp[1024];                            
        int Length = sprintf(temp, "%d", fg.arrayindex);
        write(STDOUT_FILENO,temp,Length);
        write(STDOUT_FILENO,"] ",3);
        write(STDOUT_FILENO,"(",2);
        char temp2[1024];
        Length = sprintf(temp2, "%d", fg.pid);
        write(STDOUT_FILENO,temp2,Length);;
        write(STDOUT_FILENO,")  killed  ",12);
        write(STDOUT_FILENO,fg.commandname,strlen(fg.commandname));
        write(STDOUT_FILENO,"\n",2);

        fg.arrayindex=NULL;
        fg.pid=NULL;
        fg.status=malloc(0);
        memcpy(fg.status,"",0);
        fg.commandname=malloc(0);
        memcpy(fg.commandname,"",0);

        //kill all background jobs
        for(int i=0;i<MAXJOBS;i++){   
            if(array[i].arrayindex!=NULL && array[i].pid!=NULL && array[i].killed==false){
                int p=kill(array[i].pid,SIGTERM);  
                //printf("result: %d\n",i);
            }
        }
    }
}

void install_signal_handlers() {
    for(int i=0;i<MAXJOBS;i++){
        array[i].killed=false;
    }
    struct sigaction act1;
    act1.sa_handler=handle_sigchld;
    act1.sa_flags=SA_RESTART;
    sigemptyset(&act1.sa_mask);
    sigaction(SIGCHLD,&act1,NULL);
    
    struct sigaction act2;
    act2.sa_handler=handle_sigquit;
    act2.sa_flags=SA_RESTART;
    sigaction(SIGQUIT,&act2,NULL);
    //signal(SIGQUIT,handle_sigquit);

    struct sigaction act3;
    act3.sa_handler=handle_sigint;
    act3.sa_flags=SA_RESTART;
    sigaction(SIGINT,&act3,NULL);
    //signal(SIGINT,handle_sigint);

    struct sigaction act4;
    act4.sa_handler=handle_sigtstp;
    act4.sa_flags=SA_RESTART;
    sigaction(SIGTSTP,&act4,NULL);
}

void spawn(const char **toks, bool bg) { // bg is true iff command ended with &
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);
    ///sigaddset(&mask,SIGQUIT);
    sigprocmask(SIG_BLOCK,&mask,NULL);
    //printf("entered spawn\n");
    int newprocess;
    char buffer[20];
    if(bg==true){//background
        int p = fork();
        
        if(p==0){ //it is a child
            setpgid(0,0);
            sigprocmask(SIG_UNBLOCK,&mask,NULL);
            newprocess = execvp(toks[0],toks);
            
            if(newprocess==-1){
                printf("ERROR: cannot run %s\n",toks[0]);
                for(int i=0;i<MAXJOBS;i++){
                    
                    if(getpid()==array[i].pid){
                        
                        array[i].killed=true;
                        break;
                    }
                }
                exit(0);
            }
        }
        else{
            //int p2=waitpid(p,NULL,0);
            //printf("check\n");
            for(int i=0;i<MAXJOBS;i++){
                if(array[i].arrayindex==NULL&&array[i].pid==NULL&&array[i].killed==false){
                    //printf("i=%d\n",i);
                    array[i].arrayindex=i+1;
                    array[i].pid=p;
                    array[i].status=malloc(7);
                    memcpy(array[i].status,"running",7);
                    array[i].commandname=malloc(strlen(toks[0]));
                    memcpy(array[i].commandname, toks[0], strlen(toks[0]));
                    printf("[%d] (%d)  %s\n",array[i].arrayindex,array[i].pid,array[i].commandname);
                    sigprocmask(SIG_UNBLOCK,&mask,NULL);
                    break;
                }
            }   
                 
        }
    }
    else{//forground
        int p = fork();

        if(p==0){ //it is a child
            setpgid(0,0);
            newprocess = execvp(toks[0],toks);
            if(newprocess==-1){
                sigprocmask(SIG_UNBLOCK,&mask,NULL);
                printf("ERROR: cannot run %s\n",toks[0]);
                for(int i=0;i<MAXJOBS;i++){
                    if(array[i].arrayindex==NULL&&array[i].pid==NULL&&array[i].killed==false){
                        array[i].arrayindex=i+1;
                        array[i].pid=getpid();
                        array[i].status=malloc(6);
                        memcpy(array[i].status,"killed",6);
                        array[i].commandname=malloc(strlen(toks[0]));
                        memcpy(array[i].commandname, toks[0], strlen(toks[0]));
                        array[i].killed=true;
                        printf("[%d] (%d)  %s  %s\n",array[i].arrayindex,array[i].pid,array[i].status,array[i].commandname);
                        
                        
                        //printf("%d",array[i].killed);
                        break;
                    }
                }
                exit(0);
            }
        }
        else{
            for(int i=0;i<MAXJOBS;i++){
                if(array[i].arrayindex==NULL&&array[i].pid==NULL&&array[i].killed==false){
                    array[i].arrayindex=i+1;
                    fg.arrayindex=i+1;
                    array[i].pid=p;
                    fg.pid=p;
                    array[i].status=malloc(7);
                    fg.status=malloc(7);
                    memcpy(array[i].status,"running",7);
                    memcpy(fg.status,"running",7);
                    array[i].commandname=malloc(strlen(toks[0]));
                    fg.commandname=malloc(strlen(toks[0]));
                    memcpy(array[i].commandname, toks[0], strlen(toks[0]));
                    memcpy(fg.commandname, toks[0], strlen(toks[0]));
                    //printf("[%d] (%d)  %s  %s\n",array[i].arrayindex,array[i].pid,array[i].status,array[i].commandname);
                    
                    break;
                }
            } 
            sigprocmask(SIG_UNBLOCK,&mask,NULL);

            

            while(fg.pid!=NULL){
                nanosleep(&reqtime, &remtime);
            }
            // while(waitpid(p,NULL,0)  != -1){
            //     printf("check\n");
            //     nanosleep(&reqtime, &remtime);
            // }

            //int p2=waitpid(p,WNOHANG,0);
            // for(int i=0;i<MAXJOBS;i++){
            //     if(array[i].pid==p){
            //         array[i].killed=true;
            //         break;
            //     }
            // }

            fg.arrayindex=NULL;
            fg.pid=NULL;
            fg.status=malloc(0);
            memcpy(fg.status,"",0);
            fg.commandname=malloc(0);
            memcpy(fg.commandname,"",0);
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
        while(array[i].arrayindex!=NULL && array[i].pid!=NULL && i<1024){
            //printf("jobs search line =%d\n",i);
            if(array[i].killed!=true){
                printf("[%d] (%d)  %s  %s\n",array[i].arrayindex,array[i].pid,array[i].status,array[i].commandname);
                //printf("[%d] (%d)  %s  %s  %i\n",array[i].arrayindex,array[i].pid,array[i].status,array[i].commandname,array[i].killed);
            }
            i++;
        }
    } 
}

void cmd_fg(const char **toks) {
    bool running=false;
    //check if there's fg process running
    //for(int i=0;i<MAXJOBS;i++){
    if(fg.pid!=NULL){
        running=true;
        //break;
    }
    //}
    if(running==false){
        //check cases
        if(toks[2]!=NULL || toks[1]==NULL){
            fprintf(stderr, "ERROR: fg takes exactly one argument\n");
        }
        else{
            //convert to char[]
            char *jobidtemp=malloc(strlen(toks[1])-1);
            for(int i=1;i<strlen(toks[1]);i++){
                jobidtemp[i-1]=toks[1][i];
            }
            char *processidtemp=malloc(strlen(toks[1]));
            for(int i=0;i<strlen(toks[1]);i++){
                processidtemp[i]=toks[1][i];
            }

            if(toks[1][0]=='%' && isNumber(jobidtemp)==0){
                fprintf(stderr, "ERROR: bad argument for fg: %s\n",toks[1]);
            }
            else if(toks[1][0]!='%' && isNumber(processidtemp)==0){
                fprintf(stderr, "ERROR: bad argument for fg: %s\n",toks[1]);
            }
            else{
                if(toks[1][0]=='%'){
                    int argument=atoi(jobidtemp);
                    //printf("PID = %d",atoi(jobidtemp));
                    if(array[argument-1].pid==NULL || (array[argument-1].pid!=NULL&&array[argument-1].killed==true)){
                        printf("ERROR: no job %s\n",toks[1]);
                    }
                    else{
                        int p2=kill(array[argument-1].pid,SIGCONT);
                        fg.arrayindex=array[argument-1].arrayindex;
                        fg.pid=array[argument-1].pid;
                        fg.status=malloc(7);
                        memcpy(fg.status,"running",7);
                        fg.commandname=malloc(strlen(array[argument-1].commandname));
                        memcpy(fg.commandname, array[argument-1].commandname, strlen(array[argument-1].commandname));

                        //int p2=waitpid(array[argument-1].pid,WNOHANG,0);
                        while(fg.pid!=NULL){
                             nanosleep(&reqtime, &remtime);
                        }
                        // while(waitpid(array[argument-1].pid,NULL,0) >0){
                        //      nanosleep(&reqtime, &remtime);
                        // }
                        for(int i=0;i<MAXJOBS;i++){
                            if(array[i].pid==array[argument-1].pid && strcmp(array[i].status,"suspended")!=0){
                                array[i].killed=true;
                                break;
                            }
                        }
                    }
                }
                else{
                    bool find=false;
                    for(int i=0;i<MAXJOBS;i++){
                        if(array[i].pid==atoi(processidtemp) && array[i].killed==false){
                            find=true;
                            break;
                        }
                    }
                    //printf("PID = %d",atoi(processidtemp));
                    if(find==false){
                        printf("ERROR: no PID %s\n",toks[1]);
                    }
                    else{
                        // while(waitpid(atoi(processidtemp),WNOHANG,0)!=-1){
                        //     nanosleep(1);
                        // }
                        //int p2=waitpid(atoi(processidtemp),WNOHANG,0);
                        //int p2=waitpid(atoi(processidtemp),NULL,WNOHANG);
                        int p2=kill(atoi(processidtemp),SIGCONT);

                        int index=0;
                        for(int i=0;i<MAXJOBS;i++){
                            if(array[i].pid==atoi(processidtemp)){
                                index=i;
                                break;
                            }
                        }
                        fg.arrayindex=array[index].arrayindex;
                        fg.pid=array[index].pid;
                        fg.status=malloc(7);
                        memcpy(fg.status,"running",7);
                        fg.commandname=malloc(strlen(array[index].commandname));
                        memcpy(fg.commandname, array[index].commandname, strlen(array[index].commandname));

                        while(fg.pid!=NULL){
                            nanosleep(&reqtime, &remtime);
                        }
                        // while(waitpid(atoi(processidtemp),NULL,0) > 0){
                        //     nanosleep(&reqtime, &remtime);
                        // }
                        for(int i=0;i<MAXJOBS;i++){
                            if(array[i].pid==atoi(processidtemp) && strcmp(array[i].status,"suspended")!=0){
                                array[i].killed=true;
                                break;
                            }
                        }
                    }
                        
                }
            }
        }
        
    }
    
}

void cmd_bg(const char **toks) {
    if(toks[2]!=NULL || toks[1]==NULL){
        fprintf(stderr, "ERROR: bg takes exactly one argument\n");
    }
    else{
        //convert to char[]
        char *jobidtemp=malloc(strlen(toks[1])-1);
        for(int i=1;i<strlen(toks[1]);i++){
            jobidtemp[i-1]=toks[1][i];
        }
        char *processidtemp=malloc(strlen(toks[1]));
        for(int i=0;i<strlen(toks[1]);i++){
            processidtemp[i]=toks[1][i];
        }

        if(toks[1][0]=='%' && isNumber(jobidtemp)==0){
            fprintf(stderr, "ERROR: bad argument for bg: %s\n",toks[1]);
        }
        else if(toks[1][0]!='%' && isNumber(processidtemp)==0){
            fprintf(stderr, "ERROR: bad argument for bg: %s\n",toks[1]);
        }
        else{
            if(toks[1][0]=='%'){
                int argument=atoi(jobidtemp);
                //printf("PID = %d",atoi(jobidtemp));
                if(array[argument-1].pid==NULL || (array[argument-1].pid!=NULL && array[argument-1].killed==true)){
                    printf("ERROR: no job %s\n",toks[1]);
                }
                else{
                    array[argument-1].status=malloc(7);
                    memcpy(array[argument-1].status,"running",7);
                    int p2=kill(array[argument-1].pid,SIGCONT);
                    printf("[%d] (%d)  %s\n",array[argument-1].arrayindex,array[argument-1].pid,array[argument-1].commandname);
                }
            }
            else{
                bool find=false;
                int index=0;
                for(int i=0;i<MAXJOBS;i++){
                    if(array[i].pid==atoi(processidtemp) && array[i].killed==false){
                        index=i;
                        find=true;
                        break;
                    }
                }
                //printf("PID = %d",atoi(processidtemp));
                if(find==false){
                    printf("ERROR: no PID %s\n",toks[1]);
                }
                else{
                    for(int i=0;i<MAXJOBS;i++){
                        if(array[i].pid==atoi(processidtemp)){
                            array[i].status=malloc(7);
                            memcpy(array[i].status,"running",7);
                            break;
                        }
                    }
                    int p2=kill(atoi(processidtemp),SIGCONT);
                    printf("[%d] (%d)  %s\n",array[index].arrayindex,array[index].pid,array[index].commandname);
                }
            }
        }
    }
}

void cmd_slay(const char **toks) {
    if(toks[2]!=NULL || toks[1]==NULL){
        fprintf(stderr, "ERROR: slay takes exactly one argument\n");
    }
    else{
        //convert to char[]
        char *jobidtemp=malloc(strlen(toks[1])-1);
        for(int i=1;i<strlen(toks[1]);i++){
            jobidtemp[i-1]=toks[1][i];
        }
        char *processidtemp=malloc(strlen(toks[1]));
        for(int i=0;i<strlen(toks[1]);i++){
            processidtemp[i]=toks[1][i];
        }

        if(toks[1][0]=='%' && isNumber(jobidtemp)==0){
            fprintf(stderr, "ERROR: bad argument for slay: %s\n",toks[1]);
        }
        else if(toks[1][0]!='%' && isNumber(processidtemp)==0){
            fprintf(stderr, "ERROR: bad argument for slay: %s\n",toks[1]);
        }
        else{
            if(toks[1][0]=='%'){
                char *temp=malloc(strlen(toks[1])-1);
                for(int i=1;i<strlen(toks[1]);i++){
                    temp[i-1]=toks[1][i];
                }
                //printf("argument: %s\n",temp);
                int argument=atoi(temp);
                if((array[argument-1].arrayindex==NULL && array[argument-1].pid==NULL) || (array[argument-1].pid!=NULL&&array[argument-1].killed==true)){
                    printf("ERROR: no job %s\n",toks[1]);
                }
                else if(array[argument-1].arrayindex!=NULL && array[argument-1].pid!=NULL){
                    int temp=kill(array[argument-1].pid, SIGTERM);
                    printf("result: %d\n", temp);
                    array[argument-1].killed=true;
                    printf("[%d] (%d)  %s  %s\n",array[argument-1].arrayindex,array[argument-1].pid,"killed",array[argument-1].commandname);
                }
            }
            else{
                int processid=atoi(toks[1]);
                //printf("processid: %d\n",processid);
                int removeid=NULL;
                for(int i=0;i<MAXJOBS;i++){
                    //printf("%d\n",array[i].pid);
                    if((array[i].arrayindex==NULL&&array[i].pid==NULL) || (array[i].pid!=NULL&&array[i].killed==true)){
                        printf("ERROR: no PID %s\n",toks[1]);
                        break;
                    }
                    else if(array[i].pid==processid){
                        //array[i].arrayindex=NULL;
                        int temp=kill(array[i].pid, SIGTERM);
                        printf("result: %d\n", temp);
                        array[i].killed=true;
                        if(strcmp(toks[0],"fromsig")!=0){
                            printf("[%d] (%d)  %s  %s\n",array[i].arrayindex,array[i].pid,"killed",array[i].commandname);
                        }
                        break;
                    }
                }
            }
        }
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
    
    if (strcmp(toks[0], "quit") == 0) {
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

int getdigit(int number){  
    int counter=0; 
    while(number!=0){  
        number=number/10;  
        counter++;  
    }  
    return counter;  
}  

int isNumber(char s[]){
    for (int i=0; i<strlen(s); i++){
        if (isdigit(s[i]) == 0){
            return 0;
        }
    }
    return 1;
}