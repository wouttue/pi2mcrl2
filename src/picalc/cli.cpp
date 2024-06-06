#include "cli.hpp"

namespace picalc {

// very quickly made, not very good quality CLI
Expected<CliOptions> parse_args(const std::vector<std::string>& args) {
    std::vector<Error> errors;
    std::optional<std::string> input_file;
    std::optional<std::string> output_file;
    bool raw_output = false;
    bool verbose = false;

    int i = 0;
    while (i < args.size()) {
        if (args[i].size() > 0 && args[i][0] == '-') {
            // read option
            if (args[i] == "-o" || args[i] == "--output") {
                if (i + 1 >= args.size()) {
                    errors.push_back(Error(
                        ErrorType::CLI,
                        SourceLocation(0, i),
                        "No value given for option --output"
                    ));
                    continue;
                }
                output_file = args[i + 1];
                i++;
            } else if (args[i] == "--raw-output") {
                raw_output = true;
            } else if (args[i] == "--verbose") {
                verbose = true;
            }
        } else {
            input_file = args[i];
        }

        i++;
    }

    if (!input_file.has_value()) {
        errors.push_back(Error(
            ErrorType::CLI,
            SourceLocation(0, 0),
            "No input file specified"
        ));
    }

    if (errors.size() > 0) {
        return Expected<CliOptions>(std::move(errors));
    } else {
        return Expected<CliOptions>(CliOptions {
            raw_output,
            verbose,
            *input_file,
            output_file,
        });
    }
}

} // end namespace picalc
