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
#include <string.h>
#include <stdlib.h>

char **parse(int argc, char * argv[])
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

void execute(char **argv)
{
	pid_t c_pid, pid;
	int status;

	c_pid = fork();

 	if (c_pid == 0)
 	{


    	printf("Child: executing\n");

    	//execute ls
    	execvp(argv[0], argv);
    	//only get here if exec failed
    	perror("execve failed");
  	}

  	else if (c_pid > 0)
  	{


    	if( (pid = wait(&status)) < 0)
    	{
      		perror("wait");
      		_exit(1);
    	}

    printf("Parent: finished\n");

  	}

  	else
  	{
    	perror("fork failed");
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
    command = parse(argc,argv);
    execute(command);
    //Stage One Ends Here

	return 0;
}
