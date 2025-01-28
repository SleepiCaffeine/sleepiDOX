#include "Headers/RegexFileParser.hpp"
#include "Headers/slpUtility.hpp"
#include <fstream>
#include <sstream>


std::vector< std::smatch > Sleepi::getRegexMatches(const std::string& content, const std::string& match) {
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

std::ifstream Sleepi::openReadFile(const std::string_view& fileName)
{
  std::ifstream file(fileName.data());
  if (!file) {
  throw std::runtime_error("Failed to open file");
  }
  return file;
}

std::string Sleepi::extractFileContent(const std::ifstream& file_stream)
{
  std::ostringstream oss;
  oss << file_stream.rdbuf();
  return oss.str();
}

std::ofstream Sleepi::openWriteFile(const std::string_view& fileName)
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

std::vector<Sleepi::DOXScope> Sleepi::extractScopeMatches(const std::string& fileContent) {

  std::vector<Sleepi::DOXScope> all_matches;
  std::smatch smatch;

  // Regex that matches classes, class name, and inheritance. Comment could be false due to lack of updates
  // [1] - scope type (class or struct)
  // [2] - scope name
  std::regex scope_regex(R"((class|struct|namespace)\s+([^\d][\w]*)\s*(?::\s*public|private|protected?\s*[^\d][\w]*)?\s*\{)");
  auto start_iter = fileContent.cbegin();
   
  while (std::regex_search(start_iter, fileContent.cend(), smatch, scope_regex)) {
    if (!smatch.ready())
      break;

    // Iterator is now right after scope name
    start_iter = smatch[2].second; 
    // This is always non-negative, so later casting to size_t is safe
    const size_t pos = 
        static_cast<size_t>(std::distance(fileContent.cbegin(), start_iter));         
    
    size_t length     = 1;
    size_t braceCount = 0;
    for (size_t i = pos + 1; i < fileContent.size(); ++i) {
      if (fileContent[i] == '{')
        ++braceCount;
      else if (fileContent[i] == '}')
        --braceCount;

      if (!braceCount)
        break;
      ++length;
    }


    const auto location = std::pair(pos, length);
    Sleepi::DOXScope scope( smatch[2].str(), location);

      // Check to see if it belongs in some other scope
      // Only checking one scope "above", because if it's double nested,
      // then the parent will have the highest scope saved already
      if (!all_matches.empty() &&
          location.first > all_matches.back().location.first && location.second < all_matches.back().location.second) {
        scope.parentScope = std::make_shared<Sleepi::DOXScope>(all_matches.back());
      }

      all_matches.push_back(scope);
    }

  return all_matches;

  // Appendix:
  // The reason why namespaces are parsed seperately is because when I initially added "namespace" as an option alongside "class|struct"
  // It would inexplicably cause a std::regex_error::error_stack regardless of how much I simplified the regex.
  // This behavior did not translate to my other machine, hence I didn't really bother fixing it,
  // and instead opted to parse it separately. Later on I will reattempt to combine both regexes to save time and memory.

}

Sleepi::DOXContainer Sleepi::isolateEntries(const std::string& fileContent) {

  Sleepi::DOXContainer entries;
  Sleepi::DOXEntry entry;
  const char* FUNCTION_REGEX = R"((\/\/[ \t]*@sleepiDOX[^\n]*\n?|\/\*[ \n\t]*@sleepiDOX[\s\S]*?\*\/)|(^[ \t]*[a-zA-Z_][\w\s:<>,*&]*\s+([a-zA-Z_]\w*)\s*\([\s\S]*?\)\s*(const)?\s*(noexcept)?\s*;))";
  const auto functionMatches = Sleepi::getRegexMatches(fileContent, FUNCTION_REGEX);
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
      std::string functionName       = rltrim(match[3].str());

      Sleepi::DOXFunction function{ functionName, entry };

      function.location.first  = static_cast<size_t>(std::distance(fileContent.cbegin(), match[2].first));
      function.location.second = match[2].length();

      function.entry.at(Sleepi::ENTRY_FUNCTION_DEFINTION) = functionDefinition;
      function.entry.at(Sleepi::ENTRY_FUNCTION_NAME)      = functionName;

      entries.push_back(function);
      entry = {};
    }
  }

  return entries;
}

std::string Sleepi::getScopeSyntax(const Sleepi::DOXScope& scope) {
  std::string syntax = scope.scopeName;
  auto _scope = scope.parentScope;
  while (_scope) {
    syntax.insert(0, _scope->scopeName + "::");
    _scope = _scope->parentScope;
  }
  if (!syntax.empty())
    syntax += "::";

  return syntax;
}


void Sleepi::assignParentScopes(Sleepi::DOXContainer& functions, const std::vector<Sleepi::DOXScope>& scopes) {

  // Tries to find the last scope that encompasses the function
  // This will end up being the most inner scope, which is saved as the index into the
  // Scope vector. Afterwards the scope in that index is cast as a pointer and the Sleepi::DOXFunction is complete.


  
  for (Sleepi::DOXFunction& function : functions) {
    int counter{ 0 }, index{ -1 };
    const auto funcEnd = function.location.first + function.location.second;

    for (const Sleepi::DOXScope& scope : scopes) {
      const auto scopeEnd = scope.location.first + scope.location.second;

      if (function.location.first >= scope.location.first && funcEnd <= scopeEnd) {
        index = counter;
      }
      ++counter;
    }
  

    std::string className;

    if (index != -1) {
      function.scope = std::make_shared<Sleepi::DOXScope>(scopes.at(index));
      className      = getScopeSyntax(scopes.at(index));
    }


    if (!className.empty()) {
      std::string& functionDefinition = function.entry.at(Sleepi::ENTRY_FUNCTION_DEFINTION);
      std::string& functionName       = function.entry.at(Sleepi::ENTRY_FUNCTION_NAME);

      functionDefinition.insert(functionDefinition.find(functionName), className);
      functionName.insert(0, className);
    }
  }
}
