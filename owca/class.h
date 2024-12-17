#ifndef _RC_Y_CLASS_H
#define _RC_Y_CLASS_H

#include "obj_constructor_base.h"
#include "structinfo.h"
#include "global.h"

namespace owca {
	class owca_class;
	class owca_global;
	class owca_vm;
	class gc_iteration;
	class owca_namespace;
	namespace __owca__ {
		class exec_variable;
		class internal_class;
		class virtual_machine;
		class obj_constructor_base;
	}
}

namespace owca {
	namespace __owca__ {
		class object_base_user {
		protected:
			virtual ~object_base_user() { }
		public:
			void destroy(owca_vm &) { }
		};
	}
	template <class Z> class owca_object_base : public __owca__::object_base_user {
	private:
		template <typename A> friend struct __owca__::structinfo::T;
		template <typename A> struct __aliaser__ {
			typedef __owca__::structinfo::T2<A> T;
		};
	};

	class owca_class {
		__owca__::internal_class *ic;

		DLLEXPORT void _set_struct(__owca__::structinfo type);
	public:
		DLLEXPORT owca_class(owca_namespace &nspace, const std::string &name);
		DLLEXPORT ~owca_class();

		class obj_constructor : public __owca__::obj_constructor_function {
			friend class owca_class;
			friend class __owca__::internal_class;
			__owca__::internal_class &ic;
			owca_global g;
			DLLEXPORT void _write(const __owca__::exec_variable &);
			DLLEXPORT void _read(__owca__::exec_variable &val) const;
			DLLEXPORT obj_constructor(__owca__::internal_class &ic_, const owca_string &ident_);
		public:
			obj_constructor(obj_constructor &&) = default;
			using __owca__::obj_constructor_function::operator =;
		};
		DLLEXPORT obj_constructor operator [] (const owca_string &ident);
		template <class A> void set_struct() {
			_set_struct(__owca__::structinfo::make<A>());
		}
		DLLEXPORT owca_global construct();
		DLLEXPORT void add_inheritance(const owca_global &g);
	};
}

#endif
