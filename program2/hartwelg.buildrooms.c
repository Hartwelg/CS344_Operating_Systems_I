//sources: 2.2 readings, stackoverflow, geeksforgeeks.org
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
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

//set all array values to -1
void initArrays()
{
    int i = 0, j = 0;
    for (i = 0; i < 7; i++)
    {
        chosenRoomNames[i] = -1;
        for (j = 0; j < 7; j++)
        {
            outboundConnections[i][j] = 0;
        }
    }
}
//returns a random number representing the index in availableRoomNames
int GetRandomAvailableRoom()
{
    srand(time(NULL));
    int number = rand() % 10;
    return number;
}
//returns random room from chosenRooms
int GetRandomChosenRoom()
{
    srand(time(NULL));
    int number = rand() % 7;
    return number;
}

//checks whether two given rooms have connections to each other
int ConnectionAlreadyMade(int current, int other)
{
        //if current is connected to other, then other should be connected to current
        if (outboundConnections[current][other] == 1 && outboundConnections[other][current] == 1)
        {
                return 1;
        }
        else
        {
                return 0;
        }
}
//checks whether two given integers are the same, representing the same room within availableRoomNames
int IsSameRoom(int current, int other)
{
        if (current == other)
        {
                return 1;
        }
        else
        {
                return 0;
        }
}
//changes the index within the outboundConnections array to 1, representing that current has a connection to other
void MakeRoomConnection(int current, int other)
{
        outboundConnections[current][other] = 1;
}
//checks whether a given room has <=6 connections to other rooms
int CanAddConnectionFrom(int other)
{
//i is for the following loop, counter is to check the number of room connections
int i = 0, counter = 0;

for (i = 0; i < 7; i++)
{
        //checks the column in outboundConnections for the room passed into the function. If a "1" is found, then a connection exists, and counter is incremented
        if (outboundConnections[other][i] == 1)
        {
                counter++;
        }
}
        // if the counter is < 6, then a connection can be added, otherwise, the room is full
        if (counter < 6)
        {
                return 1;
        }
        else
        {
                return 0;
        }
}
//adds a connection between two randomly selected rooms in chosenRoomNames
void AddConnection()
{
        int firstRoom = 0;
        int secondRoom = 0;

        //initiate infinite loop
        while(1)
        {
                //randomly select firstRoom
                firstRoom = GetRandomChosenRoom();
                //if a connection can be made, break loop and find a room to connect to
                if (CanAddConnectionFrom(firstRoom) == 1)
                {
                        break;
                }
        }
        //find a secondRoom that can have a connection made to it, isn't the same as firstRoom, and isn't already connected to firstRoom
        while(1)
        {
            secondRoom = GetRandomChosenRoom();
            if (CanAddConnectionFrom(secondRoom) == 1 && IsSameRoom(firstRoom, secondRoom) == 0 && ConnectionAlreadyMade(firstRoom, secondRoom) == 0)
            {
                break;
            }
        }

        //connect firstRoom to secondRoom
        MakeRoomConnection(firstRoom, secondRoom);
        //connect secondRoom back to firstRoom, otherwise user can't travel both ways
        MakeRoomConnection(secondRoom, firstRoom);
}
//checks whether every room has >3 && <6 connections
int IsGraphFull()
{
int i = 0, j = 0, counter = 0, total = 0;
//loops through each index in top row
for (i = 0; i < 7; i++)
{
    counter = 0;
        //loops through each index in a column
        for (j = 0; j < 7; j++)
        {
                //increment connection counter if a "1" is found, representing a connection to another room
                if (outboundConnections[i][j] == 1)
                {
                        counter++;
                }
        }
        //increment total if a room has >3 connections, representing that the room has enough connections
        if (counter >= 3 && counter <= 6)
        {
            total++;
        }
        
    }
//if each room is full of connections, return true
        if (total >= 7)
        {
                return 1;
        }
        else
        {
                return 0;
        }
return 0;
}
//fills the outboundConnections array with room connections
int fillGraph()
{
        while (IsGraphFull() == 0)
        {
                AddConnection();
        }
}

