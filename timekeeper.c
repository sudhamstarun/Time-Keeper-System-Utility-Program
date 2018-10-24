/*
FileName: main.c
Student Name: Tarun Sudhams
Student Number: 3035253876
Development Platform: MACOSX 10.14 with gcc compiler and Sublime Text(tested under Ubuntu 18.04) 
Compilation: gcc timekeeper_3035253876.c -o timekeeper
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
#include <ctype.h>


int parentProcessPID = 0;   
int childProcessPID = 0;
int timeStatisticsTrigger = 1;      
int cont = 1;       
int firstProcess = 0;
int pipeCounter = 0;   
int maintainPipes = 0;
int n = 0;

// Basic Signal handling function to catch signals
void signalhandler(int signum)
{
	printf("Signal %d is caught for %d times\n", signum, ++n);
}

//Harcoding the signals listing in order to prompt when required 
char *signame[]={"INVALID", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1", "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGPOLL", "SIGPWR", "SIGSYS", NULL};

//Basic function for signal handling
static void handleyoursignal(void)
{
	struct sigaction satmp;
	sigemptyset(&satmp.sa_mask);
	satmp.sa_flags = 0;
	satmp.sa_handler = signalhandler;
	sigaction(SIGINT, &satmp, NULL);
}

//Function to trigger time statistics and context switching
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

    printf("real: %0.2fs, user:%0.2fs, system: %0.2fs, context_switch: %d\n",uptime - (starttime)/sysconf(_SC_CLK_TCK), ut*1.0f/sysconf(_SC_CLK_TCK), st*1.0f/sysconf(_SC_CLK_TCK), final_value);
}


void execution(int argc, char *argv[])
{

	/* ------------ Declaring variables for statisics ------------ */
	handleyoursignal();

	/* ------------ End of Variables for statistics------------ */

	/* ------------ Declaring variables for execution and piping ------------ */

	int * mainInputCommands = (int *) malloc (sizeof(int) * argc);
	int counterOfPipe = 1;
	int pfd1[2];
	int pfd2[2];
	int currentRunningPID = 0;
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
// Adapted from the Lab 3  Piping and made
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
	pid_t childPIDs[randomIntegerOne];
    
    /* ------------Process Creation Happens Here------------ */
    
	for(int randomIntegerTwo = 0; randomIntegerTwo < randomIntegerOne; randomIntegerTwo++)
	{
		handleyoursignal();
		pid = fork();
		childPIDs[randomIntegerTwo] = pid;

		if(pid < 0)
		{
			printf("ERROR: Forking Failed\n");
		}

		else if (pid == 0)
		{
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
		close(pfd1[0]);    // closing both ends of the pipe opened earlier
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


	for(int randomIntegerTwo = 0; randomIntegerTwo < randomIntegerOne; randomIntegerTwo++)
	{
        /* ------------Handling the process output based on the interruption or termination of the process------------ */

		siginfo_t infop;
		infop.si_pid = -1;
		waitid(P_PID,(int) childPIDs[randomIntegerTwo], &infop, WEXITED | WNOWAIT | WSTOPPED);
	
		waitid(P_PID,(int) childPIDs[randomIntegerTwo], &infop, WEXITED | WNOWAIT | WSTOPPED);
		
		if(WEXITSTATUS(infop.si_status) == -1)
		{

			printf("Error executing the command\n");
		}


		else if(WIFSIGNALED(infop.si_status))
		{
			// adapted from https://stackoverflow.com/questions/16509614/signal-number-to-name
            
			printf("The command \"%s\" interrupted by signal number = %d (%s)\n", argv[counterOfPipe], WTERMSIG(infop.si_status), signame[WTERMSIG(infop.si_status)]);
			dotimeStatisticsTrigger(childPIDs[randomIntegerTwo]);
		}

		else
		{
			printf("The command \"%s\" terminated with returned status code = %d\n", argv[counterOfPipe] ,WEXITSTATUS(infop.si_status));
			dotimeStatisticsTrigger(childPIDs[randomIntegerTwo]);
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

int main(int argc, char* argv[])
{
   execution(argc, argv);

   return 0;
}
