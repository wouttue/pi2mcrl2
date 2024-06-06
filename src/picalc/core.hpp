#ifndef PICALC_CORE_H
#define PICALC_CORE_H

#include <cassert>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace picalc {

struct SourceLocation final {
    explicit SourceLocation(uint32_t line, uint32_t character) :
        line(line),
        character(character) {}

    uint32_t line;
    uint32_t character;
};

std::ostream& operator<<(std::ostream& os, SourceLocation const& loc);

struct Identifier final {
    explicit Identifier(std::string value) : value(std::move(value)) {}

    std::string value;

    bool operator==(const Identifier& rhs) const {
        return value == rhs.value;
    }
};

enum class ErrorType {
    CLI,
    SYNTAX,
    IDENTIFIER
};

struct Error final {
    explicit Error(ErrorType error_type, SourceLocation loc, std::string message) :
        error_type(error_type),
        loc(loc),
        message(std::move(message)) {}

    ErrorType error_type;
    SourceLocation loc;
    std::string message;
};

template <typename T>
struct Expected final {
    Expected(T value) : value(std::move(value)), errors() {}

    explicit Expected(std::vector<Error> errors) :
        value(std::nullopt),
        errors(std::move(errors)) {}

    explicit Expected(Error error) :
        value(std::nullopt),
        errors(std::vector<Error>({ std::move(error) })) {}

    bool has_value() const {
        return value.has_value();
    }

    explicit operator bool() const {
        return value.has_value();
    }

    T& operator*() {
        assert(value.has_value());
        return *value;
    }

    std::optional<T> value;
    std::vector<Error> errors;
};

} // end namespace picalc

namespace std {
    template<>
    struct hash<picalc::Identifier> {
        size_t operator()(const picalc::Identifier& id) const {
            return hash<std::string>()(id.value);
        }
    };
}

#endif
