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

			static constexpr const unsigned int max_depth = 4;

			struct Substr {
				String *child;
				size_t start, size;
				unsigned int depth;
			};
			struct Add {
				String *left, *right;
				unsigned int depth;
			};
			struct Mult {
				String *child;
				size_t count;
				unsigned int depth;
			};
			mutable std::variant<std::string, Substr, Mult, Add> data;
            mutable size_t hash_value = 0;
            mutable bool hash_calculated = false;

			template <typename ... F> auto visit(F &&...fns) const {
				struct overloaded : F... {
					using F::operator()...;
				};
				return std::visit(overloaded{std::forward<F>(fns)...}, data);
			}
	
			std::string_view type() const override { return "String"; }
			std::string to_string() const override;
			void gc_mark(VM& vm, GenerationGC generation_gc) override;
			
			String(std::string txt) : data(std::move(txt)) {}
			String(Substr p) : data(p) {}
			String(Mult p) : data(p) {}
			String(Add p) : data(p) {}

			void flatten() const;
            std::string_view text() const;
			size_t size() const;
            size_t hash() const;
			unsigned int depth() const;
			void iterate_over_content(std::function<void(std::string_view)> tmp) const;
		};
	}
}

#endif
