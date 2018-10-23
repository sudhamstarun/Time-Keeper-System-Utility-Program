#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>



int fground = 0;  //Store the PID of process running in foreground
int bground = 0;
int timeX = 1;    //mark with 1 if timeX is to be performed
int cont = 1;   //when cont = 1, user is prompted for new input
int noOfPipes = 0;    //number of pipes involved in piping seq
int leader = 0;   //Process group leader for pipes processes
int pipeHandler = 0;

void sigint_handler(int signum, siginfo_t *sig, void *v) {
  if(fground == 0)    //means no process running in the foreground, just print new line to get behavior 
    printf("\n");
}

void dotimeX(int procID){
  char str[50], state;
  FILE * file;
  int z;
  unsigned long long i, x;
  unsigned long h, ut, st;
  
  unsigned long long int starttime;
  
  sprintf(str, "/proc/%d/stat", procID);  //store file name in str
  file = fopen(str, "r");     //open file
  if (file == NULL) {     //if file is null, failed to open, so proc unavailable
    printf("Error in open my proc file for %s\n", str); //show error and exit
    exit(0);
  }

  float uptime;             
  FILE* proc_uptime_file = fopen("/proc/uptime", "r");    //get current time since system up to calculate runtime of process
  fscanf(proc_uptime_file, "%f", &uptime);

  fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu", &z, str, &state, &z, &z, &z, &z, &z, (unsigned *)&z, &h, &h, &h, &h, &ut, &st, &h, &h, &h, &h, &h, &h, &starttime); //read all params and save the ones needed
  fclose(file);

  printf("\nPID \t CMD \t\t RTIME \t\t UTIME \t\t STIME \n");
  printf("%d \t %-10s \t %0.2f s \t %0.2f s \t %0.2f s \n", procID, str, uptime - (starttime)/sysconf(_SC_CLK_TCK), ut*1.0f/sysconf(_SC_CLK_TCK), st*1.0f/sysconf(_SC_CLK_TCK));
}

void sigchild_handler(int signum, siginfo_t *sig, void *v) {
  int i;
  if(fground == sig->si_pid ){              //fground process
    cont = 1;               //ask user for prompt again
    fground = 0;  
    if(timeX == 1){               //if fground process without pipe exits and timex is 1
      dotimeX(sig->si_pid);           //perform timeX
      timeX = 0;
    }
  }else if(sig->si_pid != fground && noOfPipes == 0){       //non-piping bground process
    printf("[%d] 1Process Terminated!\n", sig->si_pid);   
    bground = 0;
  }
  
  if(sig->si_pid == pipeHandler){             //if the pipe handler myshell exited, reset some variables to indicate end of pipe
    cont = 1;
    pipeHandler = 0;
    timeX = 0;
  }
  
  if(getpgid(sig->si_pid) == leader){           //if the ended command belongs to the process group, it is a piping cmd. 
    siginfo_t infop;
    while(waitid(P_PGID, leader, &infop, WEXITED|WNOWAIT)==0){    //Look for other processes that may have ended in the process group. 
      if(waitpid(infop.si_pid, NULL, 0) == infop.si_pid){   //waitpid on them
        printf("[%d] 2Process Terminated!\n", infop.si_pid);  //Since foreground piping handled below, all these processes are bground processes so must output terminated statement
      }
    }

    if(waitid(P_PGID, leader, &infop, WEXITED|WNOWAIT) == -1){    //if no more processes in the process group, piping cmds have ended. exit on the pipe handler
      exit(0);
    }
  }
  id_t child_pid = waitpid(sig->si_pid, NULL, 0);         //waitpid on all other processes
}

void sigusr1_handler(int signum) {} //Defines a signal handler for SIGUSR1 that is used by the child process to unpause and start execution

void sigusr2_handler(int signum, siginfo_t *sig, void *v) { //When the parent receives the SIGUSR2 command, it signifies that the child is ready for execution.
  kill(sig->si_pid, SIGUSR1); //send the SIGUSR1 to the child process to unblock the pause and start execution.
}           

void sigkill_handler(int signum) {
  printf("Killed\n");
}

