/*
FileName: main.c
Student Name: Tarun Sudhams
Student Number: 3035253876
Development Platform: MACOSX 10.14 with gcc compiler and Sublime Text(tested under Ubuntu 18.04 
Remarks:
*/
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>


int parentProcessPID = 0;   
int childProcessPID = 0;
int timeStatisticsTrigger = 1;      
int cont = 1;       
int firstProcess = 0;   
int maintainPipes = 0;

void sigIntHandler(int signum, siginfo_t *signal, void *v)
 {
    if(parentProcessPID == 0)
    {
        printf("I have received SIGINT. But I am not going to terminate. :)\n");
    }       
        
}
void dotimeStatisticsTrigger(int processID)
{
    char inputStringOne[50];
    char state;
    char inputStringTwo[50];
    
    FILE * file;
    int z;
    unsigned long long i, x;
    unsigned long h, ut, st;
    unsigned long long int starttime;
    
    sprintf(inputStringOne, "/proc/%d/stat", processID);    
    file = fopen(inputStringOne, "r");

    if (file == NULL)
    {                                                                               
        printf("Error in open my proc file for %s\n", inputStringOne);  
        exit(0);
    }

    float uptime;                           
    FILE* proc_uptime_file = fopen("/proc/uptime", "r");        
    fscanf(proc_uptime_file, "%f", &uptime);
    
    FILE *filefile;
    char lineToBeProcessed[100];
    sprintf(inputStringTwo, "/proc/%d/status", processID);  
    filefile = fopen(inputStringTwo, "r");
    int One;
    int Two;
    int final_value;
    
    while (fgets(lineToBeProcessed, sizeof(lineToBeProcessed), filefile) != NULL)
    {
        char *endValue;
        char *registerIterator;
        char *registerIteratorValue;

        endValue = strchr(lineToBeProcessed, '\n');

        if (endValue != NULL)
        {
            *endValue = '\0'; 
        }

        endValue = strchr(lineToBeProcessed, ':');

        if (endValue != NULL)
        {
            endValue[0] = '\0';
            registerIterator = strdup(lineToBeProcessed);

            if (registerIterator == NULL)
            {
                continue;
            }

            endValue += 1;

            while ((endValue[0] != '\0') && (isspace((int) endValue[0]) != 0))
            {
                endValue++;
            }

            registerIteratorValue = strdup(endValue);

            if (registerIteratorValue != NULL)
            {
                if(strcmp(registerIterator,"voluntary_ctxt_switches") == 0)
                {
                    One = atoi(registerIteratorValue);
                }
                    

                if(strcmp(registerIterator,"nonvoluntary_ctxt_switches") == 0)
                {
                    Two = atoi(registerIteratorValue);
                }
                free(registerIteratorValue);
            }
            free(registerIterator);
        }
    }

    final_value = One + Two;

    fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu", &z, inputStringOne, &state, &z, &z, &z, &z, &z, (unsigned *)&z, &h, &h, &h, &h, &ut, &st, &h, &h, &h, &h, &h, &h, &starttime); //read all params and save the ones needed
    fclose(file);

    
    printf("\nThe Command '%s' termintaed  with return status code: %d\n", inputStringOne, 0);
    printf("real: %0.2fs, user:%0.2fs, system: %0.2fs, context_switch: %d\n",uptime - (starttime)/sysconf(_SC_CLK_TCK), ut*1.0f/sysconf(_SC_CLK_TCK), st*1.0f/sysconf(_SC_CLK_TCK), final_value);
}

/*void sigChildHandler(int signum, siginfo_t *signal, void *v)
 {
    int randomIntegerOne;

    if(parentProcessPID == signal->si_pid )
    {                           
        cont = 1;                               
        parentProcessPID = 0;

        if(timeStatisticsTrigger == 1)
        {                               
            dotimeStatisticsTrigger(signal->si_pid);
            exit(0);                                                     
        }
    }

    else if(signal->si_pid != parentProcessPID && pipeCounter == 0)
    {                                                                                               
        printf("[%d] 1Process Terminated!\n", signal->si_pid);      
        childProcessPID = 0;
    }
    
    if(signal->si_pid == maintainPipes)
    {                                                                           
        cont = 1;
        maintainPipes = 0;
    }
    
    if(getpgid(signal->si_pid) == firstProcess)
    {                                                                                                                   
        siginfo_t processInfo;

        while(waitid(P_PGID, firstProcess, &processInfo, WEXITED|WNOWAIT)==0)
        {                                                                                           
            if(waitpid(processInfo.si_pid, NULL, 0) == processInfo.si_pid)
            {                                                                                                           
                printf("[%d] 2Process Terminated!\n", processInfo.si_pid);  
            }
        }

        if(waitid(P_PGID, firstProcess, &processInfo, WEXITED|WNOWAIT) == -1)
        {                                                                                   
            exit(0);
        }
    }

    id_t child_pid = waitpid(signal->si_pid, NULL, 0);                  
}*/

