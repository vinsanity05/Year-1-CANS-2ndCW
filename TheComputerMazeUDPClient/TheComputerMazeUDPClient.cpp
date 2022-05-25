// TheComputerMazeUDPClient.cpp : Defines the entry point for the console application.
//

/*
*	Author: Vince Verdadero (19009246)
*	Created: 13 / 03 / 20
*	Last edited: 22 / 04 / 20
*	Description: A program used to find it's way around a new Computing building called Y-Block.
*	This program will:
					Open locked doors
					Collect keys and store into a file.
					Pick items up and sort backpack.
					Read books and attend events.
					collect valuable items
					Navigate autonomously.
*	User advice: None
*/


#include "stdafx.h"
#include <winsock2.h>
#include <time.h>
#include <conio.h>


#pragma comment(lib, "wsock32.lib")

#define STUDENT_NUMBER		"19009246"
#define STUDENT_FIRSTNAME	"Vince"
#define STUDENT_FAMILYNAME	"Verdadero"

#define IP_ADDRESS_SERVER	"127.0.0.1"
//#define IP_ADDRESS_SERVER "164.11.80.69"


#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define MAX_FILENAME_SIZE 500

#define MAX_BUFFER_SIZE   5000
#define MAX_STRING_SIZE   200
#define MAX_NO_TOKENS     50

#define MAX_ITEMS_IN_ROOM		20
#define MAX_ITEMS_IN_BACKPACK	50

#define NO_MOVE -1

#define OPTION_MOVE_NORTH 1
#define OPTION_MOVE_SOUTH 2
#define OPTION_MOVE_EAST 3
#define OPTION_MOVE_WEST 4
#define OPTION_MOVE_UP 5
#define OPTION_MOVE_DOWN 6

#define OPTION_UNLOCK_NORTH 7
#define OPTION_UNLOCK_SOUTH 8
#define OPTION_UNLOCK_EAST 9 
#define OPTION_UNLOCK_WEST 10
#define OPTION_UNLOCK_UP 11
#define OPTION_UNLOCK_DOWN 12

#define OPTION_BASE_FOR_READS 200
#define OPTION_BASE_FOR_PICKUPS 500
#define OPTION_BASE_FOR_DROPS 800
#define OPTION_BASE_FOR_DOS 1100
#define OPTION_BASE_FOR_EVENTS 1300

enum directions
{
	DIRECTION_NORTH = 0,
	DIRECTION_SOUTH = 1,
	DIRECTION_EAST = 2,
	DIRECTION_WEST = 3,
	DIRECTION_UP = 4,
	DIRECTION_DOWN = 5
};

enum direction_possible
{
	DIRECTION_NOT_PRESENT = -1,
	DIRECTION_LOCKED = 0,
	DIRECTION_OPEN = 1
};

enum item_types
{
	ITEM_NONE = 0,
	ITEM_BOOK = 1,
	ITEM_JUNK = 2,
	ITEM_EQUIPMENT = 3,
	ITEM_MANUSCRIPT = 4,
	ITEM_TEST = 5,
	ITEM_OTHER = 10
};

enum typeofroom
{
	ROOM_NONE = 0,
	ROOM_LECTURE_SMALL = 1,
	ROOM_LECTURE_MEDIUM = 2,
	ROOM_LECTURE_LARGE = 3,
	ROOM_CORRIDOR = 4,
	ROOM_LAB_SMALL = 5,
	ROOM_LAB_MEDIUM = 6,
	ROOM_LAB_LARGE = 7,
	ROOM_MEETING_ROOM = 8,
	ROOM_SEMINAR = 9,
	ROOM_HIVE = 10, //one per floor
	ROOM_COFFEESHOP = 11, //one
	ROOM_LIBRARY = 12, //only one
	ROOM_SHOP_SELL = 13,
	ROOM_SHOP_BUY = 14,
	ROOM_SHOP_BUYSELL = 15,
	ROOM_OFFICE = 16, //maybe only one door
	ROOM_LOBBY = 17, //Only one
	ROOM_EXIT = 18, //only one
	ROOM_STAIRS = 19,
	ROOM_ENTRANCE = 20 //only one
};

struct Item
{
	int  number;
	int  value;
	int  volume;
};

struct Student
{
	int level;
	int rooms_visited;
	int doors_openned;
	int number_of_moves;
	int score;
};

struct Room
{
	char name[5];
	int  type;
	int  direction[6];
	int  number_of_keys;
	int  keys[4];
	int  number_of_items;
	Item items[MAX_ITEMS_IN_ROOM];
};

