#ifndef RC_OWCA_SCRIPT_TOKENS_H
#define RC_OWCA_SCRIPT_TOKENS_H

#include "stdafx.h"
#include "owca_error_message.h"

namespace OwcaScript {
	namespace Internal {
		class UserClassToken { };
		class UserClassTokenPtr {
			const UserClassToken *tok = nullptr;
		public:
			UserClassTokenPtr(const UserClassToken &tok) : tok(&tok) {}
			bool operator == (UserClassTokenPtr other) const { return tok == other.tok; }
			bool operator != (UserClassTokenPtr other) const { return !(*this == other); }
		};
	}
}

#endif
