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

            virtual OwcaValue *get() = 0;
            virtual void next() = 0;
            virtual size_t remaining_size() = 0 ;
        };
    }

	class OwcaIterator {
        friend class Internal::VM;

        std::unique_ptr<Internal::IteratorBase> ib;

        OwcaIterator(std::unique_ptr<Internal::IteratorBase> ib) : ib(std::move(ib)) {}
	public:
        std::optional<OwcaValue> current() const;
        void next();
        size_t remaining_size() const;
	};
}

#endif