struct Backpack
{
	int number_of_items;
	Item items[MAX_ITEMS_IN_BACKPACK];
};

#define MAX_OPTIONS	50

int number_of_options;
int options[MAX_OPTIONS];

Student student;
Room room;
Backpack backpack;

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer[MAX_BUFFER_SIZE];

char Tokens[MAX_NO_TOKENS][MAX_STRING_SIZE];

char text_student[1000];
char text_backpack[1000];
char text_room[1000];
char text_keys[1000];
char text_items[1000];
char text_options[1000];

//sentOption()
// Description: This function sends a move option to the Computer Maze Marking Server
// Inputs: option - this is the move that we would like to make
//         key    - this is the key to use if we are unlocking a door
// Outputs: None
// Parameters: int key, int option
// Returns: None
// Warnings: None

void sentOption(int option, int key)
{
	char buffer[100];

	sprintf(buffer, "Option %d, %x", option, key);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));
}

/*************************************************************/
/********* Your tactics code starts here *********************/
/*************************************************************/

/*-------------------------WORKING KEYS-------------------------*/

// Function: Workingkeys
// Description: This is an array that contains the keys that work in the maze as well as the room and direction the door the keys can be used for 
// Returns: None
// Parameters: None
// Warnings: There are some keys that are shorten e.g. 0x90D, 0x172
//           There are some duplicates but open different doors 

#define NUMBER_OF_WORKING_KEYS 148

struct WorkingKeys
{
	char room_name[5]; // <f>Y<ns><ew>\0
	int  direction;
	int key;
};

WorkingKeys working_keys[NUMBER_OF_WORKING_KEYS] =
{
{"0Y00",2,0xBE27},
{"0Y01",3,0xFEA1},
{"0Y02",2,0xC4B6},
{"0Y08",1,0x92A1},
{"0Y08",2,0xEE2E},
{"0Y12",1,0xC679},
{"0Y12",3,0xD2C3},
{"0Y13",1,0x4DF3},
{"0Y13",2,0xA100},
{"0Y15",1,0xC544},
{"0Y17",1,0xD2F7},
{"0Y18",2,0xC9B0},
{"0Y19",1,0x3B45},
{"0Y19",3,0xC9B0},
{"0Y23",0,0x4DF3},
{"0Y23",2,0xB250},
{"0Y24",3,0xB250},
{"0Y25",2,0x9F4A},
{"0Y28",1,0x5468},
{"0Y30",2,0x90D},
{"0Y32",2,0x9D5F},
{"0Y33",1,0x06A1},
{"0Y33",3,0x9D5F},
{"0Y34",2,0x71DA},
{"0Y35",1,0xE366},
{"0Y35",3,0x2D35},
{"0Y35",3,0x71DA},
{"0Y40",0,0xBCD5},
{"0Y41",3,0xE4A8},
{"0Y42",2,0x29FC},
{"0Y44",1,0x51ED},
{"0Y44",2,0xBCFF},
{"0Y45",0,0xE366},
{"0Y51",3,0xBCD5},
{"0Y58",1,0x8E28},
{"0Y58",2,0xD2F7},
{"0Y60",0,0xC567},
{"0Y64",1,0x71DA},
{"0Y64",3,0x172},
{"0Y67",2,0xCC48},
{"0Y67",2,0xD2F7},
{"0Y68",3,0xD2F7},
{"0Y74",3,0x172},
{"0Y77",2,0x71DA},
{"0Y78",2,0xE4A8},
{"0Y78",3,0x71DA},
{"0Y81",0,0x172},
{"0Y82",0,0xC4B6},
{"0Y82",1,0x200F},
{"0Y82",3,0x8FD4},
{"0Y85",0,0x71DA},
{"0Y85",1,0xA100},
{"0Y86",3,0xD2F7},
{"0Y90",0,0x8E28},
{"0Y92",3,0xC4B6},
{"0Y93",0,0xBCD5},
{"0Y94",3,0x3531},
{"0Y96",0,0xD75B},
{"1Y01",2,0x8E28},
{"1Y06",3,0xFD5A},
{"1Y07",1,0xFD5A},
{"1Y07",2,0xC4B6},
{"1Y10",0,0x2A0F},
{"1Y11",2,0x90D},
{"1Y12",0,0xBE27},
{"1Y20",2,0x56D5},
{"1Y21",3,0x172},
{"1Y25",2,0xE4A8},
{"1Y26",0,0xC4B6},
{"1Y30",1,0x9F4A},
{"1Y31",1,0x90D},
{"1Y36",1,0x8E28},
{"1Y41",0,0x90D},
{"1Y43",3,0x90D},
{"1Y44",2,0xFD5A},
{"1Y51",2,0x9F4A},
{"1Y52",3,0x9F4A},
{"1Y53",2,0x71DA},
{"1Y54",1,0x8FD4},
{"1Y56",1,0x8FD4},
{"1Y57",2,0xA100},
{"1Y63",2,0x172},
{"1Y65",2,0x9F4A},
{"1Y66",0,0x8FD4},
{"1Y66",3,0x9F4A},
{"1Y71",3,0x29FC},
{"1Y72",1,0xFD5A},
{"1Y72",2,0xE4A8},
{"1Y76",2,0x172},
{"1Y78",1,0xC567},
{"1Y80",1,0xD2F7},
{"1Y81",3,0x8FD4},
{"1Y82",2,0xBCD5},
{"1Y83",3,0xBCD5},
{"1Y84",2,0x8E28},
{"1Y88",1,0xBE27},
{"1Y91",3,0xA100},
{"1Y93",2,0xA100},
{"1Y94",3,0xA100},
{"1Y96",3,0x71DA},
{"2Y06",3,0xBCD5},
{"2Y08",2,0x9F4A},
{"2Y13",1,0x90D},
{"2Y13",2,0xE4A8},
{"2Y21",0,0x90D},
{"2Y21",3,0x29FC},
{"2Y24",0,0x200F},
{"2Y25",1,0xBCD5},
{"2Y26",0,0x9F4A},
{"2Y28",1,0xE4A8},
{"2Y31",2,0xD75B},
{"2Y33",2,0xBCD5},
{"2Y36",3,0xC4B6},
{"2Y37",1,0x90D},
{"2Y38",3,0xBCD5},
{"2Y42",0,0x8FD4},
{"2Y46",0,0x8E28},
{"2Y46",2,0xA100},
{"2Y50",2,0xD75B},
{"2Y55",2,0xD75B},
{"2Y56",1,0xBCD5},
{"2Y57",1,0x9F4A},
{"2Y72",1,0x172},
{"2Y77",2,0x29FC},
{"2Y78",0,0x8FD4},
{"2Y88",2,0x71DA},
{"2Y88",3,0x29FC},
{"2Y95",2,0xD2F7},
{"2Y97",0,0x8E28},
{"3Y00",1,0x200F},
{"3Y04",1,0xFD5A},
{"3Y05",3,0x172},
{"3Y21",0,0xC4B6},
{"3Y23",3,0xD2F7},
{"3Y26",0,0x71DA},
{"3Y27",1,0xFD5A},
{"3Y34",2,0x200F},
{"3Y35",0,0xFD5A},
{"3Y38",0,0xBE27},
{"3Y61",3,0xBE27},
{"3Y63",3,0xD2F7},
{"3Y65",3,0xBE27},
{"3Y72",1,0xD2F7},
{"3Y76",0,0x172},
{"3Y76",3,0x200F},
{"3Y82",1,0xE4A8},
{"3Y87",1,0x8FD4},
{"3Y99",3,0x71DA},

};

