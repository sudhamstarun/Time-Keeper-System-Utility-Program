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
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

char* printProcessName(pid_t currentProcessID)
{
    char* name = (char*)calloc(1024,sizeof(char));

    if(name)
    {
        sprintf(name, "/proc/%d/cmdline",currentProcessID);
        FILE* f = fopen(name,"r");

        if(f)
        {
            size_t size;
            size = fread(name, sizeof(char), 1024, f);

            if(size>0)
            {
                if('\n'==name[size-1])
                {
                    name[size-1]='\0';
                }
            }
            fclose(f);
        }
    }

    return name;
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

void execution(char **argv)
{
	pid_t childProcessID, parentProcessID;
	int status;

	childProcessID = fork();

 	if (childProcessID == 0)
 	{
    	execvp(argv[0], argv);
    	perror("execve failed");
  	}

  	else if (childProcessID > 0)
  	{
        printf("Process with id: %d created for the command: %s\n", childProcessID-1, argv[0]);

    	if( (parentProcessID = wait(&status)) < 0)
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
    command = parsing(argc,argv);
    execution(command);
    //Stage One Ends Here

	return 0;
}
