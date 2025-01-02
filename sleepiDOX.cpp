#include "sleepiDOX.hpp"
#include "slpUtility.hpp"
#include "RegexFileParser.hpp"
#include <iostream>
#include <sstream>
#include <unordered_map>


inline std::string& comment_trim(std::string& str) {
    str = rltrim(str);
    const size_t token_index = str.find("@sleepiDOX");
    if (token_index != std::string::npos) {
        str.erase(token_index, 11);
    }

    if (str.size() < 2) {
        return str;
    }

    // Annoying way of deleting slashes
    // This way I don't have to rltrim(str) after
    if (str.at(0) == '/') {
        if (str.at(1) == '/') {
            str.erase(0, str.find_first_not_of("/ ", 1));
        }
        else if (str.at(1) == '*') {
            str.erase(0, str.find_first_not_of("* ", 1));
        }
    }
    
    if (str.at(0) == '*' && str.at(1) == '/') {
        str.erase(0, 2);
    }
    return str;
}



std::string getline(std::ifstream& file) {
    const int LINE_LENGTH = 256;
    char cline[LINE_LENGTH];
    file.getline(&cline[0], LINE_LENGTH, '\n');
    std::string line(cline);
    return rtrim(line);
}

// Tokenizes funciton declaration, modifies passed in integer to show which
// index points to function name (useful for when there are multiple templates beforehand)
std::vector<std::string> tokenizeFunction(const std::string& declaration, int& indexToFunctionName) {
    const size_t idxOfParen = declaration.find_first_of('(');
    // This is mainly to catch bugs easier, have no clue how this could be triggered
    // Other than a syntax error within the file itself.
    if (idxOfParen == std::string::npos) {
        throw std::logic_error("Function \"" + declaration + "\" does not have an opening paretheses!");
    }

    // Divide the string into 2 parts: [ ... ( ][ ..., ..., ...) ]
    // This way, the first part has the function name, and return type (at least in C-style languages)
    // Whereas the second part has all the parameters

    auto firstHalfEndIterator = declaration.cbegin();
    std::advance(firstHalfEndIterator, idxOfParen);

    std::string firstPart(declaration.cbegin(), firstHalfEndIterator);
    std::string secondPart(++firstHalfEndIterator, declaration.cend());


    std::istringstream firstStream(firstPart);
    std::vector<std::string> tokens;

    std::string syntaxSugar = "<>{}()[]";
    // Tokenize the return value and function name
    while (!firstStream.eof()) {
        std::string temp;
        firstStream >> temp;

        // Since it is not a syntax errror to write something along the lines of " std::map < std::string , std::complex >
        // The parser should be able to accomodate such writing styles
        if (temp.find_first_of(syntaxSugar) != std::string::npos) { tokens.back().append(temp); }
        if (temp == ",") { continue; }
        else if (temp.find(',') != std::string::npos) {
            temp.erase(temp.find(','));
            tokens.push_back(temp);
        }
        else { tokens.push_back(temp); }
    }
    indexToFunctionName = tokens.size() - 1;

    std::istringstream secondStream(secondPart);
    // Now to do the same to the parameters
    while (!secondStream.eof()) {
        std::string temp;
        secondStream >> temp;

        // Since it is not a syntax errror to write something along the lines of " std::map < std::string , std::complex >
        // The parser should be able to accomodate such writing styles
        
        if (temp.find_first_of(syntaxSugar) != std::string::npos) {
            tokens.back().append(temp);
            continue;
        }
        if (temp == ",") { continue; }
        else if (temp.find(',') != std::string::npos) {
            temp.erase(temp.find(','));
            tokens.push_back(temp);
        }
        else { tokens.push_back(temp); }
    }

    return tokens;
}



