#include "stdafx.h"
#include "base.h"
#include "debug_memory_blocks.h"
#include "tree_base.h"

namespace owca { namespace __owca__ {
#ifdef RCDEBUG_MEMORY_BLOCKS
	static unsigned int memory_id=0;
	static std::set<unsigned int> st;
#endif
	tree_base::tree_base(compiler *cmp_, unsigned int l) : cmp(cmp_),location(l),size(0)
#ifdef RCDEBUG_MEMORY_BLOCKS
		,_memory_id(++memory_id)
#endif
	{
#ifdef RCDEBUG_MEMORY_BLOCKS
		RCASSERT(st.find(_memory_id)==st.end());
		st.insert(_memory_id);
#endif
	}

	tree_base::~tree_base()
	{
#ifdef RCDEBUG_MEMORY_BLOCKS
		RCASSERT(st.find(_memory_id)!=st.end());
		st.erase(_memory_id);
#endif
	}

#ifdef RCDEBUG_MEMORY_BLOCKS
	void tree_base::check_blocks(void)
	{
		RCASSERT(st.empty());
	}
#endif

} }
