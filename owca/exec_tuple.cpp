#include "stdafx.h"
#include "base.h"
#include "exec_string.h"
#include "virtualmachine.h"
#include "exec_tuple_object.h"
#include "exec_class_int.h"
#include "exec_object.h"
#include "exec_map_object.h"
#include "vm_execution_stack_elem_external.h"
#include "exec_callparams.h"
#include "operatorcodes.h"
#include "hashfunc.h"
#include "exec_compare_arrays.h"

namespace owca { namespace __owca__ {

	D_FUNC1(tuple,new,const exec_variable*)
		{
			switch(mode) {
			CASE(0)
				mode=1;
				generator.reset();

				if (p1->mode()==VAR_NO_PARAM_GIVEN) goto done;
				CALC(10,vm->calculate_generator(&generator,*p1));
			CASE(10)
				tmp.push_back(exec_variable());
				tmp.back().reset();
				vm->prepare_call_function(&tmp.back(),generator,NULL,0);
				mode=11;
				return executionstackreturnvalue::FUNCTION_CALL;
			CASE(11)
				if (tmp.back().is_no_return_value()) {
					tmp.pop_back();
done:
					exec_tuple_object *self;
					exec_object *o=vm->allocate_tuple(self,(unsigned int)tmp.size());
					unsigned int index=0;
					for(std::list<exec_variable>::iterator it=tmp.begin();it!=tmp.end();++it) {
						self->get(index).gc_release(*vm);
						self->get(index++)=*it;
					}
					tmp.clear();
					return_value->set_object(o);
					break;
				}
				GOTO(10);
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN;
		}

		exec_variable generator,tmpvar;
		std::list<exec_variable> tmp;
		unsigned char mode;

		void create_self()
		{
			mode=0;
			generator.reset();
			tmpvar.reset();
		}
		void _mark_gc(const gc_iteration &gc) const
		{
			BASE::_mark_gc(gc);
			{
				gc_iteration::debug_info _d("tuple new: generator variable");
				generator.gc_mark(gc);
			}
			{
				gc_iteration::debug_info _d("tuple new: tmp variable");
				tmpvar.gc_mark(gc);
			}
			unsigned int index=0;
			for(std::list<exec_variable>::const_iterator it=tmp.begin();it!=tmp.end();++it,++index) {
				gc_iteration::debug_info _d("tuple new: tmp variable %d",index);
				it->gc_mark(gc);
			}
		}

		void _release_resources(virtual_machine &vm)
		{
			BASE::_release_resources(vm);
			generator.gc_release(vm);
			tmpvar.gc_release(vm);
			for(std::list<exec_variable>::iterator it=tmp.begin();it!=tmp.end();++it) {
				it->gc_release(vm);
			}
			tmp.clear();
		}
	D_END

	D_SELF1(tuple,in,exec_tuple_object*,const exec_variable *)
		{
			switch(mode) {
			CASE(0)
				if (index>=self->size()) {
					return_value->set_bool(false);
					break;
				}
				exec_variable params[2];
				params[0]=self->get(index);
				params[1]=*p1;
				CALC(1,vm->calculate_eq(&tmp,params));
			CASE(1)
				if (tmp.get_bool()) {
					return_value->set_bool(true);
					break;
				}
				++index;
				GOTO(0);
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN;
		}
		unsigned int index;
		exec_variable tmp;
		unsigned char mode;
		void create_self(void) { mode=0; index=0; }
	D_END

	D_SELF0(tuple,str,exec_tuple_object*)
		{
			switch(mode) {
			CASE(0)
				if (self->size()==0) {
					return_value->set_string(vm->allocate_string("()"));
					return executionstackreturnvalue::RETURN;
				}
				index=0;
				sb.add("( ");
				GOTO(1);
			CASE(1)
				if (index<(unsigned int)self->size()) {
					CALC(2,vm->calculate_str(&tmp,self->get(index)));
				}
				sb.add(" )");
				return_value->set_string(sb.to_string(*vm));
				return executionstackreturnvalue::RETURN;
			CASE(2)
				sb.add(tmp.get_string());
				tmp.gc_release(*vm);
				tmp.reset();
				++index;
				if (index<self->size()) sb.add(", ");
				GOTO(1);
			}
			RCASSERT(0);
			return_value->set_null(true);
			return executionstackreturnvalue::RETURN;
		}
		stringbuffer sb;
		unsigned int index;
		exec_variable tmp;
		unsigned char mode;

		void create_self()
		{
			mode=0;
			tmp.reset();
		}
		D_GCMARK
			tmp.gc_mark(gc);
		D_END
		D_RELRES
			tmp.gc_release(vm);
		D_END
	D_END

	D_SELF0(tuple,size,exec_tuple_object*)
		{
			return_value->set_int(self->size());
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(tuple,bool,exec_tuple_object*)
		{
			return_value->set_bool(self->size()>0);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	class tuple_cmp_object_base : public vm_execution_stack_elem_external_self_t1<exec_tuple_object*,exec_tuple_object*> {
	public:
		enum { FUNCTION_OBJECT_PARAMS_1,FUNCTION_OBJECT_MODE_s1 };
		typedef vm_execution_stack_elem_external_self_t1<exec_tuple_object*,exec_tuple_object*> BASE;
		typedef tuple_cmp_object_base OBJECTTYPE;
		operatorcodes oper;
		unsigned char mode;

		tuple_cmp_object_base() : mode(0) { create_self(); }
		RCLMFUNCTION executionstackreturnvalue execute(executionstackreturnvalue r)
		{
			RCASSERT(r.type()==executionstackreturnvalue::OK);
			if (mode==0) {
				evc.vm=vm;
				evc.a1=self->ptr();
				evc.s1=self->size();
				evc.a2=p1->ptr();
				evc.s2=p1->size();
				evc.result=exec_compare_arrays::START;
				evc.index=0;
				mode=1;
			}
			evc.next(oper!=E_EQ && oper!=E_NOTEQ);
			switch(evc.result) {
			case exec_compare_arrays::CALL_1:
			case exec_compare_arrays::CALL_2:
				return executionstackreturnvalue::FUNCTION_CALL;
			case exec_compare_arrays::EQ:
				switch(oper) {
				case E_EQ: return_value->set_bool(true); break;
				case E_NOTEQ: return_value->set_bool(false); break;
				case E_LESSEQ: return_value->set_bool(true); break;
				case E_MOREEQ: return_value->set_bool(true); break;
				case E_LESS: return_value->set_bool(false); break;
				case E_MORE: return_value->set_bool(false); break;
				default:
					RCASSERT(0);
				}
				break;
			case exec_compare_arrays::LESS:
				switch(oper) {
				case E_EQ: return_value->set_bool(false); break;
				case E_NOTEQ: return_value->set_bool(true); break;
				case E_LESSEQ: return_value->set_bool(true); break;
				case E_MOREEQ: return_value->set_bool(false); break;
				case E_LESS: return_value->set_bool(true); break;
				case E_MORE: return_value->set_bool(false); break;
				default:
					RCASSERT(0);
				}
				break;
			case exec_compare_arrays::MORE:
				switch(oper) {
				case E_EQ: return_value->set_bool(false); break;
				case E_NOTEQ: return_value->set_bool(true); break;
				case E_LESSEQ: return_value->set_bool(false); break;
				case E_MOREEQ: return_value->set_bool(true); break;
				case E_LESS: return_value->set_bool(false); break;
				case E_MORE: return_value->set_bool(true); break;
				default:
					RCASSERT(0);
				}
				break;
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN;
		}
		exec_compare_arrays evc;
		void _mark_gc(const gc_iteration &gc) const
		{
			BASE::_mark_gc(gc);
			gc_iteration::debug_info _d("tuple_cmp_object_base: evc variable");
			evc._mark_gc(gc);
		}

		void _release_resources(virtual_machine &vm)
		{
			BASE::_release_resources(vm);
			evc._release_resources(vm);
		}
	};

	template <int OPER> struct tuple_cmp_object : public tuple_cmp_object_base {
		tuple_cmp_object() { oper=(operatorcodes)OPER; }
	};

	D_SELF1(tuple,add,exec_tuple_object *,exec_tuple_object *)
		{
			exec_tuple_object *dst;
			exec_object *o=vm->allocate_tuple(dst,self->size()+p1->size());

			for(unsigned int i=0;i<self->size();++i) {
				dst->get(i)=self->get(i);
				dst->get(i).gc_acquire();
			}
			for(unsigned int i=0;i<p1->size();++i) {
				dst->get(self->size()+i)=p1->get(i);
				dst->get(self->size()+i).gc_acquire();
			}
			return_value->set_object(o);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(tuple,mul,exec_tuple_object *,owca_int)
		{
			if (p1<0) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant multiply by negative amount"));
				return executionstackreturnvalue::FUNCTION_CALL;
			}

			exec_tuple_object *dst;
			exec_object *o=vm->allocate_tuple(dst,(unsigned int)p1*self->size());

			for(owca_int j=0;j<p1;++j) {
				for(unsigned int k=0;k<self->size();++k) {
					dst->get((unsigned int)(j*self->size()+k))=self->get(k);
					dst->get((unsigned int)(j*self->size()+k)).gc_acquire();
				}
			}

			return_value->set_object(o);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(tuple,read_1,exec_tuple_object*,owca_int)
		{
			if (p1<-(owca_int)self->size() || p1>=self->size()) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT2("index %1 is invalid for tuple of size %2",int_to_string(p1),int_to_string((owca_int)self->size())));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			if (p1<0) p1+=(owca_int)self->size();
			*return_value=self->get((unsigned int)p1);
			return_value->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	void update_2index(owca_int &i1, owca_int &i2, owca_int size);

	RCLMFUNCTION void tuple_read_2_i(exec_tuple_object *a, virtual_machine &vm, exec_variable &ret, owca_int i1, owca_int i2)
	{
		update_2index(i1,i2,(owca_int)a->size());

		exec_tuple_object *r;
		exec_object *o=vm.allocate_tuple(r,(unsigned int)(i2-i1));
		ret.set_object(o);

		for(owca_int i=0;i<i2-i1;++i) {
			r->get((unsigned int)i)=a->get((unsigned int)(i1+i));
			r->get((unsigned int)i).gc_acquire();
		}
	}

	D_SELF2(tuple,read_2,exec_tuple_object*,const exec_variable *, const exec_variable *)
		{
			owca_int i1,i2;
			if (p1->get_int_min(i1) && p2->get_int_max(i2)) {
				tuple_read_2_i(self,*vm,*return_value,i1,i2);
				return executionstackreturnvalue::RETURN;
			}
			return_value->set_null(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(tuple,hash,exec_tuple_object *)
		{
			switch(mode) {
			CASE(0)
				index=0;
				GOTO(1);
			CASE(1)
				if (index<self->size()) {
					CALC(2,vm->calculate_hash(&tmp,self->get(index)));
				}
				return_value->set_int(hf.value() ^ reinterpret_cast<owca_int>(vm->class_tuple));
				return executionstackreturnvalue::RETURN;
			CASE(2) {
				owca_int v=tmp.get_int();
				hf.process(&v,sizeof(owca_int));
				++index; }
				GOTO(1);
			default:
				RCASSERT(0);
				return_value->set_null(true);
				return executionstackreturnvalue::RETURN;
			}
		}

		hashfunc hf;
		unsigned int index;
		exec_variable tmp;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF0(tuple,gen,exec_tuple_object*)
		{
			if (mode==0) {
				mode=1;
				index=0;
				return executionstackreturnvalue::CREATE_GENERATOR;
			}
			if (index<self->size()) {
				*return_value=self->get(index++);
				return_value->gc_acquire();
				return executionstackreturnvalue::RETURN;
			}
			return_value->set_null(true);
			return executionstackreturnvalue::RETURN;
		}
		unsigned int index;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	void virtual_machine::initialize_tuple(internal_class &c)
	{
		c._setinheritable(false);
		c._setstructure<exec_tuple_object>();

		M_OPER1(c,tuple,new,(*this,"$new","generator",dv_no_param_given));
		M_OPER0(c,tuple,str,(*this,"$str"));
		M_OPER0(c,tuple,bool,(*this,"$bool"));
		M_OPER0(c,tuple,hash,(*this,"$hash"));
		M_OPER0(c,tuple,gen,(*this,"$gen"));
		M_OPER1(c,tuple,read_1,(*this,"$read_1","index"));
		M_OPER2(c,tuple,read_2,(*this,"$read_2","from","to"));

		M_OPER1(c,tuple,in,(*this,"$in","value"));

		M_OPER1(c,tuple,add,(*this,"$add","other"));
		M_OPER1(c,tuple,mul,(*this,"$mul","other"));
		M_OPER_A(c,rmul,tuple,mul,(*this,"$rmul","other"));

		c["$eq"].set(exec_function_ptr::generate_function_1<tuple_cmp_object<E_EQ> >(*this,"$eq","other"));
		c["$noteq"].set(exec_function_ptr::generate_function_1<tuple_cmp_object<E_NOTEQ> >(*this,"$noteq","other"));
		c["$lesseq"].set(exec_function_ptr::generate_function_1<tuple_cmp_object<E_LESSEQ> >(*this,"$lesseq","other"));
		c["$moreeq"].set(exec_function_ptr::generate_function_1<tuple_cmp_object<E_MOREEQ> >(*this,"$moreeq","other"));
		c["$less"].set(exec_function_ptr::generate_function_1<tuple_cmp_object<E_LESS> >(*this,"$less","other"));
		c["$more"].set(exec_function_ptr::generate_function_1<tuple_cmp_object<E_MORE> >(*this,"$more","other"));

		M_FUNC0(c,tuple,size,(*this,"size"));
	}

} }












