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

  const enum class ScopeType {
    Namespace,
    Struct,
    Class,
    NONE
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

    - DOXContainer is a vector of functions.

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
    ScopeType type;


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
  using DOXHashMap = std::unordered_map< std::string, std::vector<DOXFunction> >;
  using _DOXScopeVector = std::vector < std::pair <std::string, std::vector<DOXFunction> > >;



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
- `Sleepi::DOXHashMap&`                   map : map of scope names to a vector of `Sleepi::DOXFunctions` that are within that namespace          
- [OPTIONAL] `const std::string_view&` title       : Text at the top of the page, at header level 1
- [OPTIONAL] `const std::string_view&` source_name : filename (or any comment) that will be added next to function descriptions to show which file they come from.

[SIDE EFFECT] `map` will have all of its keys' vectors sorted alphabetically 
*/
void generateDocFile(std::ofstream& outputFile, Sleepi::DOXHashMap& map, const std::string_view& title = "", const std::string_view& source_name = "");

/*
@sleepiDOX Fills a file with headers that link to namespace files
#### Parameters:
- `std::ofstream&` outputFile : output file stream, which will be populated with header entries
- `const std::unordered_map<std::string, Sleepi::DOXContainer>&` scopeToEntriesMap : map of scope names to a vector of Sleepi::DOXFunctions, used to determine what namespaces should be added as headers
*/
void documentTableOfScopes(std::ofstream& outputFile, const std::unordered_map<std::string, Sleepi::DOXContainer>& scopeToEntriesMap);

/*
@sleepiDOX
Creates a file `../executable_dir/path` and documents every function that has a parent scope of `scopeName`.

Functions that are nested, for example int my_func() in namespace Foo::Bar will only be documented in Bar.md, but will be shown as `int Foo::Bar::my_func()`.
#### Parameters:
- `const std::string&` path : path to the output file, either relative to the executable or absolute.
- `const std::string_view&` scopeName : name of the scope that should be documented.
- `Sleepi::DOXContainer&` functions   : vector of functions to possibly document.

[SIDE EFFECT] `functions` will be sorted alphabetically after this function!
*/
void documentScope(const std::string& path, const std::string_view& scopeName, Sleepi::DOXContainer& functions);

};