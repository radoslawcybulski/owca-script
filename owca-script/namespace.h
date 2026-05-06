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
            std::unordered_map<std::string_view, size_t> identifier_to_global_index;

            std::string_view type() const override;
			std::string to_string() const override;

			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;
            OwcaValue member(std::string_view key) const;
            std::optional<OwcaValue> try_member(std::string_view key) const;
            void set_member(std::string_view key, OwcaValue val);
            bool try_set_member(std::string_view key, OwcaValue val);

			Namespace(OwcaCode code, std::unordered_map<std::string_view, size_t> identifier_to_global_index) :
				code(std::move(code)), identifier_to_global_index(std::move(identifier_to_global_index)) {
                    globals.resize(this->identifier_to_global_index.size());
                }
		};
	}
}

#endif
