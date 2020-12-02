//sources: 2.4 readings, stackoverflow
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

void getDir();
void openFiles(char *);
void readFile(char *, char *, int);

//list of the available rooms for the program to choose from
char* availableRoomNames[10] = {"theRoom", "stairs", "dungeon", "bedroom", "stable", "yard", "foyer", "mirrors", "swirly", "walls"};
//rooms chosen for current run of the program
int chosenRoomNames[7];
//types for each chosen room
int chosenRoomTypes[7];
//array showing which rooms are connected to each other
int outboundConnections[7][7];
//array for room types
char* availableRoomTypes[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};
//struct to hold current room data
struct room{
	char name[32];
	char connection1[32];
	char connection2[32];
	char connection3[32];
	char connection4[32];
	char connection5[32];
	char connection6[32];
	int numConnections;
	char roomType[20];
};
//array to contain all room structs
struct room allRooms[7];
//global counter to keep track of current room
int curGameState = 0;

void getDir()
{
  int newestDirTime = -1; // Modified timestamp of newest subdir examined
  char targetDirPrefix[32] = "hartwelg.rooms."; // Prefix we're looking for
  char newestDirName[256]; // Holds the name of the newest dir that contains prefix
  memset(newestDirName, '\0', sizeof(newestDirName));

  DIR* dirToCheck; // Holds the directory we're starting in
  struct dirent *fileInDir; // Holds the current subdir of the starting dir
  struct stat dirAttributes; // Holds information we've gained about subdir

  dirToCheck = opendir("."); // Open up the directory this program was run in

  if (dirToCheck > 0) // Make sure the current directory could be opened
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
    {
      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
      {
        //printf("Found the prefix: %s\n", fileInDir->d_name);
        stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

        if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
        {
          newestDirTime = (int)dirAttributes.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          strcpy(newestDirName, fileInDir->d_name);
          //printf("Newer subdir: %s, new time: %d\n",
                 //fileInDir->d_name, newestDirTime);
        }
      }
    }
  closedir(dirToCheck); // Close the directory we opened
  openFiles(newestDirName);      
  }
}
//code referenced from ch2 readings
//opens a file within directory
void openFiles(char* directory)
{
  int newestDirTime = -1; // Modified timestamp of newest subdir examined
  char targetDirPrefix[32] = "room_"; // Prefix we're looking for
  char newestFileName[256]; // Holds the name of the newest dir that contains prefix
  //memset(newestDirName, '\0', sizeof(newestDirName));

  int counter = 0;
  DIR* dirToCheck; // Holds the directory we're starting in
  struct dirent *fileInDir; // Holds the current subdir of the starting dir
  //struct stat dirAttributes; // Holds information we've gained about subdir

  dirToCheck = opendir(directory); // Open up the directory this program was run in

  if (dirToCheck > 0) // Make sure the current directory could be opened
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
    {
      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
      {
      		readFile(fileInDir->d_name, directory, counter); // Read from file with this name
      		counter++;
      }
    }
  closedir(dirToCheck); // Close the directory we opened
  }
}
//code referenced from stackoverflow
//iterate through current open file and read line by line
void readFile(char* fileName, char* directory, int counter)
{
	//initialize file pointer
	FILE* fp;
	char * line = NULL;
    size_t len = 0;
    ssize_t read;

	char room_label[20], colon[8], roomName[32];	

	char fullDirName[64];

	//full path to current file
	sprintf(fullDirName, "%s/%s", directory, fileName);

	//pointer to current file
	fp = fopen(fullDirName, "r");
	//used to count number of connections
	int k = 0;
	//while file is being read, put information into struct variables and fill a struct array
	while ((read = getline(&line, &len, fp)) != -1) {
        sscanf(line, "%s %s %s", room_label, colon, roomName);

        //if room_label == "ROOM:", then put roomName into the "name" variable in the curRoom struct
        if (strcmp(room_label, "ROOM") == 0)
        {
        	if (strcmp(colon, "NAME:") == 0)
        	{
				strcpy(allRooms[counter].name, roomName);
        	}
        	else if (strcmp(colon, "TYPE:") == 0)
        	{
        		strcpy(allRooms[counter].roomType, roomName);
        	}
        }
        //else if room_label == "CONNECTION", then put roomName into a connection in the curRoom struct
        else if (strcmp(room_label, "CONNECTION") == 0)
        {
        	allRooms[counter].numConnections++;
        	if (k == 0)
        	{
        		strcpy(allRooms[counter].connection1, roomName);
        		k++;
        	}
        	else if (k == 1)
        	{
        		strcpy(allRooms[counter].connection2, roomName);
        		k++;
        	}
        	else if (k == 2)
        	{
        		strcpy(allRooms[counter].connection3, roomName);
        		k++;
        	}
        	else if (k == 3)
        	{
        		strcpy(allRooms[counter].connection4, roomName);
        		k++;
        	}
        	else if (k == 4)
        	{
        		strcpy(allRooms[counter].connection5, roomName);
       			k++;
        	}
        	else if (k == 5)
        	{
        		strcpy(allRooms[counter].connection6, roomName);
        		k++;
        	}
        }
    }

    fclose(fp);
    if (line)
        free(line);
}

