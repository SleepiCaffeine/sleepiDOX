#include "RegexFileParser.hpp"
#include "slpUtility.hpp"
#include <fstream>
#include <sstream>


std::vector< std::smatch > getRegexMatches(const std::string& content, const std::string& match) {
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

std::ifstream openReadFile(const std::string_view& fileName)
{
  std::ifstream file(fileName.data());
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

std::ofstream openWriteFile(const std::string_view& fileName)
{
  std::ofstream file(fileName.data());
  if (!file) {
  throw std::runtime_error("Failed to open file");
  }
  return file;
}

std::string getline(std::ifstream& file) {
  std::string line("", 512);
  file.getline(&line[0], line.size(), '\n');
  return rtrim(line);
}

std::vector<_scopeMatch> extractScopeMatches(const std::string& fileContent) {

  std::vector<_scopeMatch> all_matches;

  // Regex that matches classes, class name, and inheritance
  std::regex scope_regex(R"((class|struct)\s+([^\d][\w]*)\s*(?::\s*(public|private|protected)?\s*[^\d][\w]*)?\s*\{([^{}]*(?:\{[^{}]*\}[^{}]*)*)\})");
  std::smatch smatch;
  auto start_iter = fileContent.cbegin();
   
  while (std::regex_search(start_iter, fileContent.cend(), smatch, scope_regex)) {

    if (smatch.ready()) {
      start_iter = smatch[2].second;
      const ScopeResolution res = (smatch[1] == "class") ? ScopeResolution::Class : ScopeResolution::Struct;
      all_matches.emplace_back(smatch[2], std::distance(fileContent.cbegin(), start_iter), smatch.length(), res);
    }
    else break;
  }

  // Now separately extract namespaces. 
  scope_regex = std::regex(R"(namespace\s+([^\d\W]\w*)\s*\{([^{}]*)\})");
  start_iter = fileContent.cbegin();
  while (std::regex_search(start_iter, fileContent.cend(), smatch, scope_regex)) {
    if (smatch.ready()) {
      start_iter = smatch[1].second;
      all_matches.emplace_back(smatch[1], std::distance(fileContent.cbegin(), start_iter), smatch.length(), ScopeResolution::Namespace);
    }
    else break;
  }
  return all_matches;
}

void isolateEntries(const std::string& fileContent, Sleepi::DOXContainer& entries) {


  std::vector< std::shared_ptr<std::string> > scopeNames;
  // Keeps track of where class bodies begin and end, so'd, that when we query for functions
  // we don't include ones that are already within classes.
  std::vector <std::pair <size_t, size_t >> scopeBodyPositions;

  for (const auto& classMatch : extractScopeMatches(fileContent)) {
    scopeNames.push_back( std::make_shared<std::string>(classMatch.match + "::") );
    scopeBodyPositions.emplace_back(classMatch.pos, classMatch.pos + classMatch.len);
  }

  Sleepi::DOXEntry entry;
  const char* FUNCTION_REGEX = R"((\/\/[ \t]*@sleepiDOX[^\n]*\n?|\/\*[ \n\t]*@sleepiDOX[\s\S]*?\*\/)|(^[ \t]*[a-zA-Z_][\w\s:<>,*&]*\s+([a-zA-Z_]\w*)\s*\([\s\S]*?\)\s*(const)?\s*(noexcept)?\s*;))";
  const auto functionMatches = getRegexMatches(fileContent, FUNCTION_REGEX);
  for (const std::smatch& match : functionMatches) {



    if (match[1].matched) {
      entry.at(Sleepi::ENTRY_COMMENT) += commentTrim(match[1].str(), "@sleepiDOX");
    }
    if (match[2].matched) {

      // I don't want to add an entry for something that has no documentation
      // And since the only required comment is a @sleepiDOX one, I will filter based on that
      if (entry.at(Sleepi::ENTRY_COMMENT).empty()) {
        entry = {};
        continue;
      }


      std::string functionDefinition = rltrim(match[2].str());  // with return type, additional keywoprds, alladat jazz
      std::string functionName = rltrim(match[3].str());

      // Checks whether the function is within a class
      std::string className; 
      const size_t funcStart = std::distance(fileContent.cbegin(), match.prefix().second);
      const size_t funcEnd   = funcStart + match.length();

      // Check if a function is within a class
      for (size_t index = 0; index < scopeBodyPositions.size(); ++index) {
        if (funcStart >= scopeBodyPositions.at(index).first &&
          funcEnd <= scopeBodyPositions.at(index).second) {
          className += *scopeNames.at(index);
        }
      }

      if (!className.empty()) {
        functionDefinition.insert(functionDefinition.find(functionName), className);
        functionName.insert(0, className);
      }

      entry.at(Sleepi::ENTRY_FUNCTION_DEFINTION) = functionDefinition;
      entry.at(Sleepi::ENTRY_FUNCTION_NAME) = functionName;

      entries[entry.at(Sleepi::ENTRY_FUNCTION_NAME)].push_back(entry);
      entry = {};
    }
  }
}
