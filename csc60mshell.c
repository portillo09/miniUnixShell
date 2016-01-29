/* Omar Portillo
 * CSC60
 * My own UNIX Shell
* Date: 11/21/2014
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLINE 80
#define MAXARGS 20
#define BUFFER_SIZE 50

char buffer[BUFFER_SIZE];

void handle_SIGINT() { 
 write(1,buffer,strlen(buffer)); 
} 

/* ------------------------------------------------------------------- */
/*           Perform system call execvp to execute command             */
/*                 No special operators(s) detected                    */
/*   	    Handle redirection operators: < , or  >, or both           */ 
/* ------------------------------------------------------------------- */
void process_input(int argc, char **argv)
{
  int rin=0; // redirect in "<"
  int rout=0; //  redirect out ">"
  int pipeAmount = 0; //"|"
  int pid[2];
  int pipe_fd[2];
  char *prog1_argv[MAXARGS];
  char *prog2_argv[MAXARGS];
  int lin=0; // location in "left" 
  int lout=0; // location out "right"
  int pipePos;
  int inputFd;
  int outputFd;
  int i =0; 
  int j=0;
  int k=0;

  for (i=0; argv[i]!= NULL; i++)
  {
    printf("show %s\n", argv[i]);
    if (strcmp("<",argv[i])==0)
    {	
		rin++;
		lin = i;
    }
    else if (strcmp(">",argv[i])==0)
    {
		rout++;
		lout = i;
    }
    else if (strcmp(argv[i],"|") == 0) {
		pipeAmount++;
		pipePos = i;
		argv[i] = NULL;
		}
  } // end for

  if (rin > 1 || rout > 1) //error two redirects
  {
     perror("ERROR - Can't have two redirects on one line.\n");
     _exit(-1);
  }
  else if ((lin > 0) && (argv[lin+1]==NULL)) //error no file input
  {
     perror ("ERROR - No redirection file specified.\n");
     _exit(-1);
  }
   else if ((lout > 0) && (argv[lout+1]==NULL)) //error no file output
  {
     perror ("ERROR - No redirection file specified.\n");
     _exit(-1);
  }
  else if ((rin > 0) && (lin==0)) // error no command input
  {
     printf("ERROR - No command. No worries file %s is only open for reading\n", argv[1]);
     _exit(-1);
  }
  else if ((rout > 0) && (lout==0)) //error no command output
  {
     printf("ERROR - No command. Make sure file %s is not overwritten.\n", argv[1]);
     _exit(-1);
  }
  
  else if (((lin > 0) && (argv[lin+1]!=NULL)) && ((lout > 0) && (argv[lout+1]!=NULL))) // valid redirection both
  {
     inputFd = open(argv[lin+1], O_RDONLY);
     if ( inputFd < 0)
     {
		perror("error opening file or files does not exist\n");
		_exit(-1);
     } 
     dup2(inputFd, 0);
     close(inputFd);
	 
	 outputFd = open(argv[lout+1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
     if (outputFd < 0)
     {
		perror("error opening or creating file\n");
		_exit(-1);
     }
     dup2(outputFd, 1);
     close(outputFd); 
  }
  
  else if ((lin > 0) && (argv[lin+1]!=NULL)) // valid redirection input
  {
     inputFd = open(argv[lin+1], O_RDONLY);
     if ( inputFd < 0)
     {
		perror("error opening file or files does not exist\n");
		_exit(-1);
     } 
     dup2(inputFd, 0);
     close(inputFd);
  }
   else if ((lout > 0) && (argv[lout+1]!=NULL)) // valud redirection output
  {
     outputFd = open(argv[lout+1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
     if (outputFd < 0)
     {
		perror("error opening or creating file\n");
		_exit(-1);
     }
     dup2(outputFd, 1);
     close(outputFd);		 
  }
  
  if (rin)
     argv[lin]=NULL;
  if (rout)
     argv[lout]=NULL;

  execvp(argv[0], argv);
  if (execvp(argv[0], argv) == -1)
  {                                       
     perror("Shell Program");                                    
     _exit(-1);                                                                                      
  }
} // end process input

/* ----------------------------------------------------------------- */
/*                  parse input line into argc/argv format           */
/* ----------------------------------------------------------------- */
int parseline(char *cmdline, char **argv)
{
  int count = 0;
  char *separator = " \n\t";
  argv[count] = strtok(cmdline, separator);
  while ((argv[count] != NULL) && (count+1 < MAXARGS)) {
   argv[++count] = strtok((char *) 0, separator);
  }
  return count;
}
/* ----------------------------------------------------------------- */
/*                  The main program starts here                     */
/* ----------------------------------------------------------------- */
int main(void)
{
 char cmdline[MAXLINE];
 char *argv[MAXARGS];
 int argc;
 int status;
 pid_t pid;

 /* Loop forever to wait and process commands */
 while (1)
 {
  /* Step 1: Name your shell: csc60mshell - m for mini shell */ 
  printf("miniShell> ");
  fgets(cmdline, MAXLINE, stdin);
  argc = parseline(cmdline, argv); // call parseline to parse commands and options
  if (argv[0] == NULL)
	continue;  // blank entry just gets ignored
  else if (strcmp("exit", argv[0] ) == 0)
	exit(0); // exits the mini shell built in commands
  else if (strcmp("cd", argv[0] ) == 0)
  {
	if (argv[1] == NULL)
	{
		chdir(getenv("HOME")); // cd chdir with no arguments defaults to home
		setenv("PWD", getenv("HOME"), 1);  // updates the PWD directory variable too
	}
	else if (argv[1]!= NULL)
	{
		if (chdir(argv[1]) != 0) // cd dir to desired path
			perror(argv[1]); // if not sucessful print error

		setenv("PWD", argv[1], 1); // updates the PWD directory variable too
	}
  }
  else if (strcmp("pwd", argv[0] ) == 0)
	printf("%s\n", getenv("PWD")); // print current working directory
  else
  {
 	pid = fork();
  	if (pid == -1) 
    	perror("Shell Program fork error");
  	else if (pid == 0) 
		/* I am child process. I will execute the command, call: execvp */
    	process_input(argc, argv);
  	else
   { 
		/* I am parent process */
    	if (wait(&status) == -1)
			perror("Shell Program error");
    	else
      	    printf("Child returned status: %d\n",status);
	}
  }
 } // end while
 return 0;  
} // end main
