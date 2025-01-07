#pragma once
#include <string>
#include <fstream>
#include <array>
#include <unordered_map>
#include <iostream>

constexpr size_t ENTRY_FUNCTION_NAME = 0;
constexpr size_t ENTRY_COMMENT = 1;
constexpr size_t ENTRY_RETURNS = 2;
constexpr size_t ENTRY_PARAMS = 3;
constexpr size_t ENTRY_FUNCTION_DEFINTION = 4;
constexpr size_t ENTRY_FUNCTION_CLASS = 5;

using DOXEntry     = std::array<std::string, 5>;
using DOXContainer = std::unordered_map< std::string, std::vector <DOXEntry> >;

/* @sleepiDOX Extracts the command-line arguments passed in. The required flags are:
- -d : output file destrination/directory. Must exist.
- -s : source file that will be parsed.
*/
// @sleepiPARAM args : length of `argv`
// @sleepiPARAM argv : arguments passed into `int main()`
// @sleepiPARAM rfile : string that will contain input file.
// @sleepiPARAM wfile : string that will contain output file.
// @sleepiRETURNS Returns true if all operations were successful, otherwise false.
bool extractArguments(const int& args, const char* argv[], std::string& rfile, std::string& wfile);

// @sleepiDOX Fills the provided file output stream with preformatted documentation text.
// @sleepiPARAM output_file : output file stream where this function will write to. Does not perform any validation.
// @sleepiPARAM entries: hashmap of entries to document.
// @sleepiPARAM title *(optional)*: Text at the top of the page, at header level 1
void generateDocFile(std::ofstream& output_file, const DOXContainer& entries, const std::string_view& title = "");