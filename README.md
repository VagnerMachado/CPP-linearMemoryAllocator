# CPP-linearMemoryAllocator

 This project simulates a linear memory allocator application. The memory is initially
set to have  its first byte at location 0 and last available memory byte is
represented by the integer passed as program argument during execution, minus 1. When
the program loads, the user sees the prompt 'allocator >' to which the user can
input commands like the following space separated or tab separated commands examples in a line:    
* RQ P3 1024 B - Requests memory for P3 using best fit strategy for 1024 bytes allocation. Case there is not enough memory, user is warned, request is rejected.     
* RL P3 - Releases the memory allocated to P3. Case P3 is not a valid name for allocated process, the user is warned and release is rejected.   
* STAT - Prints status of memory, start and final byte, and either process number or FREE if memory is not allocated.   
* C - Compacts the memory by shifting free space towards the higher bytes in memory.
* QUIT - Quits the program.  
* HELP - Prints program usage during runtime, program does not quit.    
	
## Note  
	
All commands are case sensitive and preferably should not contain spaces in the beginning or
end of input line for guaranteed execution. For this project, B strategy, best fit, is the only
option and any other value passed other than B for the fourth item of RQ will cause
allocation to be rejected. Multiple space and tab trimming for input line was implemented, but
single space separation between entries in the line is still preferred for optimal execution.
		  
## Usage    
	
Instruction on how to run the program:    
* g++ -c allocator_VMachado.cpp          - Compiles the source code in to object file  
* g++ -c VMapp.exe allocator_VMachado.o  - Links the object file to executable.  
* ./VMapp.exe 1048576                    - Runs and allocates 1MB for the allocator app.  
* ./VMapp.exe -help                      - Prints usage information  
* PS: 1MB is lowest value allowed and will get overwritten to 1048576. Any value
	over 1048576 will extend memory.
				
## Memory slot range   
	
[ 0      :   argv[1] - 1 ]    
Where argv[1] is argument passed when executing the program and 
which lowest value can be 1048576, representing 1MB.