//adapted from http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char *trimwhitespace(char *firstChar)
{
  char *lastChar;
  while(isspace(*firstChar)) firstChar++; //move the pointer to the first character to the right if space found
  if(*firstChar == 0) //if all spaces . . .  
    return firstChar;
  lastChar = firstChar + strlen(firstChar) - 1;
  while(lastChar > firstChar && isspace(*lastChar)) lastChar--; //move the pointer to the last character to the left if space found
  *(lastChar+1) = 0;
  return firstChar;
}

char ** tokenize(char command[1024]){
  char *token, *pointerToComm, *pos;
  char ** tokens  = NULL;
  int noOfTokens;

  strcpy(command, trimwhitespace(command));         //remove trailing and leading spaces
  if ((pos=strchr(command, '\n')) != NULL)        //remove trailing newline char from command
    *pos = '\0';              //replace with EOF char
  pointerToComm = command;
  while ( (token = strsep(&pointerToComm, " ")) != NULL){
    tokens = realloc (tokens, sizeof (char*) * ++noOfTokens); //increase size of tokens array 
    if (tokens == NULL)           //if size inc fail, exit 
      exit (-1); 

    tokens[noOfTokens-1] = token;         //copy tokens to array
  }
  tokens = realloc (tokens, sizeof (char*) * (noOfTokens+1));   //add the NULL char in the end of array for execvp
  tokens[noOfTokens] = (char *)NULL;
  return tokens;
}

/*
General logic of the program: Outer while loop in main keeps the program running forever, while the inner for loop is to show the myshell prompt to the user (this is done so that inner loop can be toggled to control the idea of a background and foreground process. 
*/


