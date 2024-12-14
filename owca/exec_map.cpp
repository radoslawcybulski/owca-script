#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_map_object.h"
#include "exec_string.h"
#include "op_compare.h"
#include "exec_class_int.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_external.h"
#include "exec_callparams.h"
#include "exec_tuple_object.h"
#include "exec_sort_array.h"

namespace owca { namespace __owca__ {

	D_SELF_(map,init,exec_map_object*)
		{
			switch(mode) {
			CASE(0)
				self->map.clear(*vm);
				tmp[0]=v_self;
				if (!vm->ensure_no_map_params(cp.map)) return executionstackreturnvalue::FUNCTION_CALL;
				if (cp.normal_params_count+cp.list_params_count==1) {
					const exec_variable *p=cp.normal_params_count ? &cp.normal_params[0] : &cp.list_params[0];

					CALC(10,vm->calculate_generator(&values,*p));
				}
				else if (cp.normal_params_count+cp.list_params_count==2) {
					const exec_variable *p;
					switch(cp.normal_params_count) {
					case 0:
						p=&cp.list_params[0];
						break;
					case 1:
						p=&cp.normal_params[0];
						break;
					case 2:
						p=&cp.normal_params[0];
						break;
					default:
						RCASSERT(0);
						p = NULL;
					}

					CALC(20,vm->calculate_generator(&keys,*p));
				}
				else if (cp.normal_params_count+cp.list_params_count==0) {
					return executionstackreturnvalue::RETURN_NO_VALUE;
				}
				vm->raise_unsupported_operation();
				return executionstackreturnvalue::FUNCTION_CALL;
			CASE(10)
				CALC(11,vm->calculate_iter_next(&tmp[1],values));
			CASE(11)
				if (tmp[1].is_no_return_value()) { // done
					return executionstackreturnvalue::RETURN_NO_VALUE;
				}
				CALC(12,vm->calculate_iter_next(&tmp[2],values));
			CASE(12)
				if (tmp[2].is_no_return_value()) {
					vm->raise_missing_value_parameter();
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				NEXT(13,vm->prepare_call_operator(&tmp_res,E_ACCESS_1_WRITE,tmp));
			CASE(13)
				tmp_res.gc_release(*vm);
				tmp_res.reset();
				tmp[1].gc_release(*vm);
				tmp[2].gc_release(*vm);
				tmp[1].reset();
				tmp[2].reset();
				GOTO(10);
			CASE(20) {
				const exec_variable *p;
				switch(cp.normal_params_count) {
				case 0:
					p=&cp.list_params[1];
					break;
				case 1:
					p=&cp.list_params[0];
					break;
				case 2:
					p=&cp.normal_params[1];
					break;
				default:
					RCASSERT(0);
					p = NULL;
				}
				CALC(21,vm->calculate_generator(&values,*p)); }
			CASE(21)
				CALC(22,vm->calculate_iter_next(&tmp[1],keys));
			CASE(22)
				g_keys=!tmp[1].is_no_return_value();
				CALC(23,vm->calculate_iter_next(&tmp[2],values));
			CASE(23)
				g_vals=!tmp[2].is_no_return_value();
				if (g_keys && g_vals) {
					NEXT(24,vm->prepare_call_operator(&tmp_res,E_ACCESS_1_WRITE,tmp));
				}
				if (!g_keys && !g_vals) return executionstackreturnvalue::RETURN_NO_VALUE;
				if (!g_keys) vm->raise_missing_key_parameter();
				else vm->raise_missing_value_parameter();
				return executionstackreturnvalue::FUNCTION_CALL;
			CASE(24)
				tmp_res.gc_release(*vm);
				tmp_res.reset();
				tmp[1].gc_release(*vm);
				tmp[2].gc_release(*vm);
				tmp[1].reset();
				tmp[2].reset();
				GOTO(21);
			default:
				RCASSERT(0);
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
		}

		exec_variable keys,values,tmp[3],tmp_res;
		unsigned char mode;
		bool g_keys,g_vals;

		void create_self()
		{
			mode=0;
			keys.reset();
			values.reset();
			tmp[1].reset();
			tmp[2].reset();
			tmp_res.reset();
		}
		D_GCMARK
			keys.gc_mark(gc);
			values.gc_mark(gc);
			tmp[1].gc_mark(gc);
			tmp[2].gc_mark(gc);
			tmp_res.gc_mark(gc);
		D_END
		D_RELRES
			keys.gc_release(vm);
			values.gc_release(vm);
			tmp[1].gc_release(vm);
			tmp[2].gc_release(vm);
			tmp_res.gc_release(vm);
		D_END
	D_END

	D_SELF0(map,bool,exec_map_object*)
		{
			return_value->set_bool(self->map.size()!=0);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(map,str,exec_map_object*)
		{
			switch(mode) {
			CASE(0)
				if (self->map.size()==0) {
					return_value->set_string(vm->allocate_string("{}"));
					return executionstackreturnvalue::RETURN;
				}
				sb.add("{ ");
				first=true;
				mode=10;
			CASE(10)
				if (self->map.next(iter)) {
					if (first) first=false;
					else sb.add(", ");
					CALC(11,vm->calculate_str(&tmp,self->map.getkey(iter).k));
				}
				// done
				sb.add(" }");
				return_value->set_string(sb.to_string(*vm));
				return executionstackreturnvalue::RETURN;
			CASE(11)
				sb.add(tmp.get_string());
				tmp.gc_release(*vm);
				tmp.reset();
				CALC(12,vm->calculate_str(&tmp,self->map.getval(iter)));
			CASE(12)
				sb.add(" : ");
				sb.add(tmp.get_string());
				tmp.gc_release(*vm);
				tmp.reset();
				GOTO(10);
			default:
				RCASSERT(0);
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
		}

		stringbuffer sb;
		bool first;
		exec_map_object_iterator iter;
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

	template <unsigned int MODE, class A> class map_lookup {
		exec_map_object_iterator iter;
		exec_map_object_iterator insert_empty;
		exec_variable tmp;
		owca_int hash;
		unsigned int index,size,prevsize;
		bool insert_empty_used;
	public:
		RCLMFUNCTION executionstackreturnvalue next(A *a, const exec_variable *writeval)
		{
			unsigned char &mode=a->mode;

			switch(mode) {
			CASE(0)
				prevsize=a->self->map.size();
				CALC(1,a->vm->calculate_hash(&tmp,*a->p1));
			CASE(1)
				if (prevsize!=a->self->map.size()) {
size_changed:
					a->vm->raise_map_modified_while_being_used();
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				hash=tmp.get_int();
				if (a->self->map.size()==0) {
					// map is empty, so not found

					switch(MODE) {
					case 4: // operator in
						a->return_value->set_bool(false);
						return executionstackreturnvalue::RETURN;
					case 3: // getset
					case 0: // write_1
						break;
					case 1: // read_1 & get
					case 2: // pop
						if (writeval) {
							*a->return_value=*writeval;
							writeval->gc_acquire();
							return executionstackreturnvalue::RETURN;
						}
						a->vm->raise_key_not_found();
						return executionstackreturnvalue::FUNCTION_CALL;
					default:
						RCASSERT(0);
					}
				}
				iter=a->self->map.start_position(hash);
				index=1;
				size=a->self->map.table_size();
				insert_empty_used=false;
			CASE(2)
				if (prevsize!=a->self->map.size()) goto size_changed;

				if (!a->self->map.empty() && a->self->map.used(iter)) {
					if (a->self->map.gethash(iter)==hash) {
						// hash is same, compare
						exec_variable params[2];
						params[0]=a->self->map.getkey(iter).k;
						params[1]=*a->p1;
						CALC(3,a->vm->calculate_eq(&tmp,params));
					}
				}
				else if (a->self->map.empty() || a->self->map.empty(iter)) {
lookup_done:
					// not found
					switch(MODE) {
					case 4: // operator in
						a->return_value->set_bool(false);
						return executionstackreturnvalue::RETURN;
					case 3: // getset
					case 0: // write_1
						if (!insert_empty_used) insert_empty=iter;
						a->self->map.elem_insert(*a->vm,insert_empty,exec_map_object::key(*a->p1,hash),*writeval);
						debug_check_memory();

						*a->return_value=*writeval;
						a->p1->gc_acquire();
						writeval->gc_acquire();
						writeval->gc_acquire();
						return executionstackreturnvalue::RETURN;
					case 1: // read_1
					case 2: // pop
						if (writeval) {
							*a->return_value=*writeval;
							writeval->gc_acquire();
							return executionstackreturnvalue::RETURN;
						}
						a->vm->raise_key_not_found();
						return executionstackreturnvalue::FUNCTION_CALL;
					default:
						RCASSERT(0);
					}
				}
				else {
					RCASSERT(a->self->map.deleted(iter));
					if (!insert_empty_used) {
						insert_empty_used=true;
						insert_empty=iter;
					}
				}
	continue_:
				a->self->map.next_position(iter,index++);
				if (index >= size)
					goto lookup_done;
				GOTO(2);
			CASE(3)
				if (prevsize!=a->self->map.size()) goto size_changed;
				if (tmp.get_bool()) {
					// found
					switch(MODE) {
					case 4: // operator in
						a->return_value->set_bool(true);
						return executionstackreturnvalue::RETURN;
					case 0: // write_1
						a->self->map.getval(iter).gc_release(*a->vm);
						a->self->map.getval(iter)=*writeval;
						writeval->gc_acquire();
						*a->return_value=*writeval;
						writeval->gc_acquire();
						return executionstackreturnvalue::RETURN;
					case 1: // read_1 & get
					case 3: // getset
						*a->return_value=a->self->map.getval(iter);
						a->return_value->gc_acquire();
						return executionstackreturnvalue::RETURN;
					case 2: // pop
						*a->return_value=a->self->map.getval(iter);
						a->self->map.getkey(iter).k.gc_release(*a->vm);
						a->self->map.elem_remove(*a->vm,iter);
						return executionstackreturnvalue::RETURN;
					default:
						RCASSERT(0);
					}
				}
				goto continue_;
			default:
				RCASSERT(0);
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
		}
	};

	D_SELF1(map,in,exec_map_object*,const exec_variable*)
		{
			return mp.next(this,NULL);
		}
		map_lookup<4,OBJECTTYPE> mp;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF2(map,write_1,exec_map_object*,const exec_variable *, const exec_variable *)
		{
			return mp.next(this,p2);
		}

		map_lookup<0,OBJECTTYPE> mp;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF1(map,read_1,exec_map_object*,const exec_variable *)
		{
			return mp.next(this,NULL);
		}

		map_lookup<1,OBJECTTYPE> mp;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF1(map,eq,exec_map_object*,exec_map_object*)
		{
			if ((varsptr1!=NULL && varsptr1!=self->map.table_pointer()) || (varsptr2!=NULL && varsptr2!=p1->map.table_pointer())) {
				vm->raise_map_modified_while_being_used();
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			switch(mode) {
			CASE(0)
				if (self->map.size()!=p1->map.size()) {
return_false:
					return_value->set_bool(false);
					return executionstackreturnvalue::RETURN;
				}
				if (self->map.size()==0) break;
				if (self->map.size()==1) GOTO(40);

				varsptr1=self->map.table_pointer();
				{
					getter g = getter(self);
					RCASSERT(!sa1.create(vm,g,self->map.size()));
				}
				GOTO(1);
			CASE(1)
				if (sa1.next()) return executionstackreturnvalue::FUNCTION_CALL;
				GOTO(10);
			CASE(10)
				varsptr2=p1->map.table_pointer();
				{
					getter g = getter(p1);
					RCASSERT(!sa2.create(vm,g,p1->map.size()));
				}
				GOTO(11);
			CASE(11)
				if (sa2.next()) return executionstackreturnvalue::FUNCTION_CALL;
				GOTO(20);
			CASE(20) // keys are sorted, compare
				res1=sa1.result();
				res2=sa2.result();
			CASE(30)
				if (!res1.valid()) {
					RCASSERT(!res2.valid());
					break;
				}
				else {
					const exec_variable *key1,*key2;

					RCASSERT(res2.valid());

					key1=&res1.value();
					unsigned int chardiff=(unsigned int)((char*)key1-(char*)&self->map.table_pointer()->key.k);
					RCASSERT((chardiff % sizeof(exec_map_object::hashmap::elem))==0);
					unsigned int index1=chardiff/sizeof(exec_map_object::hashmap::elem);

					key2=&res2.value();
					chardiff=(unsigned int)((char*)key2-(char*)&p1->map.table_pointer()->key.k);
					RCASSERT((chardiff % sizeof(exec_map_object::hashmap::elem))==0);
					unsigned int index2=chardiff/sizeof(exec_map_object::hashmap::elem);

					mi1=exec_map_object_iterator(index1);
					mi2=exec_map_object_iterator(index2);

					RCASSERT(&self->map.getkey(mi1).k==key1);
					RCASSERT(&p1->map.getkey(mi2).k==key2);

					if (self->map.gethash(mi1)!=p1->map.gethash(mi2)) goto return_false;

					exec_variable tmp[2]={self->map.getkey(mi1).k,p1->map.getkey(mi2).k};
					CALC(31,vm->calculate_eq(&tmpres,tmp));
				}
			CASE(31)
				if (!tmpres.get_bool()) goto return_false;
				else {
					exec_variable tmp[2]={self->map.getval(mi1),p1->map.getval(mi2)};
					CALC(32,vm->calculate_eq(&tmpres,tmp));
				}
			CASE(32)
				if (!tmpres.get_bool()) goto return_false;
				res1.next();
				res2.next();
				GOTO(30);
			CASE(40)
				RCASSERT(self->map.next(mi1));
				RCASSERT(p1->map.next(mi2));
				if (self->map.gethash(mi1)!=p1->map.gethash(mi2)) goto return_false;
				else {
					exec_variable tmp[2]={self->map.getkey(mi1).k,p1->map.getkey(mi2).k};
					CALC(41,vm->calculate_eq(&tmpres,tmp));
				}
			CASE(41)
				if (!tmpres.get_bool()) goto return_false;
				else {
					exec_variable tmp[2]={self->map.getval(mi1),p1->map.getval(mi2)};
					CALC(42,vm->calculate_eq(&tmpres,tmp));
				}
			CASE(42)
				if (!tmpres.get_bool()) goto return_false;
				break;
			default:
				RCASSERT(0);
			}
			return_value->set_bool(true);
			return executionstackreturnvalue::RETURN;
		}

		exec_map_object_iterator mi1,mi2;
		exec_sort_array sa1,sa2;
		const void *varsptr1,*varsptr2;
		exec_sort_array_result res1,res2;
		exec_variable tmpres;
		unsigned char mode;

		void create_self()
		{
			mode=0;
			tmpres.reset();
			varsptr1=varsptr2=NULL;
		}

		class getter : public exec_sort_array_getter_base {
			exec_map_object *obj;
			exec_map_object_iterator mi;
		public:
			getter(exec_map_object *obj_) : obj(obj_) { }

			const exec_variable &next(owca_int &hash)
			{
				RCASSERT(obj->map.next(mi));
				hash=obj->map.gethash(mi);
				return obj->map.getkey(mi).k;
			}
		};
	D_END

	D_SELF1(map,noteq,exec_map_object*,exec_map_object*)
		{
			switch(mode) {
			case 0: {
				exec_variable tmp[2]={v_self,v_params[0]};
				show_in_exception_stack(false);
				vm->prepare_call_operator(return_value,E_EQ,tmp);
				mode=1;
				return executionstackreturnvalue::FUNCTION_CALL; }
			case 1:
				return_value->set_bool(!return_value->get_bool());
				break;
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN;
		}
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF2(map,pop,exec_map_object*,const exec_variable*, const exec_variable*)
		{
			if (p1->mode()==VAR_NO_PARAM_GIVEN) { // pop any key
				if (self->map.size()==0) {
					if (p2->mode()!=VAR_NO_PARAM_GIVEN) {
						*return_value=*p2;
						return_value->gc_acquire();
						return executionstackreturnvalue::RETURN;
					}
					else {
						vm->raise_unsupported_operation();
						return executionstackreturnvalue::FUNCTION_CALL;
					}
				}
				exec_map_object_iterator mi;
				RCASSERT(self->map.next(mi));

				exec_tuple_object *oo;
				exec_object *o=vm->allocate_tuple(oo,2);
				oo->get(0)=self->map.getkey(mi).k;
				oo->get(1)=self->map.getval(mi);
				return_value->set_object(o);

				self->map.elem_remove(*vm,mi);

				return executionstackreturnvalue::RETURN;
			}
			else {
				return mp.next(this,p2->mode()!=VAR_NO_PARAM_GIVEN ? p2 : NULL);
			}
		}

		map_lookup<2,OBJECTTYPE> mp;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF0(map,clone,exec_map_object*)
		{
			exec_map_object *oo;
			exec_object *o=vm->allocate_map(oo);
			oo->map.copy_from(*vm,self->map);
			return_value->set_object(o);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(map,clear,exec_map_object*)
		{
			self->map.clear(*vm);
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
	D_END

	D_SELF2(map,get,exec_map_object*,const exec_variable*,const exec_variable*)
		{
			return mp.next(this,p2->mode()!=VAR_NO_PARAM_GIVEN ? p2 : NULL);
		}
		map_lookup<1,OBJECTTYPE> mp;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF2(map,getset,exec_map_object*,const exec_variable*,const exec_variable*)
		{
			return mp.next(this,p2);
		}
		map_lookup<3,OBJECTTYPE> mp;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF0(map,size,exec_map_object*)
		{
			return_value->set_int(self->map.size());
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(map,gen,exec_map_object*)
		{
			if (mode==0) {
				mode=1;
				return executionstackreturnvalue::CREATE_GENERATOR;
			}

			if (self->map.next(index)) {
				*return_value=self->map.getkey(index).k;
				return_value->gc_acquire();
				return executionstackreturnvalue::RETURN;
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
		exec_map_object_iterator index;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF0(map,keys,exec_map_object*)
		{
			if (mode==0) {
				mode=1;
				return executionstackreturnvalue::CREATE_GENERATOR;
			}

			while(self->map.next(index)) {
				*return_value=self->map.getkey(index).k;
				return_value->gc_acquire();
				return executionstackreturnvalue::RETURN;
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
		exec_map_object_iterator index;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF0(map,values,exec_map_object*)
		{
			if (mode==0) {
				mode=1;
				return executionstackreturnvalue::CREATE_GENERATOR;
			}

			while(self->map.next(index)) {
				*return_value=self->map.getval(index);
				return_value->gc_acquire();
				return executionstackreturnvalue::RETURN;
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
		exec_map_object_iterator index;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF0(map,items,exec_map_object*)
		{
			if (mode==0) {
				mode=1;
				return executionstackreturnvalue::CREATE_GENERATOR;
			}

			while(self->map.next(index)) {
				exec_tuple_object *oo;
				exec_object *o=vm->allocate_tuple(oo,2);
				oo->get(0)=self->map.getkey(index).k;
				self->map.getkey(index).k.gc_acquire();
				oo->get(1)=self->map.getval(index);
				self->map.getval(index).gc_acquire();
				return_value->set_object(o);
				return executionstackreturnvalue::RETURN;
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
		exec_map_object_iterator index;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	void virtual_machine::initialize_map(internal_class &c)
	{
		c._setstructure<exec_map_object>();
		M_OPER_(c,map,init,(*this,"$init"));
		M_OPER0(c,map,str,(*this,"$str"));
		M_OPER0(c,map,bool,(*this,"$bool"));
		M_OPER2(c,map,write_1,(*this,"$write_1","index","value"));
		M_OPER1(c,map,read_1,(*this,"$read_1","index"));
		M_OPER0(c,map,gen,(*this,"$gen"));

		M_OPER1(c,map,in,(*this,"$in","value"));

		M_OPER1(c,map,eq,(*this,"$eq","other"));
		M_OPER1(c,map,noteq,(*this,"$noteq","other"));

		M_FUNC0(c,map,clone,(*this,"clone"));
		M_FUNC0(c,map,clear,(*this,"clear"));
		M_FUNC0(c,map,size,(*this,"size"));

		M_FUNC2(c,map,pop,(*this,"pop","key","default_value",dv_no_param_given,dv_no_param_given));
		M_FUNC2(c,map,get,(*this,"get","key","default_value",dv_no_param_given));
		M_FUNC2(c,map,getset,(*this,"getset","key","value"));

		M_FUNC0(c,map,items,(*this,"items"));
		M_FUNC0(c,map,keys,(*this,"keys"));
		M_FUNC0(c,map,values,(*this,"values"));
	}
} }












