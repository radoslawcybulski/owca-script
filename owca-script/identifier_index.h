#ifndef RC_OWCA_SCRIPT_IDENTIFIER_INDEX_H
#define RC_OWCA_SCRIPT_IDENTIFIER_INDEX_H

#include "stdafx.h"

namespace OwcaScript {
    class IdentifierIndex {
        std::uint32_t value_ = 0xffffffff;
    public:
        explicit IdentifierIndex(std::uint32_t index) : value_(index) {}

        auto value() const { return value_; }

        explicit operator bool () const { return value_ != 0xffffffff; }
        bool operator == (IdentifierIndex other) const { return value_ == other.value_; }
        bool operator != (IdentifierIndex other) const { return value_ != other.value_; }
    };
}

namespace std {
    template<>
    struct hash<OwcaScript::IdentifierIndex> {
        std::size_t operator()(OwcaScript::IdentifierIndex index) const {
            return std::hash<std::uint32_t>()(index.value() * 17);
        }
    };
}

#endif
