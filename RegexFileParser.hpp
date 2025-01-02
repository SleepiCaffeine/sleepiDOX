#pragma once
#include <vector>
#include <string>
#include <regex>

// Returns a vector of all regex matches within the provided string.
// `match` must be a raw string literal, with an R"" prefix.
// Example: R"(w+\s)"
std::vector< std::smatch > getRegexMatches(const std::string& content, const char* match);

std::ifstream openReadFile(const char* fileName);

std::string extractFileContent(const std::ifstream& file_stream);

std::ofstream openWriteFile(const char* fileName);

std::string getline(std::ifstream& file);