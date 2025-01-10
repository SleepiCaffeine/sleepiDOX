#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <array>
#include <unordered_map>

namespace Sleepi {
	constexpr size_t ENTRY_FUNCTION_NAME = 0;
	constexpr size_t ENTRY_COMMENT = 1;
	constexpr size_t ENTRY_RETURNS = 2;
	constexpr size_t ENTRY_PARAMS = 3;
	constexpr size_t ENTRY_FUNCTION_DEFINTION = 4;
	constexpr size_t ENTRY_FUNCTION_CLASS = 5;

	// Error bits used in `extractArguments`
	namespace ErrorBits {
		constexpr size_t InputDirDoesntExist = 0x01;
		constexpr size_t NoInputSpecified = 0x02;
		constexpr size_t NoInputFilesFound = 0x04;
		constexpr size_t NoOutputFileSpecified = 0x08;
	};
	

	using DOXEntry = std::array<std::string, 5>;
	using DOXContainer = std::unordered_map< std::string, std::vector <DOXEntry> >;
	struct DOXContext {
		std::vector<std::string> sourceDirs;
		std::string outputFileDir;
		size_t errorFlags;
	};
};

/* @sleepiDOX Extracts the command-line arguments passed in. The flags are:
- -d  : output file destrination/directory.
- -s  : a single source file that will be parsed.
- -fs : directory (relative to the executable) that contains header files to be parsed. Header files include *.hpp and *.h files.
*/
// @sleepiPARAM argv   : vector of std::string arguments passed through the command line
// @sleepiPARAM rfile  : string that will contain input file.
// @sleepiPARAM wfile  : string that will contain output file.
// @sleepiPARAM strict : boolean that determines whether this function will throw exceptions. Set this to false if you want to handle errors in `Sleepi::DOXContext::errorFlags` differently.
// @sleepiRETURNS Returns a `Sleepi::DOXContext` that contains parsed input/output file names.
Sleepi::DOXContext extractArguments(const std::vector<std::string>& argv, const bool strict = true);


/* @sleepiDOX Checks `errorFlags` for every error flag in `Sleepi::ErrorBits` and throws exceptions accordingly.
Typically called internally, but can be done externally if `extractArguments` doesn't do it on it's own.*/
// @sleepiPARAM `const Sleepi::DOXContext& context` : DOXContext that will be checked for any error flags set

void validateContext(const Sleepi::DOXContext& context);

// @sleepiDOX Fills the provided file output stream with preformatted documentation text.
// @sleepiPARAM outputFile : output file stream where this function will write to. Does not perform any validation.
// @sleepiPARAM entries: hashmap of entries to document.
// @sleepiPARAM title *(optional)*: Text at the top of the page, at header level 1
void generateDocFile(std::ofstream& outputFile, const Sleepi::DOXContainer& entries, const std::string_view& title = "");