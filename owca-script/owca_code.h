#ifndef RC_OWCA_SCRIPT_OWCA_CODE_H
#define RC_OWCA_SCRIPT_OWCA_CODE_H

#include "stdafx.h"

namespace OwcaScript {
	namespace Internal {
		class CodeBuffer;
		class VM;
	}

	class OwcaCode {
		friend class Internal::VM;

		std::shared_ptr<Internal::CodeBuffer> code_;
	public:
		OwcaCode(std::shared_ptr<Internal::CodeBuffer> code_) : code_(std::move(code_)) {}
	};
}

#endif
