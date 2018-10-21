/*
FileName: main.c
Student Name: Tarun Sudhams
Student Number: 3035253876
Development Platform: MACOSX 10.14 with gcc compiler and Sublime Text(tested under Ubuntu 18.04 (academy servers via x2go))
Compilation: gcc timekeeper_3015234567.c –o timekeeper
Remarks:
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h> 
#include <sys/stat.h>
#include <unistd.h>

const char* printProcessInfo(int pid)
{
	const int BUFSIZE = 4096; // should really get PAGESIZE or something instead...
	unsigned char buffer[BUFSIZE]; // dynamic allocation rather than stack/global would be better
	printf("PID value(+1)%d\n",pid+1);
	int fd = open("/proc/%d/status",pid+1);
	int nbytesread = read(fd, buffer, BUFSIZE);
	unsigned char *end = buffer + nbytesread;
	
	for (unsigned char *p = buffer; p < end; /**/)
	{ 
		puts(p);
	  	while (*p++); // skip until start of next 0-terminated section
	}

	close(fd);
}

char **parsing(int argc, char * argv[])
{

    int i; int strsize = 0;

    for (i=1; i < argc; i++)
    {
        strsize += strlen(argv[i]);
        if (argc > i+1)
            strsize++;
    }

    char *cmdstring;
    cmdstring = malloc(strsize);
    cmdstring[0] = '\0';

    for (i=1; i<argc; i++)
    {
        strcat(cmdstring, argv[i]);

        if (argc > i+1)
        {
            strcat(cmdstring, " ");
        }
    }

    char **command = malloc(8 * sizeof(char *));
    char *separator = " ";
    char *parsed;
    int index = 0;

    parsed = strtok(cmdstring, separator);
    while (parsed != NULL)
    {
        command[index] = parsed;
        index++;

        parsed = strtok(NULL, separator);
    }

    command[index] = NULL;

    return command;
}

void execution(char **argv, int argc)
{
	pid_t pid, ppid;
	int status;

	pid = fork();
	int i = 0;
 	if (pid == 0)
 	{
    	execvp(argv[0], argv);
    	perror("execve failed");
  	}

  	else if (pid > 0)
  	{
  		
  		for(i = 0; i < argc-1; i++)
  		{
  			if (strstr(argv[i], "-") == NULL)
  			{
				printf("Process with id: %d created for the command: %s\n", (int) getpid(), argv[i]);
			}
  		}
        	
        if( (ppid = wait(&status)) < 0)
    	{
      		perror("wait");
      		_exit(1);
    	}

    	printProcessInfo((int)getpid());

        printf("Parent: finished\n");
  	}

  	else
  	{
    	perror("Failed to Fork");
    	_exit(1);
  	}
}

int main(int argc, char *argv[])    
{
	if (argc < 2)
	{
		return 0;
	}

    //Stage One Begins Here
    char** command;
    command = parsing(argc,argv);
    execution(command, argc);
    //Stage One Ends Here

	return 0;
}
