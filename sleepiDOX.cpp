#include "sleepiDOX.hpp"
#include <filesystem>
#include <stdexcept>

Sleepi::DOXContext extractArguments(const std::vector<std::string>& argv, const bool strict) {
    // Possible future flags:
    // -xp  :   Would attempt to parse multiple files looking for things like #define's, "using"s, and such to find what things are defined as. [Most likely not]

    namespace sErr = Sleepi::ErrorBits;
    Sleepi::DOXContext context =  { {}, "", 0};

    // Looping up to .size() - 1, because every flag has to have a second string afterward
    for (size_t index = 0; index < argv.size() - 1; ++index) {
        const std::string_view argument = argv.at(index);

        // Flag that dictates the source file
        if (argument == "-s") {
            context.sourceDirs.push_back(argv.at(index + 1));
        }
        // Flag that dictates the destination file
        else if (argument == "-d") {
            context.outputFileDir = argv.at(index + 1);
        }
        // Flag that dictates the directory where the source files are kept
        else if (argument == "-fs") {
            namespace fs = std::filesystem;

            const std::string_view directory = argv.at(index + 1);
            const fs::path inputDirPath = directory;

            // Make sure the file actually exists, and if not, set an error flag
            if (!fs::exists(inputDirPath)) {
                context.errorFlags |= sErr::InputDirDoesntExist;
                return context;
            }

            // Loop through every file in the directory
            for (const auto& directory_entry : fs::directory_iterator(inputDirPath)) {
                
                // Ignore every file that isn't a "regular" aka text file
                if (!directory_entry.is_regular_file())
                    continue;
                // check if the file is a header file
                const fs::path current_path = directory_entry.path();
                if (current_path.extension() == ".hpp" || current_path.extension() == ".h")
                    context.sourceDirs.push_back(current_path.string());
            }

            // At this point, all the files have been found, but there's a chance there were no headers
            if (context.sourceDirs.empty())
                context.errorFlags |= sErr::NoInputFilesFound;

        }
    }

    if (context.outputFileDir.empty())
        context.errorFlags |= sErr::NoOutputFileSpecified;

    // If the -fs flag was not specified, and the input is still empty
    // This means that no input was provided at all.

    // This could've been done with some sort of bool like:
    // `bool inputProvided = false;` but eh, doesn't really change much
    if ((context.errorFlags & sErr::NoInputFilesFound) == 0
        && context.sourceDirs.empty()) {
        context.errorFlags |= sErr::NoInputSpecified;
    }

    if (strict)
        validateContext(context);

    return context;
}

void validateContext(const Sleepi::DOXContext& context) {
    namespace Err = Sleepi::ErrorBits;
    const size_t& flags = context.errorFlags;
    

    if (flags & Err::InputDirDoesntExist)
        throw std::system_error( std::error_code(), "The provided input directory does not exist.");
    if (flags & Err::NoInputFilesFound)
        throw std::system_error(std::error_code(), "The program was unable to find the file(s) specified.");
    if (flags & Err::NoInputSpecified)
        throw std::logic_error("No input flag provided. Use the flags -s or -fs to provide a file to parse.");
    if (flags & Err::NoOutputFileSpecified)
        throw std::logic_error("No output flag provided. Use the flag -d to provide the destination.");

}


void generateDocFile(std::ofstream& output_file, const Sleepi::DOXContainer& entries, const std::string_view& title) {

    using namespace Sleepi;

    // Markdown really likes double newlines
    constexpr char MD_NL[] = "\n\n";
    constexpr char H1[] = "# ";
    constexpr char H2[] = "## ";
    constexpr char H3[] = "### ";


    if (!title.empty())
        output_file << H1 << title << MD_NL;

    output_file << H2 << "Table of contents : \n";
    for (const auto& [name, entry] : entries) {
        output_file << "- [" << name << "](#" << name << ")\n";
    } output_file << "- - -\n";


    for (const auto& [name, entry] : entries) {
        output_file << H3 << name << "()\n";

        for (const auto& entry_details : entry) {
            output_file << "```" << entry_details.at(ENTRY_FUNCTION_DEFINTION) << "```\n";
            output_file << H3 << "Description:\n" << entry_details.at(ENTRY_COMMENT) << MD_NL;

            if (!entry_details.at(ENTRY_PARAMS).empty()) {
                output_file << H3 << "Params:\n";
                output_file << entry_details.at(ENTRY_PARAMS) << MD_NL;
            }

            if (!entry_details.at(ENTRY_RETURNS).empty()) {
                output_file << H3 << "Returns:\n";
                output_file << entry_details.at(ENTRY_RETURNS) << MD_NL;
            }

            output_file << MD_NL;
        }
        output_file << "- - -\n";
    }
}