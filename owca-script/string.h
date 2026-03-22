#ifndef RC_OWCA_SCRIPT_STRING_H
#define RC_OWCA_SCRIPT_STRING_H

#include "stdafx.h"
#include "allocation_base.h"
#include "line.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		struct String : public AllocationBase {
			static constexpr const Kind object_kind = Kind::String;

            mutable size_t hash_value = 0;
			const std::uint32_t size_ : 31 = 0;
			mutable std::uint32_t hash_calculated : 1 = 0;

			std::string_view type() const override { return "String"; }
			std::string to_string() const override { return std::string{ text() }; }
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) override {}
			
			String(std::uint32_t size) : size_(size) {}

            std::string_view text() const { return { pointer(), size_ };}
			size_t size() const { return size_; }
            size_t hash() const;
			char *pointer() { return (char*)this + sizeof(*this); }
			const char *pointer() const { return (char*)this + sizeof(*this); }
		};
	}
}

#endif
