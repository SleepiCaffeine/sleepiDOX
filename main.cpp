#include "slpUtility.hpp"
#include "RegexFileParser.hpp"
#include "sleepiDOX.hpp"
#include <iostream>

void isolateEntries(const std::string& fileContent, Sleepi::DOXContainer& entries) {
    // the only time I found copilot useful
    const char* CLASS_REGEX = R"((class|struct)\s+([A-Za-z_][A-Za-z_0-9]*)\s*(?::\s*(public|private|protected)?\s*[A-Za-z_][A-Za-z_0-9]*)?\s*\{((?:[^{}]*|\{(?:[^{}]*|\{[^{}]*\})*\})*)\})";


    const auto classMatches = getRegexMatches(fileContent, CLASS_REGEX);
    std::vector< std::string > classNames;
    // Keeps track of where class bodies begin and end, so'd, that when we query for functions
    // we don't include ones that are already within classes.
    std::vector <std::pair <size_t, size_t >> classBodyPositions;


    for (const std::smatch& classMatch : classMatches) {
        classNames.push_back(classMatch[2].str());
        classBodyPositions.emplace_back(classMatch.position(), classMatch.position() + classMatch.length());
    }


    Sleepi::DOXEntry entry;
    const char* FUNCTION_REGEX = R"((\/\/[ \t]*@sleepiDOX[^\n]*\n?|\/\*[ \t]*@sleepiDOX[\s\S]*?\*\/)|\/\/[ \t]*@sleepiRETURNS[ \t]*([^\n]*)|\/\/[ \t]*@sleepiPARAM[ \t]*([^\n]*)|(^[ \t]*[a-zA-Z_][\w\s:<>,*&]*\s+([a-zA-Z_]\w*)\s*\([\s\S]*?\)\s*(const)?\s*(noexcept)?\s*;))";
    const auto functionMatches = getRegexMatches(fileContent, FUNCTION_REGEX);
    for (const std::smatch& match : functionMatches) {



        if (match[1].matched) {
            entry.at(Sleepi::ENTRY_COMMENT) += commentTrim(match[1].str(), "@sleepiDOX");
        }
        if (match[2].matched) {
            entry.at(Sleepi::ENTRY_RETURNS) += commentTrim(match[2].str(), "@sleepiRETRUNS");
        }
        if (match[3].matched) {
            entry.at(Sleepi::ENTRY_PARAMS) += commentTrim(match[3].str(), "@sleepiPARAM") + "\n\n";
        }
        if (match[4].matched) {
            const std::string functionDefinition = rltrim(match[4].str());
            std::string functionName = rltrim(match[5].str());

            // Checks whether the function is within a class
            std::string className;
            size_t funcStart = fileContent.find(match[4].str()); // This can accidentally catch something else, so best to replace asap
            size_t funcEnd = funcStart + match.length();

            for (size_t index = 0; index < classBodyPositions.size(); ++index) {
                if (funcStart >= classBodyPositions.at(index).first &&
                    funcEnd <= classBodyPositions.at(index).second) {
                    className = classNames.at(index);
                    break;
                }
            }

            if (!className.empty())
                functionName = className + "::" + functionName;

            entry.at(Sleepi::ENTRY_FUNCTION_DEFINTION) = functionDefinition;
            entry.at(Sleepi::ENTRY_FUNCTION_NAME) = functionName;

            entries[entry.at(Sleepi::ENTRY_FUNCTION_NAME)].push_back(entry);
            entry = {};
        }
    }
}

void documentFile(const std::string& directory, std::string destination = "") {
    std::ifstream rfile = openReadFile(directory);
    Sleepi::DOXContainer entries;
    isolateEntries(extractFileContent(rfile), entries);

    // Make output in the same destination as input, just with the .md extension
    if (destination.empty()) {
        destination = directory;
        destination.replace(destination.find_last_of('.'), std::string::npos, ".md");
    }
    std::cout << "Writing to " << destination << '\n';
    std::ofstream outputFile = openWriteFile(destination);
    generateDocFile(outputFile, entries);
}


int main(const int args, const char* argv[]) {
    // PARSE ARGUMENTS
    std::vector<std::string> arguments(args);
    for (size_t i = 0; i < args; ++i)
        arguments.at(i) = argv[i];

    Sleepi::DOXContext context = extractArguments(arguments);
    if (context.outputFileDir.empty()) {
        for (const std::string& fileDestination : context.sourceDirs) {
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
        generateDocFile(outputFile, entries);
    }
    std::cout << "Finsihed!";
}