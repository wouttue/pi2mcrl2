#include "core.hpp"

namespace picalc {

std::ostream& operator<<(std::ostream& os, SourceLocation const& loc) {
    return os << "(" << (loc.line + 1) << ", " << (loc.character + 1) << ")";
}

} // end namespace picalc

