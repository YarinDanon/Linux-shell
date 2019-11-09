//yarin danon
//305413122
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
void redirection(char* str ,char* c, int* sumCmd , int* lengthCmd);
void freeTok(char **array);
char** cmdToken(char* str);

//Handles the case of control c
void sigHandler()
{
	signal(SIGINT,sigHandler);
	signal(SIGCHLD,sigHandler);
	while(waitpid(-1,NULL,WNOHANG)>0);
}

//Checks whether the character '&' exists at the end of the string
int checkAmpersand(char* str)
{
	char backup[510];
	strcpy(backup,str);	
	char* ptr = strtok(backup," ");
	
	while(ptr != NULL)
	{
		if(strcmp("&\n",ptr) == 0)
		{
			ptr = strtok(NULL," ");
			if(ptr == NULL)
				return 1;
		}
		else
			ptr = strtok(NULL," ");
	}
	
	return 0;

}

//Checks whether the character '>','>>','<','2>' exists at the string
int checkRedirection(char* str , char* c)
{
char backup[510];

strcpy(backup,str);
strcpy(c,str);

char* ptr ;
ptr = strtok(backup," \n");

int argc = 0;

while(ptr != NULL)
{
	
	if(strcmp(">",ptr) == 0)
	{
		strcpy(c,">");
		argc++;
	}
	else if(strcmp(">>",ptr) == 0)
	{
		strcpy(c,">>");
		argc++;
	}
	if(strcmp("<",ptr) == 0)
	{
		strcpy(c,"<");
		argc++;
	}
	if(strcmp("2>",ptr) == 0)
	{
		strcpy(c,"2>");
		argc++;
	}
	ptr = strtok(NULL," ");
}
return argc;
}

//Checks whether the character '|' exists at the string
int cheakPipe(char* str)
{
char backup[510];
strcpy(backup,str);

char* ptr = strtok(str,"|");
int argc = 0;

if(strlen(ptr) != strlen(backup))
	argc++;
return argc;		
}

//free memory
void freeTok(char **array)
{
	
	for (int i = 0; array[i] != NULL; i++)
		free(array[i]);
	free(array);
}

//the method receives a string, disintegrates the string by spaces, then enters each word into each cell
char** cmdToken(char* str)
{
	str[strlen(str)-1] = '\0';	
	char backup[510];
	strcpy(backup,str);

	char* ptr = strtok(str," ");
	int argc = 0;

	while(ptr != NULL){
		argc++;
		ptr = strtok(NULL," ");
	}

	char** argv = (char**)(malloc((argc+1)*sizeof(char*)));
	if(argv == NULL)
	{
		printf("ERR");
		exit(1);
	}
	argv[argc] = NULL;

	int i = 0;
	int length = 0;
	ptr = strtok(backup," ");
	while(ptr != NULL){
		length = strlen(ptr);
		argv[i] = (char*)malloc((length+1)*sizeof(char));
		if(argv[i] == NULL)
	{
			printf("ERR");
			freeTok(argv); 
			exit(1);
	}
		strcpy(argv[i],ptr);
		argv[i][strlen(ptr)] = '\0';
		ptr = strtok(NULL," ");
		i++;

	}
	return argv;
}

 //method that print to the CMD. 
void print()
{

	uid_t uid=0;
	char dir[512];
	struct passwd *pwd = getpwuid(uid);
	getcwd(dir,sizeof(dir));
	
	if(pwd == NULL || dir == NULL)
		exit(1);

	printf("%s@%s>",pwd->pw_name,dir);
}

//Handles case of pipe and Divides the string into 2 arrays left (the left side from the pipe)  and right (the right side from the pipe) 
void goToPipe(char* backup, int* sumCmd , int* lengthCmd)
{		
			char* left; 
			char* right;
			
			left = strtok(backup,"|");	
			right = strtok(NULL,"|");
			
			char rightBackup[510];
			stpcpy(rightBackup,right);
			
			//Divides the string into 2 arrays
			char** leftPipe = cmdToken(left);
			char** rightPipe = cmdToken(right);
			
			char c[2];
			int flag = 0;
			int file;
			int fd[2];
			pid_t leftPid, rightPid;
			pipe(fd);
			
			//Checking whether there is a case of Redirection in the string if yes find the Right case.
			if(checkRedirection(rightBackup,c) == 1)
			{		
        		flag = 1;				
				char* command = strtok(rightBackup,c);
                char* fileName = strtok(NULL,"\n");				
				if(strcmp(c,">")==0)
					file = open ((fileName+1),O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);				
				if(strcmp(c,">>")==0)
					file = open ((fileName+2),O_WRONLY|O_CREAT|O_APPEND,0600);				
				if(strcmp(c,"2>")==0)
					file = open ((fileName+2),O_WRONLY|O_CREAT|O_TRUNC,0600);
				freeTok(rightPipe);
				rightPipe = cmdToken(command);
			}	
			
			
			//fork for the left side
			if((leftPid = fork()) == 0)
			{
				close(fd[0]);
				dup2(fd[1] , STDOUT_FILENO);
				close(fd[1]);
				if(execvp(leftPipe[0],leftPipe) < 0)
				{
					printf("%s",leftPipe[0]);
					printf(":left command not found \n");
					exit(1);
				}
			}
			//fork for the right side;
			else if((rightPid = fork()) == 0)
			{
				close(fd[1]);
				dup2(fd[0], STDIN_FILENO);	

				//case of Redirection
				if(flag == 1)
				{
					if((strcmp(c,">") == 0) || (strcmp(c,">>") == 0))
						dup2(file,STDOUT_FILENO);
											
					else
					{
						if((strcmp(c,"<") == 0))
							exit(1);
						dup2(file,STDERR_FILENO);
					}
				}			
				close(fd[0]);				
				if((execvp(rightPipe[0],rightPipe))<0)
				{
					printf("%s",leftPipe[0]);
					printf(":right command not found \n");
					exit(1);
				}
			}
			else
			{
				(*sumCmd)++;
				(*lengthCmd)+= strlen(leftPipe[0]);
				close(fd[0]);
				close(fd[1]);
				wait(NULL);
				wait(NULL);
				freeTok(leftPipe);
				freeTok(rightPipe);
			}
}