// Function: useWorkingkeys
// Description: This is a function to look through these keys
// Parameters: int* use_key
// Returns: MOVE
// Warnings: None

int useWorkingKeys(int* use_key)
{
	int move = NO_MOVE;  // default is no key found
	int i;
	int door;
	for (i = 0; i < NUMBER_OF_WORKING_KEYS; i++) {
		door = working_keys[i].direction;

		if (room.direction[door] == DIRECTION_LOCKED) {
			if (strcmp(room.name, working_keys[i].room_name) == 0) {
				*use_key = working_keys[i].key;
				move = OPTION_UNLOCK_NORTH + door;
				return move;
			}
		}
	}
	return move;
}

// Function: writekeys2file
// Description: This is a function of writing keys going to a file (mykey) if you find a key that opens a door
// Parameters: int key, int floor, int ns, int ew, int door
// Returns: None
// Warnings: None

FILE* ofp;
void writekeys2file(int key, int floor, int ns, int ew, int door)
{

	if ((ofp = fopen("mykey.txt", "a")) != NULL) {
		fprintf(ofp, "{\"%dY%d%d\",", floor, ns, ew);
		fprintf(ofp, "%d,", door);
		fprintf(ofp, "0x%X}", key);
		fprintf(ofp, ",\n");

		fclose(ofp);
	}
}

/*-------------------------KNOWNN KEYS-------------------------*/

// Function: Knownkeys
// Description: This function are the known keys which unlock a specific door 
// Parameters: None
// Returns: None
// Warnings: None

#define KNOWN_KEYS 20

