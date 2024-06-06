#ifndef PICALC_PARSER_H
#define PICALC_PARSER_H

#include "ast.hpp"
#include "core.hpp"
#include "lexer.hpp"

#include <vector>

namespace picalc {

Expected<Module> parse_module(std::vector<Token> tokens);

struct Parser final {
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

    std::vector<Error> errors;

    std::optional<Module> parse_module();

    std::optional<ProcDecl> parse_proc_decl();

    std::optional<std::unique_ptr<Proc>> parse_proc();

private:
    std::vector<Token> tokens;
    size_t index = 0;

    std::optional<std::vector<Identifier>> parse_arg_list();

    std::optional<std::unique_ptr<Proc>> parse_parallel_proc();

    std::optional<std::unique_ptr<Proc>> parse_unary_proc();

    std::optional<Identifier> parse_identifier();

    bool has_token() const;

    Token const& get_token() const;

    SourceLocation get_loc() const;

    SourceLocation get_last_loc() const;

    void skip();

    bool skip_if_equal(TokenType token_type);

    std::nullopt_t eof();

    std::nullopt_t syntax_error(std::string message);

    std::nullopt_t expected(const std::string& message);
};

} // end namespace picalc

#endif
