//============================================================================
// Name        : allocator_VMachado.cpp
// Author      : Vagner Machado
// Version     : 1.0
// Copyright   : Can be used for legal purposes
// Description : See below.


/* This project simulates a linear memory allocator application. The memory is initially
set to have  its first byte at location 0 and last available memory byte is
represented by the integer passed as program argument during execution, minus 1. When
the program loads, the user sees the prompt 'allocator >' to which the user can
input commands like the following space separated or tab separated commands examples in a line:

	a. RQ P3 1024 B		Requests memory for P3 using best fit strategy for 1024 bytes allocation.
						Case there is not enough memory, user is warned, request is rejected.

	b. RL P3			Releases the memory allocated to P3.
						Case P3 is not a valid name for allocated process, the user is warned and release is rejected.

	c. STAT				Prints status of memory, start and final byte, and either process number or FREE if memory is not allocated.

	d. C				Compacts the memory by shifting free space towards the higher bytes in memory.

	e. QUIT				Quits the program.

	f. HELP				Prints program usage during runtime, program does not quit.


	Note: All commands are case sensitive and preferably should not contain spaces in the beginning or
		  end of input line for guaranteed execution. For this project, B strategy, best fit, is the only
		  option and any other value passed other than B for the fourth item of RQ will cause
		  allocation to be rejected. Multiple space and tab trimming for input line was implemented, but
		  single space separation between entries in the line is still preferred for optimal execution.

	Usage:  Instruction on how to run the program:
			g++ -c allocator_VMachado.cpp          Compiles the source code in to object file
			g++ -c VMapp.exe allocator_VMachado.o  Links the object file to executable.
			./VMapp.exe 1048576                    Runs and allocates 1MB for the allocator app.
            ./VMapp.exe -help                      Prints usage information
			PS: 1MB is lowest value allowed and will get overwritten to 1048576. Any value
				over 1048576 will extend memory.

	Memory slot range:     [ 0      :   argv[1] - 1 ]

							Where argv[1] is argument passed when executing the program and 
							which lowest value can be 1048576, representing 1MB.

==============================================================================*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <stdio.h>


//============================================================================
/*
 * Process class
 * Defines instance data and methods needed to instantiate and manipulate Processes
 */

class Process
{
private:
	//instance data
	std::string processNumber;
		int size;
		int firstByte;
		int lastByte;
		Process * next;

		//Targeted encapsulation
		friend class LinkedList;
public:

	/**
	 * Process constructor accepts process name and size needed for allocation
	 */
	Process(std::string num, int s)
	{
		processNumber = num;
		size = s;
		firstByte = 0;
		lastByte = 0;
		next = NULL;
	}

	/**
	 * toString - returns a formatted string with the process' instance data
	 */
	std::string toString()
	{
		std::stringstream ss;
		char buffer[50];
		sprintf(buffer, "%7d : %7d", firstByte, lastByte);
		ss << "Address [ " << buffer << " ] Process " << processNumber;
		return ss.str();
	}
};

//===========================================================================
/*
 * Linked List class
 * Defines the instance data and methods needed to create a linked list
 * to serve as a list of processes allocated in a memory slot
 */

class LinkedList
{
private:
	//instance data
	Process * head;
	int number;    //added a tracker variable, so far unused, but well implemented
public:
	/*
	 * LinkedList constructor
	 * Instantiates a linked lists with head and tail having NULL values.
	 */
	LinkedList()
	{
		head = NULL;
		number = 0;
	}

