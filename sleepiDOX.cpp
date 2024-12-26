#include "sleepiDOX.hpp"
#include <sstream>
#include <iostream>
#include <algorithm> 

std::ifstream openReadFile(const char* fileName)
{
    std::ifstream file(fileName);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    return file;
}

std::ofstream openWriteFile(const char* fileName)
{
    std::ofstream file(fileName);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    return file;
}

// trim from end (in place)
inline std::string& rtrim(std::string& s) {
    s.erase(s.find_last_not_of(" w\t\f\v\r\n") + 1);
    return s;
}
// trim from start (in place)
inline std::string& ltrim(std::string& s) {
    s.erase(0, s.find_first_not_of(" w\t\f\v\r\n"));
    return s;
}

inline std::string& rltrim(std::string& str) {
    return ltrim(rtrim(str));
}

inline std::string& comment_trim(std::string& str) {
    rltrim(str);        
    if (str.at(0) == '/' && ( str.at(1) == '*' || str.at(1) == '/') )
        str.erase(0, 2);
    return str;
}

std::vector<std::string> tokenizeLine(const std::string& line)
{
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    while (!iss.eof()) {
        std::string temp;
        iss >> temp;
        if (temp == "//" || temp == "/*" || (temp == "*" && tokens.empty()))
            continue;

        tokens.push_back(temp);
    }
    return tokens;
}

// Checks whether `str` contains every character in `chars` in the order provided.
// Returns true if so.
inline bool containsInOrder(const std::string& str, const std::string& chars) {
    size_t last_pos = 0;
    return std::none_of(chars.cbegin(), chars.cend(), [&str, &last_pos](char ch) {
        last_pos = str.find(ch, last_pos);
        return last_pos == std::string::npos;
    });
 
}

std::string getline(std::ifstream& file) {
    const int LINE_LENGTH = 256;
    char cline[LINE_LENGTH];
    file.getline(&cline[0], LINE_LENGTH, '\n');
    std::string line(cline);
    return rtrim(line);
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
    bool not_full_decl = false;
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
    while (true && not_full_decl) {
        std::string line = getline(file);
        function_def.append(rltrim(line) + ' ');

        // The only way to be sure it's a function def. is by checking for "... (...)"
        if (containsInOrder(function_def, "()")) {
            break;
        }
    }

    // Minor cleanup
    function_def.erase(function_def.find_last_of(")") + 1);
    rltrim(function_def);
    return function_def;
}

bool isLineCommented(const std::string& line) {
    static bool is_multiline = false;
    std::string copy = line;
    rltrim(copy);

    if (copy.at(0) == '/') {
        if (copy.at(1) == '*') {
            is_multiline = true;
            return true;
        }
        else if (copy.at(1) == '/')
            return true;
    }
    // If the very start or end of the line are ends of multiline comments
    if ( copy.at(copy.size() - 2) == '*' && copy.back() == '/' ) {
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

DOXFragment populateFragment(const FragmentKey  key)
{
    switch (key) {
    case FragmentKey::Returns:
        return DOXFragment{ FragmentKey::Returns , 2, "" };
        break;
    case FragmentKey::Functionality:
        return DOXFragment{ FragmentKey::Functionality , 2, "" };
        break;
    case FragmentKey::Params:
        return DOXFragment{ FragmentKey::Params , 2, "" };
        break;
    case FragmentKey::Warning:
        return DOXFragment{ FragmentKey::Warning, 3, "" };
        break;
    case FragmentKey::TODO:
        return DOXFragment{ FragmentKey::TODO, 4, "" };
        break;
    case FragmentKey::Custom:
        return DOXFragment{ FragmentKey::Custom, 0, "" };
        break;
    }
}

const char* getFilename(int args, const char* argv[]) {
    const int FILENAME_INDEX = 1;
    if (args < FILENAME_INDEX)
        return nullptr;
    return argv[FILENAME_INDEX];
}



int main(int args, const char* argv[]) {
    // OPEN FILE
    const char* filename = getFilename(args, argv);
    if (filename == nullptr) {
        std::cout << "No File opened.\n";
        return EXIT_FAILURE;
    }
    auto file = openReadFile(filename);

    std::vector<DOXEntry> entries;
    int entry_index = 0;

    // READ THE FILE LINE BY LINE
    while (!file.eof()) {
        // READ THE LINE AND TRIM
        std::string line = getline(file);

        // DETERMINE WHETHER OR NOT LINE STARTS OR ENDS COMMENT
        const bool commented_line = isLineCommented(line);
        DOXEntry entry;
        if (commented_line) {
            /*std::cout << "ENTRY STUFF: " << comment_trim(line) + '\n';*/
            entry.functionality.entry += comment_trim(line) + '\n';
        }
        // This line is not a comment, so therefore must be tokenized
        else {
            std::string funcdecl = extractFunctionDecl(file, line);
            std::cout << "FUNCTION DECLARATION: " << funcdecl << "\n";



            
            ++entry_index;
        }

        entries.push_back(entry);
    }
    std::cout << "Finsihed!";
}