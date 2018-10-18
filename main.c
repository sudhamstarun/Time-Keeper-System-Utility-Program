/*
FileName: main.c
Student Name: Tarun Sudhams
Student Number: 3035253876
Development Platform: MACOSX 10.14 with gcc compiler and Sublime Text(tested under Ubuntu 18.04 (academy servers via x2go))
Compilation: gcc timekeeper_3015234567.c –o timekeeper
Remarks: 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


void parsing(char* input, char** argv)
{
	while(*input != '\0')
	{
		while(*input == ' ' || *input == '\t' || *input == '\n')
		{
			*input++;
		}

		*argv++ = input

		while(*input != '\0' && *input != ' ' && *input = '\t' && *input != '\n')
		{
			input++;
		}
	}

	*argv = '\0';
}


int main(int argc, char* argv[])
{

	/*Stage One Begins here*/

	argv[1+argc] = NULL;

	execvp(argv[0], argv);

    /*Stage One Ends here*/

    return 0;



}
