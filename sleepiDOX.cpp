#include "sleepiDOX.hpp"
#include <string>
#include <iostream>
#include <unordered_map>

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

void generateDocFile(std::ofstream& output_file, const DOXContainer& entries, const std::string_view& title) {

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