#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	//buffer for holding data to move around
	char buffer[150000];
	//message for checking communication
	char* encMessage = "otp_enc";
	//file pointer for plaintext, opened for reading
	FILE* plaintextFile = fopen(argv[1], "r");
	//file pointer for key, opened for reading
	FILE* keyFile = fopen(argv[2], "r");
	//holds plaintext
	char plaintext[100000];
	//holds key
	char key[100000];

    //read text from plaintext file into plaintext variable
    fgets(plaintext, sizeof(plaintext) - 1, plaintextFile);
    //replace trailing newline with '\0'
    plaintext[strcspn(plaintext, "\n")] = '\0';
    //read data from key file into key variable
    fgets(key, sizeof(plaintext) - 1, keyFile);
    //replace trailing newline with '\0'
    key[strcspn(key, "\n")] = '\0';

    //check that input is valid
    int j;
    for (j = 0; j < strlen(plaintext); j++)
    {
    	//if plaintext characters are either uppercase or a space
    	if (isupper((int)(plaintext[j])) || isspace((int)(plaintext[j])))
    	{
    		continue;
    	}
    	else
    	{
    		error("CLIENT: input rejected"); //error otherwise
    	}
    }
    //check that key is as long as plaintext
	if (strlen(key) < strlen(plaintext))
	{
		error("CLIENT: key too short");
	}

	if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket"); //error if socket can't be opened
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting"); //error if socket can't be connected

	//send plaintext and key to daemon program to be encoded
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	//put plaintext, key, and "@@" into buffer for sending
	sprintf(buffer, "%s\n%s\n%s\n%s", plaintext, key, encMessage, "@@");

	//while buffer is not empty, send the data in it
	while(charsWritten < sizeof(buffer))
	{
		//send data
		charsWritten = send(socketFD, buffer, sizeof(buffer), 0);
	}

	//error if socket could not be written
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	//error if all data isn't sent
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	// Get return message from server
	char fullMessage[200000];
	//receive data from otp_enc_d
	do
	{
		memset(buffer, '\0', sizeof(buffer)); //initialize buffer and full message storage
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); //read in data
		strcat(fullMessage, buffer); //append data to end of completeMessage
		if (charsRead < 0) {printf("charsRead < 0\n"); break;} //break if error
		if (charsRead == 0) {printf("charsRead == 0"); break;} //break if nothing came through
	}
	while(strstr(buffer, "@@") == NULL); //read until "@@" comes through

	//take "@@" back out
	fullMessage[strcspn(fullMessage, "@@")] = '\0';

	if (charsRead < 0) error("CLIENT: ERROR reading from socket"); //error if socket data could not be read
	//print data received
	printf("%s\n", fullMessage);

	close(socketFD); // Close the socket
	return 0;
}
