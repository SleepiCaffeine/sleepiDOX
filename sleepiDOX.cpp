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
    if (str.at(0) != '/' && ( str.at(1) == '*' || str.at(1) == '/') )
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

std::string getline(std::ifstream& file) {
    const int LINE_LENGTH = 256;
    char cline[LINE_LENGTH];
    file.getline(&cline[0], LINE_LENGTH, '\n');
    std::string line(cline);
    return rtrim(line);
}


// The line is passed in, because it has already been read, hence it's a line that might contain function definition
std::vector<std::string>& tokenizeFunctionDecl(std::ifstream& file, const std::string& line) {
    // Functions can have varying signatures, templates almost encouraging multiline delcarations makes this annoying
    // Signarutes can be as simple as:
    // return-type functionName() {
    // 
    // Or as annoying as:
    // template <...>
    // return-type<...> functionName( prefix type name1,
    //                                prefix type name2, ...) additional {
    // Hence there will be a lot of annoying stuff
    

    // For now, this function will only work without any definitions or C++17 attributes


}

bool isLineCommented(const std::string& line) {
    static bool is_multiline = false;
    std::string copy = line;
    ltrim(copy);

    if (copy.at(0) != '/' || copy.at(0) == '*')
        return is_multiline;
    else if (copy.at(1) == '*') {
        is_multiline = true;
        return true;
    }
    else if (copy.at(1) == '/')
        return true;

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


    
    ;
    std::vector<DOXEntry> entries;
    int entry_index = 0;

    // READ THE FILE LINE BY LINE
    while (!file.eof()) {
        // READ THE LINE AND TRIM
        std::string line = getline(file);

        const bool commented_line = isLineCommented(line);
        if (commented_line) {
            entries.at(entry_index).functionality.entry += comment_trim(line) + '\n';
        }
        // This line is not a comment, so therefore must be tokenized
        else {
            auto tokens = tokenizeFunctionDecl(file, line);
        }

        // DETERMINE WHETHER OR NOT LINE STARTS OR ENDS COMMENT
        const std::vector<std::string> tokens = tokenizeLine(line);


        std::cout << "[\"" << tokens.at(0) << "\"";
        for (int i = 1; i < tokens.size(); ++i) {
            std::cout << ", \"" << tokens.at(i) << "\"";
        }
        std::cout << "]\n";
    }
}