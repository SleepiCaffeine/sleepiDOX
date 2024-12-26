#pragma once
#include <string>
#include <fstream>
#include <vector>

enum class FragmentKey {
	Returns,
	Functionality,
	Params,
	Warning,
	TODO,
	Custom
};

struct DOXFragment {
	FragmentKey key;
	unsigned short heading_level;
	std::string entry;
};

const DOXFragment NONE = {};

struct DOXEntry {
	bool initialized = false;
	DOXFragment functionality = NONE;
	DOXFragment returns = NONE;
	DOXFragment params = NONE;
	std::vector<DOXFragment> customs;
};



std::ifstream openReadFile(const char* fileName);
std::ofstream openWriteFile(const char* fileName);
DOXFragment populateEntry(const FragmentKey key);

std::vector<std::string> tokenizeLine(const std::string& line);
void appendReturn(std::ofstream& file, const std::string& line);
void appendParams(std::ofstream& file, const std::string& line);



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