int known_keys[KNOWN_KEYS] = {0x200F, 0xBCD5, 0xE4A8, 0x71DA,
							  0x29FC, 0xA100, 0xBE27, 0x9F4A,
							  0x8E28, 0xD75B, 0x090D, 0x0172,
							  0xD2F7, 0xC567, 0x8FD4, 0xFD5A,
							  0x4DF3, 0xB250, 0x5468, 0xC4B6 };

int option_count = 0;
char room_name[10] = " ";

int rooms_visited[5][10][10];
int doors_tried[5][10][10][4];
int keys_tried[5][10][10][4];

int use_key;
int try_key = -1;
int last_action = -1;

// Function: initRooms
// Description: This function is showing how many rooms you have visited
// Parameters: None
// Returns: None
// Warnings: None

void initRooms()
{
	int floor;
	int ns;
	int ew;
	int door;

	for (floor = 0; floor < 5; floor++)
	{
		for (ns = 0; ns < 10; ns++)
		{
			for (ew = 0; ew < 10; ew++)
			{
				rooms_visited[floor][ns][ew] = 0;

				for (door = 0; door < 4; door++)
				{
					doors_tried[floor][ns][ew][door] = -1;
					keys_tried[floor][ns][ew][door] = 0;
				}
			}
		}
	}
}

// Function: bestDirection
// Description: This function tries to find the best direction for it to move around the maze
// Parameters: None
// Returns: best_direction
// Warnings:  && (floor != 3)) - this will prevent it to stop the server crashing

int bestDirection()
{
	int best_direction = NO_MOVE; // Initialise to indicate no direction yet chosen 
	int room_visits = 20000; // Initialise this to a maximum number that can't ever be reached
	int floor;
	int ns;
	int ew;

	if (sscanf(room.name, "%1dY%1d%1d", &floor, &ns, &ew) == 3) //Parse the room name to get floor, ns and ew variables 
	{
		if (room.direction[DIRECTION_NORTH] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns - 1][ew] < room_visits) //Comparing how many times we have moved north from here 
			{
				room_visits = rooms_visited[floor][ns - 1][ew]; //So far north is the least visited direction
				best_direction = OPTION_MOVE_NORTH;
			}
		}

		if (room.direction[DIRECTION_SOUTH] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns + 1][ew] < room_visits)
			{
				room_visits = rooms_visited[floor][ns + 1][ew];
				best_direction = OPTION_MOVE_SOUTH;
			}
		}

		if (room.direction[DIRECTION_EAST] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns][ew + 1] < room_visits)
			{
				room_visits = rooms_visited[floor][ns][ew + 1];
				best_direction = OPTION_MOVE_EAST;
			}
		}

		if (room.direction[DIRECTION_WEST] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns][ew - 1] < room_visits)
			{
				room_visits = rooms_visited[floor][ns][ew - 1];
				best_direction = OPTION_MOVE_WEST;
			}
		}

		if ((room.direction[DIRECTION_UP] == DIRECTION_OPEN) && (floor != 3)) // this will prevent it to only reach up to 3 floors
		{
			if (rooms_visited[floor + 1][ns][ew] < room_visits)
			{
				room_visits = rooms_visited[floor + 1][ns][ew];
				best_direction = OPTION_MOVE_UP;
			}
		}

		if (room.direction[DIRECTION_DOWN] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor - 1][ns][ew] < room_visits)
			{
				room_visits = rooms_visited[floor - 1][ns][ew];
				best_direction = OPTION_MOVE_DOWN;
			}
		}
	}

	return best_direction;
}

// Function: unlockdoor
// Description: This function tries to see if it can unlock a door
// Parameters: int* use_key
// Returns: MOVE
// Warnings: None

int unlockDoor(int* use_key)
{
	int door;
	int move = NO_MOVE;
	int key;
	int floor;
	int ns;
	int ew;

	if (sscanf(room.name, "%1dY%1d%1d", &floor, &ns, &ew) == 3)
	{
		for (door = 0; door < 4; door++)
		{
			if (room.direction[door] == DIRECTION_LOCKED)
			{
				doors_tried[floor][ns][ew][door]++;
				key = doors_tried[floor][ns][ew][door];

				if (key < KNOWN_KEYS)
				{
					move = OPTION_UNLOCK_NORTH + door;
					//sentOption(move, known_keys[key]);
					*use_key = known_keys[key];
					printf("Move = %d, key = 0x%4X\n", move, known_keys[key]);
					keys_tried[floor][ns][ew][door] = known_keys[key];
					return move;
				}
			}
		}
	}

	return move;
}

