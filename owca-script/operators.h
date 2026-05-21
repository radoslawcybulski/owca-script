#ifndef RC_OWCA_SCRIPT_OPERATORS_H
#define RC_OWCA_SCRIPT_OPERATORS_H

#include "stdafx.h"
#include "allocation_base.h"

namespace OwcaScript {
    class OwcaValue;
    namespace Internal {
        class Executor;

		struct Operators2 {
			OwcaValue (*add)(Executor &e, OwcaValue left, OwcaValue right);
			OwcaValue (*sub)(Executor &e, OwcaValue left, OwcaValue right);
			OwcaValue (*mul)(Executor &e, OwcaValue left, OwcaValue right);
			OwcaValue (*div)(Executor &e, OwcaValue left, OwcaValue right);
			OwcaValue (*mod)(Executor &e, OwcaValue left, OwcaValue right);
			OwcaValue (*bin_and)(Executor &e, OwcaValue left, OwcaValue right);
			OwcaValue (*bin_or)(Executor &e, OwcaValue left, OwcaValue right);
			OwcaValue (*bin_xor)(Executor &e, OwcaValue left, OwcaValue right);
			OwcaValue (*bin_lshift)(Executor &e, OwcaValue left, OwcaValue right);
			OwcaValue (*bin_rshift)(Executor &e, OwcaValue left, OwcaValue right);
			bool (*less)(Executor &e, OwcaValue left, OwcaValue right);
			bool (*eq)(Executor &e, OwcaValue left, OwcaValue right);
			bool (*is)(Executor &e, OwcaValue left, OwcaValue right);

			Operators2();
            Operators2(AllocationBase::Kind);
		};
    }
}

#endif