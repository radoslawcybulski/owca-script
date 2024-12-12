#ifndef _RC_Y_VM_H
#define _RC_Y_VM_H

#include "global.h"
#include "exec_array_object.h"
#include "exec_tuple_object.h"
#include "compile_visible_items.h"

namespace owca {
	class owca_global;
	class owca_local;
	class owca_message_list;
	class owca_source_file;
	class owca_vm;
	class owca_class;
	class owca_string_buffer;
	class owca_namespace;
	enum exceptioncode;
	class owca_user_function_base_object;

	namespace __owca__ {
		class exec_variable;
		class exec_object;
		class exec_array_object;
		class exec_tuple_object;
		class exec_namespace;
		class virtual_machine;
		class obj_constructor_base;
		template <class A, class B, bool RETVAL> class user_function_base;
		template <class A, class B> class user_function_base_simple;
	}
}

namespace owca {
	class owca_vm {
		friend class __owca__::virtual_machine;
		friend class owca_global;
		friend class owca_local;
		friend class owca_class;
		friend class owca_string_buffer;
		friend class __owca__::obj_constructor_base;
		template <class A, class B, bool RETVAL> friend class __owca__::user_function_base;
		template <class A, class B> friend class __owca__::user_function_base_simple;
		friend class owca_user_function_base_object;

		__owca__::virtual_machine *vm;
		DLLEXPORT owca_vm(const owca_vm &);
		DLLEXPORT void operator = (const owca_vm &);
		DLLEXPORT void _set(__owca__::exec_variable &dst, const owca_local &l) const;

		DLLEXPORT std::vector<unsigned char> compile_impl(owca_message_list &errorswarnings, const owca_source_file &fs, const __owca__::compile_visible_items &);
		DLLEXPORT __owca__::exec_object *allocate_array(__owca__::exec_array_object *&oo) const;
		DLLEXPORT __owca__::exec_object *allocate_tuple(__owca__::exec_tuple_object *&oo, unsigned int size) const;
	public:
		DLLEXPORT owca_vm();
		DLLEXPORT virtual ~owca_vm();

		DLLEXPORT void set_print_function(void (*printfnc)(const std::string &));
		DLLEXPORT owca_global construct_builtin_exception(exceptioncode code, const std::string &txt);
		DLLEXPORT owca_function_return_value compile(owca_global &execresult, owca_namespace &result, owca_message_list &errorswarnings, const owca_source_file &fs);
		DLLEXPORT std::vector<unsigned char> compile(owca_message_list &errorswarnings, const owca_source_file &fs);

		template <class A> std::vector<unsigned char> compile(owca_message_list &errorswarnings, const owca_source_file &fs, A itbegin, A itend)
		{
			class cmpidentiterclass : public __owca__::compile_visible_items {
				mutable A current;
				A end;
			public:
				cmpidentiterclass(A itbegin, A itend) : current(itbegin),end(itend) { }
				const char *get(unsigned int &sz) const override {
					if (current!=end) {
						const char *z=*current;
						++current;
						return z;
					}
					return NULL;
				}
			};
			return compile_impl(errorswarnings,fs,cmpidentiterclass(itbegin,itend));
		}
		DLLEXPORT void run_gc();
		DLLEXPORT owca_function_return_value resume_execution(void);

		DLLEXPORT unsigned int stack_get_depth_count() const;
		DLLEXPORT bool stack_get_element(owca_global &ret, unsigned int depth);

		DLLEXPORT owca_global string(const std::string &t) const;
		DLLEXPORT owca_global string(const char *) const;
		DLLEXPORT owca_global string(const char *, unsigned int len) const;
		template <class A> owca_global list(A begin, A end) const {
			size_t size=std::distance(begin,end);
			__owca__::exec_array_object *oo;
			__owca__::exec_object *o=allocate_array(oo);
			oo->resize(*vm,(unsigned int)size);
			owca_global ret(vm);
			ret._object.set_object(o);
			for(unsigned int i=0;i<size;++i,++begin)
                _set(oo->get(i),*begin);
			return ret;
		}
		owca_global list() const { return list((owca_global*)0,(owca_global*)0); }
		template <class A> owca_global tuple(A begin, A end) const {
			size_t size=std::distance(begin,end);
			__owca__::exec_tuple_object *oo;
			__owca__::exec_object *o=allocate_tuple(oo,(unsigned int)size);
			owca_global ret(vm);
			ret._object.set_object(o);
			for(unsigned int i=0;i<size;++i,++begin) _set(oo->get(i),*begin);
			return ret;
		}
		owca_global tuple() const { return tuple((owca_global*)0,(owca_global*)0); }
		DLLEXPORT owca_global map() const;

		DLLEXPORT owca_global executing_function();
		DLLEXPORT owca_global create_namespace(const std::string &file_name) const;
	};
}
#endif