//creates the directory for the room files to go into
void createDir()
{
        int procId = getpid();
        char dirName[32] = "hartwelg.rooms.";
        //full name for directory
        char fullDirName[64];
        //concatenate dirName prefix and process ID together
        sprintf(fullDirName, "%s%d", dirName, procId);
        //create directory with fullDirName
        int directory = mkdir(fullDirName, 0755);
        
        int i = 0, j = 0;
        //hold pointer to a file
        FILE *fp;
        //buffer for info to go into file
        char buffer[256];
        //set buffer to empty
        memset(buffer, '\0', sizeof(buffer));
        for (i = 0; i < 7; i++)
        {
            //set buffer to empty before adding to it
            memset(buffer, '\0', sizeof(buffer));
            //name of file to write to
            char fileName[64];
            //create file within directory called "room_<roomName>"
            sprintf(fileName, "%s/room_%s", fullDirName, availableRoomNames[chosenRoomNames[i]]);
            //open file
            fp = fopen(fileName, "w+");
            //write to buffer
            sprintf(buffer, "ROOM NAME: %s\n", availableRoomNames[chosenRoomNames[i]]);
            //used for connection number
            int counter = 1;
            for (j = 0; j < 7; j++)
            {
                //if a connection exists, append connection info to buffer, and increment counter
                if (ConnectionAlreadyMade(i, j) == 1)
                {
                    sprintf(buffer + strlen(buffer), "CONNECTION %d: %s\n", counter, availableRoomNames[chosenRoomNames[j]]);
                    counter++;
                }
            }
            //append room type to end of buffer
            sprintf(buffer + strlen(buffer), "ROOM TYPE: %s\n", availableRoomTypes[chosenRoomTypes[i]]);
            //write buffer to file
            fprintf(fp, "%s", buffer);
            //close file
            fclose(fp);
        }
}
//swap two integers 
void swap (int *a, int *b) 
{ 
    int temp = *a; 
    *a = *b; 
    *b = temp; 
} 
  
//print an array 
void printArray (int arr[], int n) 
{ 
    int i = 0;
    for (i = 0; i < n; i++) 
        printf("%d ", arr[i]); 
    printf("\n"); 
} 
  
//generate a random permutation of availableRoomNames 
void randomize ( int arr[], int n ) 
{ 
    // Use a different seed value so that we don't get same 
    // result each time we run this program 
    srand ( time(NULL) ); 
  
    // Start from the last element and swap one by one. We don't 
    // need to run for the first element that's why i > 0 
    int i = 0, j = 0;
    for (i = n-1; i > 0; i--) 
    { 
        // Pick a random index from 0 to i 
        j = rand() % (i+1); 
  
        // Swap arr[i] with the element at random index 
        swap(&arr[i], &arr[j]); 
    } 
} 
//fill chosenRooms array
 void ChooseRooms()
 {
    int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; 
    int n = 10;
    randomize (arr, n); 

    int i = 0;
    for (i = 0; i < 7; i++)
    {
        chosenRoomNames[i] = arr[i];
    }

 }
//assign room types to each room in chosenRooms
void initRoomTypes()
{
    chosenRoomTypes[0] = 0;
    chosenRoomTypes[6] = 2;
    chosenRoomTypes[1] = 1;
    chosenRoomTypes[2] = 1;
    chosenRoomTypes[3] = 1;
    chosenRoomTypes[4] = 1;
    chosenRoomTypes[5] = 1;
}

int main()
{
    initArrays();
    initRoomTypes();
    
    fillGraph();
    
    ChooseRooms();
    
    // int i = 0, j = 0;
    // for (i = 0; i < 7; i++)
    // {
    //     for (j = 0; j < 7; j++)
    //     {
    //         printf("%d\t", outboundConnections[i][j]);
    //     }
    //     printf("\n\n");
    // }
    
    createDir();
}
        