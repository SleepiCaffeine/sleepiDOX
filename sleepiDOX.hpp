#pragma once
#include <string>
#include <fstream>
#include <array>



#define ENTRY_FUNCTION_NAME 0
#define ENTRY_COMMENT 1
#define ENTRY_RETURNS 2
#define ENTRY_PARAMS 3
#define ENTRY_FUNCTION_DEFINTION 4
#define ENTRY_FUNCTION_CLASS 5

using DOXEntry = std::array<std::string, 5>;


std::ifstream openReadFile(const char* fileName);
std::ofstream openWriteFile(const char* fileName);
	/*
	*   How this will work - sleepiDOX will scan each line of every file provided.
	*	It will look for "keys", the main one will be "@sleepiDOX"
	*	Upon finding said key, it will start saving each line it reads until it encounters a function declaration.
	* 
	*	The method will work as such:
	*		- Scan every line
	*		- If the beginning of the line contains @sleepiDOX:
	*			- Create a basic template of what the entry will look like: "Returns", "Functionality", "Parameters"
	*			- Add The entire comment* into "Functionality"
	*			_ *: there are reserved keys like @Returns, @Param, @Warning, @TODO, @Heading# which will have their own separate entries, and will overrdie defaults
	*			- When sleepiDOX reaches the function declaration - it will add the "Returns" and "Parameters" properties.
	
			- If the beginning of the line contains @sleepiCustomDOX
				- It will serialize the entire comment looking for "@_____" style keys, and will populate the entry as provided
				- It will not add any additional properties to the entry.
	*/
