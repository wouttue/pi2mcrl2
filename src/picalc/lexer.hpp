#ifndef PICALC_LEXER_H
#define PICALC_LEXER_H

#include "core.hpp"

#include <ostream>
#include <string>
#include <vector>

namespace picalc {

enum class TokenType {
    IDENTIFIER,
    NUMBER,
    AGENT, // agent
    TAU, // tau
    SEMICOLON, // ;
    OPENING_PAREN, // (
    CLOSING_PAREN, // )
    OPENING_BRACKET, // [
    CLOSING_BRACKET, // ]
    LESS_THAN, // <
    GREATER_THAN, // >
    COMMA, // ,
    APOSTROPHE, // '
    EQUALS, // =
    PERIOD, // .
    PLUS, // +
    PIPE, // |
    CIRCONFLEX, // ^
    TILDE // ~
};

std::ostream& operator<<(std::ostream& os, const TokenType& token);

struct Token final {
    explicit Token(TokenType token_type, std::string value, SourceLocation loc) :
        token_type(token_type),
        value(std::move(value)),
        loc(loc) {}

    // std::string to_string() const; // TODO

    TokenType token_type;
    std::string value;
    SourceLocation loc;
};

Expected<std::vector<Token>> tokenize(const std::string& input);

} // end namespace picalc

#endif