	/*
	 * addProcessToList -  adds the Process passed as parameter to the
	 * linked list at the specified position. Process is added either to:
	 * 		. empty linked list
	 * 		.not empty linked list
	 *
	 * Those two scenarios are analyzed before adding Process
	 * Method is called after findSpotInList() returning the best fit place in linked list.
	 * NULL processes are rejected.
	 */
	void addProcessToList(Process * p, int position)
	{
		//std::cout << "PUT AT POSITION " << position << "\n";
		//rejects NULL processes
		if (p == NULL)
			return;

		//case position is zero, there can be processes after or list can be empty
		Process * traverse = head;
		if (position == 0)
		{
			if(head == NULL) //empty list
			{
				head = p;
				p->firstByte = 0;
				p->lastByte = p-> size - 1;
			}
			else  			//not empty list
			{
				p->firstByte = 0;
				p->lastByte = p-> size - 1;
				p->next = head;
				head = p;
			}
			return;
		}

		//otherwise iterate to position
		int tracker = 0;
		while(tracker != (position - 1))
		{
			traverse = traverse->next;
			tracker++;
		}

		//update instance data for new process
		p->firstByte = traverse->lastByte+1;
		p->lastByte = p->firstByte + p->size - 1;
		p->next = traverse->next;
		traverse->next = p;
		number++;
		return;
	}

	/*
	 * removeProcessbyNumber - removes a process in the linked list by processNumber
	 * A Process to be deleted can be either in
	 * 		. head
	 * 		. tail
	 * 		. middle
	 *
	 * 	Those three cases are inspected to perform a deletion.
	 * 	The process name must match in order to delete (p1, p2, ...)
	 *
	 *  deleteProcess: indicates if process removed must also be deleted (deallocated).
	 *  Returns the size of the process removed, or -1 if the process does not exist
	 *  in the list of allocated processes
	 */
	int removeProcessByNumber(std::string procNum, bool deleteProcess)
	{
		//traverser and follower
		Process *traverse = head;
		Process *previous = head;

		//traverse the linked list searching for process name
		while (traverse != NULL)
		{
			//case a match
			if ((procNum.compare(traverse->processNumber)) == 0)
			{
				//case match in the head
				if (traverse == head)
				{
					if (head->next != NULL) //case head is not the only process
						head = head->next;
					else
						head = NULL;		//case head was the only one process
				}

				//case match is in the middle or end
				else
					previous->next = traverse->next;

				//free memory
				if (deleteProcess)
				{
					delete traverse;
					number--;
					return traverse->size;
				}
				//keep memory
				else
				{
					traverse->next = NULL;
					number--;
					return traverse->size;
				}
			}
			//otherwise step forward to compare next
			else
			{
				if (traverse != head)
					previous = previous->next;
				traverse = traverse->next;
			}
		}
		return -1; //case not a match for parameter is found
	}

	/**
	 * findSpotInList - finds a spot in the linked list of processes to
	 * allocate a chunk of size 'size' using the best fit strategy, leaving the
	 * smallest possible free gap between the newly inserted process and its neighbors.
	 * This is just a beautiful function, proud of it!
	 */
	int findSpotInList(int size, int max)
	{
		//if list is empty add to beginning
		if(head == NULL)
		{
			if(size <= max +1)
				return 0;    // case memory is empty and process fits
			else
				return -1;   //though free size > memory size. Last case to be found!
		}

		//assume the gap is largest value in system
		int leftGap = INT_MAX;

		//winner returns -1 if there is not place in memory for the size
		int winner = -1;

		//counts the hops needed to get to the best place
		int counter = 0;

		//traverser
		Process * trav = head;

		//case the first process is not using byte zero
		if(trav->firstByte != 0)
		{
			int tempGap = trav->firstByte;
			//if gap fits the size, make it the winner so far
			if(tempGap >= size )
			{
				winner = 0;
				leftGap = trav->firstByte - size;
			}
		}

		counter = 1;

		//other spaces are checking in a fragmented memory
		while(trav != NULL)
		{
			//if you get to the end, see if last byte of last process is last byte in memory
			if(trav->next == NULL)
			{
				int tempGap = max - trav->lastByte;
				if(tempGap >= size && tempGap - size < leftGap)
				{
					winner = counter; //set hops to here
					leftGap = tempGap - size;
				}
			}
			//case the match is in a gap betwen processes
			else if(trav->lastByte < trav->next->firstByte -1)
			{
				int tempGap = trav->next->firstByte - trav->lastByte - 1;
				if(tempGap >= size && tempGap - size < leftGap)
				{
					winner = counter; //set hops to here
					leftGap = tempGap - size;
				}
			}
			counter++; //increment hop
			trav = trav->next;
		}
		return winner;
	}

