#ifndef PICALC_MCRL2CONVERTER_H
#define PICALC_MCRL2CONVERTER_H

#include "ast.hpp"

#include <unordered_map>

namespace picalc {

struct MCRL2ConverterState {
    std::string indentation_string;
    int indentation_counter;
    std::string output;
    std::vector<Error> errors;
    std::unordered_map<Identifier, int> name_map;
    std::unordered_map<Identifier, int> variable_map;
    int name_counter;
};

void convert_module_to_mcrl2(const Module& module, MCRL2ConverterState& state);

Expected<std::string> convert_module_to_mcrl2(const Module& module);

} // end namespace picalc

#endif
