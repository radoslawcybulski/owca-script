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
		class ImplExprCompare;
		struct AllocationBase;
		struct RuntimeFunction;
		struct RuntimeFunctions;
		class VM;
		struct Object;
		struct BoundFunctionSelfObject;
		struct Class;
	}
	class OwcaFunctions {
		friend class Internal::ImplExprNativeFunction;
		friend class Internal::ImplExprScriptFunction;
		friend class Internal::ImplExprScriptClass;
		friend class Internal::ImplExprNativeClass;
		friend class Internal::ImplExprCompare;
		friend class Internal::VM;
		friend class OwcaValue;
		friend struct Internal::Class;

		Internal::RuntimeFunctions* functions = nullptr;
		Internal::AllocationBase* self_object = nullptr; // either Object or BoundFunctionSelfObject
	public:
		OwcaFunctions(Internal::RuntimeFunctions* functions, Internal::Object* self_object = nullptr);
		OwcaFunctions(Internal::RuntimeFunctions* functions, Internal::BoundFunctionSelfObject* self_object);

		std::string_view name() const;
	};
}

#endif
