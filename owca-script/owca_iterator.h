#ifndef RC_OWCA_SCRIPT_OWCA_ITERATOR_H
#define RC_OWCA_SCRIPT_OWCA_ITERATOR_H

#include "stdafx.h"
#include "owca_map.h"

namespace OwcaScript {
    class OwcaValue;

    namespace Internal {
        class VM;

        struct IteratorBase {
            virtual ~IteratorBase() = default;

            virtual OwcaValue next() = 0;
        };

        struct Iterator;
    }

	class OwcaIterator {
        Internal::Iterator *object;

	public:
        OwcaIterator(Internal::Iterator *object) : object(object) {}

		auto internal_value() const { return object; }

        bool completed() const;
        OwcaValue next() const;
	};
}

#endif
