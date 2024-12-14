#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_stack.h"
#include "exec_function_stack_data.h"
#include "exec_array_object.h"
#include "exec_tuple_object.h"
#include "exec_map_object.h"
#include "exec_object.h"
#include "exec_string.h"
#include "exec_string_compare.h"
#include "namespace.h"
#include "vm.h"
#include "op_base.h"
#include "message.h"
#include "exception.h"
#include "op_validate.h"
#include "op_execute.h"
#include "returnvalue.h"
#include "exec_function_ptr.h"
#include "exec_class_object.h"
#include "operatorcodes.h"
#include "vm_execution_stack_elem_base.h"
#include "exec_namespace.h"

namespace owca {
	using namespace __owca__;

	namespace __owca__ {
		bool debuggc=true;
	}

	owca_vm::owca_vm()
	{
		vm=new virtual_machine(this);
	}

	owca_vm::~owca_vm()
	{
		delete vm;
	}

	std::vector<unsigned char> owca_vm::compile_impl(owca_message_list &errorswarnings, const owca_source_file &fs, const __owca__::compile_visible_items &cc)
	{
		return vm->compile(errorswarnings,fs,cc);
	}

	__owca__::exec_object *owca_vm::allocate_array(__owca__::exec_array_object *&oo) const
	{
		return vm->allocate_array(oo);
	}

	__owca__::exec_object *owca_vm::allocate_tuple(__owca__::exec_tuple_object *&oo, unsigned int size) const
	{
		return vm->allocate_tuple(oo,size);
	}

	void owca_vm::set_print_function(void (*printfnc)(const std::string &))
	{
		vm->set_print_function(printfnc);
	}

	owca_global owca_vm::create_namespace(const std::string &file_name) const
	{
		owca_global z(*vm);
        owca_internal_string *name = vm->allocate_string(file_name);
		z._object.set_namespace(vm->allocate_namespace(name));
        name->gc_release(*vm);
		return z;
	}

	owca_global owca_vm::construct_builtin_exception(ExceptionCode code, const std::string &txt)
	{
		owca_global z(*this);
		z._object.set_object(vm->_raise_get_exception_type(code));
		z._object.gc_acquire();
		RCASSERT(z.call(z,txt,(int)code)==owca_function_return_value::RETURN_VALUE);
		return z;
	}

	class compile_1 : public compile_visible_items {
	public:
		hash_map<exec_namespace::key,exec_namespace::value> &hashmap;
		mutable hash_map_iterator it;

		compile_1(hash_map<exec_namespace::key,exec_namespace::value> &hashmap_) : hashmap(hashmap_) { }

		const char *get(unsigned int &sz) const {
			if (hashmap.next(it)) {
				owca_internal_string *s=hashmap.getkey(it).k;
				const char *b=s->data_pointer();
				sz=s->data_size();
				return b;
			}
			return NULL;
		}
	};

	owca_function_return_value owca_vm::compile(owca_global &result, owca_namespace &nspace, owca_message_list &errorswarnings, const owca_source_file &fs)
	{
		compile_1 cc = compile_1(nspace.ns->hashindex);
		std::vector<unsigned char> tmp = vm->compile(errorswarnings,fs,cc);
		owca_function_return_value rr = owca_function_return_value::RETURN_VALUE;

		if (!errorswarnings.has_errors())
			rr=nspace.apply_code(result,tmp);

		return rr;
	}

	class compile_2 : public compile_visible_items {
		const char *get(unsigned int &sz) const { return NULL; }
	};

	std::vector<unsigned char> owca_vm::compile(owca_message_list &errorswarnings, const owca_source_file &fs)
	{
		compile_2 cc;
		return vm->compile(errorswarnings,fs,cc);
	}

	owca_function_return_value owca_vm::resume_execution()
	{
		if (vm->execution_stack) {
			executionreturnvalue r=vm->execute_stack();
			switch(r) {
			case VME_EXCEPTION:
			case VME_VALUE:
				RCASSERT(vm->execution_stack->empty());
				vm->pop_execution_stack();
				break;
			default:
				RCASSERT(0);
			}
			GC(vm);
			return owca_function_return_value(r);
		}
		return owca_function_return_value::RETURN_VALUE;
	}

	owca_global owca_vm::executing_function()
	{
		vm_execution_stack_elem_base *frame=vm->execution_stack->peek_frame();
		if (frame->fnc) {
			owca_global g(*vm);
			g._object.set_function_fast(frame->fnc);
			frame->fnc->gc_acquire();
			return g;
		}
		return owca_global::null;
	}

	void owca_vm::run_gc()
	{
		vm->run_gc();
	}

	owca_global owca_vm::string(const char *c) const
	{
		const char *e=c;
		while(*e) ++e;
		return string(c,(unsigned int)(e-c));
	}

	owca_global owca_vm::string(const char *txt, unsigned int len) const
	{
		owca_global ret(vm);
		unsigned int index = 0;
		unsigned int char_count = owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, txt, len);
		if (index != 0)
			throw owca_exception(ExceptionCode::INVALID_UTF8_STRING, "invalid utf8 string");
		ret._object.set_string(vm->allocate_string(txt, len, char_count));
		return ret;
	}

	void owca_vm::_set(exec_variable &dst, const owca_local &l) const
	{
		switch(l._object.mode()) {
		case VAR_STRING:
			if (l._vm!=vm) {
				dst.set_string(vm->allocate_string(l._object.get_string()->data_pointer(), l._object.get_string()->data_size(), l._object.get_string()->character_count()));
				break;
			}
		case VAR_NULL:
		case VAR_INT:
		case VAR_REAL:
		case VAR_BOOL:
_set:
			(dst=l._object).gc_acquire();
			break;
		case VAR_GENERATOR:
		case VAR_PROPERTY:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_OBJECT:
		case VAR_WEAK_REF:
		case VAR_NAMESPACE:
			if (l._vm!=vm) throw owca_exception(ExceptionCode::INVALID_VM, "different VMs in use");
			goto _set;
		default:
			RCASSERT(0);
		}
	}

	owca_global owca_vm::map(void) const
	{
		exec_map_object *oo;
		exec_object *o=vm->allocate_map(oo);
		owca_global ret(vm);
		ret._object.set_object(o);
		return ret;
	}

	owca_global owca_vm::string(const std::string &t) const
	{
		return string(t.c_str(),(unsigned int)t.size());
	}

	unsigned int owca_vm::stack_get_depth_count() const
	{
		vm_execution_stack *s=vm->execution_stack;
		unsigned int cnt=0;

		while(s) {
			cnt+=s->count();
			s=s->prev();
		}
		return cnt;
	}

	bool owca_vm::stack_get_element(owca_global &ret, unsigned int depth)
	{
		exec_stack_element_object *se;
		vm_execution_stack *s=vm->execution_stack;

		while(s) {
			if (depth<s->count()) break;
			depth-=s->count();
			s=s->prev();
		}
		if (s==NULL) return false;
		ret._object.gc_release(*ret._vm);
		ret._update_vm(vm);
		ret._object.reset();
		ret._object.set_object(vm->allocate_stack_element(se));
		se->set_element(s->peek_frame_indexed(depth));
		return true;
	}

}

namespace owca {
	namespace __owca__ {
		void debug_check_memory(void)
		{
#if defined WIN32 || defined WIN64
			RCASSERT(_CrtCheckMemory());
#endif
		}
	}
}

