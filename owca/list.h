#include "obj_constructor_base.h"

#ifndef _RC_Y_LIST_H
#define _RC_Y_LIST_H

namespace owca {
	class owca_global;
	class owca_local;
	class owca_list;
	namespace __owca__ {
		class virtual_machine;
		class exec_array_object;
		class exec_object;
		class obj_constructor_base;
	}
}

namespace owca {

	class owca_list {
		friend class owca_local;
		friend class __owca__::obj_constructor_base;
		__owca__::exec_array_object *ao;
		__owca__::exec_object *obj;
		__owca__::virtual_machine *vm;

		DLLEXPORT void _set(owca_int index, const __owca__::exec_variable &value);
		DLLEXPORT void _get(owca_int index, __owca__::exec_variable &value) const;
		DLLEXPORT unsigned int _update_index(owca_int index, bool allow_one_index_after_size) const;
	public:
		owca_list() : vm(NULL),obj(NULL),ao(NULL) { }
		bool not_bound() const { return ao==NULL; }

		bool empty(void) const { return size() == 0; }
		DLLEXPORT unsigned int size() const;
		DLLEXPORT void resize(unsigned int);
		DLLEXPORT void clear();

		DLLEXPORT void push_front(const owca_global &);
		DLLEXPORT void push_back(const owca_global &);
		DLLEXPORT owca_global pop_front();
		DLLEXPORT owca_global pop_back();
		DLLEXPORT void insert(owca_int index, const owca_global &);
		DLLEXPORT owca_global pop(owca_int index);
		DLLEXPORT owca_global clone() const;

		DLLEXPORT void set(owca_int index, const owca_global &value);
		DLLEXPORT owca_global get(owca_int index) const;
		DLLEXPORT owca_global get(owca_int from, owca_int to) const;

		class obj_constructor : public __owca__::obj_constructor_base {
			friend class owca_list;
			owca_list *ls;
			owca_int index;
			obj_constructor(owca_list *ls_, owca_int index_) : obj_constructor_base(*ls_->vm,NULL),ls(ls_),index(index_) { }
			DLLEXPORT void _write(const __owca__::exec_variable &);
			DLLEXPORT void _read(__owca__::exec_variable &val) const;
			DLLEXPORT void operator = (const obj_constructor &);
		public:
			obj_constructor(obj_constructor &&) = default;
			using __owca__::obj_constructor_base::operator =;
			~obj_constructor() {
				if (!used) read();
			}
		};
		DLLEXPORT obj_constructor operator [] (owca_int index);
	};
}

#endif
