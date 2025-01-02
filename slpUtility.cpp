#include "slpUtility.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>



// Checks whether `str` contains every character in `chars` in the order provided.
// Returns true if so.
bool containsInOrder(const std::string& str, const std::string& chars) {
    size_t last_pos = 0;
    return std::none_of(chars.cbegin(), chars.cend(), [&str, &last_pos](char ch) {
        last_pos = str.find(ch, last_pos);
        return last_pos == std::string::npos;
        });
}

std::ifstream openReadFile(const char* fileName)
{
    std::ifstream file(fileName);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    return file;
}

// Extract the rdbuf() within `file_stream` into string stream
// and return string
std::string extractFileContent(const std::ifstream& file_stream)
{
    std::ostringstream oss;
    oss << file_stream.rdbuf();
    return oss.str();
}

std::ofstream openWriteFile(const char* fileName)
{
    std::ofstream file(fileName);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    return file;
}

// trim from end
std::string rtrim(const std::string& s) {
    std::string copy{ s };
    const size_t pos = copy.find_last_not_of(" w\t\f\v\r\n") + 1;
    if (pos >= copy.size())
        return copy;
    return copy.erase(pos);
}
// trim from start
std::string ltrim(const std::string& s) {
    std::string copy{ s };
    return copy.erase(0, copy.find_first_not_of(" w\t\f\v\r\n"));
}

std::string rltrim(const std::string& str) {
    return ltrim(rtrim(str));
}
