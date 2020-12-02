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
	//holds ciphertext data
	char *ciphertext;
	//holds key data
	char *key;
	//holds encrypted data
	char plaintext[100000];
	//number to hold int value of cipher character
	int ciphertextNum = 0;
	//number to hold int value of key character
	int keyNum = 0;
	//number to hold int value of plaintext character
	int plaintextNum = 0;
	//otp_dec_d message
	char* decMessage = "otp_dec";
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
	if (listenSocketFD < 0) error("ERROR opening socket"); //error if socket could not be opened

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding"); //error if socket could not be bound
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	//loop on accept (taken from 4.3)
	while(1)
	{

		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect

		//Accept
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
				//no error, receive data and do decryption

				char completeMessage[100000], readBuffer[100000]; //initialize buffer and full message storage
				memset(completeMessage, '\0', sizeof(completeMessage)); //clear the buffer

				//receive data from otp_dec
				do
				{
					memset(readBuffer, '\0', sizeof(readBuffer)); //clear buffer
					charsRead = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer) - 1, 0); //read in data
					strcat(completeMessage, readBuffer); //append data to end of completeMessage
					if (charsRead < 0) {printf("charsRead < 0\n"); break;} //break if error
					if (charsRead == 0) {printf("charsRead == 0"); break;} //break if nothing comes through
				}
				while(strstr(readBuffer, "@@") == NULL); //read until "@@" comes through

				//remove "@@" from completeMessage
				completeMessage[strcspn(completeMessage, "@@")] = '\0';
				
				//retreive variables from completeMessage
				ciphertext = strtok(completeMessage, "\n");
				key = strtok(NULL, "\n");
				decMessage = strtok(NULL, "\n");

				//check if communication is from the correct program
				if(strcmp(decMessage, "otp_dec") != 0)
				{
					error("SERVER: wrong program"); //error if communication is not with otp_dec
				}		

				int i;
				for (i = 0; i < strlen(ciphertext); i++) //loop through full length of message
				{
					ciphertextNum = ciphertext[i]; //character in cipher
					keyNum = key[i]; //character in key
					if (ciphertextNum == 32) //if cipher character is a space
					{
						ciphertextNum = 0; //set to 0 because 0 is a space in keygen array
					}
					else
					{
						ciphertextNum -= 64; //else subtract 64 to get char < 27 because of keygen array
					}
					if (keyNum == 32) //if character in key is a space
					{
						keyNum = 0; //set to 0 because 0 is a space in keygen array
					}
					else
					{
						keyNum -= 64; //else subtract 64 to get char < 27 because of keygen array
					}
					plaintextNum = ((ciphertextNum - keyNum) + 27) % 27; //decode the character
					if (plaintextNum == 0) //if plaintext character is a space
					{
						plaintextNum = 32; //set to ascii space
					}
					else
					{
						plaintextNum += 64; //else add 64 to get it to ascii uppercase
					}
					plaintext[i] = plaintextNum; //add character to decoded message
				}
				//send ciphertext back to otp_enc
				sprintf(plaintext, "%s%s", plaintext, "@@"); //add "@@" to end of plaintext to be sent back
				//while not all the data is sent
				while(charsRead < sizeof(plaintext))
				{
					//send the data
					charsRead = send(establishedConnectionFD, plaintext, sizeof(plaintext), 0);
				}
				//error if socket could not be written to
				if (charsRead < sizeof(ciphertext) - 1) error("SERVER: ERROR writing to socket");
				exit(0); //close child process
			}
			//parent process
			else
			{
				pid_t curPid = waitpid(pid, &curStatus, 0); //wait on child process
				close(establishedConnectionFD); //Close the existing socket which is connected to the client
			}
		}
	if (establishedConnectionFD < 0) error("ERROR on accept"); //error if socket could not be accepted
	}
	close(listenSocketFD); // Close the listening socket
	return 0; //return
}