	/*
	 * toString - Return a string with data for the Processes in the linked list
	 * by calling the toString defined in class Process. Case there are not processes
	 * it returns free. Another beautiful method ;)
	 */
	std::string toString(int max)
	{
		std::stringstream ss;
		ss << "\n";

		Process * traverse = head;
		//case the list is  totally empty
		if (traverse == NULL)
		{
			char buffer[25];
			sprintf(buffer, "%7d : %7d", 0, max);
			ss << "Address [ " << buffer << " ] " << "Free\n\n";
			return ss.str();
		}
		int delimiter = 0;

		//traverse to produce printouts
		while (traverse != NULL)
		{
			//case there is not gap between adjacent processes, print process, update delimiter
			if(traverse->firstByte == delimiter)
			{
				ss << traverse->toString() << "\n";
				delimiter = traverse->lastByte + 1;
			}
			//case there is a gap between adjacent processes, update delimiter, print gap and process.
			else
			{
				char buffer[50];
				sprintf(buffer, "%7d : %7d", delimiter, traverse->firstByte - 1);
				ss << "Address [ " << buffer << " ] " << "Free\n";
				ss << traverse->toString() << "\n";
				delimiter = traverse->lastByte + 1;
			}
			//step forward
			traverse = traverse->next;
		}
		//case the last process is not using the last available byte, print last gap
		if(delimiter != max + 1)
		{
			char buffer[50];
			sprintf(buffer, "%7d : %7d", delimiter, max);
			ss << "Address [ " << buffer << " ] " << "Free\n";
		}
		ss << "\n";
		return ss.str();
	}

	/*
	 * compactProcesses - compacts the processes in the list towards the low memory
	 * bytes, placed side by side without gap and leaving free memory, if any,
	 * towards the high bytes in memory
	 */
	void compactProcesses(int lastByte)
	{
		//if list is empty, no compaction needed
		if(head == NULL)
			return;

		//traverser through the processes in the list
		Process  * trav = head;

		//custom update the first process addresses
		trav->firstByte = 0;
		trav->lastByte = trav->size + trav->firstByte - 1;

		//set delimiter for iterations
		int nextFirst = trav->lastByte + 1;
		trav = trav->next;

		//iterate throught the processes and update the first and last byte for each one, update the delimiter
		while(trav != NULL)
		{
			trav->firstByte = nextFirst;
			trav->lastByte = trav->size + trav->firstByte - 1;
			nextFirst = trav->lastByte + 1; //delimiter
			trav = trav->next;
		}
	}

}; //end of LinkedList class

//======================================================================
/*
 *  Memory class- defines the instance data and methods needed to
 * instantiate manipulate a linear chunk of memory
 */
class Memory
{

private:
	LinkedList processes;
	int max;

public:

	/*
	 * Memory constructor - defined the number of available bytes to be allocated.
	 * Note the value - 1 is passed into constructor to account for the max - 1
	 * memory requirement
	 */
	Memory(int m)
{
		max = m;
}
	/**
	 * findMemorySlot - Finds available memory for required
	 * size using the  best fit strategy. Calls the linked list method
	 * that seeks the best place for the process with required size.
	 * Passes max to it so process list know the last byte to
	 * be analyzed.
	 */ 
	int findMemorySlot(int sizeRequired)
	{
		return processes.findSpotInList(sizeRequired, max);
	}


	/**
	 * toString - returns a string with the information specified in
	 * the assignment
	 */ 
	std::string toString()
	{
		return processes.toString(max);
	}