int main() {
  char command[1024], commandCopy[1024], *token, *commandTok, *pointerToComm, *pointerToCommCopy, *pos, str[50];
  char ** tokens  = NULL;
  
  int noOfTokens, noOfCommands, j, z;
  pid_t who;
  
  /* ------------ END OF VAR DECLARATION ------------ */
  
  /* ------------ ATTACH SIGNAL HANDLERS ------------ */
  signal(SIGUSR1, sigusr1_handler);   //SIGUSR1 handler
  signal(SIGKILL, sigkill_handler);   //SIGKILL handler

  struct sigaction sc;        //SIGCHILD handler
  sigaction(SIGCHLD, NULL, &sc);
  sc.sa_flags = SA_SIGINFO;
  sc.sa_sigaction = sigchild_handler;
  sigaction(SIGCHLD, &sc, NULL);
  
  struct sigaction si;        //SIGINT handler
  sigaction(SIGINT, NULL, &si);
  si.sa_flags = SA_SIGINFO;
  si.sa_sigaction = sigint_handler;
  sigaction(SIGINT, &si, NULL);
  
  struct sigaction su2;       //SIGUSR2 handler
  sigaction(SIGUSR2, NULL, &su2);
  su2.sa_flags = SA_SIGINFO;
  su2.sa_sigaction = sigusr2_handler;
  sigaction(SIGUSR2, &su2, NULL);
  /* ------------ END OF SIGNAL HANDLERS ATTACHMENT ------------ */

  while(1 == 1){
    while(cont == 1){
      char ** commands  = NULL;       //reset all variables to represent a fresh command
      noOfTokens = 0;
      noOfCommands = 0;
      bground = 0;
      
      printf("## myshell $ ");      
        
      if(fgets (command, 1024, stdin) != NULL && strcmp(command, "\n") != 0 )
      { //checks if the input is valid and not just a new line
        /* ------------ Prepare string for tokenization ------------ */
        strcpy(command, trimwhitespace(command));   //remove trailing and leading spaces    
        if ((pos=strchr(command, '\n')) != NULL)    //remove newline char due to pressing enter from command
          *pos = '\0';  
          
        if ((pos=strchr(command, '&')) != NULL){  //find position of first &
          if((int)strlen(pos) == 1){    //if its the last character . . . .
            *pos = '\0';      //. . . . replace with EOF char
            bground = 1;      //set variable to mark this process as a bground process
          }
        }
        strcpy(command, trimwhitespace(command));   //remove trailing and leading spaces  
        if(strlen(command) == 0) continue;    //empty line i.e. enter pressed or input was a bunch of spaces or input was just a '&'
        /* ------------ END OF PREPARATION ------------ */
      
        /* ------------ Handle Piping------------ */
        int fail = 0;
        if (strchr(command, '|') == NULL) {   //look for the piping character
          noOfPipes = 0;        //if not found, no piping = simple execution
        }
        else
        {
          strcpy(commandCopy, command);
          pointerToCommCopy = commandCopy;
          while ((commandTok = strsep(&pointerToCommCopy, "|")) != NULL && fail == 0)
          {  //tokenize the string based on the | character to separate all the commands
            commands = realloc (commands, sizeof (char*) * ++noOfCommands);   //grow the commands array to fit new command
            if (commands == NULL)             //if realloc fail, exit myshell
              exit (-1); 
        
            strcpy(commandTok, trimwhitespace(commandTok)); //trim whitespace to prepare command for tokenisation 
            if((int)strlen(commandTok) == 0){   //if any command token is empty, the piping seq is incomplete. mark the fail variable to indicate this
              fail = 1;
            }
            
            commands[noOfCommands-1] = commandTok;
          }
          noOfPipes = noOfCommands - 1;       //save number of pipes (to be used in piping later)
        }
        
        if(fail == 1){              //if fail=1, piping seq is incomplete so show error and ask for new command
          printf("myshell: Incomplete '|' sequence\n");
          continue; 
        }
        /* ------------ END OF PIPING ------------ */
        
        /* ------------ Start of Tokenization ------------ */
        pointerToComm = command;            
        while ( (token = strsep(&pointerToComm, " ")) != NULL)
        {     //Seperate the command using ' ' as the delim
          tokens = realloc (tokens, sizeof (char*) * ++noOfTokens); //grow the tokens array
          if (tokens == NULL)
            exit (-1); 
          
          tokens[noOfTokens-1] = token;         
        }
        tokens = realloc (tokens, sizeof (char*) * (noOfTokens+1));   //add one more space for the NULL needed at the end of the execvp array
        tokens[noOfTokens] = (char *)NULL;          //add NULL to the end of the array
        /* ------------ END OF TOKENIZATION ------------ */
        
        /* ------------ Handling & ------------ */
        for (j = 0; j < noOfTokens; j++){
          if(strcmp(tokens[j], "&") == 0){        //Look for the & in middle of the command
            fail = 1;             //if found, set fail to 1 to show error to user and break execution
            break;              //exit from the for loop since & is already found
          }
        }
        
        if(fail == 1){                //if & found in the middle, show error and then ask user for new input
          printf("myshell: '&' should not appear in the middle of the command line\n");
          continue; 
        } 
        /* ------------ END OF HANDLING & ------------ */
        
        /* ------------ Handling EXIT ------------ */
        if(strcmp(tokens[0], "exit") == 0){         //Check if first token is exit
          if(noOfTokens == 1){            //Exit must be standalone
            printf("myshell: Terminated\n");
            exit(0);            //exit successfully
          }
          else{ 
            printf("myshell: \"exit\" with other arguments!!!\n");  //There is more than 1 token i.e. exit is not alone
            continue;           //Prompt user for new cmd
          }
        }
        /* ------------ END OF EXIT HANDLING ------------ */

        /* ------------ Handling timeX ------------ */      
        if(strcmp(tokens[0], "timeX") == 0)
        {                //Check if first token is timeX
          if(noOfTokens == 1)
          {                  //Error if only one token, since timeX is not a standalone func
            printf("myshell: \"timeX\" cannot be a standalone command\n");
            continue;                 //go back user prompt
          }
          else
          { 
            if(bground  == 1)
            {                //Error if bground & specified, since timeX can only run in foreground
              printf("myshell: \"timeX\" cannot be run in background mode\n");
              bground = 0;
              continue;
            }
            else
            {
              timeX = 1;                //indicate that timeX must be performed on processes
              int idx;
              if(noOfPipes == 0)
              {             //remove the "timeX" token so that tokenization is not affected
                for (idx = 1; idx <= noOfTokens; ++idx)
                  tokens[idx-1] = tokens[idx];
              }
              else{                   
                //piping, so final tokens have not been created yet. Hence, timeX must be removed from the command string and not the tokens array.
                strncpy(commands[0], commands[0]+6, strlen(commands[0])-4); //get substring without "timeX"           
              }
            }
          }
        }
        /* ------------ END OF timeX HANDLING ------------ */ 
      
        /* ------------ Command Execution ------------ */     
        if(noOfPipes == 0){ 
          who = fork();       //Non-piping execution
          if(who==0){
            if(bground == 1){   // Add a signal mask for SIGINT for background processes so that they ignore the SIGINT
              sigset_t new;
              sigemptyset(&new);
              sigaddset(&new, SIGINT);
              sigprocmask(SIG_BLOCK, &new, NULL);
            }
            //kill((int)getppid(), SIGUSR2); //Tell the parent that I am ready to execute my command
            //pause(); //Pause, wait for SIGUSR1 from parent. Once received, proceed to exec command.
            if (execvp(tokens[0], tokens) == -1) { 
            //replace current image with the image of the program we want to execute
              perror("myshell: Error");
              exit(-1);
            } 
          }
          else{       //in parent       
            if(bground != 1){ //if process is a foreground process
              fground = (int)who;
              cont = 0; //set cont=0 to stop myshell from prompting for next input
            }
            //sleep(1);   //for implementing SIGUSR1, must sleep

            tokens = NULL;    //reset commands and tokens variable
            command[0] = '\0';
          } 
        }else{
          who = fork();       
          if(who==0){             
            int k = 0;  
            int pipesArr[2 * noOfPipes];          //allocate 2 pipe ends for each pipe present in the command
            for (k = 0; k < noOfPipes; k++) pipe(pipesArr + k * 2);   //create the pipes

            int cmdExec = 0;
            while (cmdExec <= noOfPipes)
            {        
              pid_t pipeChild = fork();       //create new child for each pipe command          
              if(leader == 0)           //if a group leader is not defined, make this process the group leader
                leader = pipeChild;
                
              if (pipeChild == 0) {               //in the child
                if(bground == 1){   // Add a signal mask for SIGINT for background processes
                  sigset_t new;
                  sigemptyset(&new);
                  sigaddset(&new, SIGINT);
                  sigprocmask(SIG_BLOCK, &new, NULL);
                }
            
                if (cmdExec != 0)         //for all commands except the first, replace the stdin with the appropriate pipe-end
                  dup2(pipesArr[(cmdExec - 1) * 2], 0); 
                if (cmdExec != noOfPipes)       //for all commands except the last, replace the stdout with the appropriate pipe-end
                  dup2(pipesArr[cmdExec * 2 + 1], 1);   
                for (k = 0; k < 2*noOfPipes; k++) 
                  close(pipesArr[k]);   
            
                char ** tokens = tokenize(commands[cmdExec]);     //create tokens from the command to be executed
  
                //kill((int)getppid(), SIGUSR2); //Tell the parent that I am ready to execute my command
                //pause(); //Pause, wait for SIGUSR1 from parent. Once received, proceed to exec command.             
                if (execvp(tokens[0], tokens) == -1) {        //replace current image with the image of the program we want to execute
                  perror("myshell: Error");       //if error, show to user and exit
                  exit(-1);
                }
              } else {
                if(setpgid(pipeChild, leader) == -1)        //move all child process to new process group to track later
                  perror("myshell: Error");       //show error to user if cant move to PG
              }
              cmdExec++;
            }
            for (k = 0; k < 2 * noOfPipes; k++) 
              close(pipesArr[k]);     //close all pipe ends in parent
          
            if(bground == 0)
            {       //In foreground piped processes
              sigset_t new;       //Adds a signal mask for SIGCHLD to stop the signal handler from being invoked
              sigemptyset(&new);
              sigaddset(&new, SIGCHLD);
              sigprocmask(SIG_BLOCK, &new, NULL);
            
              for (k = 0; k <= noOfPipes; k++){
                siginfo_t infop;
                infop.si_pid = -1;
                if(timeX == 1){
                  while( waitid(P_PGID, leader, &infop, WEXITED | WNOWAIT) < 0);  //Wait for process to exit but leave in process table
                  dotimeX(infop.si_pid);            //perform timeXs on waited process
                }
                waitpid(infop.si_pid, NULL, 0); //if timeX=0, pid is -1 so basically wait for any child. If timex=1, waitpid on the child exited above 
              }
              timeX = 0;
              sigprocmask(SIG_UNBLOCK, &new, NULL);         //Remove SIGCHLD mask
              exit(0);
            }
            
          }
          else
          {
            pipeHandler = (int)who;
            if(bground != 1)  //if process is a foreground process
              cont = 0; //set cont=0 to stop myshell from prompting for next input
          }   
        }
        /* ------------ End of Command Execution ------------ */      
      }
    }
  }
}
