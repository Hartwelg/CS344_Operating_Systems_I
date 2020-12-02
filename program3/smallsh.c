//references: stackoverflow, other students, class readings, lectures
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

//stores input from command line
char input[3000];
//array of arguments
char* arguments[512];
//used to hold place of next argument
int numArguments = 0;
//used to access current argument
char* argPiece;
//holds current directory for cd command
char curDir[3000];
//self explanatory
int isBackgroundProcess = 0;
//checks whether background processes are allowed
int isBackgroundAllowed = 1;
//array of child process id's
int childProcesses[100];
//size of childProcesses
int numChildProcesses;
//current exit status, used for status command
int curStatus = 0;
//file descriptor for redirection
int fileDesc;
//input file
char* inFile = NULL;
//output file
char* outFile = NULL;

//check all child processes
void checkAllChildProcesses()
{
	int i = 0;
	//loop through array of child process id's to check which have been exited
	for (i = 0; i < numChildProcesses; i++)
	{
		pid_t curResult = -5;
		if (childProcesses[i] != -5)
		{
			pid_t curResult = waitpid(childProcesses[i], &curStatus, WNOHANG);
		}
		if (curResult > 0)
		{
			//set this to -5 to show that it has exited
			childProcesses[i] = -5;
			if (WIFEXITED(curStatus))
			{
				printf("Child %d exited with status: %d\n", childProcesses[i], WEXITSTATUS(curStatus));
				fflush(stdout);
			}
			else if (WIFSIGNALED(curStatus))
			{
				printf("Child %d exited with status: %d\n", childProcesses[i], WTERMSIG(curStatus));
				fflush(stdout);
			}
		}
	}
}
	