	/**
	 * addProcessToMemory - adds a process location specified by place
	 * parameter. Used after findMemorySlot returns the best fit location
	 * for the process.
	 */ 
	void addProcessToMemory(Process * p, int place)
	{
		processes.addProcessToList(p, place);
	}

	/**
	 * removeProcessFromMemory - Attempts to remove a process by name. If the Process
	 * exists, it is removed and true is returned, else the process does not exist
	 * and then false is returned and no removal is done. Case processes have the same
	 * name, only the first match starting from low memory is removed.
	 */ 
	bool removeProcessFromMemory(std::string procName)
	{
		int value = processes.removeProcessByNumber(procName, false);
		if(value < 0)
			return false;
		return true;
	}

	/*
	 * compactMemory - compacts the linear memory as defined in LinkedList class.
	 * Moves processes towards low end of bytes and frees up space towards
	 * the high bytes in memory. A method to a method is used ofr encapsulation.
	 */
	void compactMemory(int lastByte)
	{
		processes.compactProcesses(lastByte);
	}

}; //end of Memory class 

//=================================================================

/**
 * class Allocator - Defines the behavior of a memory allocator app
 */ 
class Allocator
{
private:
	/**
	 * parseUserInput - Process and validates a line of input from
	 * the user. Returns an array of strings with data entered by 
	 * the user. Trims spaces and tabs from beginning, end, and multiple spaces between commands.
	 * Also trims tabs. Best to have no spaces in beginning and end, and just
	 * a space between words in same command. Nice flow for the custom method.
	 */ 
	std::string * parseUserInput(std::string userInput)
	{
		//array for the results, initiate all to empty.
		std::string *result = new std::string[4];
		for (int i = 0; i < 4; i++)
			result[i] = "";

		//skip any possible leading blanks
		int place = 0;
		char x = userInput[place++];
		while ((x == ' ' || x == '\t') && place < userInput.size() + 1)
			x = userInput[place++];

		//get the first space delimited entry
		while (x != ' ' && x != '\t' && place < userInput.size() + 1)
		{
			result[0] += x;
			x = userInput[place++];
		}

		//check if it is a one entry command, if so return array with it.
		if (strcmp(result[0].c_str(), "STAT") == 0 ||
				strcmp(result[0].c_str(), "C") == 0 ||
				strcmp(result[0].c_str(), "QUIT") == 0 ||
				strcmp(result[0].c_str(), "HELP") == 0)
			return result;

		//skip multiple spaces between entries
		while ((x == ' ' || x == '\t') && place < userInput.size() + 1)
			x = userInput[place++];

		//get the second entry
		while (x != ' ' && x != '\t' && place < userInput.size() + 1)
		{
			result[1] += x;
			x = userInput[place++];
		}

		// check it it is a two entry command, if so return array.
		if (strcmp(result[0].c_str(), "RL") == 0)
			return result;

		//skip possible blanks between entries
		while ((x == ' ' || x == '\t') && place < userInput.size() + 1)
			x = userInput[place++];

		//get third entry, case it  it a RQ
		while (x != ' ' && x != '\t' && place < userInput.size() + 1)
		{
			result[2] += x;
			x = userInput[place++];
		}

		//skips possible blanks between entries
		while ((x == ' ' || x == '\t') && place < userInput.size() + 1)
			x = userInput[place++];

		//get fourth entry
		while (x != ' ' && x != '\t' && place < userInput.size() + 1)
		{
			result[3] += x;
			x = userInput[place++];
		}
		//finally return array with four entries. 
		return result;
	}

