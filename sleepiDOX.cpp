#include "sleepiDOX.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>
#include <algorithm>
#include "RegexFileParser.hpp"

Sleepi::DOXContext extractArguments(const std::vector<std::string>& argv, const bool strict) {
  // Possible future flags:
  // -xp  :   Would attempt to parse multiple files looking for things like #define's, "using"s, and such to find what things are defined as. [Most likely not]

  namespace sErr = Sleepi::ErrorBits;
  Sleepi::DOXContext context =  { {}, "", 0};

  // Looping up to .size() - 1, because every flag has to have a second string afterward
  for (size_t index = 1; index < argv.size() - 1; index += 2) {
    const std::string_view argument = argv.at(index);

    // Flag that dictates the source file
    if (argument == "-s") {
      context.sourceDirs.push_back(argv.at(index + 1));
    }
    // Flag that dictates the destination file
    else if (argument == "-d") {
      context.outputFileDir = argv.at(index + 1);
    }
    // Flag that dictates the directory where the source files are kept
    else if (argument == "-fs") {
      namespace fs = std::filesystem;

      std::string directory = argv.at(index + 1);
      if (directory.at(0) == '-')
        directory = argv.at(0);

      fs::path inputDirPath = directory;
      if (inputDirPath.has_filename())
        inputDirPath = inputDirPath.parent_path();


      // Make sure the directory actually exists, and if not, set an error flag
      if (!fs::exists(inputDirPath)) {
        context.errorFlags |= sErr::InputDirDoesntExist;
        return context;
      }

      // Loop through every file in the directory
      for (const auto& directory_entry : fs::directory_iterator(inputDirPath)) {
        
        // Ignore every file that isn't a "regular" aka text file
        if (!directory_entry.is_regular_file())
          continue;
        // check if the file is a header file
        const fs::path current_path = directory_entry.path();
        if (current_path.extension() == ".hpp" || current_path.extension() == ".h")
          context.sourceDirs.push_back(current_path.string());
      }

      // At this point, all the files have been found, but there's a chance there were no headers
      if (context.sourceDirs.empty())
        context.errorFlags |= sErr::NoInputFilesFound;

    }
  }

   // Commented because new default behavior will now extract each file separately
   // Will remove after a stable version
   /* if (context.outputFileDir.empty())
    context.errorFlags |= sErr::NoOutputFileSpecified;*/

  // If the -fs flag was not specified, and the input is still empty
  // This means that no input was provided at all.

  // This could've been done with some sort of bool like:
  // `bool inputProvided = false;` but eh, doesn't really change much
  if ((context.errorFlags & sErr::NoInputFilesFound) == 0
    && context.sourceDirs.empty()) {
    context.errorFlags |= sErr::NoInputSpecified;
  }

  if (strict)
    validateContext(context);

  return context;
}

void validateContext(const Sleepi::DOXContext& context) {
  namespace Err = Sleepi::ErrorBits;
  const size_t& flags = context.errorFlags;
  

  if (flags & Err::InputDirDoesntExist)
    throw std::system_error( std::error_code(), "The provided input directory does not exist.");
  if (flags & Err::NoInputFilesFound)
    throw std::system_error(std::error_code(), "The program was unable to find the file(s) specified.");
  if (flags & Err::NoInputSpecified)
    throw std::logic_error("No input flag provided. Use the flags -s or -fs to provide a file to parse.");
  if (flags & Err::NoOutputFileSpecified)
    throw std::logic_error("No output flag provided. Use the flag -d to provide the destination.");

}

std::string replaceString(std::string& source, const std::string_view& search, const std::string_view& replace ) {
  std::string::size_type pos = 0;
  while ( (pos = source.find(search, pos)) != std::string::npos) {
  source.replace(pos, search.length(), replace);
  pos += replace.size();
  }
  return source;
}

