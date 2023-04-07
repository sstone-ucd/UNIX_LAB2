#include "apue.h"
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/*lab 2: expand the shell to read file expansion characters
FILE DESCRIPTORS TO REDIRECT: 

0:stdin
1:stdout
2:stderr


 < redirect stdin from keyboard to file, open file, apply command to text in file 
				cat < inputFile.txt will print the data in the file by redirecting the text in the file to the cat command
			Plan: search through array and look for index with < 
			if i = < then, i-1 = command and i+1 = filename
			fd = open(filename)
			dup2(fd, 0)
			close(fd)
			execvp(array[0], )



 > open file and overwirte (or create it if it doesnt exist) and redirect the input to the file: 
 				ls > newFile.txt will print the ls results to newFile.txt
 >& open and overwirte file (or create a new file if it doesnt exist) and redirect the output and errors of the command to the file
				ls &> output.txt will print the ls data and any stderr info to output.txt
				>& is equivalent to 2>&1

*/


#define MAXOPTIONS 10
/*signal catching funcion*/
#include <fcntl.h>
static void sig_int(int);

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
	if(signal(SIGINT, sig_int) == SIG_ERR)
		err_sys("signal error"); 
	
	/*print %% at the start of every command prompt line*/
	printf("%% "); 
	/*while there is data being collected from the commandline*/
	while(fgets(buf, MAXLINE, stdin) != NULL){
		/*if the line ends with a new line character swap it with 0*/
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; 
		/*if the process failed to form, notify user*/
		if ((pid = fork()) < 0){
			err_sys("fork error"); 
		}
		/*if the fork was sucessful run the child process*/
		else if (pid == 0){

            int fd,  stdin_saved=-1, stdout_saved=-1, stderr_saved=-1;
			/*count the whitespaces in the buffer*/
			for (i=0; i<strlen(buf); i++){
				if (isspace(buf[i])){
					/*increment count to determine what size array to declare*/
					count++;
				}
			}
			/*tokenize buffer to pull out command options and implement them in execvp*/
			token = strtok(buf, " ");
			/*allocate array that points to tokens with size count*/
			char ** options = malloc(count +1 * sizeof(token));
			options[count+1] = NULL; 
			/*while there is data to pull out of the buffer*/
			for (i =0; i<count; i++){
				while (token != NULL){
					/*push data in to the options array*/
					options[i%count] = token;
					/*move to the next token*/
					token = strtok (NULL, " "); 
					i++; 
				}
			}
			/*Traverse the arguments and look for redirection operators*/
			
			for(i=0; i<count; i++){
				if (*options[i]=='<'){

					char *fName;
                    fName = malloc(sizeof(char)*strlen(options[i+1]+1));
                    strcpy(fName, options[i+1]);

					int fdIN = open(fName,O_WRONLY|O_TRUNC|O_CREAT|S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR);

                    printf("print fName: %s\n", fName);
					dup2(fdIN, 0);
					close(fdIN); 
				}/*
				else if (options[i%count] == ">"){
					char *fName; 
					memcpy(fName, options[i+1]);
					int fdOUT = fopen(fName);
					dup2(fdOUT, 1);
					close(fdOut); 
				}
				else if (options[i%count] == ">"){
					char *fName; 
					memcpy(fName, options[i+1]);
					int fdOUT = fopen(fName);
					int fdERR = fopen(fName);
					dup2(fdOUT, 1);
					dup2(fdERR, 2);
					close(fdOUT); 
					close(fdERR); 
					

				}*/
			}

			execvp(options[0], options);
			/*free the data */
			j=count; 
			while (j--){
				free(options[j]); 
			}
			free (options);
 
			/*send in the command and options*/
			err_ret("couldn't execute: %s", buf);
			exit(127);
		}
		if ((pid = waitpid(pid, &status, 0)) < 0)
			err_sys("waitpid error");
		printf("%% "); 
	}/*end while loop*/	
	exit(0);
}/*end main*/

void
sig_int(int signo){
	printf("inturrpt\n%% ");
}
