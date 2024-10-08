//Dimitris Bisias 4273
//csd4273@csd.uoc.gr

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>

void execute_command(char** command);
void red();
void blue();
void reset();
void print_prompt();
void pipe_handler(char* line);
int gvar_handler(char* line);
int alpha_check(char* txt);
void z_handler(int signum);
void c_handler(int signum);
void quit_handler(int signum);
char* quote_remover(char* text);

pid_t ppid, cpid, stoppedchild;
int status;


int main(int argc, char** argv) {
    char* user;
    char path[100];
    char* line = NULL;
    size_t len;
    char*** commands;
    char *token, *token2, *ptr1, *ptr2;
    int i = 0, j = 0, argcnt = 0, abort; 
    int stdin_fd, stdout_fd;    
    
    signal(SIGTSTP, z_handler);
    signal(SIGINT, c_handler);
    signal(SIGQUIT, quit_handler);

    ppid = getpid();
    cpid = ppid; //for signal handler checks
    struct termios tty;

    while(1){

        tcgetattr(STDOUT_FILENO, &tty);
        tty.c_iflag |= IXON;
        tty.c_cc[VSTOP] = 19; //ctrl+s
        tty.c_cc[VSTART] = 17; //ctrl+q
        tcsetattr(STDOUT_FILENO, TCSANOW, &tty);

        print_prompt();
        
        len = 0;
        getline(&line, &len, stdin);
        //if enter is pressed just print prompt again and wait for new command
        if(!strcmp(line,"\n")) 
            continue;        

        line = strtok(line,"\n"); // Strip new line character from command
        
        //create new global variable if possible and print prompt again
        if(gvar_handler(line))
            continue;
        


        commands =(char***)malloc(strlen(line)*sizeof(char**));

        //Save stdin and stdout fd's so that they can be restored later if they change
        stdin_fd = dup(0);
        stdout_fd = dup(1);
        abort = 0; //reset abort boolean to 0

        for(char* str = line, i = 0; ;i++, str = NULL) { //str = NULL for subsequent strtok_r calls
            token = strtok_r(str, ";", &ptr1);
            if(token == NULL) {
                break;
            }
            //Search for "|" character in line, if it is found, pipe_handler is called
            if(strchr(token,'|') != NULL) {
                pipe_handler(token);
                continue;
            }

            commands[i] = (char**)malloc(strlen(token)*sizeof(char*));

            for(char* str2 = token, j = 0; ;j++, str2 = NULL) {
                token2 = strtok_r(str2, " ", &ptr2);
                if(token2 == NULL) {
                    break;
                }
                else if(*token2 == '$') {
                    token2 = quote_remover(getenv(token2+1));
                }
                commands[i][j] = token2;
                argcnt++;
            }

            commands[i][argcnt] = NULL; //Add Null after the command (mandatory for exec)
            if(!abort) {
                execute_command(commands[i]);
            }
            argcnt = 0; //reset argument counter
            dup2(stdin_fd, 0); //reset stdin
            dup2(stdout_fd, 1); //reset stdout
            free(commands[i]);
        }
        free(commands);
    }
    return 1;
}

void z_handler(int signum) {
    if(cpid != ppid){
        printf("Moving %d to background\n", cpid);
        stoppedchild = cpid;
        kill(cpid, SIGTSTP);
    }
}

void c_handler(int signum) {
    if(cpid != ppid){
        printf("Interrupting process %d\n", cpid);
        kill(cpid, SIGINT);
    }
}

void quit_handler(int signum) {
    if(cpid != ppid) {
        printf("Quitting process %d\n", cpid);
        kill(cpid, SIGQUIT);
    }
}

//removes quotes if any, otherwise returns text as is
char* quote_remover(char* text) {
    if (!text)
        return text;
    
    int len = strlen(text);
    char* ret;
    if(*text == '"' && *(text+len-1) == '"') {
        *(text+len-1) = '\0';
        ret = text+1;
        return ret;
    }
    return text;
}

int gvar_handler(char* line) {
    char* p = strchr(line, '=');
    int len = strlen(line);
    
    if(p != NULL && *(p-1) != ' ') {
        int length = p - line;
        char** gvars;
        gvars = (char**)malloc(2*sizeof(char*));
        gvars[0] = (char*)malloc((length+1)*sizeof(char));
        gvars[1] = (char*)malloc((len-length+1)*sizeof(char));

        strncpy(gvars[0], line, length);
        gvars[0][length] = '\0';
        strcpy(gvars[1], line+length+1);
        if(!alpha_check(gvars[0]))
            return 1; // invalid name, return 1 but dont set the variable
        setenv(gvars[0], gvars[1], 1);
        return 1;
    }
    return 0;
}

