#include "ast.hpp"

namespace picalc {

std::ostream& operator<<(std::ostream& os, ZeroProc const& proc) {
    return os << "0";
}

std::ostream& operator<<(std::ostream& os, TauPrefixProc const& proc) {
    return os << "tau . " << *proc.proc;
}

std::ostream& operator<<(std::ostream& os, PositivePrefixProc const& proc) {
    return os << proc.subject.value << "(" << proc.object.value << ") . " << *proc.proc;
}

std::ostream& operator<<(std::ostream& os, NegativePrefixProc const& proc) {
    return os << proc.subject.value << "'" << proc.object.value << " . " << *proc.proc;
}

std::ostream& operator<<(std::ostream& os, MatchProc const& proc) {
    return os << "[" << proc.name1.value << " = " << proc.name2.value << "] " << *proc.proc;
}

std::ostream& operator<<(std::ostream& os, RestrictionProc const& proc) {
    return os << "(^" << proc.name.value << ") " << *proc.proc;
}

std::ostream& operator<<(std::ostream& os, SumProc const& proc) {
    return os << "(" << *proc.lhs << " + " << *proc.rhs << ")";
}

std::ostream& operator<<(std::ostream& os, ParallelProc const& proc) {
    return os << "(" << *proc.lhs << " | " << *proc.rhs << ")";
}

std::ostream& operator<<(std::ostream& os, IdentifierProc const& proc) {
    os << proc.variable_id.value;
    if (proc.args.size() > 0) {
        os << "(";
        for (int i = 0; i < proc.args.size(); i++) {
            if (i > 0) {
                os << ", ";
            }
            os << proc.args[i].value;
        }
        os << ")";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Proc const& proc) {
    const_cast<Proc&>(proc).visit<void>([&](auto& proc) {
        os << proc;
    });
    return os;
}

std::ostream& operator<<(std::ostream& os, ProcDecl const& decl) {
    os << "agent " << decl.id.value;
    if (decl.params.size() > 0) {
        os << "(";
        for (int i = 0; i < decl.params.size(); i++) {
            if (i > 0) {
                os << ", ";
            }
            os << decl.params[i].value;
        }
        os << ")";
    }
    os << " = " << *decl.proc << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, Module const& mod) {
    for (auto& decl : mod.declarations) {
        os << decl << "\n";
    }
    return os;
}

} // end namespace picalc
