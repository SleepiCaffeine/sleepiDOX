#pragma once
#include <vector>
#include <string>
#include <regex>
#include "sleepiDOX.hpp"



namespace Sleepi {

/*
@sleepiDOX A function to find and extract all substrings that match a provided regex.
#### Parameters:
- `const std::string&` content : string to parse for substrings
- `const std::string&` match   : regular expresion with which to parse `content`
#### Returns:
A vector containing `std::match_results` of every matched expression*/
std::vector< std::smatch > getRegexMatches(const std::string& content, const std::string& match);
/*
@sleepiDOX A function that returns an input file stream of the speficied file.Throws a `std::runtime_error` if the file could not be opened.
#### Parameters:
- `const std::string_view&` fileName : directory relative to the executable
#### Returns:
a valid `std::ifstream`*/
std::ifstream openReadFile(const std::string_view& fileName);
/*
@sleepiDOX A function that transfers the buffer of the filestream into a string
#### Parameters:
- `const std::ifstream&` file_stream : a valid file stream
#### Returns:
the entire `::rdbuf()` of `file_stream`*/
std::string extractFileContent(const std::ifstream& file_stream);
  // @sleepiDOX A function that returns an output file stream of the speficied file. Throws a `std::runtime_error` if the file could not be opened.
  // @sleepiPARAM `std::string_view` fileName : directory relative to the executable
  // @sleepiRETURNS a valid `std::ofstream`
std::ofstream openWriteFile(const std::string_view& fileName);

std::vector<Sleepi::DOXScope> extractScopeMatches(const std::string& fileContent);

Sleepi::DOXContainer isolateEntries(const std::string& fileContent);

std::string getScopeSyntax(const Sleepi::DOXScope& scope);

void assignParentScopes(Sleepi::DOXContainer& functions, const std::vector<Sleepi::DOXScope>& scopes);

std::string getline(std::ifstream& file);

};