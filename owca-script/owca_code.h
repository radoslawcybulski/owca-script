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

		const auto &internal_value() const { return code_; }

		std::vector<unsigned char> serialize_to_binary() const;
		bool compare(const OwcaCode &other) const;
	};
}

#endif
