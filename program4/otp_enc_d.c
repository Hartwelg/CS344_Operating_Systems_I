#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	//large buffer for moving data around
	char buffer[150000];
	struct sockaddr_in serverAddress, clientAddress;
	//holds plaintext data
	char *plaintext;
	//holds key data
	char *key;
	//holds encrypted data
	char ciphertext[100000];
	//number to hold int value of cipher character
	int ciphertextNum = 0;
	//number to hold int value of key character
	int keyNum = 0;
	//number to hold int value of plaintext character
	int plaintextNum = 0;
	//message used to compare for correct communication
	char* encMessage = "otp_enc";
	//current status of child process
	int curStatus = 0;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding"); //error if socket could not be bound
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	//loop on accept (taken from 4.3)
	while(1)
	{
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect

		//accept
		if (establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo))
		{				
			//get current pid
			int pid = getpid();
			//fork on accept
			pid = fork();
			
			//if there was an error with forking
			if (pid < 0)
			{
				printf("Error with forking\n");
				curStatus = 1;
			}
			//child process
			else if (pid == 0)
			{
				//no error, receive data and do encryption

				char completeMessage[100000], readBuffer[10000];
				memset(completeMessage, '\0', sizeof(completeMessage)); //clear the buffer

				//receive data from otp_enc
				do
				{
					memset(readBuffer, '\0', sizeof(readBuffer)); //initialize buffer and full message storage
					charsRead = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer) - 1, 0); //read in data
					strcat(completeMessage, readBuffer); //append data to end of completeMessage
					if (charsRead < 0) {printf("charsRead < 0\n"); break;} //break if error
					if (charsRead == 0) {printf("charsRead == 0"); break;} //break if nothing came through
				}
				while(strstr(readBuffer, "@@") == NULL); //read until "@@" comes through

				//remove "@@" from completeMessage
				completeMessage[strcspn(completeMessage, "@@")] = '\0';
				
				//retreive variables from completeMessage
				plaintext = strtok(completeMessage, "\n"); //plaintext data
				key = strtok(NULL, "\n"); //key data
				encMessage = strtok(NULL, "\n"); //encoder message - to be checked for communication with correct process

				//check if communication is from the correct program
				if(strcmp(encMessage, "otp_enc") != 0)
				{
					error("SERVER: wrong program"); //error if communication is not with otp_enc
				}

				int i;
				for (i = 0; i < strlen(plaintext); i++) //loop through full length of message
				{
					plaintextNum = plaintext[i] - 64; //character in plaintext - 64 because space is 0 in keygen array
					keyNum = key[i] - 64; //character in key - 64 because space is 0 in keygen array
					if (plaintextNum == -32) //if plaintext character is a space
					{
						plaintextNum = 0; //set to 0 because 0 is a space in keygen array
					}
					if (keyNum == -32) //if key character is space
					{
						keyNum = 0; //set to 0 because 0 is space in keygen array
					}
					ciphertextNum = (plaintextNum + keyNum) % 27; //encode the character
					if (ciphertextNum == 0) //if character == 0
					{
						ciphertextNum = -32; //set to -32
					}
					ciphertext[i] = ciphertextNum + 64; //place in ciphertext variable
				}
				//send ciphertext back to otp_enc

				//add "@@" back to message to send back
				sprintf(ciphertext, "%s%s", ciphertext, "@@");

				//send data until all is sent
				while(charsRead < sizeof(ciphertext))
				{
					//send data
					charsRead = send(establishedConnectionFD, ciphertext, sizeof(ciphertext), 0);
				}
				
				//error if socket could not be written to
				if (charsRead < sizeof(ciphertext) - 1) error("SERVER: ERROR writing to socket");
				exit(0); //exit child process
			}
			//parent process
			else
			{
				pid_t curPid = waitpid(pid, &curStatus, 0); //wait on child process if > 5 are running
				close(establishedConnectionFD); //close connection
			}
		}
	//error if socket could not be accepted
	if (establishedConnectionFD < 0) error("ERROR on accept");
	}
	close(listenSocketFD); // Close the listening socket
	return 0; 
}
