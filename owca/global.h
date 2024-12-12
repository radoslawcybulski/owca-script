#include "local.h"

#ifndef _RC_Y_GLOBAL_H
#define _RC_Y_GLOBAL_H

namespace owca {
	class owca_global;
	class owca_local;
	class owca_parameters;
	class owca_vm;
	class owca_list;
	class owca_map;
	class owca_tuple;
	class owca_namespace;
	class owca_string_buffer;
	namespace __owca__ {
		class exec_variable;
		class virtual_machine;
		class local_obj_constructor;
	}
	class owca_user_function_base_object;
}

namespace owca {
	class owca_global : public owca_local {
		friend class owca_user_function_base_object;
		friend class __owca__::virtual_machine;
		friend class __owca__::local_obj_constructor;
		friend class owca_local;
		friend class owca_parameters;
		friend class owca_namespace;
		friend class owca_vm;
		friend class owca_string_buffer;
		friend class __owca__::obj_constructor_base;
		friend class owca_map;
		friend class owca_list;
		friend class owca_tuple;
		friend class owca_string;
		friend class owca_class;
		DLLEXPORT void _attach();
		DLLEXPORT void _detach();
		owca_global *_next,*_prev;
		DLLEXPORT virtual void _update_vm(__owca__::virtual_machine *vm);
		owca_global(__owca__::virtual_machine &vm_) : owca_local(&vm_) { _attach(); }
		owca_global(__owca__::virtual_machine *vm_) : owca_local(vm_) { if (_vm) _attach(); }
		owca_global(__owca__::virtual_machine &vm_, const __owca__::exec_variable &object_) : owca_local(vm_,object_) { _attach(); }
		owca_global(__owca__::exectype etyp) : owca_local(etyp),_next(NULL),_prev(NULL) { }
	public:
		owca_global() : _next(NULL),_prev(NULL) { }
		DLLEXPORT owca_global(owca_vm &vm);
		owca_global(const owca_local &l) : _next(NULL),_prev(NULL),owca_local(l) { if (_vm) _attach(); }
		owca_global(const owca_global &l) : _next(NULL),_prev(NULL),owca_local(l) { if (_vm) _attach(); }
		~owca_global() { if (_vm) _detach(); }
		owca_global(bool b) : _next(NULL),_prev(NULL),owca_local(b) { if (_vm) _attach(); }
		owca_global(char b) : _next(NULL),_prev(NULL),owca_local(b) { if (_vm) _attach(); }
		owca_global(unsigned int i) : _next(NULL),_prev(NULL),owca_local(i) { if (_vm) _attach(); }
		owca_global(int i) : _next(NULL),_prev(NULL),owca_local(i) { if (_vm) _attach(); }
		owca_global(unsigned long long i) : _next(NULL),_prev(NULL),owca_local(i) { if (_vm) _attach(); }
		owca_global(long long i) : _next(NULL),_prev(NULL),owca_local(i) { if (_vm) _attach(); }
		owca_global(double r) : _next(NULL),_prev(NULL),owca_local(r) { if (_vm) _attach(); }
		owca_global(long double r) : _next(NULL),_prev(NULL),owca_local(r) { if (_vm) _attach(); }
		owca_global(const char *s) : _next(NULL),_prev(NULL),owca_local(s) { if (_vm) _attach(); }
		owca_global(const owca_string &s) : _next(NULL),_prev(NULL),owca_local(s) { if (_vm) _attach(); }
		owca_global(const std::string &s) : _next(NULL),_prev(NULL),owca_local(s) { if (_vm) _attach(); }
		owca_global(const owca_list &s) : _next(NULL),_prev(NULL),owca_local(s) { if (_vm) _attach(); }
		owca_global(const owca_map &s) : _next(NULL),_prev(NULL),owca_local(s) { if (_vm) _attach(); }
		owca_global(const owca_tuple &s) : _next(NULL),_prev(NULL),owca_local(s) { if (_vm) _attach(); }

#define Z(T) T operator = (T t) { *(owca_local*)this=owca_local(t); return t; }
		Z(bool)
		Z(char)
		Z(unsigned int)
		Z(int)
		Z(unsigned long long)
		Z(long long)
		Z(double)
		Z(long double)
		Z(float)
		Z(const char*)
		Z(const std::string &)
		Z(const owca_string &)
		Z(const owca_list &)
		Z(const owca_map &)
		Z(const owca_tuple &)
#undef Z
		const owca_local &operator = (const owca_global &l) {
			return *(owca_local*)this=l;
		}
		const owca_local &operator = (const owca_local &l) {
			return *(owca_local*)this=l;
		}
	};
}

#endif
