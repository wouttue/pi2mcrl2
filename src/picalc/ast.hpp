#ifndef PICALC_AST_H
#define PICALC_AST_H

#include "core.hpp"

#include <memory>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace picalc {

struct Proc;

struct ZeroProc final {};

struct TauPrefixProc final {
    explicit TauPrefixProc(std::unique_ptr<Proc> proc) :
        proc(std::move(proc)) {}

    std::unique_ptr<Proc> proc;
};

struct PositivePrefixProc final {
    explicit PositivePrefixProc(
        Identifier subject,
        Identifier object,
        std::unique_ptr<Proc> proc
    ) :
        subject(std::move(subject)),
        object(std::move(object)),
        proc(std::move(proc)) {}

    Identifier subject;
    Identifier object;
    std::unique_ptr<Proc> proc;
};

struct NegativePrefixProc final {
    explicit NegativePrefixProc(
        Identifier subject,
        Identifier object,
        std::unique_ptr<Proc> proc
    ) :
        subject(std::move(subject)),
        object(std::move(object)),
        proc(std::move(proc)) {}

    Identifier subject;
    Identifier object;
    std::unique_ptr<Proc> proc;
};

struct MatchProc final {
    explicit MatchProc(
        Identifier name1,
        Identifier name2,
        std::unique_ptr<Proc> proc
    ) :
        name1(std::move(name1)),
        name2(std::move(name2)),
        proc(std::move(proc)) {}

    Identifier name1;
    Identifier name2;
    std::unique_ptr<Proc> proc;
};

struct RestrictionProc final {
    explicit RestrictionProc(Identifier name, std::unique_ptr<Proc> proc) :
        name(std::move(name)),
        proc(std::move(proc)) {}

    Identifier name;
    std::unique_ptr<Proc> proc;
};

struct SumProc final {
    explicit SumProc(std::unique_ptr<Proc> lhs, std::unique_ptr<Proc> rhs) :
        lhs(std::move(lhs)),
        rhs(std::move(rhs)) {}

    std::unique_ptr<Proc> lhs;
    std::unique_ptr<Proc> rhs;
};

struct ParallelProc final {
    explicit ParallelProc(std::unique_ptr<Proc> lhs, std::unique_ptr<Proc> rhs) :
        lhs(std::move(lhs)),
        rhs(std::move(rhs)) {}

    std::unique_ptr<Proc> lhs;
    std::unique_ptr<Proc> rhs;
};

struct IdentifierProc final {
    explicit IdentifierProc(
        Identifier variable_id,
        std::vector<Identifier> args
    ) :
        variable_id(std::move(variable_id)),
        args(std::move(args)) {}

    Identifier variable_id;
    std::vector<Identifier> args;
};

struct Proc final {
    explicit Proc(ZeroProc proc, SourceLocation loc) :
        variants(std::move(proc)),
        loc(loc) {}

    explicit Proc(TauPrefixProc proc, SourceLocation loc) :
        variants(std::move(proc)),
        loc(loc) {}

    explicit Proc(PositivePrefixProc proc, SourceLocation loc) :
        variants(std::move(proc)),
        loc(loc) {}

    explicit Proc(NegativePrefixProc proc, SourceLocation loc) :
        variants(std::move(proc)),
        loc(loc) {}

    explicit Proc(MatchProc proc, SourceLocation loc) :
        variants(std::move(proc)),
        loc(loc) {}

    explicit Proc(RestrictionProc proc, SourceLocation loc) :
        variants(std::move(proc)),
        loc(loc) {}

    explicit Proc(SumProc proc, SourceLocation loc) :
        variants(std::move(proc)),
        loc(loc) {}

    explicit Proc(ParallelProc proc, SourceLocation loc) :
        variants(std::move(proc)),
        loc(loc) {}

    explicit Proc(IdentifierProc proc, SourceLocation loc) :
        variants(std::move(proc)),
        loc(loc) {}

    template <typename R, typename T, typename... Args>
    R visit(T&& visitor, Args&&... args) {
        // return std::visit(visitor, variants);
        return std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ZeroProc>) {
                return visitor(arg, std::forward<Args>(args)...);
            } else if constexpr (std::is_same_v<T, TauPrefixProc>) {
                return visitor(arg, std::forward<Args>(args)...);
            } else if constexpr (std::is_same_v<T, PositivePrefixProc>) {
                return visitor(arg, std::forward<Args>(args)...);
            } else if constexpr (std::is_same_v<T, NegativePrefixProc>) {
                return visitor(arg, std::forward<Args>(args)...);
            } else if constexpr (std::is_same_v<T, MatchProc>) {
                return visitor(arg, std::forward<Args>(args)...);
            } else if constexpr (std::is_same_v<T, RestrictionProc>) {
                return visitor(arg, std::forward<Args>(args)...);
            } else if constexpr (std::is_same_v<T, SumProc>) {
                return visitor(arg, std::forward<Args>(args)...);
            } else if constexpr (std::is_same_v<T, ParallelProc>) {
                return visitor(arg, std::forward<Args>(args)...);
            } else if constexpr (std::is_same_v<T, IdentifierProc>) {
                return visitor(arg, std::forward<Args>(args)...);
            } else {
                assert(false && "fail");
            }
        }, variants);
    }

    SourceLocation get_loc() const {
        return loc;
    }

private:
    std::variant<
        ZeroProc,
        TauPrefixProc,
        PositivePrefixProc,
        NegativePrefixProc,
        MatchProc,
        RestrictionProc,
        SumProc,
        ParallelProc,
        IdentifierProc
    > variants;

    SourceLocation loc;
};

struct ProcDecl final {
    explicit ProcDecl(
        Identifier id,
        std::vector<Identifier> params,
        std::unique_ptr<Proc> proc,
        SourceLocation loc
    ) :
        id(std::move(id)),
        params(std::move(params)),
        proc(std::move(proc)),
        loc(loc) {}

    ProcDecl(ProcDecl&& rhs) = default;
    ProcDecl& operator=(ProcDecl&& rhs) = default;

    Identifier id;
    std::vector<Identifier> params;
    std::unique_ptr<Proc> proc;
    SourceLocation loc;
};

struct Module final {
    explicit Module(std::vector<ProcDecl> declarations) :
        declarations(std::move(declarations)) {}

    Module(Module&& rhs) = default;
    Module& operator=(Module&& rhs) = default;

    std::vector<ProcDecl> declarations; // it shouldn't need
};

std::ostream& operator<<(std::ostream& os, Proc const& decl);

std::ostream& operator<<(std::ostream& os, ProcDecl const& decl);

std::ostream& operator<<(std::ostream& os, Module const& mod);

} // end namespace picalc

#endif
