#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_class_int.h"
#include "exec_array_object.h"
#include "op_compare.h"
#include "exec_object.h"
#include "exec_string.h"
#include "exec_map_object.h"
#include "vm_execution_stack_elem_external.h"
#include "exec_callparams.h"
#include "operatorcodes.h"
#include "exec_compare_arrays.h"
#include "exec_sort_array.h"

namespace owca { namespace __owca__ {

	D_SELF1(array,init,exec_array_object*,const exec_variable *)
		{
			switch(mode) {
			CASE(0)
				mode=1;
				generator.reset();

				if (p1->mode()==VAR_NO_PARAM_GIVEN) {
					self->resize(*vm,0);
					return executionstackreturnvalue::RETURN_NO_VALUE;
				}
				owca_int i;
				switch(p1->mode()) {
				case VAR_INT:
					i=p1->get_int();
					goto init_size;
				case VAR_REAL:
					i=(owca_int)p1->get_real();
					if ((owca_real)i!=p1->get_real()) {
						vm->raise_invalid_param(*p1,vm->class_int);
						return executionstackreturnvalue::FUNCTION_CALL;
					}
			init_size:
					if (i<0) {
						vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant create an array with a negative amount of elements"));
						return executionstackreturnvalue::FUNCTION_CALL;
					}
					self->resize(*vm,(unsigned int)i);
					break;
				default:
					CALC(10,vm->calculate_generator(&generator,*p1));
				}
				break;
			CASE(10)
				tmp.push_back(exec_variable());
				tmp.back().reset();
				vm->prepare_call_function(&tmp.back(),generator,NULL,0);
				mode=11;
				return executionstackreturnvalue::FUNCTION_CALL;
			CASE(11)
				if (tmp.back().is_no_return_value()) {
					tmp.pop_back();
					self->resize(*vm,(unsigned int)tmp.size());
					unsigned int index=0;
					for(std::list<exec_variable>::iterator it=tmp.begin();it!=tmp.end();++it) {
						self->get(index).gc_release(*vm);
						self->get(index++)=*it;
					}
					tmp.clear();
					break;
				}
				GOTO(10);
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}

		exec_variable generator,tmpvar;
		std::list<exec_variable> tmp;
		unsigned char mode;

		void create_self()
		{
			generator.reset();
			tmpvar.reset();
			mode=0;
		}
		void _mark_gc(const gc_iteration &gc) const
		{
			BASE::_mark_gc(gc);
			{
				gc_iteration::debug_info _d("array init: generator variable");
				generator.gc_mark(gc);
			}
			{
				gc_iteration::debug_info _d("array init: tmp variable");
				tmpvar.gc_mark(gc);
			}
			unsigned int index=0;
			for(std::list<exec_variable>::const_iterator it=tmp.begin();it!=tmp.end();++it,++index) {
				gc_iteration::debug_info _d("array init: tmp variable %d",index);
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

	D_SELF0(array,size,exec_array_object*)
		{
			return_value->set_int(self->size());
			return executionstackreturnvalue::RETURN;
		}
	D_END

	unsigned int random();

	D_SELF0(array,shuffle,exec_array_object*)
		{
			for(unsigned int i=self->size()-1;i>0;--i) {
				unsigned int index = random() % i;

				exec_variable v=self->get(i);
				self->get(i)=self->get(index);
				self->get(index)=v;
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
	D_END

	D_SELF0(array,sort,exec_array_object*)
		{
			if (mode==0) {
				mode=1;

				{
					getter g = getter(self->ptr());
					if (sa.create(vm,g,self->size())) return executionstackreturnvalue::RETURN_NO_VALUE;
				}
				varsptr=self->ptr();
			}
			if (varsptr!=self->ptr()) {
				vm->raise_list_modified_while_being_sorted();
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			if (sa.next()) return executionstackreturnvalue::FUNCTION_CALL;

			exec_sort_array_result result=sa.result();
			unsigned int index=0;
			exec_variable *newtable=exec_array_object::_allocate_table(*vm,self->size());

			while(result.valid()) {
				newtable[index++]=result.value();
				result.next();
			}

			newtable=self->_swap_table(newtable);
			exec_array_object::_release_table(*vm,newtable);

			return executionstackreturnvalue::RETURN_NO_VALUE;
		}

		class getter : public exec_sort_array_getter_base {
			const exec_variable *ptr;
			unsigned int index;
		public:
			getter(const exec_variable *ptr_) : ptr(ptr_),index(0) { }

			const exec_variable &next(owca_int &hash)
			{
				hash=0;
				return ptr[index++];
			}
		};

		exec_sort_array sa;
		void *varsptr;
		unsigned char mode;
		void create_self(void) { mode=0; }

		void _mark_gc(const gc_iteration &gc) const
		{
			BASE::_mark_gc(gc);
			gc_iteration::debug_info _d("array sort: sa variable");
			sa._mark_gc(gc);
		}

		void _release_resources(virtual_machine &vm)
		{
			BASE::_release_resources(vm);
			sa._release_resources(vm);
		}
	D_END

	D_SELF1(array,resize,exec_array_object*,owca_int)
		{
			if (p1<0) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant resize to the negative amount"));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			self->resize(*vm,(unsigned int)p1);
			return_value->reset();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	RCLMFUNCTION static void array_add(virtual_machine &vm, exec_array_object *dst, exec_array_object *s1, exec_array_object *s2)
	{
		dst->resize(vm,(unsigned int)(s1->size()+s2->size()));
		for(unsigned int i=0;i<s1->size();++i) {
			dst->get(i)=s1->get(i);
			dst->get(i).gc_acquire();
		}
		for(unsigned int i=0;i<s2->size();++i) {
			dst->get(s1->size()+i)=s2->get(i);
			dst->get(s1->size()+i).gc_acquire();
		}
	}

	D_SELF1(array,in,exec_array_object*,const exec_variable *)
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

	D_SELF1(array,add,exec_array_object*,exec_array_object*)
		{
			exec_array_object *oo;
			exec_object *o=vm->allocate_array(oo);
			array_add(*vm,oo,self,p1);
			return_value->set_object(o);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	//D_SELF1(array,radd,exec_array_object*,exec_array_object*)
	//	{
	//		exec_array_object *oo;
	//		exec_object *o=vm->allocate_array(oo);
	//		array_add(*vm,oo,p1,self);
	//		return_value->set_object(o);
	//		return executionstackreturnvalue::RETURN;
	//	}
	//D_END

	class array_cmp_object_base : public vm_execution_stack_elem_external_self_t1<exec_array_object*,exec_array_object*> {
	public:
		enum { FUNCTION_OBJECT_PARAMS_1,FUNCTION_OBJECT_MODE_s1 };
		typedef vm_execution_stack_elem_external_self_t1<exec_array_object*,exec_array_object*> BASE;
		typedef array_cmp_object_base OBJECTTYPE;
		operatorcodes oper;
		unsigned char mode;

		array_cmp_object_base() : mode(0) { create_self(); }
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
			gc_iteration::debug_info _d("array_cmp_object_base: evc variable");
			evc._mark_gc(gc);
		}

		void _release_resources(virtual_machine &vm)
		{
			BASE::_release_resources(vm);
			evc._release_resources(vm);
		}
	};

	template <int OPER> struct array_cmp_object : public array_cmp_object_base {
		array_cmp_object() { oper=(operatorcodes)OPER; }
	};

	D_SELF1(array,mul,exec_array_object*,owca_int)
		{
			if (p1<0) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant multiply by negative amount"));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			else {
				exec_array_object *dst;
				exec_object *oo=vm->allocate_array(dst);

				dst->resize(*vm,(unsigned int)(p1*self->size()));
				for(owca_int j=0;j<p1;++j) {
					for(unsigned int k=0;k<self->size();++k) {
						dst->get((unsigned int)(j*self->size()+k))=self->get(k);
						dst->get((unsigned int)(j*self->size()+k)).gc_acquire();
					}
				}

				return_value->set_object(oo);
				return executionstackreturnvalue::RETURN;
			}
		}
	D_END
	D_SELF1(array,sadd,exec_array_object*,exec_array_object*)
		{
			if (p1->size()>0) {
				unsigned int ss=self->size();
				self->resize(*vm,ss+(unsigned int)p1->size());
				for(unsigned int i=ss;i<(unsigned int)self->size();++i) {
					(self->get(i)=p1->get(i-ss)).gc_acquire();
				}
			}
			*return_value=v_self;
			v_self.gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END
	D_SELF1(array,smul,exec_array_object*,owca_int)
		{
			if (p1<0) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant multiply by negative amount"));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			if (p1==0) {
				self->resize(*vm,0);
			}
			else if (p1>1) {
				unsigned int ss=self->size();
				self->resize(*vm,ss*(unsigned int)p1);
				for(unsigned int i=ss;i<(unsigned int)self->size();i+=ss) {
					for(unsigned int j=i;j<i+ss;++j) {
						(self->get(j)=self->get(j-i)).gc_acquire();
					}
				}
			}
			*return_value=v_self;
			v_self.gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(array,read_1,exec_array_object*,owca_int)
		{
			if (p1<-(owca_int)self->size() || p1>=(owca_int)self->size()) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT2("index %1 is invalid for array of size %2",int_to_string(p1),int_to_string(self->size())));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			if (p1<0) p1+=self->size();
			*return_value=self->get((unsigned int)p1);
			return_value->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	void update_2index(owca_int &i1, owca_int &i2, owca_int size);

	RCLMFUNCTION exec_object *array_read_2_i(exec_array_object *a, virtual_machine &vm, owca_int i1, owca_int i2)
	{
		exec_object *oo=vm.allocate_object(vm.class_array,0);
		exec_array_object *r=vm.data_from_object<exec_array_object>(oo);

		update_2index(i1,i2,a->size());
		if (i1<i2) {
			r->resize(vm,(unsigned int)(i2-i1));
			for(owca_int i=0;i<i2-i1;++i) {
				r->get((unsigned int)i)=a->get((unsigned int)(i1+i));
				r->get((unsigned int)i).gc_acquire();
			}
		}
		return oo;
	}

	D_SELF2(array,read_2,exec_array_object*,const exec_variable*,const exec_variable*)
		{
			owca_int i1,i2;
			if (p1->get_int_min(i1) && p2->get_int_max(i2)) {
				return_value->set_object(array_read_2_i(self,*vm,i1,i2));
				return executionstackreturnvalue::RETURN;
			}
			else {
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
		}
	D_END

	D_SELF2(array,write_1,exec_array_object*,owca_int,const exec_variable*)
		{
			if (p1<-(owca_int)self->size() || p1>=(owca_int)self->size()) {
				vm->raise_invalid_integer(OWCA_ERROR_FORMAT2("index %1 is invalid for array of size %2",int_to_string(p1),int_to_string(self->size())));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			if (p1<0) p1+=self->size();
			exec_variable &v=self->get((unsigned int)p1);
			v.gc_release(*vm);
			v=*p2;
			v.gc_acquire();
			*return_value=v;
			return_value->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF3(array,write_2,exec_array_object*,const exec_variable*,const exec_variable*,const exec_variable*)
		{
			switch(mode) {
			CASE(0)
				if (p1->get_int_min(i1) && p2->get_int_max(i2)) {
					update_2index(i1,i2,self->size());

					CALC(1,vm->calculate_generator(&generator,*p3));
				}
				else {
					return executionstackreturnvalue::RETURN_NO_VALUE;
				}
			CASE(1)
				if (!tmp.empty() && tmp.back().is_no_return_value()) GOTO(2);
				tmp.push_back(exec_variable());
				tmp.back().reset();

				vm->prepare_call_function(&tmp.back(),generator,NULL,0);
				return executionstackreturnvalue::FUNCTION_CALL;
			CASE(2)
				tmp.pop_back();

				if (i2-i1!=tmp.size()) {
					exec_array_object back(*vm,0);

					self->swap(&back);
					self->resize(*vm,(unsigned int)back.size()+(unsigned int)tmp.size()-(unsigned int)(i2-i1));
					for(owca_int i=0;i<i1;++i) self->get((unsigned int)i)=back.get((unsigned int)i);
					for(owca_int i=0;i<(unsigned int)back.size()-i2;++i) self->get((unsigned int)i1+ (unsigned int)tmp.size()+(unsigned int)i)=back.get((unsigned int)(i2+i));
					for(owca_int i=i1;i<i2;++i) back.get((unsigned int)i).gc_release(*vm);
					back._release_array(*vm);
				}
				else {
					for(owca_int i=i1;i<i2;++i) self->get((unsigned int)i).gc_release(*vm);
				}

				{
					unsigned int index=(unsigned int)i1;
					for(std::list<exec_variable>::iterator it=tmp.begin();it!=tmp.end();++it) self->get(index++)=*it;
				}

				tmp.clear();
				*return_value=*p3;
				return_value->gc_acquire();
				break;
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN;
		}
		owca_int i1,i2;
		exec_variable generator;
		std::list<exec_variable> tmp;
		unsigned char mode;

		void create_self()
		{
			mode=0;
			generator.reset();
		}
		void _mark_gc(const gc_iteration &gc) const
		{
			BASE::_mark_gc(gc);
			{
				gc_iteration::debug_info _d("array write_2: generator variable");
				generator.gc_mark(gc);
			}
			unsigned int index=0;
			for(std::list<exec_variable>::const_iterator it=tmp.begin();it!=tmp.end();++it,++index) {
				gc_iteration::debug_info _d("array write_2: tmp variable %d",index);
				it->gc_mark(gc);
			}
		}

		void _release_resources(virtual_machine &vm)
		{
			BASE::_release_resources(vm);
			generator.gc_release(vm);
			for(std::list<exec_variable>::iterator it=tmp.begin();it!=tmp.end();++it) it->gc_release(vm);
			tmp.clear();
		}
	D_END

	D_SELF0(array,str,exec_array_object*)
		{
			switch(mode) {
			CASE(0)
				if (self->size()==0) {
					return_value->set_string(vm->allocate_string("[]"));
					return executionstackreturnvalue::RETURN;
				}
				index=0;
				sb.add("[ ");
				GOTO(1);
			CASE(1)
				if (index<(unsigned int)self->size()) {
					CALC(2,vm->calculate_str(&tmp,self->get(index)));
				}
				sb.add(" ]");
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
			return executionstackreturnvalue::RETURN_NO_VALUE;
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
		void _mark_gc(const gc_iteration &gc) const
		{
			BASE::_mark_gc(gc);
			gc_iteration::debug_info _d("array str: tmp variable");
			tmp.gc_mark(gc);
		}
		void _release_resources(virtual_machine &vm)
		{
			BASE::_release_resources(vm);
			tmp.gc_release(vm);
		}
	D_END

	D_SELF0(array,bool,exec_array_object*)
		{
			return_value->set_bool(self->size()>0);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(array,gen,exec_array_object*)
		{
			if (mode==0) {
				index=0;
				mode=1;
				return executionstackreturnvalue::CREATE_GENERATOR;
			}
			if (index<self->size()) {
				*return_value=self->get(index++);
				return_value->gc_acquire();
				return executionstackreturnvalue::RETURN;
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}

		unsigned int index;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	RCLMFUNCTION static bool insert_item(virtual_machine &vm, exec_variable *return_value, exec_array_object *self, owca_int p1, const exec_variable *p2)
	{
		if (p1<-(owca_int)self->size() || p1>self->size()) {
			vm.raise_invalid_integer(OWCA_ERROR_FORMAT2("p1 %1 is invalid for array of size %2",int_to_string(p1),int_to_string(self->size())));
			return false;
		}
		if (p1<0) p1+=self->size();
		self->insert(vm,(unsigned int)p1,*p2);
		*return_value=*p2;
		p2->gc_acquire();
		return true;
	}

	D_SELF2(array,insert,exec_array_object*,owca_int,const exec_variable*)
		{
			if (insert_item(*vm,return_value,self,p1,p2)) return executionstackreturnvalue::RETURN;
			return executionstackreturnvalue::FUNCTION_CALL;
		}
	D_END

	D_SELF1(array,push_front,exec_array_object*,const exec_variable*)
		{
			RCASSERT(insert_item(*vm,return_value,self,0,p1));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(array,push_back,exec_array_object*,const exec_variable*)
		{
			RCASSERT(insert_item(*vm,return_value,self,(unsigned int)self->size(),p1));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	RCLMFUNCTION static bool pop_item(virtual_machine &vm, exec_variable *return_value, exec_array_object *self, owca_int index, const exec_variable *defparam)
	{
		if (index>=-(owca_int)self->size() && index<self->size()) {
			if (index<0) index+=self->size();
			self->pop(*return_value,vm,(unsigned int)index);
			return true;
		}
		else if (defparam) {
			*return_value=*defparam;
			defparam->gc_acquire();
			return true;
		}
		else if (self->size()==0) {
			vm.raise_invalid_integer(OWCA_ERROR_FORMAT("cant pop from empty list"));
			return false;
		}
		else {
			vm.raise_invalid_integer(OWCA_ERROR_FORMAT2("index %1 is invalid for array of size %2",int_to_string(index),int_to_string(self->size())));
			return false;
		}
	}

	D_SELF2(array,pop,exec_array_object*,owca_int,const exec_variable*)
		{
			if (pop_item(*vm,return_value,self,p1,p2->mode()!=VAR_NO_PARAM_GIVEN ? p2 : NULL)) return executionstackreturnvalue::RETURN;
			return executionstackreturnvalue::FUNCTION_CALL;
		}
	D_END

	D_SELF1(array,pop_front,exec_array_object*,const exec_variable*)
		{
			if (pop_item(*vm,return_value,self,0,p1->mode()!=VAR_NO_PARAM_GIVEN ? p1 : NULL)) return executionstackreturnvalue::RETURN;
			return executionstackreturnvalue::FUNCTION_CALL;
		}
	D_END

	D_SELF1(array,pop_back,exec_array_object*,const exec_variable*)
		{
			if (pop_item(*vm,return_value,self,-1,p1->mode()!=VAR_NO_PARAM_GIVEN ? p1 : NULL)) return executionstackreturnvalue::RETURN;
			return executionstackreturnvalue::FUNCTION_CALL;
		}
	D_END

	D_SELF1(array,find,exec_array_object*,const exec_variable*)
		{
			switch(mode) {
			CASE(0)
				index=0;
				GOTO(1);
			CASE(1)
				if (index<self->size()) {
					exec_variable z[2]={self->get(index),*p1};
					CALC(2,vm->calculate_eq(&tmp,z));
				}
				break;
			CASE(2)
				if (tmp.get_bool()) {
					return_value->set_int(index);
					return executionstackreturnvalue::RETURN;
				}
				++index;
				GOTO(1);
			default:
				RCASSERT(0);
			}
			return_value->set_int(-1);
			return executionstackreturnvalue::RETURN;
		}
		exec_variable tmp;
		unsigned int index;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF1(array,find_all,exec_array_object*,const exec_variable*)
		{
			switch(mode) {
			CASE(0)
				index=0;
				mode=1;
				return executionstackreturnvalue::CREATE_GENERATOR;
			CASE(1)
				if (index<self->size()) {
					exec_variable z[2]={self->get(index),*p1};
					CALC(2,vm->calculate_eq(&tmp,z));
				}
				break;
			CASE(2)
				if (tmp.get_bool()) {
					return_value->set_int(index++);
					mode=1;
					return executionstackreturnvalue::RETURN;
				}
				++index;
				GOTO(1);
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
		exec_variable tmp;
		unsigned int index;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	void virtual_machine::initialize_array(internal_class &c)
	{
		c._setstructure<exec_array_object>();
		M_OPER1(c,array,init,(*this,"$init","generator",dv_no_param_given));
		M_OPER0(c,array,str,(*this,"$str"));
		M_OPER0(c,array,bool,(*this,"$bool"));
		M_OPER0(c,array,gen,(*this,"$gen"));
		M_OPER1(c,array,read_1,(*this,"$read_1","index"));
		M_OPER2(c,array,read_2,(*this,"$read_2","from","to"));
		M_OPER2(c,array,write_1,(*this,"$write_1","index","value"));
		M_OPER3(c,array,write_2,(*this,"$write_2","from","to","value"));

		M_OPER1(c,array,in,(*this,"$in","value"));

		M_OPER1(c,array,add,(*this,"$add","other"));
		M_OPER1(c,array,mul,(*this,"$mul","other"));
		M_OPER_A(c,rmul,array,mul,(*this,"$rmul","other"));
		M_OPER1(c,array,sadd,(*this,"$sadd","other"));
		M_OPER1(c,array,smul,(*this,"$smul","other"));

		c["$eq"].set(exec_function_ptr::generate_function_1<array_cmp_object<E_EQ> >(*this,"$eq","other"));
		c["$noteq"].set(exec_function_ptr::generate_function_1<array_cmp_object<E_NOTEQ> >(*this,"$noteq","other"));
		c["$lesseq"].set(exec_function_ptr::generate_function_1<array_cmp_object<E_LESSEQ> >(*this,"$lesseq","other"));
		c["$moreeq"].set(exec_function_ptr::generate_function_1<array_cmp_object<E_MOREEQ> >(*this,"$moreeq","other"));
		c["$less"].set(exec_function_ptr::generate_function_1<array_cmp_object<E_LESS> >(*this,"$less","other"));
		c["$more"].set(exec_function_ptr::generate_function_1<array_cmp_object<E_MORE> >(*this,"$more","other"));

		M_FUNC0(c,array,size,(*this,"size"));
		M_FUNC0(c,array,shuffle,(*this,"shuffle"));
		M_FUNC0(c,array,sort,(*this,"sort"));
		M_FUNC1(c,array,find,(*this,"find","obj"));
		M_FUNC1(c,array,find_all,(*this,"find_all","obj"));
		M_FUNC1(c,array,resize,(*this,"resize","size"));
		M_FUNC2(c,array,insert,(*this,"insert","index","value"));
		M_FUNC1(c,array,push_front,(*this,"push_front","value"));
		M_FUNC1(c,array,push_back,(*this,"push_back","value"));
		M_FUNC2(c,array,pop,(*this,"pop","index","default_value",dv_no_param_given));
		M_FUNC1(c,array,pop_front,(*this,"pop_front","default_value",dv_no_param_given));
		M_FUNC1(c,array,pop_back,(*this,"pop_back","default_value",dv_no_param_given));
	}

} }