//checks if the given string ony has 'a-z', 'A-Z' or '_' characters in it
int alpha_check(char* txt) {
    int len = strlen(txt);

    for(int i = 0; i < len; ++i) {
        if((*(txt + i) >= 65 && *(txt + i) <= 90) || (*(txt + i) >= 97 && *(txt + i) <= 122) || *(txt + i) == 95)
            continue;
        else
            return 0;
    }

    return 1;
}

void pipe_handler(char* line) {
    char* token, *token2,*ptr, *ptr2;
    size_t len;
    int i = 0, j = 0, cmd_cnt = 0, arg_cnt = 0;
    char*** pipe_commands;
    int redir_flag = 0;
    int output_fd, append_fd;

    pipe_commands = (char***)malloc(strlen(line)*sizeof(char**));

    for(char* str = line, i = 0; ;i++, str = NULL) {
        token = strtok_r(str,"|",&ptr);
        if(token == NULL) {
            pipe_commands[i] = NULL; //Add NULL at the end of the commands, so there is a terminating condition for the 'for' loop
            break;
        }
        pipe_commands[i] = (char**)malloc(strlen(token)*sizeof(char*));
        cmd_cnt++;
        for(char* str = token, j=0; ;j++, str = NULL) {
            token2 = strtok_r(str, " ", &ptr2);
            if(token2 == NULL) {
                break;
            }
            else if(*token2 == '$') {
                    token2 = quote_remover(getenv(token2+1));
                }
            pipe_commands[i][j] = token2;
            arg_cnt++;
        }
        pipe_commands[i][arg_cnt] = NULL;
        arg_cnt = 0;
    }

    int fd[cmd_cnt][2];
    //fd[0] - read
    //fd[1] - write
    int stdin_fd = dup(0);
    int stdout_fd = dup(1);

    for(i = 0; pipe_commands[i] != NULL; i++) {
        pipe(fd[i]);
        if(i != 0) {
            dup2(fd[i-1][0],0);
            close(fd[i-1][0]);
            close(fd[i-1][1]);
        }
        if(pipe_commands[i+1] != NULL) {
            dup2(fd[i][1], 1);
        }
        else {
            if(redir_flag == 1) {
                dup2(output_fd, 1);
            }
            else if(redir_flag == 2) {
                dup2(append_fd, 1);
            }
            else {
                dup2(stdout_fd, 1);
            }
        }
        execute_command(pipe_commands[i]);
    }
    //reset stdin/out for later use
    dup2(stdin_fd, 0);
    dup2(stdout_fd, 1);
}

void execute_command(char** command) {
    if(strcmp(command[0],"cd") == 0) {
        chdir(command[1]);
        return;
    }
    else if(strcmp(command[0],"exit") == 0) {
        exit(0);
    }
    else if(strcmp(command[0], "fg") == 0) {
        if(!stoppedchild)
            return;
        else if(kill(stoppedchild, 0) == -1)
            if (errno == ESRCH) // child that was previously stopped, has finished execution
                return;
        
        printf("Bringing child %d to foreground\n", stoppedchild);
        cpid = stoppedchild;
        kill(stoppedchild, SIGCONT);
        waitpid(-1, &status, WUNTRACED);
        cpid = ppid;
    }
    else {        
        pid_t pid = fork();
        if(pid < 0) {
            fprintf(stderr,"fork() failed.\n");
            return;
        }
        //Child proccess
        else if(pid == 0) {
            signal(SIGTSTP, SIG_DFL);
            execvp(command[0], command);
            perror("exec");
            exit(1);
        }
        //Parent proccess
        else {
            cpid = pid;
            waitpid(-1, &status, WUNTRACED);
            cpid = ppid; //so that ^Z or ^C doesnt work if no child is active
            return;
        }
    }
}

void print_prompt() {
    char* user = getlogin();
    char path[100];        
    getcwd(path, sizeof(path));
    red();
    printf("%s@cs345sh",user);
    reset();
    printf(":");
    blue();
    printf("%s",path);
    reset();
    printf("$ ");

}

void red() {
    printf("\033[1;31m");
}

void blue() {
    printf("\033[1;34m");
}

void reset () {
    printf("\033[0m");
}