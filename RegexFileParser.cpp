#include "RegexFileParser.hpp"
#include "slpUtility.hpp"
#include <fstream>
#include <sstream>

// This will catch every comment single line, or not
// Multilines will be caught as one
// (\/\/.*?$)|(\/\*(.|\n)*?\*\\)

std::vector< std::smatch > getRegexMatches(const std::string& content, const char* match) {
	std::regex provided_regex(match);
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

// Funciton that accepts a string, and a regex (of 2 groups)
// Will replace every instance of group 2 with group 1.
// FIXME: gets caught by some dumb #defines
void replaceCodeAliases(std::string& content, const std::regex& regex) {
	std::smatch match;

	auto start = content.cbegin();
	auto end = content.cend();
	// Because each loop, the string gets reallocated, it's impossible to get an iterator
	// to the first non-matched character. Therefore, this loop is not really efficient...
	// Maybe I could try doing std::advance(.cbegin(), match[1].position() + match[2].str().length()) or something
	while (std::regex_search(start, end, match, regex)) {
		std::string original = match[2].str();
		std::string alias = match[1].str();
		std::regex aliasRegex("\\b" + alias + "\\b");
		content = std::regex_replace(content, aliasRegex, original);

		start = content.cbegin();
		std::advance(start, match.position(1) + match[2].length());
		end = content.cend();
	}
}

// Function to preprocess the code to handle #define, using, and typedef directives
// Currently only works for one file
std::string preprocessCode(const std::string& content) {
    std::string preprocessedContent = content;

    // Handle #define directives
    std::regex defineRegex(R"(#define\s+(\w+)\s+(.+))");
	replaceCodeAliases(preprocessedContent, defineRegex);

    // Handle using directives
    std::regex usingRegex(R"(using\s+(\w+)\s*=\s*([\w:]+);)");
	replaceCodeAliases(preprocessedContent, usingRegex);

    // Handle typedef directives
    std::regex typedefRegex(R"(typedef\s+([\w:]+)\s+(\w+);)");
	replaceCodeAliases(preprocessedContent, typedefRegex);

    return preprocessedContent;
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

