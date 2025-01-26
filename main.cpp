#include "slpUtility.hpp"
#include "RegexFileParser.hpp"
#include <iostream>
#include <filesystem>



void HandleContext(const Sleepi::DOXContext& context);


int main(const int args, const char* argv[]) {
  std::cout << Sleepi::logo << "\n\n";

  std::cout << "Parsing arguments...\n";
  std::vector<std::string> arguments(args);
  for (size_t i = 0; i < args; ++i)
    arguments.at(i) = argv[i];

  Sleepi::DOXContext context = extractArguments(arguments);
  HandleContext(context);
  std::cout << "Finsihed!";
}

void HandleContext(const Sleepi::DOXContext& context) {
  std::cout << "Parsing files, and generating docs..." << std::endl;

  Sleepi::DOXContainer          entries;
  std::vector<Sleepi::DOXScope> scopes;
  std::unordered_map<std::string, std::string> scopeToSourceMap;


  // global scope
  scopes.emplace_back(Sleepi::GLOBAL_SCOPE, std::pair<size_t, size_t>(0, -1));
  scopeToSourceMap[scopes.back().scopeName] = Sleepi::GLOBAL_SCOPE;


  // Parse every source file
  for (const std::string_view& sourceFile : context.sourceDirs) {
    std::cout << "Parsing " << sourceFile << '\n';
    const std::string fileContents = extractFileContent(openReadFile(sourceFile));
    auto file_scopes = extractScopeMatches(fileContents);
    auto file_entries      = isolateEntries(fileContents);

    // Map scope to source file so a table could be made properly
    for (const auto& scope : file_scopes) {
      scopeToSourceMap[scope.scopeName] = sourceFile;
    }

    assignParentScopes(file_entries, file_scopes);
    scopes.insert(scopes.cend(), file_scopes.cbegin(), file_scopes.cend());

    // If each file individually
    if (!context.outputFileDir.empty())
      continue;

    std::filesystem::path path(sourceFile);
    path.replace_extension(".md");
    documentFile(path.string(), file_scopes, file_entries, sourceFile);
  }

  // Either generate a separate Table of Contents file for easier navigation
  // Or simply push everything into one file
  
  // Makes a single file with the entire table of entries, and every entry all in one
  if (!context.outputFileDir.empty()) {
    std::ofstream output_file = openWriteFile(context.outputFileDir);
    generateDocFile(output_file, entries, scopes, "", "");
  }
  // Make a separate file as a namespace list, which directs to other files
  else {
    std::ofstream output_file = openWriteFile("TableOfEntries.md");
    // only passing scopes, because they are sorted
    documentTableOfEntries(output_file, scopes, scopeToSourceMap);
  }

  std::cout << "Finished!";
}