#ifndef RC_OWCA_SCRIPT_TOKENS_H
#define RC_OWCA_SCRIPT_TOKENS_H

#include "stdafx.h"
#include "owca_error_message.h"

namespace OwcaScript {
	class FunctionToken {
		const void* tok = nullptr;
	public:
		explicit FunctionToken(const void* tok) : tok(tok) {}

		auto value() const { return tok; }

		bool operator == (FunctionToken o) const { return tok == o.tok; }
		bool operator != (FunctionToken o) const { return tok != o.tok; }
	};
	class ClassToken {
		const void* tok = nullptr;
	public:
		explicit ClassToken(const void* tok) : tok(tok) {}

		auto value() const { return tok; }

		bool operator == (ClassToken o) const { return tok == o.tok; }
		bool operator != (ClassToken o) const { return tok != o.tok; }
	};
}

#endif
