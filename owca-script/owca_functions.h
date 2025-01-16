#ifndef RC_OWCA_SCRIPT_OWCA_FUNCTIONS_H
#define RC_OWCA_SCRIPT_OWCA_FUNCTIONS_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaVM;
	class OwcaValue;

	namespace Internal {
		class ImplExprNativeFunction;
		class ImplExprScriptFunction;
		class ImplExprScriptClass;
		class ImplExprNativeClass;
		struct RuntimeFunction;
		struct RuntimeFunctions;
		class VM;
		struct Object;
	}
	class OwcaFunctions {
		friend class Internal::ImplExprNativeFunction;
		friend class Internal::ImplExprScriptFunction;
		friend class Internal::ImplExprScriptClass;
		friend class Internal::ImplExprNativeClass;
		friend class Internal::VM;
		friend class OwcaValue;

		Internal::RuntimeFunctions* functions = nullptr;
		Internal::Object* self_object = nullptr;
	public:
		OwcaFunctions(Internal::RuntimeFunctions* functions, Internal::Object* self_object=nullptr) : functions(functions), self_object(self_object) {}

		std::string_view name() const;
	};
}

#endif
