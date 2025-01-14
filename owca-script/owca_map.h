#ifndef RC_OWCA_SCRIPT_OWCA_MAP_H
#define RC_OWCA_SCRIPT_OWCA_MAP_H

#include "stdafx.h"
#include "allocation_base.h"

namespace OwcaScript {
	namespace Internal {
		class VM;
		class ImplExprIndexRead;
		class ImplExprIndexWrite;
		struct DictionaryShared;
	}

	class OwcaMap {
		friend class Internal::VM;
		friend class Internal::ImplExprIndexRead;
		friend class Internal::ImplExprIndexWrite;

		Internal::DictionaryShared *dictionary;

		OwcaMap(Internal::DictionaryShared* dictionary) : dictionary(dictionary) {}
	public:
		~OwcaMap() = default;

		std::string to_string() const;
		size_t size() const;
	};
}

#endif
