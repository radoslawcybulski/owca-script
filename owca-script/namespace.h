#ifndef RC_OWCA_SCRIPT_NAMESPACE_H
#define RC_OWCA_SCRIPT_NAMESPACE_H

#include "owca-script/executor.h"
#include "stdafx.h"
#include "allocation_base.h"
#include "ast_function.h"
#include "owca_vm.h"
#include "owca_code.h"
#include "owca_value.h"
#include "exec_buffer.h"
#include "executor.h"
#include <unordered_map>
#include <vector>

namespace OwcaScript {
	namespace Internal {
		struct Namespace : public AllocationBase {
			static constexpr const Kind object_kind = Kind::Namespace;

			OwcaCode code;
            std::vector<OwcaValue> globals;
            OwcaValue *constants;
            std::string_view type() const override;
			std::string to_string() const override;

			void gc_mark(GenerationGC generation_gc) const override;
            OwcaValue member(IdentifierIndex key) const;
            std::optional<OwcaValue> try_member(IdentifierIndex key) const;
            void set_member(IdentifierIndex key, OwcaValue val);
            bool try_set_member(IdentifierIndex key, OwcaValue val);

            OwcaValue member(std::string_view key) const;
            std::optional<OwcaValue> try_member(std::string_view key) const;
            void set_member(std::string_view key, OwcaValue val);
            bool try_set_member(std::string_view key, OwcaValue val);

			Namespace(OwcaCode code) : code(std::move(code)) {
                globals.resize(this->code.globals_count());
                constants = this->code.constants();
            }

            OwcaValue bound_function_self_object() override;
		};
	}
}

#endif
