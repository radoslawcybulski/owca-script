#ifndef _RC_Y_NAMESPACE_H
#define _RC_Y_NAMESPACE_H

#include "global.h"
#include "obj_constructor_base.h"

namespace owca {
	class owca_global;
	class owca_local;
	class owca_class;
	class owca_namespace;
	struct owca_call_parameters;
	class owca_vm;
	namespace __owca__ {
		class exec_namespace;
	}
}

namespace owca {
	class owca_namespace {
		friend class owca_local;
		friend class owca_class;
		friend class owca_vm;
		friend __owca__::exec_namespace;
		__owca__::exec_namespace *ns;
	public:
		owca_namespace() : ns(NULL) { }

		class obj_constructor : public __owca__::obj_constructor_function {
			friend class owca_namespace;
			owca_global g;
			DLLEXPORT void _write(const __owca__::exec_variable &);
			DLLEXPORT void _read(__owca__::exec_variable &val) const;
			DLLEXPORT obj_constructor(owca_namespace &ns_, __owca__::owca_internal_string *name_);
			DLLEXPORT void operator = (const obj_constructor &);
		public:
			obj_constructor(obj_constructor &&) = default;

			using __owca__::obj_constructor_function::operator =;
		};

		friend class obj_constructor;
		DLLEXPORT obj_constructor operator [] (const owca_string &);

		bool not_bound() const { return ns==NULL; }
		DLLEXPORT void clear();
        DLLEXPORT owca_global copy(std::string file_name);
		DLLEXPORT owca_function_return_value apply_code(owca_global &result, const std::vector<unsigned char> &);
		DLLEXPORT bool validate_code(const std::vector<unsigned char> &) const;
		DLLEXPORT owca_vm &vm();
        DLLEXPORT std::string get_file_name() const;
	};
}

#endif
