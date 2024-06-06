#include "parser.hpp"

#include <cassert>

namespace picalc {

Expected<Module> parse_module(std::vector<Token> tokens) {
    Parser parser(std::move(tokens));
    auto result = parser.parse_module();
    if (!result) {
        assert(parser.errors.size() > 0);
        return Expected<Module>(std::move(parser.errors));
    }

    assert(parser.errors.size() == 0);
    return Expected<Module>(std::move(*result));
}

std::optional<Module> Parser::parse_module() {
    std::vector<ProcDecl> declarations;
    bool error = false;
    while (has_token()) {
        auto next = parse_proc_decl();
        if (!next) {
            error = true;
        }
        if (!error) {
            declarations.push_back(std::move(*next));
        } else {
            // fast-forward to the next semicolon
            while (has_token() && get_token().token_type != TokenType::SEMICOLON) {
                skip();
            }
            if (has_token()) {
                skip();
            }
        }
    }

    if (error) {
        return std::nullopt;
    }

    return Module(std::move(declarations));
}

std::optional<ProcDecl> Parser::parse_proc_decl() {
    auto loc = get_loc();

    if (!skip_if_equal(TokenType::AGENT)) {
        return expected("`agent` keyword to start process declaration");
    }
    auto id = parse_identifier();
    if (!id) return std::nullopt;

    std::vector<Identifier> params;
    if (skip_if_equal(TokenType::OPENING_PAREN)) {
        auto list = parse_arg_list();
        if (!list) return std::nullopt;
        params = std::move(*list);

        if (!skip_if_equal(TokenType::CLOSING_PAREN)) {
            return expected("a closing parenthesis at the end of parameter list");
        }
    }

    if (!skip_if_equal(TokenType::EQUALS)) {
        return expected("an `=` after process name and parameter list");
    }

    auto proc = parse_proc();
    if (!proc) return std::nullopt;

    if (!skip_if_equal(TokenType::SEMICOLON)) {
        return expected("a `;` at the end of process declaration");
    }

    return ProcDecl(
        std::move(*id),
        std::move(params),
        std::move(*proc),
        loc
    );
}

std::optional<std::unique_ptr<Proc>> Parser::parse_proc() {
    auto result = parse_parallel_proc();
    if (!result) return std::nullopt;

    while (has_token() && get_token().token_type == TokenType::PLUS) {
        auto loc = get_loc();

        skip();

        auto rhs = parse_parallel_proc();
        if (!rhs) return std::nullopt;

        result = std::make_unique<Proc>(
            SumProc(std::move(*result), std::move(*rhs)),
            loc
        );
    }

    return result;
}

std::optional<std::vector<Identifier>> Parser::parse_arg_list() {
    if (!has_token()) {
        return eof();
    }

    std::vector<Identifier> result;

    auto token = &get_token();
    while (token->token_type == TokenType::IDENTIFIER) {
        result.push_back(Identifier(token->value));
        skip();
        if (!has_token() || get_token().token_type != TokenType::COMMA) {
            break;
        }
        skip();
        if (!has_token()) {
            break;
        }
        token = &get_token();
    }

    return result;
}

std::optional<std::unique_ptr<Proc>> Parser::parse_parallel_proc() {
    auto result = parse_unary_proc();
    if (!result) return std::nullopt;

    while (has_token() && get_token().token_type == TokenType::PIPE) {
        auto loc = get_loc();

        skip();

        auto rhs = parse_unary_proc();
        if (!rhs) return std::nullopt;

        result = std::make_unique<Proc>(
            ParallelProc(std::move(*result), std::move(*rhs)),
            loc
        );
    }

    return result;
}

std::optional<std::unique_ptr<Proc>> Parser::parse_unary_proc() {
    if (!has_token()) {
        return eof();
    }

    auto token_type = get_token().token_type;
    auto loc = get_loc();

    // tau . UnaryProc
    if (token_type == TokenType::TAU) {
        skip();
        if (!skip_if_equal(TokenType::PERIOD)) {
            return expected("`.` in tau prefix");
        }

        auto suffix = parse_unary_proc();
        if (!suffix) return std::nullopt;

        return std::make_unique<Proc>(
            TauPrefixProc(std::move(*suffix)),
            loc
        );
    }

    // A ( a_1, ..., a_n ) or A
    // a ( b ) . UnaryProc
    // a ' b . UnaryProc
    if (token_type == TokenType::IDENTIFIER) {
        auto subject = get_token().value;
        skip();
        if (skip_if_equal(TokenType::OPENING_PAREN)) {
            if (skip_if_equal(TokenType::CLOSING_PAREN)) {
                // A ( )
                return std::make_unique<Proc>(
                    IdentifierProc(
                        Identifier(std::move(subject)),
                        std::vector<Identifier>()
                    ),
                    loc
                );
            }

            auto args = parse_arg_list();
            if (!args) return std::nullopt;

            if (!skip_if_equal(TokenType::CLOSING_PAREN)) {
                return expected("`)` as closing delimiter for recursive call");
            }

            if (skip_if_equal(TokenType::PERIOD)) {
                // A ( a ) . UnaryProc
                if ((*args).size() != 1) {
                    return syntax_error("an input prefix must have only one object");
                }

                auto proc = parse_unary_proc();
                if (!proc) return std::nullopt;

                return std::make_unique<Proc>(
                    PositivePrefixProc(
                        Identifier(std::move(subject)),
                        (*args)[0],
                        std::move(*proc)
                    ),
                    loc
                );
            }

            return std::make_unique<Proc>(
                IdentifierProc(
                    Identifier(std::move(subject)),
                    std::move(*args)
                ),
                loc
            );
        } else if (skip_if_equal(TokenType::APOSTROPHE)) {
            auto object = parse_identifier();
            if (!object) return std::nullopt;

            if (!skip_if_equal(TokenType::PERIOD)) {
                return expected("`.` in an output prefix");
            }

            auto proc = parse_unary_proc();
            if (!proc) return std::nullopt;

            return std::make_unique<Proc>(
                NegativePrefixProc(
                    Identifier(std::move(subject)),
                    std::move(*object),
                    std::move(*proc)
                ),
                loc
            );
        } else {
            return std::make_unique<Proc>(
                IdentifierProc(
                    Identifier(std::move(subject)),
                    std::vector<Identifier>()
                ),
                loc
            );
        }
    }

    // [ a = b ] UnaryProc
    if (token_type == TokenType::OPENING_BRACKET) {
        skip();
        auto id1 = parse_identifier();
        if (!id1) return std::nullopt;

        if (!skip_if_equal(TokenType::EQUALS)) {
            return expected("an = sign in match clause");
        }

        auto id2 = parse_identifier();
        if (!id2) return std::nullopt;

        if (!skip_if_equal(TokenType::CLOSING_BRACKET)) {
            return expected("a ] at the end of the match clause");
        }

        auto proc = parse_unary_proc();
        if (!proc) return std::nullopt;

        return std::make_unique<Proc>(
            MatchProc(std::move(*id1), std::move(*id2), std::move(*proc)),
            loc
        );
    }

    // 0
    if (token_type == TokenType::NUMBER) {
        if (get_token().value != "0") {
            return syntax_error("The only number that can be used as a process is 0");
        }
        skip();
        return std::make_unique<Proc>(ZeroProc(), loc);
    }

    // ( ^ a ) UnaryProc
    // ( Proc )
    if (token_type == TokenType::OPENING_PAREN) {
        skip();

        if (skip_if_equal(TokenType::CIRCONFLEX)) {
            auto name = parse_identifier();
            if (!name) return std::nullopt;

            if (!skip_if_equal(TokenType::CLOSING_PAREN)) {
                return expected("`)` symbol in a restriction process");
            }

            auto proc = parse_unary_proc();
            if (!proc) return std::nullopt;

            return std::make_unique<Proc>(
                RestrictionProc(std::move(*name), std::move(*proc)),
                loc
            );
        }

        auto proc = parse_proc();
        if (!proc) return std::nullopt;

        if (!skip_if_equal(TokenType::CLOSING_PAREN)) {
            return expected("closing `)`");
        }

        return proc;
    }

    return expected("a process");
}

std::optional<Identifier> Parser::parse_identifier() {
    if (!has_token()) {
        return eof();
    }
    auto token = get_token();
    if (token.token_type == TokenType::IDENTIFIER) {
        skip();
        return Identifier(token.value);
    } else {
        return expected("an identifier");
    }
}

bool Parser::has_token() const {
    return index < tokens.size();
}

Token const& Parser::get_token() const {
    assert(has_token());
    return tokens[index];
}

SourceLocation Parser::get_loc() const {
    assert(has_token());
    return tokens[index].loc;
}

SourceLocation Parser::get_last_loc() const {
    if (tokens.size() == 0) {
        return SourceLocation(0, 0);
    } else {
        return tokens[tokens.size() - 1].loc;
    }
}

void Parser::skip() {
    index++;
}

bool Parser::skip_if_equal(TokenType token_type) {
    if (has_token() && get_token().token_type == token_type) {
        skip();
        return true;
    } else {
        return false;
    }
}

std::nullopt_t Parser::eof() {
    errors.push_back(Error {
        ErrorType::SYNTAX,
        get_last_loc(),
        "Reached unexpected end of file"
    });
    return std::nullopt;
}

std::nullopt_t Parser::syntax_error(std::string message) {
    errors.push_back(Error {
        ErrorType::SYNTAX,
        has_token() ? get_token().loc : get_last_loc(),
        std::move(message)
    });
    return std::nullopt;
}

std::nullopt_t Parser::expected(const std::string& message) {
    errors.push_back(Error {
        ErrorType::SYNTAX,
        has_token() ? get_token().loc : get_last_loc(),
        "Expected " + message // TODO print token found
    });
    return std::nullopt;
}

} // end namespace picalc