// Function: saveKeys
// Description: This function will save keys when you have found a new key 
// Parameters: None
// Returns: None
// Warnings: None

void saveKeys()
{
	int floor;
	int ns;
	int ew;
	int door;

	for (floor = 0; floor < 5; floor++)
	{
		for (ns = 0; ns < 10; ns++)
		{
			for (ew = 0; ew < 10; ew++)
			{
				for (door = 0; door < 4; door++)
				{
					//if ((keys_tried[floor][ns][ew][door] > 0) && (room.direction[door] == DIRECTION_OPEN))
					if ((doors_tried[floor][ns][ew][door] >= 0) && (doors_tried[floor][ns][ew][door] < KNOWN_KEYS) && (room.direction[door] == DIRECTION_OPEN))
					{
						printf("Room %dY%d%d Door %d Key 0x%4X\n", floor, ns, ew, door, keys_tried[floor][ns][ew][door]);
						writekeys2file(keys_tried[floor][ns][ew][door], floor, ns, ew, door);
					}
				}
			}
		}
	}
	//getchar();
	//getchar();
	//getchar();
	//getchar();
}

// Function: getRandomMove
// Description: This function will generate a random move which is valid
// Parameters: None
// Returns: MOVE
// Warnings: None

int getRandomMove()
{
	int move;

	if (strcmp(room_name, room.name) != 0) option_count = 0;
	move = options[option_count];
	option_count = (option_count + 1) % number_of_options;

	return move;
}

// Function: inpickUplist
// Description: This function will show a list of the items you have picked up 
// Parameters: int item_number
// Returns: rc
// Warnings: None

bool inPickUpList(int item_number)
{
	bool rc = false;
	int i;

	for (i = 0; i < number_of_options; i++)
	{
		if (options[i] == (OPTION_BASE_FOR_PICKUPS + item_number))
		{
			return true;
		}
	}

	return rc;
}

// Function: checkItem
// Description: This function checks the best items and removes the worst item if it ever finds a better item that has better value out of the backpack
// Parameters: None
// Returns: MOVE
// Warnings: None

int checkItem()
{
	int best_found_item = 0;
	int best_found_value = -1;
	int best_found_volume = 1;
	float best_found_ratio = -1.0;

	int worst_backpack_item = 0;
	int worst_backpack_value = 1000;
	int worst_backpack_volume = 1;
	float worst_backpack_ratio = 1000.0;

	int i;
	int j;
	int move = NO_MOVE;	//default no move

	//are there any items in the room?
	if (room.number_of_items > 0)
	{
		//loop through the items
		for (i = 0; i < room.number_of_items; i++)
		{
			if (best_found_ratio < (float)room.items[i].value / (float)room.items[i].volume)
			{
				best_found_item = room.items[i].number;
				best_found_value = room.items[i].value;
				best_found_volume = room.items[i].volume;
				best_found_ratio = (float)best_found_value / (float)best_found_volume;
			}
		}

		//did we find a positive value
		if (best_found_value > 0)
		{
			if (inPickUpList(best_found_item))
			{
				//we can pick up an item because our worst_backpack isn't full
				return OPTION_BASE_FOR_PICKUPS + best_found_item;
			}
			else {
				if (backpack.number_of_items > 0)
				{
					for (j = 0; j < backpack.number_of_items; j++)
					{
						if (worst_backpack_ratio > (float)room.items[i].value / (float)room.items[i].volume)
						{
							worst_backpack_item = backpack.items[i].number;
							worst_backpack_value = backpack.items[i].value;
							worst_backpack_volume = backpack.items[i].volume;
							worst_backpack_ratio = (float)backpack.items[i].value / (float)backpack.items[i].volume;
						}
					}

					if (worst_backpack_ratio < best_found_ratio)
					{
						return(OPTION_BASE_FOR_DROPS + worst_backpack_item);
					}
				}
			}
		}
	}

	return move;
}

// Function: Itemvalue
// Description: This function checks the item value
// Parameters: int item
// Returns: return_value
// Warnings: None

int itemValue(int item)
{
	int return_value = -1;

	for (int i = 0; i < room.number_of_items; i++)
	{
		if (item == room.items[i].number) {
			return_value = room.items[i].value;
		}
	}

	return return_value;
}

// Function: readBook
// Description: This function will allow it to read books
// Parameters: None
// Returns: MOVE
// Warnings: None


int readBook()
{
	int move = NO_MOVE;
	int i;
	int option;
	int item;

	for (i = 0; i < number_of_options; i++)
	{
		option = options[i];
		if ((option >= OPTION_BASE_FOR_READS) && (option < OPTION_BASE_FOR_PICKUPS))  //then this is a valid event
		{
			item = option - OPTION_BASE_FOR_READS;

			if (itemValue(item) > 0) //check it has value
			{
				move = option;
				return move;
			}
		}
	}

	return move;
}

