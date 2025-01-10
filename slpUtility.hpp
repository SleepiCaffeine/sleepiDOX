#pragma once
#include <string>


bool containsInOrder(const std::string& str, const std::string_view& chars);

// trim from end
std::string rtrim(const std::string_view& s);
// trim from start
std::string ltrim(const std::string_view& s);
// trim from both sides
std::string rltrim(const std::string_view& str);

// Removes given token from string. Additionally removes multiline-comment end token.
// Returns an edited copy of the string
std::string commentTrim(const std::string_view& comment, const std::string_view& token);


