#ifndef _RC_Y_EXEC_PROPERTY_H
#define _RC_Y_EXEC_PROPERTY_H

#include "exec_base.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		class exec_base;
		class exec_function_ptr;
		class exec_property;
		class virtual_machine;
	}
}

namespace owca { namespace __owca__ {
	class exec_property : public exec_base {
		friend class virtual_machine;
		exec_property() { read=write=NULL; }
	public:
		exec_function_ptr *read,*write;
	protected:
		~exec_property() { }
		void _mark_gc(const gc_iteration &gc) const;
		void _release_resources(virtual_machine &vm);
		std::string str() const;
	public:
		void gc_acquire() { _gc_acquire(); }
		void gc_release(virtual_machine &vm) { _gc_release(vm); }
		void gc_mark(const gc_iteration &gi) const { _gc_mark(gi); }
	};
} }
#endif
