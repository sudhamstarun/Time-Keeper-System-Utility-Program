/*
FileName: main.c
Student Name: Tarun Sudhams
Student Number: 3035253876
Development Platform: MACOSX 10.14 with gcc compiler and Sublime Text(tested under Ubuntu 18.04 
Compilation: gcc timekeeper_3015234567.c –o timekeeper
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
int pipeCounter = 0;        
int firstProcess = 0;   
int maintainPipes = 0;

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

void sigChildHandler(int signum, siginfo_t *signal, void *v)
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
}

void sigkill_handler(int signum)
{
    printf("Killed\n");
}

void sigIntHandler(int signum, siginfo_t *signal, void *v)
 {
    if(parentProcessPID == 0)
    {
        printf("I have received SIGINT. But I am not going to terminate. :)\n");
    }       
        
}

char *removeWhiteSpaces(char *inputChar)
{
    char *endChar;
    
    while(isspace(*inputChar))
    {
         inputChar++; 
    }
    if(*inputChar == 0)
    {
        return inputChar;
    } 
        
    endChar = inputChar + strlen(inputChar) - 1;

    while(endChar > inputChar && isspace(*endChar))
    {
        endChar--; 

    } 

    
    *(endChar+1) = 0;
    
    return inputChar;
}

char ** parsing(char mainInputCommand[1024])
{
    char * stringToken, *pointerTomainInputCommand, *pointerPosition;
    char ** stringTokenizers  = NULL;
    int stringTokenizersCounters;

    strcpy(mainInputCommand, removeWhiteSpaces(mainInputCommand));              
    if ((pointerPosition = strchr(mainInputCommand, '\n')) != NULL)
    {
        *pointerPosition = '\0';
    }               
                                    
    pointerTomainInputCommand = mainInputCommand;

    while ( (stringToken = strsep(&pointerTomainInputCommand, " ")) != NULL)
    {
        stringTokenizers = realloc (stringTokenizers, sizeof (char*) * ++stringTokenizersCounters); 
        if (stringTokenizers == NULL)
        {
            exit (-1); 
        }                       
            
        stringTokenizers[stringTokenizersCounters-1] = stringToken;                 
    }

    stringTokenizers = realloc (stringTokenizers, sizeof (char*) * (stringTokenizersCounters+1));       
    stringTokenizers[stringTokenizersCounters] = (char *)NULL;
    return stringTokenizers;
}


int main()
{
    char mainInputCommand[1024];
    char duplicateMainInputCommand[1024];
    char *stringToken;
    char *mainInputCommandTokenizer;
    char *pointerToMainInputCommand;
    char *duplicatePointerToMainInputCommand;
    char *pointerPosition;
    char str[50];
    char ** stringTokenizers  = NULL;
    
    int stringTokenizersCounter;
    int mainInputCommandCounter;
    int randomIntegerOne;
    int randomIntegerTwo;
    pid_t currentRunningProcessId;
    
    
    signal(SIGKILL, sigkill_handler);       
    struct sigaction SigChild;              
    sigaction(SIGCHLD, NULL, &SigChild);
    SigChild.sa_flags = SA_SIGINFO;
    SigChild.sa_sigaction = sigChildHandler;
    sigaction(SIGCHLD, &SigChild, NULL);
    
    struct sigaction SigInt;                
    sigaction(SIGINT, NULL, &SigInt);
    SigInt.sa_flags = SA_SIGINFO;
    SigInt.sa_sigaction = sigIntHandler;
    sigaction(SIGINT, &SigInt, NULL);

    while(1 == 1)
    {
        while(cont == 1)
        {
            char ** incomingCommands  = NULL;   
            stringTokenizersCounter = 0;
            mainInputCommandCounter = 0;
            childProcessPID = 0;
            
            printf("Enter your command here $ ");          
                
            if(fgets (mainInputCommand, 1024, stdin) != NULL && strcmp(mainInputCommand, "\n") != 0 )
            {
                strcpy(mainInputCommand, removeWhiteSpaces(mainInputCommand));

                if ((pointerPosition=strchr(mainInputCommand, '\n')) != NULL)
                {
                    *pointerPosition = '\0';
                }   
  
                strcpy(mainInputCommand, removeWhiteSpaces(mainInputCommand));   

                if(strlen(mainInputCommand) == 0)
                {
                    cont = 0;
                }      

                int fail = 0;

                if (strchr(mainInputCommand, '!') == NULL)
                {       
                    pipeCounter = 0;                
                }

                else
                {
                    strcpy(duplicateMainInputCommand, mainInputCommand);
                    duplicatePointerToMainInputCommand = duplicateMainInputCommand;
                    
                    while ((mainInputCommandTokenizer = strsep(&duplicatePointerToMainInputCommand, "!")) != NULL && fail == 0)
                    {    
                        incomingCommands = realloc (incomingCommands, sizeof (char*) * ++mainInputCommandCounter);      
                        
                        if (incomingCommands == NULL)
                        {
                            exit (-1); 
                        }                          
                
                        strcpy(mainInputCommandTokenizer, removeWhiteSpaces(mainInputCommandTokenizer));    
                        
                        if((int)strlen(mainInputCommandTokenizer) == 0)
                        {        
                            fail = 1;
                        }
                        
                        incomingCommands[mainInputCommandCounter-1] = mainInputCommandTokenizer;
                    }
                    
                    pipeCounter = mainInputCommandCounter - 1;              
                }
                
                if(fail == 1)
                {                          
                    printf("Please check your piping again\n");
                    cont = 0; 
                }

                pointerToMainInputCommand = mainInputCommand;

                while ( (stringToken = strsep(&pointerToMainInputCommand, " ")) != NULL)
                {           
                    stringTokenizers = realloc (stringTokenizers, sizeof (char*) * ++stringTokenizersCounter);  
                    
                    if (stringTokenizers == NULL)
                    {
                        exit (-1); 
                    }
                    
                    stringTokenizers[stringTokenizersCounter-1] = stringToken;                  
                }

                stringTokenizers = realloc (stringTokenizers, sizeof (char*) * (stringTokenizersCounter+1));        
                stringTokenizers[stringTokenizersCounter] = (char *)NULL;                   

                for (randomIntegerOne = 0; randomIntegerOne < stringTokenizersCounter; randomIntegerOne++)
                {
                    if(strcmp(stringTokenizers[randomIntegerOne], "&") == 0){               
                        fail = 1;                   
                        break;                          
                    }
                }
                

                if(strcmp(stringTokenizers[0], "exit") == 0)
                {                   
                    if(stringTokenizersCounter == 1)
                    {                       
      					//dotimeStatisticsTrigger((int) currentRunningProcessId);
                        exit(0);                        
                    }
                    else
                    {   
                        cont = 0;                       
                    }
                }
        
                if(strcmp(stringTokenizers[0], "timeStatisticsTrigger") == 0)
                
                {
                    timeStatisticsTrigger = 1;                              
                    int idx;
                    if(pipeCounter == 0)
                    {                           
                        for (idx = 1; idx <= stringTokenizersCounter; ++idx)
                        {
                            stringTokenizers[idx-1] = stringTokenizers[idx];
                        }
                    }
                    
                    else
                    {                                    
                        
                        strncpy(incomingCommands[0], incomingCommands[0]+6, strlen(incomingCommands[0])-4);                     
                    }
                }
                    
                

                if(pipeCounter == 0) // If there is no case of piping
                { 
                    currentRunningProcessId = fork();

                              
                    if(currentRunningProcessId==0)
                    {
                        if(childProcessPID == 1){   
                            sigset_t new;
                            sigemptyset(&new);
                            sigaddset(&new, SIGINT);
                            sigprocmask(SIG_BLOCK, &new, NULL);
                        } 
                        
                        printf("The process with process ID %d created for the command: %s\n",(int) getpid(), stringTokenizers[0]);
                        if (execvp(stringTokenizers[0], stringTokenizers) == -1)
                        { 
                            perror("myshell: Error");
                            exit(-1);
                        }
                    }
                    
                    else
                    {                           
                        if(childProcessPID != 1)
                        {   
                            parentProcessPID = (int)currentRunningProcessId;
                            cont = 0;   
                        }
                        

                        stringTokenizers = NULL;        
                        mainInputCommand[0] = '\0';
                        cont = 0;
                    } 

                }

                else
                {
                    currentRunningProcessId = fork();   

                    if(currentRunningProcessId==0)
                    {                           
                        int k = 0;  
                        int pipesArr[2 * pipeCounter]; 

                        for (k = 0; k < pipeCounter; k++)
                        {
                            pipe(pipesArr + k * 2); 
                        }   

                        int cmdExec = 0;

                        while (cmdExec <= pipeCounter)
                        {               
                            pid_t pipeChild = fork();

                            if(firstProcess == 0)                       
                            {
                                firstProcess = pipeChild;
                            }   
                                
                            if (pipeChild == 0)
                            {                           
                                if(childProcessPID == 1)
                                {       
                                    sigset_t new;
                                    sigemptyset(&new);
                                    sigaddset(&new, SIGINT);
                                    sigprocmask(SIG_BLOCK, &new, NULL);
                                }
                        
                                if (cmdExec != 0)
                                {
                                    dup2(pipesArr[(cmdExec - 1) * 2], 0); 
                                }               
                                    
                                if (cmdExec != pipeCounter)             
                                    dup2(pipesArr[cmdExec * 2 + 1], 1);     
                                for (k = 0; k < 2*pipeCounter; k++) 
                                    close(pipesArr[k]);     
                        
                                char ** stringTokenizers = parsing(incomingCommands[cmdExec]);

                                printf("The process with process ID %d created for the command: %s\n",(int) getpid(), stringTokenizers[0]);          

                                if (execvp(stringTokenizers[0], stringTokenizers) == -1)
                                 {              
                                    perror("myshell: Error");               
                                    exit(-1);
                                }
                            } 

                            else
                            {
                                if(setpgid(pipeChild, firstProcess) == -1)
                                {
                                	perror("Error"); // 
                                }              
                                                   
                         	}

                            cmdExec++;
                        }

                        for (k = 0; k < 2 * pipeCounter; k++)
                        {
                            close(pipesArr[k]);
                        } 
                                     
                    	cont = 0;
                    	//dotimeStatisticsTrigger((int) getpid());

                        if(childProcessPID == 0)
                        {               
                            sigset_t new;               
                            sigemptyset(&new);
                            sigaddset(&new, SIGCHLD);
                            sigprocmask(SIG_BLOCK, &new, NULL);
                        
                            for (k = 0; k <= pipeCounter; k++)
                            {
                                siginfo_t processInfo;
                                processInfo.si_pid = -1;
                                if(timeStatisticsTrigger == 1)
                                {
                                    while( waitid(P_PGID, firstProcess, &processInfo, WEXITED | WNOWAIT) < 0);  
                                    dotimeStatisticsTrigger(processInfo.si_pid);
                                                            
                                }
                                waitpid(processInfo.si_pid, NULL, 0);   
                            }
                            sigprocmask(SIG_UNBLOCK, &new, NULL);     

                            for(k = 0; k <= pipeCounter; k++)
                            {
                            	//dotimeStatisticsTrigger((int) getpid());
                            }              

                            exit(0);
                        }
                        
                    }

                    else
                    {
                        maintainPipes = (int)currentRunningProcessId;
                        //dotimeStatisticsTrigger(maintainPipes);
                       	exit(0);
                    }       
                }       
            }
        }
    }
}
