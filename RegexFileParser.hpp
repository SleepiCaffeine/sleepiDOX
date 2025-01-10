#pragma once
#include <vector>
#include <string>
#include <regex>

// Returns a vector of all regex matches within the provided string.
// `match` must be a raw string literal, with an R"" prefix.
// Example: R"(w+\s)"
std::vector< std::smatch > getRegexMatches(const std::string& content, const char* match);
std::string preprocessCode(const std::string& content);

std::ifstream openReadFile(const std::string_view& fileName);

std::string extractFileContent(const std::ifstream& file_stream);

std::ofstream openWriteFile(const std::string_view& fileName);

std::string getline(std::ifstream& file);