#include "sleepiDOX.hpp"
#include "slpUtility.hpp"
#include "RegexFileParser.hpp"
#include <iostream>
#include <unordered_map>

/* @sleepiDOX Extracts the command-line arguments passed in. The required flags are:
- -d : output file destrination/directory. Must exist.
- -s : source file that will be parsed.
*/
// @sleepiPARAM args : length of `argv`
// @sleepiPARAM argv : arguments passed into `int main()`
// @sleepiPARAM rfile : string that will contain input file.
// @sleepiPARAM wfile : string that will contain output file.
// @sleepiRETURNS Returns true if all operations were successful, otherwise false.
bool extractArguments(const int& args, const char* argv[], std::string& rfile, std::string& wfile) {
    // Possible future flags:
    // -fs  :   Parses entire folder for files
    // -dd  :   Destination directory (relative to executable)
    // -xp  :   Would attempt to parse multiple files looking for things like #define's, "using"s, and such to find what things are defined as. [Most likely not]
    
    
    if (args <= 4) {
        std::cout << "You need to supply the file to parse with the flag \"-s\", and destination file with \"-d\"" << std::endl;
        return false;
    }

    bool have_rfile = false, have_wfile = false;
    if (argv[1][0] != '-' || argv[3][0] != '-') {
        std::cout << "You need to supply the file to parse with the flag \"-s\", and destination file with \"-d\"" << std::endl;
        std::cout << "Found: \"" << argv[1] << "\", \"" << argv[3] << "\"" << std::endl;
    }


    if (argv[1][1] == 's' || argv[3][1] == 's') {
        rfile = argv[2 + ((argv[1][1] == 's') ? 0 : 2)];
    }
    else {
        std::cout << "No \"-s\" flag detected! Please reference the documentation for required flags.\n";
        std::cout << "Found: \"" << argv[1] << "\", \"" << argv[3] << "\"" << std::endl;
        return false;
    }
    if (argv[1][1] == 'd' || argv[3][1] == 'd') {
        wfile = argv[2 + ((argv[1][1] == 'd') ? 0 : 2)];
    }
    else {
        std::cout << "No \"-d\" flag detected! Please reference the documentation for required flags." << std::endl;
        std::cout << "Found: \"" << argv[1] << "\", \"" << argv[3] << "\"" << std::endl;
        return false;
    }


    return true;
}

// @sleepiDOX Fills the provided file output stream with preformatted documentation text.
// @sleepiPARAM output_file : output file stream where this function will write to. Does not perform any validation.
// @sleepiPARAM entries: hashmap of entries to document.
// @sleepiPARAM title *(optional)*: Text at the top of the page, at header level 1
void generateDocFile(std::ofstream& output_file, const std::unordered_map<std::string, std::vector<DOXEntry>>& entries, const std::string_view& title = "") {

    // Markdown really likes double newlines
    constexpr char MD_NL[] = "\n\n";
    constexpr char H1[] = "# ";
    constexpr char H2[] = "## ";
    constexpr char H3[] = "### ";


    if (!title.empty())
        output_file << H1 << title << MD_NL;

    output_file << H2 << "Table of contents : \n";
    for (const auto& [name, entry] : entries) {
        output_file << "- [" << name << "](#" << name << ")\n";
    } output_file << "- - -\n";


    for (const auto& [name, entry] : entries) {
        output_file << H3 << name << "()\n";

        for (const auto& entry_details : entry) {
            output_file << "```" << entry_details.at(ENTRY_FUNCTION_DEFINTION) << "```\n";
            output_file << H3 << "Description:\n" << entry_details.at(ENTRY_COMMENT) << MD_NL;

            if (!entry_details.at(ENTRY_PARAMS).empty()) {
                output_file << H3 << "Params:\n";
                output_file << entry_details.at(ENTRY_PARAMS) << MD_NL;
            }

            if (!entry_details.at(ENTRY_RETURNS).empty()) {
                output_file << H3 << "Returns:\n";
                output_file << entry_details.at(ENTRY_RETURNS) << MD_NL;
            }

            output_file << MD_NL;
        }
        output_file << "- - -\n";
    }
}

int main(const int args, const char* argv[]) {
    // PARSE ARGUMENTS
    std::string rfilename, wfilename;
    if (extractArguments(args, argv, rfilename, wfilename) != true) {
        return EXIT_FAILURE;
    }

    // OPEN READ FILE
    std::ifstream rfile     = openReadFile(rfilename.c_str());
    std::string fileContent = extractFileContent(rfile);
    std::unordered_map<std::string, std::vector<DOXEntry>> entries;


    // the only time I found copilot useful
    //const char* CLASS_REGEX = R"((class|struct)\s+([A-Za-z_][A-Za-z_0-9]*)\s*(?:\s*:\s*(public|private|protected)?\s*[A-Za-z_][A-Za-z_0-9]*)?\s*{)";
    const char* CLASS_REGEX = R"((class|struct)\s+([A-Za-z_][A-Za-z_0-9]*)\s*(?::\s*(public|private|protected)?\s*[A-Za-z_][A-Za-z_0-9]*)?\s*\{)";

    const auto classMatches = getRegexMatches(fileContent, CLASS_REGEX);

    std::vector< std::string > classNames;
    // Keeps track of where class bodies begin and end, so'd, that when we query for functions
    // we don't include ones that are already within classes.
    std::vector <std::pair <size_t, size_t >> classBodyPositions;


    for (const std::smatch& classMatch : classMatches) {
        classNames.push_back(classMatch[2].str());
        classBodyPositions.emplace_back(classMatch.position(), classMatch.position() + classMatch.length());
    }


    DOXEntry entry;
    const char* FUNCTION_REGEX = R"((\/\/[ \t]*@sleepiDOX[^\n]*\n?|\/\*[ \t]*@sleepiDOX[\s\S]*?\*\/)|\/\/[ \t]*@sleepiRETURNS[ \t]*([^\n]*)|\/\/[ \t]*@sleepiPARAM[ \t]*([^\n]*)|(^[ \t]*[a-zA-Z_][\w\s:<>,*&]*\s+([a-zA-Z_]\w*)\s*\([\s\S]*?\)\s*(const)?\s*(noexcept)?\s*;))";
    const auto functionMatches = getRegexMatches(fileContent, FUNCTION_REGEX);
    for (const std::smatch& match : functionMatches) {

        

        if (match[1].matched) {
            entry.at(ENTRY_COMMENT) = commentTrim(match[1].str(), "@sleepiDOX");
        }
        if (match[2].matched) {
            entry.at(ENTRY_RETURNS) = commentTrim(match[2].str(), "@sleepiRETRUNS");
        }
        if (match[3].matched) {
            entry.at(ENTRY_PARAMS) = commentTrim(match[3].str(), "@sleepiPARAM") + '\n';
        }
        if (match[4].matched) {
            const std::string functionDefinition = rltrim(match[4].str());
            std::string functionName = rltrim(match[5].str());

            // Checks whether the function is within a class
            std::string className;
            size_t funcStart = match.position();
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

            entry.at(ENTRY_FUNCTION_DEFINTION) = functionDefinition;
            entry.at(ENTRY_FUNCTION_NAME) = functionName;

            entries[entry.at(ENTRY_FUNCTION_NAME) ].push_back(entry);
            entry = {};
        }
    }

    std::ofstream outputFile = openWriteFile(wfilename.c_str());
    generateDocFile(outputFile, entries);
    std::cout << "Finsihed!";
}