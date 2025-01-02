#pragma once
#include <string>
#include <fstream>


bool containsInOrder(const std::string& str, const std::string& chars);

std::ifstream openReadFile(const char* fileName);

std::string extractFileContent(const std::ifstream& file_stream);

std::ofstream openWriteFile(const char* fileName);

// trim from end
std::string rtrim(const std::string& s);
// trim from start
std::string ltrim(const std::string& s);
// trim from both sides
std::string rltrim(const std::string& str);