// Function: attendEvent
// Description: This function will allow it to attend events/seminars
// Parameters: None
// Returns: MOVE
// Warnings: None

int attendEvent()
{
	int move = NO_MOVE; // default is no event found
	int i;
	int option;

	for (i = 0; i < number_of_options; i++)
	{
		option = options[i];
		if (option >= OPTION_BASE_FOR_EVENTS)  // then this is a valid event
		{
			move = option;
			return move;
		}
	}
	return move;
}

// Function: doStuff
// Description: This function will do stuff e.g. test sheets and guides
// Parameters: None
// Returns: MOVE
// Warnings: None

int doStuff()
{
	int move = NO_MOVE; // default is no dos found
	int i;
	int option;

	for (i = 0; i < number_of_options; i++)
	{
		option = options[i];
		if ((option >= OPTION_BASE_FOR_DOS) && (option < OPTION_BASE_FOR_EVENTS))  // then this is a dos
		{
			move = option;
			return move;
		}
	}
	return move;
}

// Function: pickupstuff
// Description: This function will allow to pick up items 
// Parameters: None
// Returns: MOVE
// Warnings: None

int pickupStuff()
{
	int move = NO_MOVE;
	int i;

	if (room.number_of_items > 0 && backpack.number_of_items <= 20)
	{
		if (room.number_of_items > 0)
		{
			for (i = 0; i < room.number_of_items; i++)
			{
				if (room.items[i].value > 0 && room.items[i].number < 300)
				{
					printf("Move = %d\n", OPTION_BASE_FOR_PICKUPS + room.items[i].number);
					move = OPTION_BASE_FOR_PICKUPS + room.items[i].number;
					return move;
				}
			}
		}
	}

	return move;
}

// Function: check_unlocked
// Description: This function will check the keys that you have found and check to see if it can unlock a closed door
// Parameters: None
// Returns: None
// Warnings: None

void check_unlocked()
{
	int unlock = false;

	if (last_action >= OPTION_UNLOCK_NORTH && last_action <= OPTION_UNLOCK_WEST)
	{
		printf("Checking if unlock worked\n");

		switch (last_action)
		{
		case OPTION_UNLOCK_NORTH:
			if (room.direction[DIRECTION_NORTH] == DIRECTION_OPEN)
			{
				unlock = true;
			}
			break;

		case OPTION_UNLOCK_SOUTH:
			if (room.direction[DIRECTION_SOUTH] == DIRECTION_OPEN)
			{
				unlock = true;
			}
			break;

		case OPTION_UNLOCK_EAST:
			if (room.direction[DIRECTION_EAST] == DIRECTION_OPEN)
			{
				unlock = true;
			}
			break;

		case OPTION_UNLOCK_WEST:
			if (room.direction[DIRECTION_WEST] == DIRECTION_OPEN)
			{
				unlock = true;
			}
			break;

		default:
			break;
		}

		if (unlock == true)
		{
			printf("!!!WE UNLOCKED THE DOOR WITH KEY %x\n", use_key);
			saveKeys();
		}

		else
		{
			printf("Door did not unlock\n");
		}
	}
}

// Function: yourMove
// Description: This function will call the other functions to make the moves
// Parameters: None
// Returns: None
// Warnings: By commenting useWorkingKeys(&use_key) and remove the comment of unlockDoor(&use_key) - this will get the keys from the mykeyfile 
//           By commenting readbook, attendevents, etc - the score will be less.


void yourMove()
{
	int move = NO_MOVE;  // no valid move assigned yet
	int i;
	char chr;
	int floor;
	int ns;
	int ew;

	// Record where we have been...

	floor = room.name[0] - '0';
	ns = room.name[2] - '0';
	ew = room.name[3] - '0';

	if (sscanf(room.name, "%1dY%1d%1d", &floor, &ns, &ew) == 3)
	{
		rooms_visited[floor][ns][ew]++;  // record where you have been
	}

	//check if door unlock was successful
	check_unlocked();

	if (move == NO_MOVE) move = readBook();
	if (move == NO_MOVE) move = doStuff();
	if (move == NO_MOVE) move = attendEvent();
	//if (move == NO_MOVE) move = pickupStuff();
	if (move == NO_MOVE) move = checkItem();
	if (move == NO_MOVE) move = useWorkingKeys(&use_key); // Didn't pick anything up, so try unlocking a door
	//if (move == NO_MOVE) move = unlockDoor(&use_key);     // Didn't pick anything up, so try unlocking a door
	if (move == NO_MOVE) move = bestDirection();          // it didn't try to unlock a door, so move in best direction
	if (move == NO_MOVE) move = getRandomMove();          // always returns a valid move

	if (student.number_of_moves < 2000)
	{
		sentOption(move, use_key);
	}


	// Store the last action taken 
	last_action = move;

	printf("Move = %d\n", move);

	if (_kbhit())
	{
		chr = getchar();
		if (chr == 's')
		{
			saveKeys();
		}
	}
}

