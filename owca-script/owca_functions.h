#ifndef RC_OWCA_SCRIPT_OWCA_FUNCTIONS_H
#define RC_OWCA_SCRIPT_OWCA_FUNCTIONS_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaVM;
	class OwcaValue;

	namespace Internal {
		class ImplExprNativeFunction;
		class ImplExprScriptFunction;
		struct RuntimeFunction;
		struct RuntimeFunctions;
		class VM;
	}
	class OwcaFunctions {
		friend class Internal::ImplExprNativeFunction;
		friend class Internal::ImplExprScriptFunction;
		friend class Internal::VM;
		friend class OwcaValue;

		Internal::RuntimeFunctions* functions = nullptr;

		void add(Internal::RuntimeFunction);
	public:
		OwcaFunctions(OwcaVM &);

		std::string_view name() const;
	};
}

#endif
