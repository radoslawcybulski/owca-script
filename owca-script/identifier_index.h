#ifndef RC_OWCA_SCRIPT_IDENTIFIER_INDEX_H
#define RC_OWCA_SCRIPT_IDENTIFIER_INDEX_H

#include "stdafx.h"

namespace OwcaScript {
    namespace Internal {
        enum class IdentifierIndexKind : std::uint8_t {
            Local,
            Constant,
            Global,
        };
    class IdentifierIndex {
        std::uint32_t value_ = 0xffffffff;
    public:
        IdentifierIndex() = default;
        IdentifierIndex(IdentifierIndexKind kind, std::uint32_t index) : value_(index | (static_cast<std::uint32_t>(kind) << 30)) {}

        auto value() const { return value_; }
        auto index() const { return value_ & 0x3fffffff; }
        auto kind() const { return static_cast<IdentifierIndexKind>(value_ >> 30); }

        explicit operator bool () const { return value_ != 0xffffffff; }
        bool operator == (const IdentifierIndex &other) const { return value_ == other.value_; }
        bool operator != (const IdentifierIndex &other) const { return value_ != other.value_; }
        };
    }
}

#endif
