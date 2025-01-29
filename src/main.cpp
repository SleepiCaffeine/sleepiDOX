#include "Headers/slpUtility.hpp"
#include "Headers/RegexFileParser.hpp"
#include <iostream>
#include <filesystem>

void HandleContext(const Sleepi::DOXContext& context);

int main(const int args, const char* argv[]) {
  std::cout << Sleepi::logo << "\n\n";

  std::cout << "Parsing arguments...\n";
  std::vector<std::string> arguments(args);
  for (size_t i = 0; i < args; ++i)
    arguments.at(i) = argv[i];

  Sleepi::DOXContext context = Sleepi::extractArguments(arguments);
  HandleContext(context);
  std::cout << "Finsihed!";
}

void HandleContext(const Sleepi::DOXContext& context) {
  std::cout << "Parsing files, and generating docs..." << std::endl;

  Sleepi::DOXContainer          entries;
  std::vector<Sleepi::DOXScope> scopes;
  std::unordered_map<std::string, Sleepi::DOXContainer> scopeToEntriesMap;


  // global scope
  scopes.emplace_back(Sleepi::GLOBAL_SCOPE, std::pair<size_t, size_t>(0, -1));
  scopeToEntriesMap[scopes.back().scopeName] = {};


  // Parse every source file
  for (const std::string_view& sourceFile : context.sourceDirs) {
    std::cout << "Parsing " << sourceFile << '\n';
    const std::string fileContents = Sleepi::extractFileContent(Sleepi::openReadFile(sourceFile));
    auto file_scopes       = Sleepi::extractScopeMatches(fileContents);
    auto file_entries      = Sleepi::isolateEntries(fileContents);


    Sleepi::assignParentScopes(file_entries, file_scopes);
    scopes.insert(scopes.cend(), file_scopes.cbegin(), file_scopes.cend());

    // Append entries as neccesary
    for (const auto& scope : file_scopes) {
      for (const auto& function : file_entries) {
        if (function.scope && function.scope.get()->scopeName == scope.scopeName)
          scopeToEntriesMap[scope.scopeName].push_back(function);
      }
    }
  }

  if (context.outputFileDir.empty()) {
    for (auto& [scope_name, funcs] : scopeToEntriesMap) {
      if (scope_name == Sleepi::GLOBAL_SCOPE)
        continue;

      Sleepi::documentScope(scope_name + ".md", scope_name, funcs);
    }
    std::ofstream ofile = Sleepi::openWriteFile("TableOfEntries.md");
    Sleepi::documentTableOfScopes(ofile, scopeToEntriesMap);
  }

  // Either generate a separate Table of Contents file for easier navigation
  // Or simply push everything into one file
  
  // Makes a single file with the entire table of entries, and every entry all in one
  if (!context.outputFileDir.empty()) {
    std::ofstream output_file = Sleepi::openWriteFile(context.outputFileDir);
    Sleepi::generateDocFile(output_file, entries, scopes, "", "");
  }


}