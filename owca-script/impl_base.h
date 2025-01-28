#ifndef RC_OWCA_SCRIPT_IMPL_BASE_H
#define RC_OWCA_SCRIPT_IMPL_BASE_H

#include "stdafx.h"
#include "owca_value.h"
#include "line.h"

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

			virtual void execute_impl(OwcaVM &) const = 0;

			void execute(OwcaVM &) const;
		};

		class ImplExpr : public ImplBase {
		public:
			using ImplBase::ImplBase;

			virtual OwcaValue execute_impl(OwcaVM &) const = 0;

			OwcaValue execute(OwcaVM &) const;
		};
	}
}

#endif