void execution(int argc, char *argv[])
{

	/* ------------ Declaring variables for statisics ------------ */
	signal(SIGINT, SIG_IGN);

	/* ------------ End of Variables for statistics------------ */

	/* ------------ Declaring variables for execution and piping ------------ */

	int * mainInputCommands = (int *) malloc (sizeof(int) * argc);
	int counterOfPipe = 1;
	int pfd1[2];
	int pfd2[2];
	int currentRunningPID = 0;
	pid_t childPIDs[argc];
	pid_t pid;
	int counterOfArg = 0;
	int CurrentStatusOfPID;
	int randomIntegerOne = 0;

	/* ------------ End of Declaring Variables for execution and piping------------ */

	/* ------------ Execution and Piping Starts Here------------ */

	for(int randomIntegerTwo = 1; randomIntegerTwo < argc; randomIntegerTwo++)
	{
		if(strcmp(argv[randomIntegerTwo],"!") == 0)
		{
			mainInputCommands[randomIntegerOne] = randomIntegerTwo - counterOfPipe;
			randomIntegerOne = randomIntegerOne + 1;
			counterOfPipe = randomIntegerTwo + 1;
		}
	}

	mainInputCommands[randomIntegerOne] = argc - counterOfPipe;
	randomIntegerOne= randomIntegerOne + 1;

	/* ------------Different Cases for Piping Start Here----------- */

	if(randomIntegerOne == 2)
	{
		pipe(pfd1);
	}

	else if(randomIntegerOne > 2)
	{
		pipe(pfd1);
		pipe(pfd2);
	}

	counterOfPipe = 1;

	for(int randomIntegerTwo = 0; randomIntegerTwo < randomIntegerOne; randomIntegerTwo++)
	{
		pid = fork();

		if(pid < 0)
		{
			printf("ERROR: Forking Failed\n");
		}

		else if (pid == 0)
		{
			childPIDs[randomIntegerTwo] = pid;
			signal(SIGINT, sigIntHandler);
			printf("Process with id: %d created for the command: %s\n", (int) getpid(), argv[counterOfPipe]);
			break;
		}
	

		counterOfPipe += mainInputCommands[randomIntegerTwo] + 1;
		currentRunningPID++;
		sleep(1);
	}

if(pid > 1)
{
	if(randomIntegerOne == 2)
	{
		close(pfd1[0]);   // closing both ends of the pipe opened earlier
		close(pfd1[1]);
	}

	if(randomIntegerOne > 2)
	{
		close(pfd1[0]);
		close(pfd1[1]);
		close(pfd2[0]);
		close(pfd2[1]);
	}

	counterOfPipe = 1;


	for(int randomIntegerTwo = 0; randomIntegerTwo < randomIntegerOne; randomIntegerTwo)
	{
		pid_t child_pid = waitpid(-1, &CurrentStatusOfPID, 0);
	

		if(WEXITSTATUS(CurrentStatusOfPID) == -1)
		{
			printf("Error executing the command\n");
		}

		else
		{

			//dotimeStatisticsTrigger(child_pid);
			
		}

		counterOfPipe = counterOfPipe + mainInputCommands[randomIntegerTwo] + 1;
		sleep(1);
	}
}

sleep(1);

if (pid==0)
  {
    char ** argumentVector = (char **) malloc(sizeof(char*) * mainInputCommands[currentRunningPID]);

    for (int randomIntegerTwo = counterOfPipe; randomIntegerTwo < counterOfPipe + mainInputCommands[currentRunningPID]; randomIntegerTwo++)
    {
      argumentVector[counterOfArg] = (char *) malloc(sizeof(char) * strlen(argv[randomIntegerTwo]));
      strcpy(argumentVector[counterOfArg], argv[randomIntegerTwo]);
      counterOfArg++;
    }

    if (randomIntegerOne == 1)
    {

      if (execvp(argumentVector[0],argumentVector) == -1)
      {
        printf("execvp: No such file or directory\n");
        exit(-1);
      }
    }

    if (randomIntegerOne == 2)
    {

      if (currentRunningPID == 0)
      {
        close(pfd1[0]);
        dup2(pfd1[1], STDOUT_FILENO);
        close(pfd1[1]);
      }

      else if (currentRunningPID == 1)
      {
        close(pfd1[1]);
        dup2(pfd1[0], STDIN_FILENO);
        close(pfd1[0]);
      }

      if (execvp(argumentVector[0],argumentVector) == -1)
      {
        printf("execvp: No such file or directory\n");
        exit(-1);
      }
    }
    if (randomIntegerOne > 2)
    {
      if (currentRunningPID == 0)
      {
        close(pfd2[0]);
        close(pfd2[1]);
        close(pfd1[0]);
        dup2(pfd1[1], STDOUT_FILENO);
        close(pfd1[1]);
      }

      else if (currentRunningPID == randomIntegerOne - 1)
      {
        close(pfd1[0]);
        close(pfd1[1]);
        close(pfd2[1]);
        dup2(pfd2[0], STDIN_FILENO);
        close(pfd2[0]);
      }

      else
      {
        close(pfd1[1]);
        close(pfd2[0]);
        dup2(pfd1[0], STDIN_FILENO);
        dup2(pfd2[1], STDOUT_FILENO);
        close(pfd1[0]);
        close(pfd2[1]);
      }

      if (execvp(argumentVector[0],argumentVector) == -1)
      {
        printf("execvp: No such file or directory\n");
        exit(-1);
      }
    }
    exit(0);
   }
}

void SIGKILLHandler(int signum)
{
    printf("Killed\n");
}

int main(int argc, char* argv[])
{
   execution(argc, argv);

   return 0;

}
