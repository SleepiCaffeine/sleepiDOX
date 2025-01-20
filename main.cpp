#include "slpUtility.hpp"
#include "RegexFileParser.hpp"
#include "sleepiDOX.hpp"
#include <iostream>
#include <filesystem>




int main(const int args, const char* argv[]) {
  // PARSE ARGUMENTS
  std::cout <<R"(
  ____  _         _ ____   _____  __
 / ___|| | ___  ___ _ __ (_)  _ \ / _ \ \/ /
 \___ \| |/ _ \/ _ \ '_ \| | | | | | | \  / 
  ___) | |  __/  __/ |_) | | |_| | |_| /  \ 
 |____/|_|\___|\___| .__/|_|____/ \___/_/\_\
           |_|)" << "\n\n";

  std::cout << "Parsing files...\n";
  std::vector<std::string> arguments(args);
  for (size_t i = 0; i < args; ++i)
  arguments.at(i) = argv[i];

  Sleepi::DOXContext context = extractArguments(arguments);
  if (context.outputFileDir.empty()) {
  for (const std::string& fileDestination : context.sourceDirs) {
    std::filesystem::path path = fileDestination;
    std::cout << "Generating " << path.filename().string() << '\n';
    documentFile(fileDestination);
  }
  }
  else {
  Sleepi::DOXContainer entries;
  for (const std::string& dir : context.sourceDirs) {
    std::ifstream rfile = openReadFile(dir);
    isolateEntries(extractFileContent(rfile), entries);
  }
  std::ofstream outputFile = openWriteFile(context.outputFileDir);
  std::cout << "Generating " << context.outputFileDir << '\n';
  generateDocFile(outputFile, entries, "", "bunch");
  }
  std::cout << "Finsihed!";
}