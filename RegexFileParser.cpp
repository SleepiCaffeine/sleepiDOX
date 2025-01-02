#include "RegexFileParser.hpp"
#include "slpUtility.hpp"
#include <fstream>
#include <sstream>

// This will catch every comment single line, or not
// Multilines will be caught as one
// (\/\/.*?$)|(\/\*(.|\n)*?\*\\)

std::vector< std::smatch > getRegexMatches(const std::string& content, const char* match) {
	std::regex provided_regex(match, std::regex_constants::ECMAScript);
	std::smatch smatch;

	auto start_iter = content.cbegin();
	std::vector<std::smatch> all_matches;
	while (std::regex_search(start_iter, content.cend(), smatch, provided_regex)) {
		all_matches.push_back(smatch);

		// UB if std::match_results::ready() == false
		// (https://en.cppreference.com/w/cpp/regex/match_results/suffix)
		if (smatch.ready())
			start_iter = smatch.suffix().first;
		else break;
	}

		return all_matches;
}


std::ifstream openReadFile(const char* fileName)
{
	std::ifstream file(fileName);
	if (!file) {
		throw std::runtime_error("Failed to open file");
	}
	return file;
}

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

std::string getline(std::ifstream& file) {
	const int LINE_LENGTH = 512;
	char cline[LINE_LENGTH];
	file.getline(&cline[0], LINE_LENGTH, '\n');
	std::string line(cline);
	return rtrim(line);
}

