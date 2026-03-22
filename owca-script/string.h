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

			mutable std::string data;
            mutable size_t hash_value = 0;
            mutable bool hash_calculated = false;

			template <typename ... F> auto visit(F &&...fns) const {
				struct overloaded : F... {
					using F::operator()...;
				};
				return std::visit(overloaded{std::forward<F>(fns)...}, data);
			}
	
			std::string_view type() const override { return "String"; }
			std::string to_string() const override { return data; }
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) override {}
			
			String(std::string txt) : data(std::move(txt)) {}

			void flatten() const;
            std::string_view text() const { return data; }
			size_t size() const { return data.size(); }
            size_t hash() const;
		};
	}
}

#endif
