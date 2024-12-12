#ifndef _RC_Y_BUILD_EXPRESSION_H
#define _RC_Y_BUILD_EXPRESSION_H

#include "cmp_base.h"

namespace owca {
	namespace __owca__ {
		class source;
		class compiler;
		class tree_expression;
	}
}

namespace owca { namespace __owca__ {
	tree_expression *bt_oper_1(compiler *cmp, source &s);
	tree_expression *build_tree_expression(compiler *cmp, source &s);
	tree_expression *build_tree_assignment(compiler *cmp, source &s, unsigned int mode=0);
} }
#endif
