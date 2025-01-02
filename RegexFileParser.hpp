#pragma once
#include <vector>
#include <string>
#include <regex>

// Should be good for checking functions
// ([A-Za-z_\(\*&][\w:< >,&\*\(\)]*\s+)+([A-Za-z_]\w*)\s*\(((\s*[A-Za-z_\(\*&][\w:<>,&\*\(\)]*\s+)+([A-Za-z_]\w*)\s*,?)*\s*\)\s*([A-Za-z_]\w*\s*)*;
// Full description: (?<returnType>[A-Za-z_\(\*&][\w:< >,&\*\(\)]*\s+)+(?<funcName>[A-Za-z_]\w*)\s*\((?<PARAMETER>(?<paramType>\s*[A-Za-z_\(\*&][\w:<>,&\*\(\)]*\s+)+(?<paramName>[A-Za-z_]\w*)\s*,?)*\s*\)\s*(?<Qualifiers>[A-Za-z_]\w*\s*)*; 

// Returns a vector of all regex matches within the provided string.
// `match` must be a raw string literal, with an R"" prefix.
// Example: R"(w+\s)"
std::vector< std::smatch > getRegexMatches(const std::string& content, const char* match);