	/**
	 * printUsage - returns a string with usage contents for the user. This can be accessed 
	 * during app execution with -help argument or by entering HELP to allocator>  during
	 * the program runtime, which does not halt the execution.
	 */ 
	std::string printUsage()
	{
		std::stringstream ss;
		ss <<
				"\n	This project simulates a linear memory allocator application. The memory is initially\n" <<
				"	set to have its first byte at location 0 and last available memory byte is \n" <<
				"	represented by the integer passed as program argument during execution, minus 1. When \n" <<
				"	the program loads, the user sees the prompt 'allocator >' to which the user can\n" <<
				"	input commands like the following space separated or tab separated example commands in a line:\n\n" <<
				"	a. RQ P3 1024 B	Requests memory for P3 using best fit strategy for 1024 bytes allocation.\n" <<
				"			Case there is not enough memory, user is warned, and request is rejected.\n\n" <<
				"	b. RL P3	Releases the memory allocated to P3.\n" <<
				"			Case P3 is not a valid allocated process name, the user is warned and release command is rejected.\n\n" <<
				"	c. STAT		Prints status of memory, start and final byte, and process name or FREE if memory is not allocated .\n\n" <<
				"	d. C		Compacts the memory by shifting free space towards the higher bytes in memory.\n\n" <<
				"	e. QUIT		Quits the program.\n\n" <<
				"	f. HELP		Prints program usage during runtime, program does not quit.\n\n" <<
				"	Note: All commands are case sensitive and should not contain spaces in the beginning or \n" <<
				"	      end of input line for guaranteed execution. For this project, B strategy, best fit, is the only option and any other value\n" <<
				"	      passed other than B for the fourth item of RQ will cause allocation to be rejected. Multiple space and tab trimming\n" <<
				"	      for input line was implemented, but single space separation between entries in the line is still preferred for optimal execution.\n\n" <<
				"	Usage:  Instruction for how to run the program:\n" <<
				"			g++ -c allocator_VMachado.cpp          Compiles the source code in to object file\n" <<
				"			g++ -c VMapp.exe allocator_VMachado.o  Links the object file to executable.\n" <<
				"			./VMapp.exe 1048576                    Runs and allocates 1MB for the allocator app.\n" <<
				"			./VMapp.exe -help                      Prints usage information\n\n" <<
				"			PS: 1MB is lowest value allowed and will get overwritten to 1048576. Any value\n" <<
				"			    over 1048576 will extend memory allocation.\n\n" <<
				"	Memory slot range: [ 0      :   argv[1] - 1 ] \n\n"  <<
				"	Where argv[1] is argument passed when executing the program and \n" <<
				"	which lowest value can be 1048576, representing 1MB.\n\n";
		return ss.str();
	}

