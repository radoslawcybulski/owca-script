#ifndef RC_OWCA_SCRIPT_RUNTIME_FUNCTION_H
#define RC_OWCA_SCRIPT_RUNTIME_FUNCTION_H

#include "stdafx.h"
#include "allocation_base.h"
#include "ast_function.h"

namespace OwcaScript {
	namespace Internal {
		class CodeBuffer;

		struct RuntimeFunction {
			std::vector<OwcaValue> values_from_parents;
			std::shared_ptr<CodeBuffer> code;
			std::span<AstFunction::CopyFromParent> copy_from_parents;
			std::span<std::string_view> identifier_names;
			std::string_view name;
			unsigned int param_count = 0;
			ImplStat *body = nullptr;

			explicit operator bool() const { return code != nullptr; }
		};

		struct RuntimeFunctions : public AllocationBase {
			std::unordered_map<unsigned int, RuntimeFunction> functions;
			std::string_view name;

			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(VM &vm, GenerationGC generation_gc) override;
		};
	}
}

#endif
