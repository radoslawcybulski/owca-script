#ifndef _RC_Y_EXEC_CLASS_INT_H
#define _RC_Y_EXEC_CLASS_INT_H

#include "class.h"
#include "structinfo.h"

namespace owca {
	class owca_class;
	class gc_iteration;
	namespace __owca__ {
		struct structinfo;
		class compiler;
		class exec_class_object;
		class exec_object;
		class exec_variable;
		class internal_class;
		class virtual_machine;
		class exec_namespace;
	}
}

namespace owca {
	namespace __owca__ {
		class internal_class {
			friend class compiler;
			friend class virtual_machine;
			friend class exec_class_object;
			friend class owca_class::obj_constructor;
			internal_class *next,*prev;
			std::map<std::string,exec_variable> mp;
			std::list<exec_object *> _inherited;
			std::string _name;
			bool _inheritable,_constructable;
			structinfo sinfo;
		public:
			static bool _check_name(const char *txt, unsigned int size);
			void _add_inherit(exec_object *o);

			class obj_constructor : public __owca__::obj_constructor_base {
				friend class owca_class;
				friend class __owca__::internal_class;
				__owca__::internal_class &ic;
				owca_global g;
				void _write(const __owca__::exec_variable &);
				void _read(__owca__::exec_variable &val) const;
				obj_constructor(__owca__::internal_class &ic_, const owca_string &ident_);
				//void operator = (const obj_constructor &);
			public:
				obj_constructor(obj_constructor &&) = default;
				using __owca__::obj_constructor_base::operator =;
				~obj_constructor() {
					if (!used) read();
				}
			};
			obj_constructor operator [] (const owca_string &s);

			void _setstructure(structinfo type) {
				sinfo=type;
			}
			template <class A> void _setstructure() {
				sinfo=structinfo::make<A>();
			}
			void _setstoragesize(unsigned int bytes) { sinfo.storagespace=bytes; }
			void _setconstructor(void (*fnc)(void *, unsigned int oversize, virtual_machine &vm)) { sinfo.constructor=fnc; }
			void _setdestructor(void (*fnc)(void *, virtual_machine &vm)) { sinfo.destructor=fnc; }
			void _setmarker(void (*fnc)(const void *, const gc_iteration &gc)) { sinfo.marker=fnc; }
			void _setinheritable(bool b) { _inheritable=b; }
			void _setconstructable(bool b) { _constructable=b; }

			const std::string &name() const { return _name; }
			std::string create(exec_object *&o);

			virtual_machine &vm;
			exec_namespace *nspace;

			internal_class(virtual_machine &vm_, const std::string &name_);
			internal_class(exec_namespace &nspace_, const std::string &name_);
			~internal_class();
			void gc_mark(gc_iteration &g);
		};
	}
}
#endif
