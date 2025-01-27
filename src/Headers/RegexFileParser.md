## Table of contents : 
- [Sleepi](#1)

- - -
### Sleepi

- [std::string extractFileContent(const std::ifstream& file_stream)](#f1)
- [std::vector\< std::smatch \> getRegexMatches(const std::string& content, const std::string& match)](#f2)
- [std::ifstream openReadFile(const std::string_view& fileName)](#f3)
- [std::ofstream openWriteFile(const std::string_view& fileName)](#f4)

- - -
<h3 id="f1"> std::string Sleepi::extractFileContent(const std::ifstream& file_stream)</h3>

`RegexFileParser.hpp`

### Description:

A function that transfers the buffer of the filestream into a string



- - -

<h3 id="f3"> std::vector< std::smatch > Sleepi::getRegexMatches(const std::string& content, const std::string& match)</h3>

`RegexFileParser.hpp`

### Description:

A function to find and extract all substrings that match a provided regex.



- - -

<h3 id="f5"> std::ifstream Sleepi::openReadFile(const std::string_view& fileName)</h3>

`RegexFileParser.hpp`

### Description:

A function that returns an input file stream of the speficied file. Throws a `std::runtime_error` if the file could not be opened.



- - -

<h3 id="f7"> std::ofstream Sleepi::openWriteFile(const std::string_view& fileName)</h3>

`RegexFileParser.hpp`

### Description:

A function that returns an output file stream of the speficied file. Throws a `std::runtime_error` if the file could not be opened.



- - -



<p style="font-size : 10;">Made using <a href="https://github.com/SleepiCaffeine/sleepiDOX">sleepiDOX</a></p>