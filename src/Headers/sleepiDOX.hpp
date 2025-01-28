#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <array>
#include <unordered_map>

namespace Sleepi {

  constexpr size_t ENTRY_FUNCTION_NAME = 0;
  constexpr size_t ENTRY_COMMENT = 1;
  constexpr size_t ENTRY_FUNCTION_DEFINTION = 2;
  constexpr size_t ENTRY_FUNCTION_CLASS = 3;

  // Error bits used in `extractArguments`
  namespace ErrorBits {
    constexpr size_t InputDirDoesntExist = 0x01;
    constexpr size_t NoInputSpecified = 0x02;
    constexpr size_t NoInputFilesFound = 0x04;
    constexpr size_t NoOutputFileSpecified = 0x08;
  };
  
  const std::string GLOBAL_SCOPE = "!";

  const char logo[] = R"(
  ____  _                 _ ____   _____  __
 / ___|| | ___  ___ _ __ (_)  _ \ / _ \ \/ /
 \___ \| |/ _ \/ _ \ '_ \| | | | | | | \  / 
  ___) | |  __/  __/ |_) | | |_| | |_| /  \ 
 |____/|_|\___|\___| .__/|_|____/ \___/_/\_\
                   |_|)";

  /* 
  Reference guide for the different types:
    - DOXEntry keeps all the comments, and needs to be rewritten NOW.
      Since there are multiple "function name" variables, this is how they are saved:
      Example function - "CustomType SomeScope::function(int& a);"
        - DOXEntry.at(ENTRY_FUNCTION_DEFINITION) = "CustomType SomeScope::function(int& a);"
        - DOXEntry.at(ENTRY_FUNCTION_NAME)       = "SomeScope::function"
        - DOXFunction::name                      = "function"
        
      Each is saved for different purposes.

    - DOXContext saves information about which files should be parsed, how, and where.
      
    - DOXFunction saves information about a specific function definition,
        mainly name, and comments.

    - DOXContainer is a vector of functions. DOXScopes use this to know which functions belong to it.
        The reason this is done is so the functions belonging to a specific scope could be immediately
        pushed to the output file, rather than requiring to loop over every function found within that file
        and checking whether each function belongs to that class.
        Although that's not a terrible idea....

        Both strategies:
        1) Each scope stores its own function:
          - Have to sort multiple times
          - Instant access
        2) Only pointer to scope
          - Can sort entire function list onceCan you give 
          - Have to loop over whole list for each entry

  */



  using DOXEntry = std::array<std::string, 4>;

  struct DOXContext {
    std::vector<std::string> sourceDirs;
    std::string outputFileDir;
    size_t errorFlags;
  };
  
  struct DOXScope {
    std::string scopeName;
    std::pair<size_t, size_t> location;                 // Should be used as (pos, len)
    std::shared_ptr<DOXScope> parentScope{nullptr};     // a linked list... in real life...

    ~DOXScope() = default;
    DOXScope(const std::string_view& name, const std::pair<size_t, size_t>&  loc) noexcept;
    DOXScope(DOXScope const&) = default;
    DOXScope(DOXScope&&) = default;
    DOXScope& operator=(const DOXScope& other);
    DOXScope& operator=(DOXScope&& other) noexcept = default;
  };

  struct DOXFunction {
    std::string name;
    DOXEntry entry;
    std::shared_ptr<DOXScope> scope;
    std::pair<size_t, size_t> location;    // Use as (pos, len)

    ~DOXFunction() = default;
    DOXFunction(const std::string_view& name, const DOXEntry& entry) noexcept;
    DOXFunction(DOXFunction const&) = default;
    DOXFunction(DOXFunction&&) = default;
    DOXFunction& operator=(const DOXFunction& other);
    DOXFunction& operator=(DOXFunction&& other) noexcept = default;
  };

  using DOXContainer = std::vector<DOXFunction>;



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
- `std::ofstream&`                 outputFile : output file stream where this function will write to. Does not perform any validation.
- `Sleepi::DOXContainer&`          entries    : hashmap of entries to document.
- `std::vector<Sleepi::DOXScope>&` scopes     :
- - [OPTIONAL] `const std::string_view&` title       : Text at the top of the page, at header level 1
- - [OPTIONAL] `const std::string_view&` source_name : filename (or any comment) that will be added next to function descriptions to show which file they come from.
*/
void generateDocFile(std::ofstream& outputFile, Sleepi::DOXContainer& entries, std::vector<Sleepi::DOXScope>& scopes, const std::string_view& title = "", const std::string_view& source_name = "");

void documentTableOfEntries(std::ofstream& output_file, const std::vector<Sleepi::DOXScope>& scopes,
  const std::unordered_map<std::string, std::string>& scopeToSourceMap);

void documentTableOfScopes(std::ofstream& output_file, const std::unordered_map<std::string, Sleepi::DOXContainer>& scopeToEntriesMap);

void documentFile(const std::string& path, std::vector<Sleepi::DOXScope>& scopes, Sleepi::DOXContainer& functions, const std::string_view& source = "");

void documentScope(const std::string& path, const std::string_view& scope_name, Sleepi::DOXContainer& functions);

  };