#include "mcrl2converter.hpp"

namespace picalc {

static void add_indentation(MCRL2ConverterState& state) {
    for (int i = 0; i < state.indentation_counter; i++) {
        state.output += state.indentation_string;
    }
}

static std::optional<int> get_bound_name(
    const Identifier& id,
    SourceLocation loc,
    MCRL2ConverterState& state
) {
    auto it = state.name_map.find(id);
    if (it == state.name_map.end()) {
        state.errors.push_back(Error {
            ErrorType::IDENTIFIER,
            loc,
            std::string("Unbound name `") + id.value + "`"
        });
        return std::nullopt;
    }
    return it->second;
}

// returns (new name number, old name number)
static std::pair<int, std::optional<int>> add_bound_name(
    const Identifier& id,
    MCRL2ConverterState& state
) {
    int new_value = state.name_counter++;
    auto it = state.name_map.find(id);
    std::optional<int> old_value;
    if (it != state.name_map.end()) {
        old_value = it->second;
    } else {
        old_value = std::nullopt;
    }
    state.name_map[id] = new_value;

    return {new_value, old_value};
}

static void remove_bound_name(
    const Identifier& id,
    std::pair<int, std::optional<int>> old,
    MCRL2ConverterState& state
) {
    if (old.second.has_value()) {
        state.name_map[id] = old.second.value();
    } else {
        state.name_map.erase(id);
    }
    state.name_counter--;
}

static void convert_proc_to_mcrl2(const Proc& proc, MCRL2ConverterState& state);

static void convert_binary_proc_to_mcrl2(
    const std::string& kind,
    const Proc& lhs,
    const Proc& rhs,
    MCRL2ConverterState& state
) {
    state.output += kind;
    state.output += "(\n";
    state.indentation_counter++;
    add_indentation(state);
    convert_proc_to_mcrl2(lhs, state);
    state.output += ",\n";
    add_indentation(state);
    convert_proc_to_mcrl2(rhs, state);
    state.output += "\n";
    state.indentation_counter--;
    add_indentation(state);
    state.output += ")";
}

static void convert_prefix_proc_to_mcrl2(
    const std::string& kind,
    int subject_name,
    int object_name,
    const Proc& proc,
    MCRL2ConverterState& state
) {
    state.output += kind;
    state.output += "(";
    state.output += std::to_string(subject_name);
    state.output += ", ";
    state.output += std::to_string(object_name);
    state.output += ",\n";
    state.indentation_counter++;
    add_indentation(state);
    convert_proc_to_mcrl2(proc, state);
    state.indentation_counter--;
    state.output += "\n";
    add_indentation(state);
    state.output += ")";
}

static void convert_proc_to_mcrl2(
    const ZeroProc& proc,
    SourceLocation loc,
    MCRL2ConverterState& state
) {
    state.output += "zero";
}

static void convert_proc_to_mcrl2(
    const TauPrefixProc& proc,
    SourceLocation loc,
    MCRL2ConverterState& state
) {
    state.output += "tau_prefix(";
    convert_proc_to_mcrl2(*proc.proc, state);
    state.output += ")";
}

static void convert_proc_to_mcrl2(
    const PositivePrefixProc& proc,
    SourceLocation loc,
    MCRL2ConverterState& state
) {
    auto subject_name = get_bound_name(proc.subject, loc, state);
    if (!subject_name) return;

    auto object_name = add_bound_name(proc.object, state);

    convert_prefix_proc_to_mcrl2(
        "input_prefix",
        *subject_name,
        object_name.first,
        *proc.proc,
        state
    );

    remove_bound_name(proc.object, object_name, state);
}

static void convert_proc_to_mcrl2(
    const NegativePrefixProc& proc,
    SourceLocation loc,
    MCRL2ConverterState& state
) {
    auto subject_name = get_bound_name(proc.subject, loc, state);
    auto object_name = get_bound_name(proc.object, loc, state);
    if (!subject_name || !object_name) return;

    convert_prefix_proc_to_mcrl2(
        "output_prefix",
        *subject_name,
        *object_name,
        *proc.proc,
        state
    );
}

static void convert_proc_to_mcrl2(
    const MatchProc& proc,
    SourceLocation loc,
    MCRL2ConverterState& state
) {
    auto matched_name1 = get_bound_name(proc.name1, loc, state);
    auto matched_name2 = get_bound_name(proc.name2, loc, state);
    if (!matched_name1 || !matched_name2) return;

    state.output += "match_proc(";
    state.output += std::to_string(*matched_name1);
    state.output += ", ";
    state.output += std::to_string(*matched_name2);
    state.output += ",\n";
    state.indentation_counter++;
    add_indentation(state);
    convert_proc_to_mcrl2(*proc.proc, state);
    state.output += "\n";
    state.indentation_counter--;
    add_indentation(state);
    state.output += ")";
}

static void convert_proc_to_mcrl2(
    const RestrictionProc& proc,
    SourceLocation loc,
    MCRL2ConverterState& state
) {
    auto restricted_name = add_bound_name(proc.name, state);

    state.output += "restriction(";
    state.output += std::to_string(restricted_name.first);
    state.output += ",\n";
    state.indentation_counter++;
    add_indentation(state);
    convert_proc_to_mcrl2(*proc.proc, state);
    state.indentation_counter--;
    state.output += "\n";
    add_indentation(state);
    state.output += ")";

    remove_bound_name(proc.name, restricted_name, state);
}

static void convert_proc_to_mcrl2(
    const SumProc& proc,
    SourceLocation loc,
    MCRL2ConverterState& state
) {
    convert_binary_proc_to_mcrl2(
        "summation",
        *proc.lhs,
        *proc.rhs,
        state
    );
}

static void convert_proc_to_mcrl2(
    const ParallelProc& proc,
    SourceLocation loc,
    MCRL2ConverterState& state
) {
    convert_binary_proc_to_mcrl2(
        "parallel_composition",
        *proc.lhs,
        *proc.rhs,
        state
    );
}

static void convert_proc_to_mcrl2(
    const IdentifierProc& proc,
    SourceLocation loc,
    MCRL2ConverterState& state
) {
    auto var = state.variable_map.find(proc.variable_id);
    if (var == state.variable_map.end()) {
        state.errors.push_back(Error(
            ErrorType::IDENTIFIER,
            loc,
            "Unknown recursion variable `" + proc.variable_id.value + "`"
        ));
        return;
    }

    state.output += "identifier(";
    state.output += std::to_string(var->second);
    state.output += ", [";
    bool error = false;
    for (int i = 0; i < proc.args.size(); i++) {
        auto name = get_bound_name(proc.args[i], loc, state);
        if (!name) continue;
        if (i > 0) {
            state.output += ", ";
        }
        state.output += std::to_string(*name);
    }
    state.output += "])";
}

static void convert_proc_to_mcrl2(
    const Proc& proc,
    MCRL2ConverterState& state
) {
    auto loc = proc.get_loc();
    const_cast<Proc&>(proc).visit<void>([&](auto& arg) {
        convert_proc_to_mcrl2(arg, loc, state);
    });
}

static void convert_decl_to_mcrl2(
    const ProcDecl& decl,
    MCRL2ConverterState& state
) {
    // add parameter names to map
    std::vector<std::pair<int, std::optional<int>>> pairs;
    for (auto& id : decl.params) {
        pairs.push_back(add_bound_name(id, state));
    }

    state.output += "pi_definition(";
    state.output += std::to_string(decl.params.size());
    state.output += ",\n";
    state.indentation_counter++;
    add_indentation(state);
    convert_proc_to_mcrl2(*decl.proc, state);
    state.indentation_counter--;
    state.output += "\n";
    add_indentation(state);
    state.output += ")";

    // remove parameter names from map
    for (int i = pairs.size() - 1; i >= 0; i--) {
        remove_bound_name(decl.params[i], pairs[i], state);
    }
}

void convert_module_to_mcrl2(const Module& mod, MCRL2ConverterState& state) {
    std::optional<int> main_index;

    bool error = false;
    for (int i = 0; i < mod.declarations.size(); i++) {
        auto& decl = mod.declarations[i];
        auto it = state.variable_map.find(decl.id);
        if (it != state.variable_map.end()) {
            state.errors.push_back(Error {
                ErrorType::IDENTIFIER,
                decl.loc,
                "Duplicate process variable name"
            });
            error = true;
        }
        state.variable_map[decl.id] = i;

        if (decl.id.value == "Main") {
            main_index = i;
        }
    }

    if (!main_index.has_value()) {
        state.errors.push_back(Error {
            ErrorType::IDENTIFIER,
            SourceLocation(0, 0),
            "No starting process found; it should be called `Main`"
        });
        error = true;
    }

    if (error) return;

    state.output += "(lambda id': PiIdentifier . pi_definition(0, zero))";
    for (auto& decl : mod.declarations) {
        int index = state.variable_map.find(decl.id)->second;
        state.output += "[";
        state.output += std::to_string(index);
        state.output += " -> ";
        convert_decl_to_mcrl2(decl, state);
        state.output += "]";
    }

    state.output += ",\n";
    add_indentation(state);
    state.output += "identifier(";
    state.output += std::to_string(*main_index);
    state.output += ", [";
    auto& main_decl = mod.declarations[*main_index];
    for (int i = 0; i < main_decl.params.size(); i++) {
        if (i > 0) {
            state.output += ", ";
        }
        state.output += std::to_string(i);
    }
    state.output += "])";
}

Expected<std::string> convert_module_to_mcrl2(const Module& mod) {
    MCRL2ConverterState state {
        "    ",
        0,
        "",
        std::vector<Error>(),
        std::unordered_map<Identifier, int>(),
        std::unordered_map<Identifier, int>()
    };
    convert_module_to_mcrl2(mod, state);

    if (state.errors.size() > 0) {
        return Expected<std::string>(std::move(state.errors));
    }

    return std::move(state.output);
}

} // end namespace picalc
