#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_COMMAND_LENGTH 1000
#define MAX_TOKEN_COUNT 100
#define MAX_TOKEN_LENGTH 100

void interactive_mode();
void batch_mode(char* filename);
void parse_command(char* command);
int execute_command(char** tokens, int token_count);

int main(int argc, char* argv[]) {

    if (argc == 1) {
        interactive_mode();
    } else if (argc == 2) {
        batch_mode(argv[1]);
    } else {
        printf("inadequate number of args");
        exit(1);
    }

    return 0;
}

void interactive_mode() {
    char* command;
    using_history();
    while (1) {
        char* cwd = getcwd(NULL, 0);
        char* prompt = malloc(strlen(cwd) + 3);
        sprintf(prompt, "%s%% ", cwd);
        free(cwd);
        command = readline(prompt);
        free(prompt);
        if (*command) {
            add_history(command);      
            parse_command(command);
            free(command);
        }
    }
}

void batch_mode(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Unable to open file %s\n", filename);
        exit(1);
    }

    char command[MAX_COMMAND_LENGTH];
    while (fgets(command, MAX_COMMAND_LENGTH, file) != NULL) {
    	write(STDOUT_FILENO, "\n", strlen("\n"));
       	write(STDOUT_FILENO,command, strlen(command));	
        command[strcspn(command, "\r\n")] = 0; 
        parse_command(command);
    }

    fclose(file);
}

void pipe_launch(char **cmd1 , char **cmd2){
    
    int fd[2]; 
    pipe(fd);
    pid_t pid;
    for (int i=0 ; i<2 ;i++){
            pid=fork();
            if(pid==-1){			
            		close(fd[1]);
            		printf("Child process could not be created\n");
            		break;
            }
            if(pid==0){
                    if(i == 0){
		                   dup2(fd[1], STDOUT_FILENO);
		                   execvp(cmd1[0], cmd1);
		            	   perror("Command can't be executed");
            	       exit(1);
            	        }
                    else{
            	       	dup2(fd[0],STDIN_FILENO);
            	       	execvp(cmd2[0], cmd2);
            	        perror("Command can't be executed");
            	        exit(1);         	       
                        }
    			}	
            else {
            		 waitpid(pid,NULL,0);
            		 if(i == 0){
            		     close(fd[1]);
            		 }else { 
            		     close(fd[0]);  
            		 }
               }
         }
}



void redirection_launch(char **cmd1 , char **cmd2 , char* operator){
   FILE *read_fp;
   FILE *fp;
   char file[1000] = "";
   char command1[1000] = "";
   char str[10000];
   for(int i=0 ; cmd1[i]!=NULL ; i++){
    strcat(command1,cmd1[i]);
    strcat(command1," ");
  
   }
   read_fp = popen(command1,"r");
   if(read_fp == NULL) {
       perror("popen");
       return;
   }
   for(int i=0 ; cmd2[i]!=NULL ; i++){
     strcat(file,cmd2[i]);
     if(cmd2[i+1] != NULL)
        strcat(file," ");
   }
   if(strcmp(operator , ">>") == 0){
     fp = fopen(file ,"a");
   }
   else { 
   fp = fopen(file ,"w+");
   }
   if(fp == NULL) {
       perror("fopen");
       return;
   }
   while(fgets(str , sizeof(str),read_fp)){
   	fputs(str,fp);
   }
   pclose(read_fp);
   fclose(fp);
}



void execute_composed_command(char **cmd1, char **cmd2, char* operator) {
    int cmd1_status, cmd2_status;
    if(strcmp(cmd1[0], "quit") == 0){
    	exit(0);
    }
    if(strcmp(operator, "|") == 0){
        pipe_launch(cmd1 ,cmd2)  ;          
     }
    else if(strcmp(operator , ">") == 0 || strcmp(operator , ">>") == 0  ){
        redirection_launch(cmd1,cmd2,operator) ;
    }
    else {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(cmd1[0], cmd1);
        perror("Command can't be executed");
        exit(1);
    } else if (pid > 0) {
        wait(&cmd1_status);
        if(strcmp(operator, "&&") == 0){
            if(WEXITSTATUS(cmd1_status) == 0){
                if(strcmp(cmd2[0], "quit") == 0){
	                 exit(0);
		}	    
                pid = fork();
                if (pid == 0) {
                   execvp(cmd2[0], cmd2);
                    perror("Command can't be executed");
                    exit(1);
                } else if (pid > 0) {
                    wait(&cmd2_status);
                } else {
                    perror("fork failed");
                }
            }
        }
        else if(strcmp(operator, "||") == 0){
            if(WEXITSTATUS(cmd1_status) != 0){
              if(strcmp(cmd2[0], "quit") == 0){
			    	       exit(0);
			    }
                pid = fork();
                if (pid == 0) {
                   execvp(cmd2[0], cmd2);
                    perror("Command can't be executed");
                    exit(1);
                } else if (pid > 0) {
                    wait(&cmd2_status);
                } else {
                    perror("fork failed");
                }
            }
        } else if (strcmp(operator, ";") == 0) {
            pid = fork();
            if (pid == 0) {
                execvp(cmd2[0], cmd2);
                perror("Command can't be executed");
                exit(1);
            } else if (pid > 0) {
                wait(&cmd2_status);
            } else {
                perror("fork failed");
            }
        } else {
            printf("Invalid operator %s\n", operator);
        }
    } else {
        perror("fork failed");
    }
    }
}


int execute_command(char** tokens, int token_count) {
    char* cmd = tokens[0];
    char* args[token_count + 1];
    for (int i = 0; i < token_count; i++) {
        args[i] = tokens[i];
    }
    args[token_count] = NULL;

    if (strcmp(cmd, "quit") == 0) {
        exit(0);
    }

    pid_t pid = fork();
    if (pid == 0) {
       
        execvp(cmd, args);
        perror("Command can't be executed");
        exit(1);
    } else if (pid > 0) {
        int status;
        wait(&status);
        return WEXITSTATUS(status);
    } else {
        perror("fork failed");
        return 1;
    }



}
void parse_command(char* command) {
    char* tokens[MAX_TOKEN_COUNT];
    int token_count = 0;
    char* token = strtok(command, " ");
    while (token != NULL) {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }

    if (token_count == 0) {
        return;
    }
    tokens[token_count] = NULL; 
    int i;
    for (i = 0; i < token_count; i++) {
        if (strcmp(tokens[i], ";") == 0 || strcmp(tokens[i], "&&") == 0 || strcmp(tokens[i], "||") == 0 || strcmp(tokens[i], "|") == 0 || strcmp(tokens[i], ">>") == 0  || strcmp(tokens[i], ">") == 0)      { 
            break;
        }
    }
    if (i < token_count) {
   
        char *cmd1[i + 1]; 
        for(int j=0 ; j<i ; j++){
            cmd1[j]= tokens[j];
        }
        cmd1[i] = NULL;
      
        char *cmd2[token_count - i]; 
        for(int j=i+1 ;j<token_count ; j++){
		cmd2[j-i-1]= tokens[j];
}
	cmd2[token_count-i-1] = NULL;
	char* operator = tokens[i];
	execute_composed_command(cmd1, cmd2, operator);
	} 
  else {
	execute_command(tokens, token_count);
	}
}
