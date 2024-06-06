
#include "picalc/cli.hpp"
#include "picalc/lexer.hpp"
#include "picalc/mcrl2converter.hpp"
#include "picalc/parser.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

const char* help_message = R"(pi2mcrl2 - a small tool for building mCRL2 models from pi-calculus expressions
Usage:
$ pi2mcrl2 <file.picalc> <args...>
where <args...> is some of:
    [--output <file>]       -o  Specifies the output .mcrl2 file
    [--verbose]                 Prints extra information
    [--raw-output]              Do not give a full valid mCRL2 specification, only print the generated part
)";

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "No arguments specified\n" << help_message;
        return 1;
    }

    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    // NOTE: the CLI does not have a very high quality, because I did not deem
    // this very important to have a working application.
    auto expected_options = picalc::parse_args(args);
    if (!expected_options.has_value()) {
        for (auto& error : expected_options.errors) {
            std::cerr << error.loc << ": " << error.message << "\n";
        }
        return 1;
    }
    auto options = *expected_options;

    std::ifstream file(options.input_file);
    std::stringstream buffer;
    buffer << file.rdbuf();

    auto tokens = picalc::tokenize(buffer.str());
    if (!tokens.has_value()) {
        for (auto& error : tokens.errors) {
            std::cerr << error.loc << ": " << error.message << "\n";
        }
        return 1;
    }

    auto mod = picalc::parse_module(std::move(*tokens));
    if (!mod.has_value()) {
        for (auto& error : mod.errors) {
            std::cerr << error.loc << ": " << error.message << "\n";
        }
        return 1;
    }

    if (options.verbose) {
        std::cerr << "[verbose] module: " << *mod << "\n";
    }

    auto mcrl2 = picalc::convert_module_to_mcrl2(*mod);
    if (!mcrl2.has_value()) {
        for (auto& error : mcrl2.errors) {
            std::cerr << error.loc << ": " << error.message << "\n";
        }
        return 1;
    }

    std::string output;
    if (options.raw_output) {
        output = std::move(*mcrl2);
    } else {
        std::ifstream mcrl2_def_file("./mcrl2/picalc.mcrl2");
        std::stringstream buffer;
        buffer << mcrl2_def_file.rdbuf();
        output = buffer.str();
        output += "init PiInterpreter(\n% BEGIN GENERATED\n";
        output += *mcrl2;
        output += "\n% END GENERATED\n);";
    }

    if (options.output_file.has_value()) {
        std::ofstream output_file(*options.output_file);
        output_file << output;
    } else {
        std::cout << output;
    }

    return 0;
}