//Handles all types of redirection and open the current file.
void redirection(char* str ,char* c, int* sumCmd , int* lengthCmd)
{
	char backup[510];
	strcpy(backup,str);

	char* left;
	char* right;

	left = strtok(backup,c);
	right = strtok(NULL,"\n");

	char** leftExe = cmdToken(left);
	char* ptr = strtok(str," \n >> > < 2> ");

	(*sumCmd)++;
	(*lengthCmd)+= strlen(leftExe[0]);

	int fd ;
	pid_t leftPid;

	if((leftPid = fork()) == 0)
	{
		//Checking what type redirection
		while(ptr != NULL)
		{
			if(strcmp(">",ptr) == 0)
			{
				fd = open ((right+1),O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);
				dup2(fd,STDOUT_FILENO);
			}

			if(strcmp(">>",ptr) == 0)
			{
				fd = open ((right+2),O_WRONLY|O_CREAT|O_APPEND,0600);
				dup2(fd,STDOUT_FILENO);
			}
			if(strcmp("<",ptr) == 0)
			{
				fd = open ((right+1),O_RDONLY,0600);
				if(fd < 0)
				{
					perror("no such file");
					exit(1);
				}
				dup2(fd,STDIN_FILENO);
			}
			if(strcmp("2>",ptr) == 0)
			{
				fd = open ((right+2),O_WRONLY|O_CREAT|O_TRUNC,0600);
				dup2(fd,STDERR_FILENO);
			}
			ptr = strtok(NULL," ");
		}
		
		if((execvp(leftExe[0],leftExe) )< 0) //checks if the commend is leagle
			{
				close(fd);
				printf("%s",leftExe[0]);
				printf(": command not found \n");
				exit(1);	
			}
			
			close(fd);
	}	
		if(leftPid < 0)
		{
			printf("ERR\n");
			exit(1);
		}
		else
			wait(NULL);
		
		freeTok(leftExe);
		
	   

}

//run a processes in the background Given the mark '&' at the end of the string.
void ampersand(char* str, int* sumCmd , int* lengthCmd)
{
	char* newStr = strtok(str,"&");
	char** arr = cmdToken(newStr);
	pid_t pid;
	pid = fork();
	if(pid < 0)
		exit(1);
	if(pid == 0)
	{
		if(execvp(arr[0],arr) < 0) //checks if the commend is leagle
		{	
			printf("%s",arr[0]);
			printf(": command not found \n");
			exit(1);
		}			
	}
	else //update data and freeing the memory.
	{
		(*sumCmd)++;
		(*lengthCmd)+= strlen(arr[0]);
		freeTok(arr);		
	} 
}

int main()
{
	
	signal(SIGINT,sigHandler);
	signal(SIGCHLD,sigHandler);
	int lengthCmd = 0;
	int sumCmd = 0;
	char str[510] ;
	pid_t pid;

	while(1)
	{
		print();
		fgets(str , 510 , stdin);		
		char backup[510];
		char backup2[510];
		char backup3[510];
		strcpy(backup,str);
		strcpy(backup2,str);
		strcpy(backup3,str);
		
		if(strcmp(str,"\n") == 0)
			continue;
		
		if((cheakPipe(str)) == 1)
		{
			goToPipe(backup , &sumCmd , &lengthCmd);
			continue;
		}
		char c[2];
		if((checkRedirection(str , c) == 1))
		{
			redirection(backup , c, &sumCmd , &lengthCmd);
			continue;
		}
		
		if(checkAmpersand(backup2) == 1)
		{
			printf("good\n");
			ampersand(backup2,&sumCmd,&lengthCmd);
			continue;
		}
		
		char** arr = cmdToken(backup3);
		
		if(strcmp(arr[0],"done") == 0) // ending the program.
		{
			printf("Num of cmd = %d \n" , sumCmd);
			printf("Cmd length = %d \n" , lengthCmd);
			printf("Bye ! \n");
			freeTok(arr);
			exit(1);
			
		}	
		if(strcmp(arr[0],"cd") == 0 )//changing current file.
		{
			chdir(arr[1]);
			freeTok(arr);
			continue;
		}
		
		
		pid = fork();
		
		if(pid < 0)
			exit(1);
		if(pid == 0)
		{
			if(execvp(arr[0],arr) < 0) //checks if the commend is leagle
			{
				
				printf("%s",arr[0]);
				printf(": command not found \n");
				exit(1);
			}
			
		}
		else //update data and freeing the memory.
		{
			wait(NULL);
			sumCmd++;
			lengthCmd+= strlen(arr[0]);
			freeTok(arr);
		} 
	}
return 0;
}