int main()
{
//read files in most recent directory
getDir();
//get start room for game
int i = 0;
for (i = 0; i < 7; i++)
{
	if(strcmp(allRooms[i].roomType, "START_ROOM") == 0)
	{
		curGameState = i;
	}
}
//run the game
while(strcmp(allRooms[curGameState].roomType, "END_ROOM") != 0)
{
	printf("CURRENT LOCATION: %s\n", allRooms[0].name);
	printf("POSSIBLE CONNECTIONS: ");
	int i = 0;
	for (i = 0; i < allRooms[curGameState].numConnections; i++)
	{
		if (i == 0)
		{
			printf("%s", allRooms[curGameState].connection1);
		}
		else if (i == 1)
		{
			printf(", %s", allRooms[curGameState].connection2);
		}
		else if (i == 2)
		{
			printf(", %s", allRooms[curGameState].connection3);
		}
		else if (i == 3)
		{
			printf(", %s", allRooms[curGameState].connection4);
		}
		else if (i == 4)
		{
			printf(", %s", allRooms[curGameState].connection5);
		}
		else if (i == 5)
		{
			printf(", %s", allRooms[curGameState].connection6);
		}
	}
	printf(".\n");
	printf("WHERE TO? >");
	char* roomInput;
	size_t bytesRead;
	getline(&roomInput, &bytesRead, stdin);

	int doesConnectionExist = 0;

	//if (strcmp(allRooms[curGameState].name))
	for (i = 0; i < allRooms[curGameState].numConnections; i++)
	{
		if (i == 0)
		{
			if (strcmp(allRooms[curGameState].connection1, roomInput) == 0)
			{
				doesConnectionExist = 1;
				break;
			}
		}
		else if (i == 1)
		{
			if (strcmp(allRooms[curGameState].connection2, roomInput) == 0)
			{
				doesConnectionExist = 1;
				break;
			}
		}
		else if (i == 2)
		{
			if (strcmp(allRooms[curGameState].connection3, roomInput) == 0)
			{
				doesConnectionExist = 1;
				break;
			}
		}
		else if (i == 3)
		{
			if (strcmp(allRooms[curGameState].connection4, roomInput) == 0)
			{
				doesConnectionExist = 1;
				break;
			}
		}
		else if (i == 4)
		{
			if (strcmp(allRooms[curGameState].connection5, roomInput) == 0)
			{
				doesConnectionExist = 1;
				break;
			}
		}
		else if (i == 5)
		{
			if (strcmp(allRooms[curGameState].connection6, roomInput) == 0)
			{
				doesConnectionExist = 1;
				break;
			}
		}
	}
	int j = 0;
		for (j = 0; j < 7; j++)
		{
			if (doesConnectionExist == 1)
			{
				if (strcmp(roomInput, allRooms[j].name) == 0)
				{
					curGameState = j;
				}
			}
		}
}
//char* input;
return 0;
}