/*************************************************************/
/********* Your tactics code ends here ***********************/
/*************************************************************/

int getTokens(char* instring, char seperator)
{
	int  number_of_tokens;
	char chr;
	int  state;
	int  i;
	int  j;


	for (i = 0; i < MAX_NO_TOKENS; i++)
	{
		for (j = 0; j < MAX_STRING_SIZE; j++)
		{
			Tokens[i][j] = '\0';
		}
	}

	number_of_tokens = -1;
	chr = instring[0];
	state = 0;
	i = 0;

	while (chr != '\0')
	{
		switch (state)
		{
		case 0:  // Initial state
			if (chr == seperator)
			{
				number_of_tokens++;
				state = 1;
			}
			else if ((chr == ' ') || (chr == '\t') || (chr == '\n'))
			{
				state = 1;
			}
			else
			{
				number_of_tokens++;
				j = 0;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		case 1:  // Leading white space
			if (chr == seperator)
			{
				number_of_tokens++;
				state = 1;
			}
			else if ((chr == ' ') || (chr == '\t') || (chr == '\n'))
			{
				state = 1;
			}
			else
			{
				number_of_tokens++;
				j = 0;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		case 2:  // Collecting chars
			if (chr == seperator)
			{
				//number_of_tokens++;
				state = 1;
			}
			else
			{
				j++;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		default:
			break;
		}

		i++;
		chr = instring[i];
	}

	return (number_of_tokens + 1);
}



bool getline(FILE* fp, char* buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);

		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}



void printRoom()
{
	int i;

	printf("Room\n");
	printf("Room = %s, Room type = %d\n", room.name, room.type);

	printf("Directions = ");
	for (i = 0; i < 6; i++)
	{
		printf("%d  ", room.direction[i]);
	}
	printf("\n");

	if (room.number_of_keys > 0)
	{
		printf("Keys = ");
		for (i = 0; i < room.number_of_keys; i++)
		{
			printf("0x%X  ", room.keys[i]);
		}
		printf("\n");
	}
	else
	{
		printf("No keys in this room\n");
	}

	if (room.number_of_items > 0)
	{
		for (i = 0; i < room.number_of_items; i++)
		{
			printf("Item=%d, Value=%d, Volume=%d\n", room.items[i].number, room.items[i].value, room.items[i].volume);
		}
	}
	else
	{
		printf("No items in this room\n");
	}

	printf("\n");
}


void printStudent()
{
	printf("Student\n");
	printf("Level=%d,  Number of rooms visited = %d,  Number of doors openned = %d,  Number of moves = %d,  Score = %d\n", student.level, student.rooms_visited, student.doors_openned, student.number_of_moves, student.score);
	printf("\n");
}


void printBackpack()
{
	int i;

	printf("Backpack\n");

	if (backpack.number_of_items > 0)
	{
		for (i = 0; i < backpack.number_of_items; i++)
		{
			printf("Item=%d, Value=%d, Volume=%d\n", backpack.items[i].number, backpack.items[i].value, backpack.items[i].volume);
		}
	}
	else
	{
		printf("Your backpack is empty\n");
	}
	printf("\n");
}


void printOptions()
{
	int i;

	printf("Options\n");
	printf("Options = ");
	for (i = 0; i < number_of_options; i++)
	{
		printf("%d  ", options[i]);
	}
	printf("\n");
	printf("\n");
}




void communicate_with_server()
{
	char buffer[4096];
	int  len = sizeof(SOCKADDR);
	int  i;
	char* p;
	int	 number_of_tokens;


	sprintf_s(buffer, "Register  %s %s %s", STUDENT_NUMBER, STUDENT_FIRSTNAME, STUDENT_FAMILYNAME);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));

	while (true)
	{
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&client_addr, &len) != SOCKET_ERROR)
		{
			p = ::inet_ntoa(client_addr.sin_addr);

			if ((strcmp(IP_ADDRESS_SERVER, "127.0.0.1") == 0) || (strcmp(IP_ADDRESS_SERVER, p) == 0))
			{
				printf("%s\n\n", buffer);

				number_of_tokens = getTokens(buffer, '|');

				if (number_of_tokens == 6)
				{
					strcpy(text_student, Tokens[0]);
					strcpy(text_backpack, Tokens[1]);
					strcpy(text_room, Tokens[2]);
					strcpy(text_keys, Tokens[3]);
					strcpy(text_items, Tokens[4]);
					strcpy(text_options, Tokens[5]);

					printf("Student  = '%s'\n", text_student);
					printf("Backpack = '%s'\n", text_backpack);
					printf("Room     = '%s'\n", text_room);
					printf("Keys     = '%s'\n", text_keys);
					printf("Items    = '%s'\n", text_items);
					printf("Options  = '%s'\n", text_options);

					student.level = -1;
					student.rooms_visited = -1;
					student.doors_openned = -1;
					student.number_of_moves = -1;

					if (sscanf(text_student, "%d,%d,%d,%d,%d", &student.level, &student.rooms_visited, &student.doors_openned, &student.number_of_moves, &student.score) == 5)
					{
					}

					if (strlen(text_backpack) > 0)
					{
						backpack.number_of_items = getTokens(text_backpack, '&');

						if (backpack.number_of_items > 0)
						{
							for (i = 0; i < backpack.number_of_items; i++)
							{
								if (i < MAX_ITEMS_IN_BACKPACK)
								{
									backpack.items[i].number = -1;

									if (sscanf(Tokens[i], "%d, %d, %d", &backpack.items[i].number, &backpack.items[i].value, &backpack.items[i].volume) == 3)
									{
									}
								}
							}
						}
					}
					else
					{
						backpack.number_of_items = 0;
					}

					sscanf(text_room, "%s ,%d, %d, %d, %d, %d, %d, %d", &room.name, &room.type, &room.direction[DIRECTION_NORTH], &room.direction[DIRECTION_SOUTH], &room.direction[DIRECTION_EAST], &room.direction[DIRECTION_WEST], &room.direction[DIRECTION_UP], &room.direction[DIRECTION_DOWN]);

					if (strlen(text_keys) > 0)
					{
						room.number_of_keys = getTokens(text_keys, '&');

						if (room.number_of_keys > 0)
						{
							for (i = 0; i < room.number_of_keys; i++)
							{
								if (i < 4)
								{
									room.keys[i] = -1;

									if (sscanf(Tokens[i], "%x", &room.keys[i]) == 1)
									{
									}
								}
							}
						}
					}
					else
					{
						room.number_of_keys = 0;
					}

					if (strlen(text_items) > 0)
					{
						room.number_of_items = getTokens(text_items, '&');

						if (room.number_of_items > 0)
						{
							for (i = 0; i < room.number_of_items; i++)
							{
								if (i < MAX_ITEMS_IN_ROOM)
								{
									room.items[i].number = -1;

									if (sscanf(Tokens[i], "%d, %d, %d", &room.items[i].number, &room.items[i].value, &room.items[i].volume) == 3)
									{
									}
								}
							}
						}
					}
					else
					{
						room.number_of_items = 0;
					}

					if (strlen(text_options) > 0)
					{
						number_of_options = getTokens(text_options, ',');

						if (number_of_options > 0)
						{
							for (i = 0; i < number_of_options; i++)
							{
								if (i < MAX_OPTIONS)
								{
									options[i] = -1;

									if (sscanf(Tokens[i], "%d", &options[i]) == 1)
									{
									}
								}
							}
						}
					}
					else
					{
						number_of_options = 0;
					}
				}

				printStudent();
				printBackpack();
				printRoom();
				printOptions();

				//control pause
				//system("timeout /t 120");

				yourMove();
			}
		}
		else
		{
			printf_s("recvfrom error = %d\n", WSAGetLastError());
		}
	}

	printf_s("Student %s\n", STUDENT_NUMBER);
}




int main()
{
	char chr = '\0';

	printf("\n");
	printf("The Computer Maze Student Program\n");
	printf("UWE Computer and Network Systems Assignment 2 \n");
	printf("\n");

	initRooms();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	//sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	//if (!sock)
	//{	
	//	printf("Socket creation failed!\n"); 
	//}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock)
	{
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	//int ret = bind(sock_recv, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	////	int ret = bind(sock_send, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	//if (ret)
	//{
	//	printf("Bind failed! %d\n", WSAGetLastError());
	//}

	communicate_with_server();

	closesocket(sock);
	WSACleanup();

	while (chr != '\n')
	{
		chr = getchar();
	}

	return 0;
}
