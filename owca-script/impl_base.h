#ifndef RC_OWCA_SCRIPT_IMPL_BASE_H
#define RC_OWCA_SCRIPT_IMPL_BASE_H

#include "stdafx.h"
#include "owca_value.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		class ImplBase {
		public:
			const Line line;

			ImplBase(Line line) : line(line) {}

			virtual ~ImplBase() = default;
		};

		class ImplStat : public ImplBase {
		public:
			using ImplBase::ImplBase;

			virtual void execute(OwcaVM &) const = 0;
		};

		class ImplExpr : public ImplBase {
		public:
			using ImplBase::ImplBase;

			virtual OwcaValue execute(OwcaVM &) const = 0;
		};
	}
}

#endif
