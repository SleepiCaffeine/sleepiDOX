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

std::string commentTrim(const std::string_view& comment, const std::string_view& token) {
    std::string comment_copy(comment);
    
    // Remove provided token
    if (const size_t token_index = comment_copy.find(token);
        token_index != std::string::npos) {
        comment_copy.erase(0, token_index + token.size()+1);
    }
    // Remove multiline-end token
    if (const size_t multiline_end_pos = comment_copy.find("*/");
                     multiline_end_pos != std::string::npos) {
        comment_copy.erase(multiline_end_pos);
    }

    return comment_copy;
}