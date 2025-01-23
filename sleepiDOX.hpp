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
 

  struct DOXContext {
    std::vector<std::string> sourceDirs;
    std::string outputFileDir;
    size_t errorFlags;
  };
  struct DOXFunction {
    const std::string name;
    DOXEntry entry;
  };

  using DOXContainer = std::vector< std::shared_ptr<DOXFunction> >;

  struct DOXScope {
    const std::string scopeName;
    const std::pair<size_t, size_t> location;           // Should be used as (pos, len)
    std::shared_ptr<DOXScope> parentScope{ nullptr };   // a linked list... in real life...
    DOXContainer functions;
  };

  };

/*
@sleepiDOX Extracts the command-line arguments passed in. The flags are:
- -d  : output file destrination/directory.
- -s  : a single source file that will be parsed.
- -fs : directory (relative to the executable) that contains header files to be parsed. Header files include *.hpp and *.h files.
#### Parameters:
- argv   : vector of std::string arguments passed through the command lineW
- strict : boolean that determines whether this function will throw exceptions. Set this to false if you want to handle errors in `Sleepi::DOXContext::errorFlags` differently.
#### Returns:
a `Sleepi::DOXContext` that contains parsed input/output file names.
*/
Sleepi::DOXContext extractArguments(const std::vector<std::string>& argv, const bool strict = true);


/*
@sleepiDOX Checks `errorFlags` for every error flag in `Sleepi::ErrorBits` and throws exceptions accordingly.
Typically called internally, but can be done externally if `extractArguments`'s `strict` is set to false.
#### Parameters:
- `const Sleepi::DOXContext& context` : DOXContext that will be checked for any error flags set
*/
void validateContext(const Sleepi::DOXContext& context);

/*
@sleepiDOX Fills the provided file output stream with preformatted documentation text.
#### Parameters:
- outputFile : output file stream where this function will write to. Does not perform any validation.
- entries: hashmap of entries to document.
- title *(optional)*: Text at the top of the page, at header level 1
- source_name *(optional)*: filename (or any comment) that will be added next to function descriptions to show which file they come from.
*/
void generateDocFile(std::ofstream& outputFile, Sleepi::DOXContainer& entries, const std::string_view& title = "", const std::string_view& source_name = "");


void documentFile(const std::string& directory, std::string destination = "");