#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	srand(time(NULL)); //randomize
	int length = atoi(argv[1]); //change first argument to integer for use later
	int modNum = 27, counter = 0; //number to use for modulus, and a counter for while loop
	char* key = NULL; //empty key to begun with
	static char possibleChars[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //array for use in creating key

	if (length) //if length variable is present
	{
		//malloc space for key
		key = malloc(sizeof(char) * (length + 1));
		if (key) //if malloc was successful
		{
			while (counter < length) //loop for creating key, char by char
			{
				int randNum = rand() % modNum; //random number from 0 - 27 from array above
				key[counter] = possibleChars[randNum]; //add char to key
				counter++; //increment counter
			}
			key[length] = '\0'; //end key with null terminator
		}
	}
	//print out key
	printf("%s\n", key);
	return 0; //return
}