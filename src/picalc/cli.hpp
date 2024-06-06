#ifndef PICALC_CLI_H
#define PICALC_CLI_H

#include "core.hpp"

#include <string>
#include <vector>

namespace picalc {

struct CliOptions {
    bool raw_output = false;
    bool verbose = false;
    std::string input_file;
    std::optional<std::string> output_file;
};

Expected<CliOptions> parse_args(const std::vector<std::string>& args);

} // end namespace picalc

#endif