	/**
	 * validateArguments - validates the arguments passed in by the user 
	 * when executing the program. Possible arguments are -help for the 
	 * usage information OR a integer greater than 1048576 as the high
	 * byte for the memory cluster. Any value lower to this will default to 1048576
	 * while any value greater than it will extend memory available for allocation.
	 */ 
	int validateArguments(int count, char ** values)
	{
		//no argument passed. I guess i could also default to 1048576, user should read info!
		if(count == 1)
			return -1;

		//usage information
		else if(strcmp(values[1], "-help") == 0)
			return -2;

		//attempt to parse argument as integer and catch exception case it fails
		else
		{
			int input = 0;
			try
			{
				input = stoi(std::string(values[1]));
			}
			//if the value passed is not -help nor an integer, error occurs. Halt. Houston, we have a problem!
			catch(const std::invalid_argument &x)
			{
				std::cout << "\nERROR: Argument must be an integer or -help. Low values default to 1048576, or 1MB\n";
				std::cout << "        ***  " << x.what() << "  ***\n";
				return -1;
			}
			if(input > 1048576)
				return input;
		}
		return 1048576;
	}

public:
	/**
	 * Allocator constructor - initiates a allocator object.
	 */
	Allocator()
{
	 //nothing special
}
	/**
	 * run - runs the memory allocator with the arguments passed in by the user.
	 * Arguments are parsed, validated and used if they are well formatted. Error 
	 * messages will be provided case the arguments are invalid. Wrong commands passed
	 * during runtime do not halt execution, instead the provide error messages. 
	 */
	int run(int count, char ** values)
	{
		//validate arguments
		int upperBound = validateArguments(count, values);

		//case no argument
		if (upperBound == -1)
		{
			std::cout << "\n	** ERROR: Integer argument required. See usage. ** \n\n" << printUsage();
			return -1;
		}

		//case need usage information
		else if (upperBound == -2)
		{
			std::cout << "\n	** Here is some helpful information **\n\n" << printUsage();
			return -1;
		}

		 //subtract for max - 1 requirement
		Memory memory(--upperBound);

		//controls the end of loop 
		bool done = false;

		//stores the user input line
		std::string userInput;

		//loop while command != QUIT
		while (!done)
		{
			//display prompt and collect input from user
			userInput = "";
			std::cout << "allocator> ";
			std::getline(std::cin, userInput);

			//process the input from the user
			std::string *result = parseUserInput(userInput);

			//case a memory request
			if (strcmp(result[0].c_str(), "RQ") == 0)
			{
				//must be best fit strategy
				if(result[3].compare("B") == 0)
				{
					//attemPt to parse third entry as integer, reject command if it fails.
					int size;
					bool error = false;
					try
					{
						//attempted parse
						size = stoi(result[2]);
					}
					catch(const std::invalid_argument &x)
					{
						error = true;
					}
					//reject user command case process does not have an integer size
					if(error || size <= 0)
						std::cout << "Request rejected, third parameter must be a positive integer. Enter HELP for more information.\n";

					//otherwise attempt to allocate memory for request. If there is enough memory, memory is allocated
					//else command is rejected and user is warned.
					else
					{
						int slot = memory.findMemorySlot(size);

						//case not enough memory
						if (slot == -1)
						{
							std::cout << "There is not enough memory to load " << result[2] << " bytes. Enter HELP for more information.\n";
						}

						//case there is a slot with enough memory, allocate
						else
						{
							memory.addProcessToMemory(new Process(result[1], size), slot);
						}
					}
				}

				// case B is not the allocation strategy passes by the user
				else
				{
					std::cout << "Invalid Parameter for Allocation Strategy, \'B\' is only option. Enter HELP for more information.\n";
				}
			}

			//case user asks for help, usage information is printed
			else if (strcmp(result[0].c_str(), "HELP") == 0)
			{
				std::cout << printUsage();
			}

			// case command is to release memory, attempt to release
			else if (strcmp(result[0].c_str(), "RL") == 0)
			{
				bool done = memory.removeProcessFromMemory(result[1]);

				//case process name does not exist, user is warned and command rejected
				if(!done)
					std::cout << "Process does not exist. Enter HELP for more information.\n";
			}

			//case memory is to be compacted
			else if (strcmp(result[0].c_str(), "C") == 0)
			{
				memory.compactMemory(upperBound);
			}

			//case memory status is to be printed
			else if (strcmp(result[0].c_str(), "STAT") == 0)
			{
				std::cout << memory.toString();
			}

			// case the program is to terminate
			else if (strcmp(result[0].c_str(), "QUIT") == 0)
			{
				done = true;
				std::cout << "\nAllocator terminated\n";
			}

			//catch all 
			else
			{
				std::cout << "Invalid Input. Enter HELP for more information.\n";
			}
		}
		return 0;
	}
};

/**
 * main function - instantiates and runs an allocator.
 * Returns the value returned by the allocator where
 * 0 indicates successful execution and negative values
 * indicated the execution failed. 
 */ 
int main(int argc, char ** argv)
{
	Allocator allocator;
	return allocator.run(argc, argv);
}


//  Vagner Machado  -  QCID 23651127  -  COVID Spring 2020


/*TEST DATA

RQ P0 262144 B
RQ P1 262144 B
RQ P2 262200 B
STAT
RL P1
RQ P3 200000 B
STAT
RQ P4 200000 B
STAT
C
STAT
QUIT
 */
