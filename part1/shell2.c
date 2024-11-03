
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

static void sig_int(int);
#define MAXOPTIONS 21
#define MAXLINE 80
int
main(void)
{
	/*from apue.h*/
	char buf[MAXLINE];
	pid_t pid; 
	int status;
	/*capture buffer tokens*/ 
	char *token; 
	/*count the number of string to tokenize including last token*/
	int count = 1; 
	int i =0, j=0; 
	int fd,  stdin_saved=-1, stdout_saved=-1, stderr_saved=-1;
	char * options[MAXOPTIONS];
	memset(options, 0, MAXOPTIONS); 
	if(signal(SIGINT, sig_int) == SIG_ERR)
		printf("signal error\n"); 
	
	/*print %% at the start of every command prompt line*/
	printf("%% "); 
	/*while there is data being collected from the commandline*/
	while(fgets(buf, MAXLINE, stdin) != NULL){
		/*if the line ends with a new line character swap it with 0*/
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; 
		/*if the process failed to form, notify user*/
		if ((pid = fork()) < 0){
			printf("fork error\n"); 
		}
		/*if the fork was sucessful run the child process*/
		else if (pid == 0){

            
			/*count the whitespaces in the buffer*/
			for (i=0; i<strlen(buf); i++){
				if (isspace(buf[i])){
					
					count++;
				}
			}
			/*tokenize buffer to pull out command options and implement them in execvp*/
			token = strtok(buf, " \t\n");

			/*while there is data to pull out of the buffer*/
			int i =0; 
			while (token != NULL){
				/*push data in to the options array*/
				if (strcmp(token, ">")==0){
					token = strtok (NULL, " \t\n");
					/*Open the file to redirect std in to*/
					if((fd = open(token,O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR))<0){
						printf("open error for %s\n", token); 
						 
					}
					/*duplicate the stdin fd*/
					stdout_saved = dup(1);
					/*verify its mapped correctly*/
					if(dup2(fd, 1)<0){
						printf("open error for %s\n", token); 
						exit(4); 
					}
					/*close the file*/
					close(fd); 
					

				}
				else if (strcmp(token, "<")==0){
				 
					token = strtok (NULL, " \t\n");
					/*Open the file to redirect std in to*/
					if((fd = open(token,O_RDONLY, S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR))<0){
						printf("open error for %s\n", token); 
					}
					/*duplicate the stdin fd*/
					stdin_saved = dup(0);
					/*verify its mapped correctly*/
					if(dup2(fd, 0)<0){
						printf("open error for %s\n",token); 
						exit(4); 
					}
					/*close the file*/
					close(fd); 
				}
				else if(strcmp(token, ">&")==0){
					token = strtok (NULL, " \t\n");
					
					/*Open the file to redirect std in to*/
					if((fd = open(token,O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR))<0){
						printf("open error for %s\n", token); 
						exit(3); 
					}
					/*duplicate the stdin fd*/
					stdout_saved = dup(1);
					stderr_saved = dup(2); 
					/*verify its mapped correctly*/
					if(dup2(fd, 1)<0){
						printf("open error for %s\n", token); 
						exit(4); 
					}
          /*close the file*/
					close(fd); 
          if(dup2(1, 2)<0){
						printf("open error for %s\n", token); 
						exit(4); 
					}

				}

				else{ 
					options[i] = (char *)malloc(strlen(token)+1); 
					strcpy(options[i], token); 
					/* printf("%s\n", options[i]);  */
					/*move to the next token*/
					token = strtok (NULL, " \t\n"); 
				}
				i++; 
			}


			
			if ((execvp(options[0], options)) <0 ){
				printf("EXEC FAILED\n "); 
			}
			
			if (stdin_saved > -1){
				close(0); 
				dup2(stdin_saved, 0); 
				close(stdin_saved); 
				stdin_saved = -1;

			}
			if (stdout_saved > -1){
				close(1); 
				dup2(stdout_saved, 1); 
				close(stdout_saved); 
				stdout_saved = -1;

			}
			if (stderr_saved > -1){
				close(2); 
				dup2(stderr_saved, 2); 
				close(stderr_saved); 
				stderr_saved = -1;
			}
 
			/*send in the command and options*/
			printf("couldn't execute: %s\n", buf);
			exit(127);
		}
		if ((pid = waitpid(pid, &status, 0)) < 0)
		  printf("waitpid error\n");
		printf("%% "); 
	}/*end while loop*/	
	exit(0);
}/*end main*/

void
sig_int(int signo){
	printf("inturrpt\n%% ");
}