std::string extractFunctionDecl(std::ifstream& file, std::string last_read_line) {
    // Functions can have varying signatures, templates almost encouraging multiline delcarations makes this annoying
    // Signarutes can be as simple as:
    // return-type functionName() {
    // 
    // Or as annoying as:
    // template <...>
    // return-type<...> functionName( prefix type name1,
    //                                prefix type name2, ...) additional {
    // Hence there will be a lot of annoying stuff

    // The line is passed in, because it could possibly contain a function definintion. Example:
    // "/*
    // ...
    // *\ void fn() {"
    

    // For now, this function will only work without any definitions or C++17 attributes

    // Determine whether the previously read line contains a function definiton:
    // The shortest possible function is "_ z(){", so the line must contain at least 8 characters (accounting for "*\")
    // [TECHNICALLY]: it can be one character if #defined, but that's stupid, and I won't adjust my code for that.
    // If you #define an entire declaration, and expect this to auto-document - you stupid as hell. Make a custom entry for that...

    std::string function_def;
    bool not_full_decl = true;
    if (last_read_line.length() > 8) {

        // Remove everything up to (possible) end of the comment
        const size_t comment_index = last_read_line.find_first_of('/');
        if (comment_index != std::string::npos) {
            last_read_line.erase(0, comment_index + 1);
        }

        // Remove everything up to a possible comment - will fuck up if there is a param with a default value that contains "/".
        // FIXME : CHANGE THIS TO NOT ERASE DEFAULT PARAMS
        const size_t end_comment_index = last_read_line.find_last_of('/');
        if (end_comment_index != std::string::npos)
            last_read_line.erase(end_comment_index);

        // Now gotta check if there is a complete function declaration
        not_full_decl = !containsInOrder(last_read_line, "()");
        function_def = last_read_line;
    }
    // Otherwise, we just gotta keep reading stupid empty lines until we encounter enough substance to classify it as a function definition
    while (not_full_decl) {
        std::string line = getline(file);
        function_def.append(rltrim(line) + ' ');

        // The only way to be sure it's a function def. is by checking for "... (...)"
        // This could fail if there is a function call as a default parameter
        if (containsInOrder(function_def, "()")) {
            break;
        }
    }

    // Minor cleanup
    function_def.erase(function_def.find_last_of(")"));
    rltrim(function_def);
    return function_def;
}

bool isLineCommented(const std::string& line) {
    static bool is_multiline = false;
    std::string copy = line;
    rltrim(copy);

    if (copy.size() < 2) {
        return is_multiline;
    }


    if (copy.at(0) == '/') {
        if (copy.at(1) == '*') {
            is_multiline = true;
            return true;
        }
        else if (copy.at(1) == '/')
            return true;
    }
    // If the very start or end of the line are ends of multiline comments
    if (copy.at(copy.size() - 2) == '*' && copy.back() == '/') {
        is_multiline = false;
        return true;
    }

    // If line doesn't contain any tokens due to it being in a multiline:
    if (copy.find_first_not_of("/*") == 0)
        return is_multiline;


    // If we've reached this spot, this means that there is a multiline comment
    // that does not have its multiline end token at the end of the line
    // Thus there must be code beyond it
    is_multiline = false;
    return false;
}


// Parses commandline arguments to find supplied read/write files. Will have to probably rewrite this to be more flexible
// for future flags. 
bool extractArguments(const int& args, const char* argv[], std::string& rfile, std::string& wfile) {
    // Possible future flags:
    // -fs  :   Parses entire folder for files
    // -fe  :   Changes the file extension that is checked for (for example "-fe py" would look for .py files). Default it .h/.hpp
    // -fea :   Adds to the list of file extensions to parse
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

    const char* FUNCTION_REGEX = R"(([a-zA-Z_][\w\s:<>,*&]*\s+[a-zA-Z_]\w*\s*\([^)]*\)\s*;))";
    auto matches = getRegexMatches(file_content, FUNCTION_REGEX);

    for (const std::smatch& match : matches) {
        std::cout << "Found function: " << match[1] << '\n';
    }

    std::cout << "Finsihed!";
}