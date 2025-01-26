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
		struct AllocationBase;
		struct RuntimeFunction;
		struct RuntimeFunctions;
		class VM;
		struct Object;
		struct BoundFunctionSelfObject;
	}
	class OwcaFunctions {
		friend class Internal::ImplExprNativeFunction;
		friend class Internal::ImplExprScriptFunction;
		friend class Internal::ImplExprScriptClass;
		friend class Internal::ImplExprNativeClass;
		friend class Internal::VM;
		friend class OwcaValue;

		Internal::RuntimeFunctions* functions = nullptr;
		Internal::AllocationBase* self_object = nullptr; // either Object or BoundFunctionSelfObject
	public:
		OwcaFunctions(Internal::RuntimeFunctions* functions, Internal::Object* self_object = nullptr);
		OwcaFunctions(Internal::RuntimeFunctions* functions, Internal::BoundFunctionSelfObject* self_object);

		std::string_view name() const;
	};
}

#endif
