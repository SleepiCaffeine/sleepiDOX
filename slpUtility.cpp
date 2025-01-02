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

std::string comment_trim(const std::string& str) {
    std::string copy = rltrim(str);
    const size_t token_index = str.find("@sleepiDOX");
    if (token_index != std::string::npos) {
        copy.erase(token_index, 11);
    }

    if (copy.size() < 2) {
        return copy;
    }

    // Annoying way of deleting slashes
    // This way I don't have to rltrim(copy) after
    if (copy.at(0) == '/') {
        if (copy.at(1) == '/') {
            copy.erase(0, copy.find_first_not_of("/ ", 1));
        }
        else if (copy.at(1) == '*') {
            copy.erase(0, copy.find_first_not_of("* ", 1));
        }
    }

    if (copy.at(0) == '*' && copy.at(1) == '/') {
        copy.erase(0, 2);
    }
    return copy;
}