//exit the shell
int exitShell()
{
	int i = 0;
	//loop through array of child processes to kill each one
	for (i = 0; i < numChildProcesses; i++)
	{
		pid_t curPid = childProcesses[i];
		if (curPid != -1)
		{
			//kill the process
			kill(curPid, SIGINT);
		}
	}
	exit(0);
	return 0;
}
//get the current status of the program
void getStatus(int curStatus)
{
	//if process was exited
	if (WIFEXITED(curStatus))
	{
		//return exit status and flush stdout
		printf("Exit status of process: %d\n", WEXITSTATUS(curStatus));
		fflush(stdout);
	}
	else
	{
		//return status and flush stdout
		printf("Process terminated with signal: %d\n", WTERMSIG(curStatus));
		fflush(stdout);
	}
}
//catch the ctrl + z command
void catchCtrlz(int signo)
{
	//changing to foreground
	if (isBackgroundAllowed == 1)
	{
		char* message = "\nEntering foreground-only mode (& is now ignored)\n";
		write(1, message, 51);
		fflush(stdout);
		//sleep(5);
		//set whether background processes are allowed
		isBackgroundAllowed = 0;
	}
	//changing to background (if isBackgroundAllowed == 0)
	else
	{
		char* message = "\nExiting foreground-only mode\n";
		write(1, message, 30);
		fflush(stdout);
		//sleep(5);
		//set whether background processes are allowed
		isBackgroundAllowed = 1;
	}
}
int main()
{
	//initialize arguments array and child processes array
	int j = 0;
	for (j = 0; j < 512; j++)
	{
		//efficiency (took this out of a second loop that was right after this one)
		if (j <= 100)
		{
			childProcesses[j] = -5;
		}
		arguments[j] = NULL;
	}

	//set numChildProcesses to 0 for array stuff later
	numChildProcesses = 0;

	//ctrl + c
	struct sigaction interrupt = {0};
	interrupt.sa_handler = SIG_IGN;
	sigfillset(&interrupt.sa_mask);
	interrupt.sa_flags = 0;
	sigaction(SIGINT, &interrupt, NULL);

	//ctrl + z
	struct sigaction stop = {0};
	stop.sa_handler = catchCtrlz;
	sigfillset(&stop.sa_mask);
	stop.sa_flags = 0;
	sigaction(SIGTSTP, &stop, NULL);

	//allow background processes by default when shell runs
	isBackgroundAllowed = 1;

	while(1)
	{
		pid_t curPid = getpid();
		//used to set exited processes to -1 in the arguments array
		checkAllChildProcesses();

		
		isBackgroundProcess = 0;
		
		//flush all your crap down the drain and output a prompt
		fflush(stdout);
		fflush(stdin);
		printf(": ");
		//rule #2: double tap
		fflush(stdout);
		fflush(stdin);

		//get input from the user
		fgets(input, sizeof(input), stdin);

		//end input line
		int i = 0;
		//run to the end of the input line
		for(i = 0; i < sizeof(input); i++)
		{
			//if a newline exists, change it to a null terminator and exit the loop
			if (input[i] == '\n')
			{
				input[i] = '\0';
				break;
			}
		}

		//fill arguments array
		numArguments = 0;

		//gets the first argument
		argPiece = strtok(input, " ");

		//if empty line, do nothing
		if (argPiece == NULL)
		{
			continue;
		}
		if (strchr(input, '&'))
		{
			if (isBackgroundAllowed == 1)
			{
				isBackgroundProcess = 1;
				printf("background\n");
			}
			else
			{
				isBackgroundProcess = 0;
				input[sizeof(input)] = '\0';
				printf("foreground\n");
			}
		}

		while (argPiece != NULL)
		{
			//input file redirection
			if (strcmp(argPiece, "<") == 0)
			{
				//get the name of the file
				argPiece = strtok(NULL, " ");
				//copy name of file into inFile variable
				inFile = strdup(argPiece);
				// strcpy(inFile, argPiece);
				argPiece = strtok(NULL, " ");
			}
			else if (strcmp(argPiece, "&") == 0)
			{
				argPiece = strtok(NULL, " ");
				isBackgroundProcess = 1;
			}
			//output file redirection
			else if (strcmp(argPiece, ">") == 0)
			{
				//get the name of the file
				argPiece = strtok(NULL, " ");
				//copy name of file into outFile variable
				outFile = strdup(argPiece);
				// strcpy(outFile, argPiece);
				argPiece = strtok(NULL, " ");
			}
			//background process
			else if (strcmp(argPiece, "&") == 0)
			{
				//set background process variable
				if (isBackgroundAllowed == 1)
	            {
	                isBackgroundProcess = 1;
	                printf("background\n");
	            }
	            else
	            {
	                isBackgroundProcess = 0;
	                input[sizeof(input)] = '\0';
	                printf("foreground\n");
	            }
			}
			//argument
			else
			{
				//strcpy was giving me segfaults, so i'm using strdup
				arguments[numArguments] = strdup(argPiece);
				//get next argument
				argPiece = strtok(NULL, " ");
				int i = 0;
				while (arguments[numArguments][i])
				{
					if (arguments[numArguments][i] == '$' && arguments[numArguments][i] == '$')
					{
						arguments[numArguments][i] = '\0';
						sprintf(arguments[numArguments], "%s%d", arguments[numArguments], curPid);
					}
					i++;
				}
				//increment numArguments
				numArguments++;
			}
		}

		//insert NULL at end of arguments array to indicate the end of the array
		arguments[numArguments] = NULL;

		//if cd is entered
		if (strcmp(arguments[0], "cd") == 0)
		{
			//if there is a path to cd to
			if (arguments[1] != NULL)
			{
				chdir(arguments[1]);
				//getcwd(curDir, sizeof(curDir));
			}
			//if no path was entered
			else
			{
				char* homeDir = getenv("HOME");
				chdir(homeDir);
			}
		}
		//if a comment is entered, or the user enters nothing, ignore it and continue
		else if (strncmp(arguments[0], "#", 1) == 0 || strcmp(arguments[0], "") == 0)
		{
			continue;
		}
		//if the user enters "exit," kill all child processes and exit the program
		else if (strcmp(arguments[0], "exit") == 0)
		{
			exitShell();
		}
		//if the user enters "status," return the exit status of the last foreground process
		else if (strcmp(arguments[0], "status") == 0)
		{
			getStatus(curStatus);
			continue;
		}
		//if anything else is entered
		else
		{
			//set pid to -5 to show whether it is running or not
			int pid = -5, wpid;
			pid = fork();

			//this is a child process
			if (pid == 0)
			{
				//if this is a foreground process
				if (isBackgroundProcess == 0)
				{
					//can be interrupted now
					interrupt.sa_handler = SIG_DFL;
					interrupt.sa_flags = 0;
					sigaction(SIGINT, &interrupt, NULL);
				}
				//if there is input redirection
				if (inFile != NULL)
				{
					//open file
					fileDesc = open(inFile, O_RDONLY);

					//if file open failed
					if (fileDesc == -1)
					{
						fprintf(stderr, "File open failure: %s\n", inFile);
						fflush(stdout);
						exit(1);
						break;
					}
					//if dup2 failed
					else if (dup2(fileDesc, 0) == -1)
					{
						fprintf(stderr, "Error with dup2");
						fflush(stdout);
						exit(1);
						break;
					}
					//close the file
					close(fileDesc);
				}
				//if this is a background process
				else if (isBackgroundProcess == 1)
				{
					//open /dev/null to throw all data into
					fileDesc = open("/dev/null", O_RDONLY);
					//if file open failed
					if (fileDesc == -1)
					{
						fprintf(stderr, "Error opening");
						exit(1);
						break;
					}
					//if dup2 failed
					else if (dup2(fileDesc, 0) == -1)
					{
						fprintf(stderr, "Dup2 error");
						exit(1);
					}
					//close the file
					close(fileDesc);
				}
				//if there is output redirection
				if (outFile != NULL)
				{
					//open file
					fileDesc = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

					//if file open failed
					if (fileDesc == -1)
					{
						fprintf(stderr, "File open Failure: %s\n", outFile);
						fflush(stdout);
						exit(1);
					}
					//if dup2 failed
					else if (dup2(fileDesc, 1) == -1)
					{
						fprintf(stderr, "Error with dup2\n");
						fflush(stdout);
						exit(1);
					}
					//close file
					close(fileDesc);
				}
				else if (isBackgroundProcess == 1)
				{
					//open /dev/null to throw all data into
					fileDesc = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
					//if file open failed
					if (fileDesc == -1)
					{
						fprintf(stderr, "Error opening");
						exit(1);
						break;
					}
					//if dup2 failed on stdout
					else if (dup2(fileDesc, 1) == -1)
					{
						fprintf(stderr, "Dup2 error");
						exit(1);
					}
					//close the file
					close(fileDesc);
				}
				
				//execute the command
				if (execvp(arguments[0], (char* const*)arguments) < 0)
				{
					fprintf(stderr, "Unknown command: %s\n", arguments[0]);
					fflush(stdout);
					exit(1);
					break;
				}
				break;
			}
			//error with forking
			else if (pid < 0)
			{
				fprintf(stderr, "error with forking");
				fflush(stdout);
				curStatus = 1;
				break;
			}
			//this is a parent process
			else
			{
				//if this is a background process
				if (isBackgroundAllowed == 1 && isBackgroundProcess == 1)
				{
					pid_t thisPid = waitpid(pid, &curStatus, WNOHANG);
					printf("background process pid: %d\n", pid);
					childProcesses[numChildProcesses] = pid;
					numChildProcesses++;
					fflush(stdout);
					//break;
				}
				//if this is a foreground process
				else
				{
					pid_t thisPid = waitpid(pid, &curStatus, 0);
					if (WIFSIGNALED(curStatus))
		            {
		                printf("Child %d terminated by signal: %d\n", childProcesses[i], WEXITSTATUS(curStatus));
		                fflush(stdout);
		            }
				}
			}
		inFile = NULL;
		outFile = NULL;
		}
	}
}