// False - lhs is first
// True - rhs is first, or they're the same
bool alphabeticalCompare(const std::string_view& lhs, const std::string_view& rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

bool compareScopes(const Sleepi::DOXScope& lhs, const Sleepi::DOXScope& rhs) noexcept {
  return alphabeticalCompare(lhs.scopeName, rhs.scopeName);
}
bool compareFunctions(const Sleepi::DOXFunction& lhs, const Sleepi::DOXFunction& rhs) noexcept {
  return alphabeticalCompare(lhs.name, rhs.name);
}

void generateTOE(std::ofstream& output_file, Sleepi::DOXContainer& entries, std::vector<Sleepi::DOXScope>& scopes, const std::string_view& title = "") {
  
  if (!title.empty())
    output_file << "# " << title << "\n\n";

  output_file << "## Table of contents : \n";
  size_t index = 1;

  std::sort(scopes.begin(), scopes.end(), compareScopes);
  std::sort(entries.begin(), entries.end(), compareFunctions);

  // First print every class/namespace available:
  for (const Sleepi::DOXScope& scope : scopes) {
    output_file << "- [" << scope.scopeName << "](#" << index++ << ")\n";
  } output_file << "\n- - -\n";
  
  index = 0;
  // Now for each class, add all of their function links:
  for (const Sleepi::DOXScope& scope : scopes) {
    
    output_file << "### " << scope.scopeName << "\n\n";
    
    for (const Sleepi::DOXFunction& func : entries) {
      ++index;
      if (!func.scope || func.scope.get()->scopeName != scope.scopeName)
        continue;


      std::string functionDefinition = func.entry.at(Sleepi::ENTRY_FUNCTION_DEFINTION);
      functionDefinition.pop_back();
      // Remove scope, because why would you need one?
      const std::string scope_syntax = getScopeSyntax(scope);
      auto pos = functionDefinition.find( getScopeSyntax(scope) );
      functionDefinition.erase(pos, scope_syntax.length());

      // Markdown really doesn't like these in a header
      replaceString(functionDefinition, "<", "\\<");
      replaceString(functionDefinition, ">", "\\>");
      output_file << "- [" << functionDefinition << "](#f" << index << ")\n";
    }
  } output_file << "\n- - -\n";

}

void generateDocFile(std::ofstream& output_file, Sleepi::DOXContainer& entries, std::vector<Sleepi::DOXScope>& scopes, const std::string_view& title, const std::string_view& source_name){

  using namespace Sleepi;

  // Markdown really likes double newlines
  constexpr char MD_NL[] = "\n\n";
  constexpr char H1[] = "# ";
  constexpr char H2[] = "## ";
  constexpr char H3[] = "### ";


  generateTOE(output_file, entries, scopes, title);

  size_t index = 0;
  for (const auto& scope : scopes)
  {
    for (const auto& func : entries) {
      ++index;
      if (!func.scope || func.scope.get()->scopeName != scope.scopeName)
        continue;

      const auto entry = func.entry;
      std::string functionDefinition = func.entry.at(Sleepi::ENTRY_FUNCTION_DEFINTION);
      functionDefinition.pop_back();

      output_file << "<h3 id=\"f" << index++ << "\"> " << functionDefinition << "</h3>" << MD_NL;
      output_file << "`" << source_name << "`" << MD_NL;
      output_file << H3 << "Description:" << MD_NL << entry.at(ENTRY_COMMENT) << MD_NL;

      if (!entry.at(ENTRY_PARAMS).empty()) {
        output_file << H3 << "Params:" << MD_NL;
        output_file << entry.at(ENTRY_PARAMS) << MD_NL;
      }

      if (!entry.at(ENTRY_RETURNS).empty()) {
        output_file << H3 << "Returns:" << MD_NL;
        output_file << entry.at(ENTRY_RETURNS) << MD_NL;
      }
      output_file << "\n- - -" << MD_NL;
    }
  }


  output_file << MD_NL << "<p style=\"font-size : 12;\">Made using <a href=\"https://github.com/SleepiCaffeine/sleepiDOX\">sleepiDOX</a></p>";
}

void documentFile(const std::string& directory, std::string destination) {
  std::ifstream rfile = openReadFile(directory);
  Sleepi::DOXContainer entries;
  std::vector<Sleepi::DOXScope> scopes = isolateEntries(extractFileContent(rfile), entries);

  // Make output in the same destination as input, just with the .md extension
  if (destination.empty()) {
    destination = directory;
    destination.replace(destination.find_last_of('.'), std::string::npos, ".md");
  }

  std::ofstream outputFile = openWriteFile(destination);
  generateDocFile(outputFile, entries, scopes);
}



Sleepi::DOXScope& Sleepi::DOXScope::operator=(const DOXScope& scope)
{
  this->scopeName   = scope.scopeName;
  this->location    = scope.location;
  this->parentScope = scope.parentScope;

  return *this;
}

Sleepi::DOXFunction& Sleepi::DOXFunction::operator=(const DOXFunction& func)
{
  this->name = func.name;
  this->entry = func.entry;
  this->scope = func.scope;

  return *this;
}

Sleepi::DOXScope::DOXScope(const std::string_view& name, const std::pair<size_t, size_t> loc) noexcept
  : scopeName{ name }, location{ loc }, parentScope{nullptr}
{ }

Sleepi::DOXFunction::DOXFunction(const std::string name, const DOXEntry& entry) noexcept
  : name{ name }, entry{ entry }, scope{nullptr}
{ }
