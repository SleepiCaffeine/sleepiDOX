#include "sleepiDOX.hpp"
#include "slpUtility.hpp"
#include "RegexFileParser.hpp"
#include <iostream>
#include <unordered_map>


// Parses commandline arguments to find supplied read/write files. Will have to probably rewrite this to be more flexible
// for future flags. 
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


void generateDocFile(std::ofstream& output_file, const std::unordered_map<std::string, std::vector<DOXEntry>>& entries) {
    /*
    Step 1: Make a table of contents, which will have links to every function definition
    Step 2: For every function - add a header that has the function name
    Step 3: Create the entry:
        3.1: How to call the function (Comment block)
        3.2: Comment description  ( ### Description \n comment)
        3.3: Parameters (Each with heading level 4)
        3.4: Return value (### Returns \n ...)
    Step 4: Separate with a "- - -"
    Step 5: Repeat
    */
    output_file << "# Table of contents:\n";
    for (const auto& [name, entry] : entries ) {
        output_file << "- [" << name << "](#" << name << ")\n";
    }

    for (const auto& [name, entry] : entries) {
        output_file << "## " << name << "()\n";
        
        for (const auto& entry_details : entry) {
            output_file << "```" << entry_details.full_signature << ")```\n";
            output_file << "### Description:\n" << entry_details.paragraphs.at(ENTRY_COMMENT);

            // Ehhhhhhhhhhhhhhhhhhhhhhhh
            // Should rewrite this, so that params is also an array
            // temporary to make an MVP
            if (!entry_details.paragraphs.at(ENTRY_PARAMS).empty()) {
                output_file << "### Params:\n";
                output_file << entry_details.paragraphs.at(ENTRY_PARAMS);
            }

            if (!entry_details.paragraphs.at(ENTRY_RETURNS).empty()) {
                output_file << "### Returns:\n";
                output_file << entry_details.paragraphs.at(ENTRY_RETURNS);
            }

            output_file << "\n";
        }
    }
    output_file << "- - -\n";
}

int main(const int args, const char* argv[]) {
    // PARSE ARGUMENTS
    std::string rfilename, wfilename;
    if (extractArguments(args, argv, rfilename, wfilename) != true) {
        return EXIT_FAILURE;
    }

    // OPEN READ FILE
    std::ifstream rfile = openReadFile(rfilename.c_str());
    std::string file_content = extractFileContent(rfile);

    // the only time I found copilot useful
    const char* EVERYTHING_REGEX = R"((\/\/[ \t]*@sleepiDOX[^\n]*\n?|\/\*[ \t]*@sleepiDOX[\s\S]*?\*\/)|\/\/[ \t]*@sleepiRETURNS[ \t]*([^\n]*)|\/\/[ \t]*@sleepiPARAM[ \t]*([^\n]*)|(^[ \t]*[a-zA-Z_][\w\s:<>,*&]*\s+([a-zA-Z_]\w*)\s*\([\s\S]*?\)\s*(const)?\s*(noexcept)?\s*;))";



    const auto function_matches = getRegexMatches(file_content, EVERYTHING_REGEX);

    for (const std::smatch& match : function_matches) {
        if (match[1].matched) {
            std::cout << "Found @sleepiDOX comment: " << match[1] << '\n';
        }
        if (match[2].matched) {
            std::cout << "Found @sleepiRETURNS comment: " << match[2] << '\n';
        }
        if (match[3].matched) {
            std::cout << "Found @sleepiPARAM comment: " << match[3] << '\n';
        }
        if (match[4].matched) {
            std::cout << "Found function declaration: " << match[4] << '\n';
            std::cout << "Function name: " << match[5] << '\n';
        }
    }

    std::cout << "Finsihed!";
}