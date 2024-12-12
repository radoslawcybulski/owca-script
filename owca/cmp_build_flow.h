#ifndef _RC_Y_CMP_BUILD_FLOW_H
#define _RC_Y_CMP_BUILD_FLOW_H

namespace owca {
	namespace __owca__ {
		class source;
		class compiler;
		class tree_class;
		class tree_flow;
		class tree_function;
		class compile_visible_items;
	}
}

namespace owca {
	namespace __owca__ {
		tree_flow *build_tree_line(compiler *cmp, source &s);
		tree_flow *build_tree_statement(compiler *cmp, source &s);
		tree_flow *build_tree_for(compiler *cmp, source &s, const std::string &name);
		tree_flow *build_tree_while(compiler *cmp, source &s, const std::string &name);
		tree_flow *build_tree_if(compiler *cmp, source &s);
		tree_flow *build_tree_try(compiler *cmp, source &s);
		tree_function *build_tree_function(compiler *cmp, source &s);
		tree_class *build_tree_class(compiler *cmp, source &s);
		tree_flow *build_tree_return_yield_raise(compiler *cmp, source &s);
		tree_flow *build_tree_loop_control(compiler *cmp, source &s);
		tree_flow *build_tree_subblock(compiler *cmp, source &s);
		tree_function *build_tree(compiler *cmp, const compile_visible_items &visible_names);
	}
}
#endif
