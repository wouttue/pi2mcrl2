#include "lexer.hpp"

#include <cassert>
#include <utility>

namespace picalc {

std::ostream& operator<<(std::ostream& os, const TokenType& token) {
    switch (token) {
        case TokenType::IDENTIFIER:
            os << "identifier";
            break;
        case TokenType::NUMBER:
            os << "number";
            break;
        case TokenType::AGENT:
            os << "agent";
            break;
        case TokenType::SEMICOLON:
            os << ";";
            break;
        case TokenType::OPENING_PAREN:
            os << "(";
            break;
        case TokenType::CLOSING_PAREN:
            os << ")";
            break;
        case TokenType::EQUALS:
            os << "=";
            break;
        case TokenType::PERIOD:
            os << ".";
            break;
        case TokenType::PLUS:
            os << "+";
            break;
        case TokenType::PIPE:
            os << "|";
            break;
        case TokenType::CIRCONFLEX:
            os << "^";
            break;
        case TokenType::TILDE:
            os << "~";
            break;
    }
    return os;
}

Expected<std::vector<Token>> tokenize(const std::string& input) {
    std::vector<Token> result;
    uint32_t line = 0;
    uint32_t character = 0;
    uint32_t currentLine = 0;
    uint32_t currentCharacter = 0;
    TokenType currentTokenType = TokenType::IDENTIFIER;
    std::string current;
    std::vector<Error> errors;

    auto endToken = [&]() {
        if (current.size() == 0) {
            return;
        }

        SourceLocation loc(currentLine, currentCharacter);
        if (current == "agent") {
            result.push_back(Token(TokenType::AGENT, "", loc));
        } else if (current == "tau") {
            result.push_back(Token(TokenType::TAU, "", loc));
        } else if (current[0] >= '0' && current[0] <= '9') {
            result.push_back(Token(TokenType::NUMBER, std::move(current), loc));
        } else {
            result.push_back(Token(TokenType::IDENTIFIER, std::move(current), loc));
        }
        current = "";
    };

    auto put = [&](char c) {
        if (current.size() == 0) {
            currentLine = line;
            currentCharacter = character;
        }
        current.push_back(c);
        character++;
    };

    auto addSymbol = [&](TokenType tokenType) {
        endToken();
        currentLine = line;
        currentCharacter = character;
        character++;
        SourceLocation loc(currentLine, currentCharacter);
        result.push_back(Token(tokenType, "", loc));
        assert(current.size() == 0);
    };

    for (size_t i = 0; i < input.size(); i++) {
        char c = input[i];

        if (c == ' ' || c == '\t' || c == '\r') {
            endToken();
            character++;
        } else if (c == '\n') {
            endToken();
            line++;
            character = 0;
        } else if (c == '%') {
            endToken();
            character++;
            while (i < input.size() && input[i] != '\n') {
                i++;
            }
        } else if (c == ';') {
            addSymbol(TokenType::SEMICOLON);
        } else if (c == '(') {
            addSymbol(TokenType::OPENING_PAREN);
        } else if (c == ')') {
            addSymbol(TokenType::CLOSING_PAREN);
        } else if (c == '[') {
            addSymbol(TokenType::OPENING_BRACKET);
        } else if (c == ']') {
            addSymbol(TokenType::CLOSING_BRACKET);
        } else if (c == '<') {
            addSymbol(TokenType::LESS_THAN);
        } else if (c == '>') {
            addSymbol(TokenType::GREATER_THAN);
        } else if (c == ',') {
            addSymbol(TokenType::COMMA);
        } else if (c == '\'') {
            addSymbol(TokenType::APOSTROPHE);
        } else if (c == '=') {
            addSymbol(TokenType::EQUALS);
        } else if (c == '.') {
            addSymbol(TokenType::PERIOD);
        } else if (c == '+') {
            addSymbol(TokenType::PLUS);
        } else if (c == '|') {
            addSymbol(TokenType::PIPE);
        } else if (c == '^') {
            addSymbol(TokenType::CIRCONFLEX);
        } else if (c == '~') {
            addSymbol(TokenType::TILDE);
        } else if (c >= '0' && c <= '9') {
            put(c);
        } else if (
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '_'
        ) {
            if (current.size() > 0 && current[0] >= '0' && current[0] <= '9') {
                errors.push_back(Error(
                    ErrorType::SYNTAX,
                    SourceLocation(line, character),
                    "unexpected word characters in the middle of a number"
                ));
            } else {
                put(c);
            }
        } else {
            character++;
            errors.push_back(Error(
                ErrorType::SYNTAX,
                SourceLocation(line, character),
                "unknown character"
            ));
        }
    }

    endToken();

    if (errors.size() == 0) {
        return Expected<std::vector<Token>>(std::move(result));
    } else {
        return Expected<std::vector<Token>>(std::move(errors));
    }
}

} // end namespace picalc
