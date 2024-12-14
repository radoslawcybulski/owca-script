#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "executionstackreturnvalue.h"
#include "exec_set_object.h"
#include "exec_string.h"
#include "op_compare.h"
#include "exec_class_int.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_external.h"
#include "exec_callparams.h"
#include "exec_tuple_object.h"
#include "exec_sort_array.h"
#include "exec_object.h"

namespace owca { namespace __owca__ {

	D_SELF_(set,init,exec_set_object*)
		{
			switch(mode) {
			CASE(0)
				self->set.clear(*vm);
				tmp[0]=v_self;
				if (!vm->ensure_no_map_params(cp.map)) return executionstackreturnvalue::FUNCTION_CALL;
				if (cp.normal_params_count+cp.list_params_count==1) {
					const exec_variable *p=cp.normal_params_count ? &cp.normal_params[0] : &cp.list_params[0];

					CALC(10,vm->calculate_generator(&values,*p));
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
				NEXT(13,vm->prepare_call_operator(&tmp_res,E_BIN_OR_SELF,tmp));
			CASE(13)
				tmp_res.gc_release(*vm);
				tmp_res.reset();
				tmp[1].gc_release(*vm);
				tmp[1].reset();
				GOTO(10);

			default:
				RCASSERT(0);
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
		}

		exec_variable values,tmp[2],tmp_res;
		unsigned char mode;

		void create_self()
		{
			mode=0;
			values.reset();
			tmp[1].reset();
			tmp_res.reset();
		}
		D_GCMARK
			values.gc_mark(gc);
			tmp[1].gc_mark(gc);
			tmp_res.gc_mark(gc);
		D_END
		D_RELRES
			values.gc_release(vm);
			tmp[1].gc_release(vm);
			tmp_res.gc_release(vm);
		D_END
	D_END

	D_SELF0(set,bool,exec_set_object*)
		{
			return_value->set_bool(self->set.size()!=0);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(set,str,exec_set_object*)
		{
			switch(mode) {
			CASE(0)
				if (self->set.size()==0) {
					return_value->set_string(vm->allocate_string("set()"));
					return executionstackreturnvalue::RETURN;
				}
				sb.add("set( ");
				first=true;
				mode=10;
			CASE(10)
				if (self->set.next(iter)) {
					if (first) first=false;
					else sb.add(", ");
					CALC(11,vm->calculate_str(&tmp,self->set.getkey(iter).k));
				}
				// done
				sb.add(" )");
				return_value->set_string(sb.to_string(*vm));
				return executionstackreturnvalue::RETURN;
			CASE(11)
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
		exec_set_object_iterator iter;
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

	class set_lookup {
		exec_variable tmp;
		unsigned int index,size;
		unsigned char mode;
		exec_set_object *set;
		virtual_machine *vm;
		bool iter_insert_used;
	public:
		bool found;
		owca_int hash;
		exec_set_object_iterator iter;
		exec_set_object_iterator iter_insert;
		const exec_variable *value;

		void init(virtual_machine *vm_, exec_set_object *set_) {
			set=set_;
			vm=vm_;
		}
		void set_value(const exec_variable *value_) {
			value=value_;
			mode=0;
			iter_insert_used=false;
		}
		RCLMFUNCTION executionstackreturnvalue next()
		{
			switch(mode) {
			CASE(0)
				CALC(1,vm->calculate_hash(&tmp,*value));
			CASE(1)
				hash=tmp.get_int();
				if (set->set.empty()) {
					found=false;
					iter_insert=set->set.start_position(hash);
					return executionstackreturnvalue::RETURN;
				}
				iter=set->set.start_position(hash);
				index=1;
				size=set->set.table_size();
			CASE(2)
				if (set->set.table_size()!=size) {
					vm->raise_set_modified_while_being_used();
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				if (!set->set.empty() && set->set.used(iter)) {
					if (set->set.gethash(iter)==hash) {
						// hash is same, compare
						exec_variable params[2];
						params[0]=set->set.getkey(iter).k;
						params[1]=*value;
						CALC(3,vm->calculate_eq(&tmp,params));
					}
				}
				else if (set->set.empty(iter)) {
					if (!iter_insert_used) {
						iter_insert_used=true;
						iter_insert=iter;
					}
					found=false;
					return executionstackreturnvalue::RETURN;
				}
				else {
					RCASSERT(set->set.deleted(iter));
					if (!iter_insert_used) {
						iter_insert_used=true;
						iter_insert=iter;
					}
				}
continue_:
				set->set.next_position(iter,index++);
				RCASSERT(index<size);
				GOTO(2);
			CASE(3)
				if (set->set.table_size()!=size) {
					vm->raise_set_modified_while_being_used();
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				if (tmp.get_bool()) {
					found=true;
					return executionstackreturnvalue::RETURN;
				}
				goto continue_;
			default:
				RCASSERT(0);
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
		}
	};



	template <unsigned char MODE, class A> static executionstackreturnvalue setproc(A *a, exec_set_object *dst, std::vector<bool> *used)
	{
		unsigned char &mode=a->mode;
		virtual_machine *vm=a->vm;
		const exec_variable *p1=a->p1;
		set_lookup &st=a->st;
		exec_set_object_iterator &so_iter=a->so_iter;
		exec_set_object *&so=a->so;

		switch(mode) {
		CASE(0)
			switch(MODE) {
			case 0: break;
			case 1: if (dst->set.table_size()>0) used->resize(dst->set.table_size(),false); break;
			case 2: break;
			default: RCASSERT(0);
			}
			st.init(vm,dst);
			if (p1->mode()==VAR_OBJECT && (so=vm->data_from_object<exec_set_object>(p1->get_object()))!=NULL) {
				so_iter=exec_set_object_iterator();
				GOTO(10);
			}
			else {
				st.set_value(p1);
				GOTO(1);
			}
		CASE(1) {
			executionstackreturnvalue r=st.next();
			if (r.type()==executionstackreturnvalue::RETURN) {
				if (st.found) {
					exec_variable &k=dst->set.getkey(st.iter).k;
					k.gc_release(*vm);
					switch(MODE) {
					case 1: // and
						(*used)[st.iter.value()]=true;
					case 0: // or
						k=*st.value;
						st.value->gc_acquire();
						break;
					case 2: // xor
						dst->set.elem_remove(*vm,st.iter);
						break;
					default: RCASSERT(0);
					}
				}
				else {
					switch(MODE) {
					case 0: // or
					case 2: // xor
						dst->set.elem_insert(*vm,st.iter_insert,exec_set_object::key(*st.value,st.hash),exec_set_object::value());
						st.value->gc_acquire();
						break;
					case 1: // and
						break;
					default: RCASSERT(0);
					}
				}
				break;
			}
			return r; }
		CASE(10)
			if (so->set.next(so_iter)) {
				st.set_value(&so->set.getkey(so_iter).k);
				GOTO(20);
			}
			break;
		CASE(20) {
			executionstackreturnvalue r=st.next();
			if (r.type()==executionstackreturnvalue::RETURN) {
				if (st.found) {
					exec_variable &k=dst->set.getkey(st.iter).k;
					k.gc_release(*vm);
					switch(MODE) {
					case 1: // and
						(*used)[st.iter.value()]=true;
					case 0: // or
						k=*st.value;
						st.value->gc_acquire();
						break;
					case 2: // xor
						dst->set.elem_remove(*vm,st.iter);
						break;
					default: RCASSERT(0);
					}
				}
				else {
					switch(MODE) {
					case 0: // or
					case 2: // xor
						dst->set.elem_insert(*vm,st.iter_insert,exec_set_object::key(*st.value,st.hash),exec_set_object::value());
						st.value->gc_acquire();
						break;
					case 1: // and
						break;
					default: RCASSERT(0);
					}
				}
				//if (st.found) {
				//	exec_variable &k=dst->set.getkey(st.iter).k;
				//	k.gc_release(*vm);
				//	k=*st.value;
				//}
				//else {
				//	dst->set.elem_insert(*vm,st.iter,exec_set_object::key(*st.value,st.hash),exec_set_object::value());
				//}
				//st.value->gc_acquire();
				GOTO(10);
			}
			return r; }
		default:
			RCASSERT(0);
		}

		if (MODE==1) {
			exec_set_object_iterator it;
			while(dst->set.next(it)) {
				if (!(*used)[it.value()]) {
					exec_variable &k=dst->set.getkey(it).k;
					k.gc_release(*vm);
					k.reset();
					dst->set.elem_remove(*vm,it,false);
				}
			}
			dst->set.update_size(*vm);
		}
		return executionstackreturnvalue::RETURN;
	}

	D_SELF1(set,in,exec_set_object*,const exec_variable *)
		{
			switch(mode) {
			CASE(0)
				st.init(vm,self);
				st.set_value(p1);
				GOTO(1);
			CASE(1) {
				executionstackreturnvalue r=st.next();
				if (r.type()==executionstackreturnvalue::RETURN) {
					return_value->set_bool(st.found);
				}
				return r; }
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}

		set_lookup st;
		unsigned char mode;
		void create_self(void) { mode=0; }
		//D_GCMARK
		//D_END
		//D_RELRES
		//D_END
	D_END

	D_SELF1(set,sbinor,exec_set_object*,const exec_variable *)
		{
			executionstackreturnvalue rr=setproc<0>(this,self,NULL);
			if (rr.type()!=executionstackreturnvalue::RETURN) return rr;
			*return_value=v_self;
			return_value->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}

		exec_set_object *so;
		exec_set_object_iterator so_iter;
		set_lookup st;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF1(set,binor,exec_set_object*,const exec_variable *)
		{
			if (mode==0) {
				dstobj=vm->allocate_set(dst);
				dst->set.copy_from(*vm,self->set);
			}
			executionstackreturnvalue rr=setproc<0>(this,dst,NULL);
			if (rr.type()!=executionstackreturnvalue::RETURN) return rr;
			return_value->set_object(dstobj);
			dstobj=NULL;
			return executionstackreturnvalue::RETURN;
		}

		exec_set_object *so,*dst;
		exec_set_object_iterator so_iter;
		exec_object *dstobj;
		set_lookup st;
		unsigned char mode;
		void create_self(void) { mode=0; dstobj=NULL; }
		D_GCMARK
			//tmp.gc_mark(gc);
			if (dstobj) dstobj->gc_mark(gc);
		D_END
		D_RELRES
			//tmp.gc_release(vm);
			if (dstobj) dstobj->gc_release(vm);
		D_END
	D_END

	D_SELF1(set,sbinand,exec_set_object*,const exec_variable *)
		{
			executionstackreturnvalue rr=setproc<1>(this,self,&used);
			if (rr.type()!=executionstackreturnvalue::RETURN) return rr;
			*return_value=v_self;
			return_value->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}

		std::vector<bool> used;
		exec_set_object *so;
		exec_set_object_iterator so_iter;
		set_lookup st;
		unsigned char mode;
		void create_self(void) {
			mode=0;
		}
	D_END

	D_SELF1(set,binand,exec_set_object*,const exec_variable *)
		{
			if (mode==0) {
				dstobj=vm->allocate_set(dst);
				dst->set.copy_from(*vm,self->set);
			}
			executionstackreturnvalue rr=setproc<1>(this,dst,&used);
			if (rr.type()!=executionstackreturnvalue::RETURN) return rr;
			return_value->set_object(dstobj);
			dstobj=NULL;
			return executionstackreturnvalue::RETURN;
		}

		std::vector<bool> used;
		exec_set_object *so,*dst;
		exec_set_object_iterator so_iter;
		exec_object *dstobj;
		set_lookup st;
		unsigned char mode;
		void create_self(void) { mode=0; dstobj=NULL; }
		D_GCMARK
			//tmp.gc_mark(gc);
			if (dstobj) dstobj->gc_mark(gc);
		D_END
		D_RELRES
			//tmp.gc_release(vm);
			if (dstobj) dstobj->gc_release(vm);
		D_END
	D_END

	D_SELF1(set,sbinxor,exec_set_object*,const exec_variable *)
		{
			executionstackreturnvalue rr=setproc<2>(this,self,NULL);
			if (rr.type()!=executionstackreturnvalue::RETURN) return rr;
			*return_value=v_self;
			return_value->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}

		exec_set_object *so;
		exec_set_object_iterator so_iter;
		set_lookup st;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF1(set,binxor,exec_set_object*,const exec_variable *)
		{
			if (mode==0) {
				dstobj=vm->allocate_set(dst);
				dst->set.copy_from(*vm,self->set);
			}
			executionstackreturnvalue rr=setproc<2>(this,dst,NULL);
			if (rr.type()!=executionstackreturnvalue::RETURN) return rr;
			return_value->set_object(dstobj);
			dstobj=NULL;
			return executionstackreturnvalue::RETURN;
		}

		exec_set_object *so,*dst;
		exec_set_object_iterator so_iter;
		exec_object *dstobj;
		set_lookup st;
		unsigned char mode;
		void create_self(void) { mode=0; dstobj=NULL; }
		D_GCMARK
			//tmp.gc_mark(gc);
			if (dstobj) dstobj->gc_mark(gc);
		D_END
		D_RELRES
			//tmp.gc_release(vm);
			if (dstobj) dstobj->gc_release(vm);
		D_END
	D_END

	D_SELF1(set,eq,exec_set_object*,exec_set_object*)
		{
			if ((varsptr1!=NULL && varsptr1!=self->set.table_pointer()) || (varsptr2!=NULL && varsptr2!=p1->set.table_pointer())) {
				vm->raise_set_modified_while_being_used();
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			switch(mode) {
			CASE(0)
				if (self->set.size()!=p1->set.size()) {
return_false:
					return_value->set_bool(false);
					return executionstackreturnvalue::RETURN;
				}
				if (self->set.size()==0) break;
				if (self->set.size()==1) GOTO(40);

				varsptr1=self->set.table_pointer();
				{
					getter g = getter(self);
					RCASSERT(!sa1.create(vm,g,self->set.size()));
				}
				GOTO(1);
			CASE(1)
				if (sa1.next()) return executionstackreturnvalue::FUNCTION_CALL;
				GOTO(10);
			CASE(10)
				varsptr2=p1->set.table_pointer();
				{
					getter g = getter(p1);
					RCASSERT(!sa2.create(vm,g,p1->set.size()));
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
					unsigned int chardiff=(unsigned int)((char*)key1-(char*)&self->set.table_pointer()->key.k);
					RCASSERT((chardiff % sizeof(exec_set_object::hashmap::elem))==0);
					unsigned int index1=chardiff/sizeof(exec_set_object::hashmap::elem);

					key2=&res2.value();
					chardiff=(unsigned int)((char*)key2-(char*)&p1->set.table_pointer()->key.k);
					RCASSERT((chardiff % sizeof(exec_set_object::hashmap::elem))==0);
					unsigned int index2=chardiff/sizeof(exec_set_object::hashmap::elem);

					mi1=exec_set_object_iterator(index1);
					mi2=exec_set_object_iterator(index2);

					RCASSERT(&self->set.getkey(mi1).k==key1);
					RCASSERT(&p1->set.getkey(mi2).k==key2);

					if (self->set.gethash(mi1)!=p1->set.gethash(mi2)) goto return_false;

					exec_variable tmp[2]={self->set.getkey(mi1).k,p1->set.getkey(mi2).k};
					CALC(31,vm->calculate_eq(&tmpres,tmp));
				}
			CASE(31)
				if (!tmpres.get_bool()) goto return_false;
				res1.next();
				res2.next();
				GOTO(30);
			CASE(40)
				RCASSERT(self->set.next(mi1));
				RCASSERT(p1->set.next(mi2));
				if (self->set.gethash(mi1)!=p1->set.gethash(mi2)) goto return_false;
				else {
					exec_variable tmp[2]={self->set.getkey(mi1).k,p1->set.getkey(mi2).k};
					CALC(41,vm->calculate_eq(&tmpres,tmp));
				}
			CASE(41)
				if (!tmpres.get_bool()) goto return_false;
				break;
			default:
				RCASSERT(0);
			}
			return_value->set_bool(true);
			return executionstackreturnvalue::RETURN;
		}

		exec_set_object_iterator mi1,mi2;
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
			exec_set_object *obj;
			exec_set_object_iterator mi;
		public:
			getter(exec_set_object *obj_) : obj(obj_) { }

			const exec_variable &next(owca_int &hash)
			{
				RCASSERT(obj->set.next(mi));
				hash=obj->set.gethash(mi);
				return obj->set.getkey(mi).k;
			}
		};
	D_END

	D_SELF1(set,noteq,exec_set_object*,exec_set_object*)
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

	//D_SELF2(map,pop,exec_map_object*,const exec_variable*, const exec_variable*)
	//	{
	//		if (p1->mode()==VAR_NO_PARAM_GIVEN) { // pop any key
	//			if (self->map.size()==0) {
	//				if (p2->mode()!=VAR_NO_PARAM_GIVEN) {
	//					*return_value=*p2;
	//					return_value->gc_acquire();
	//					return executionstackreturnvalue::RETURN;
	//				}
	//				else {
	//					vm->raise_unsupported_operation();
	//					return executionstackreturnvalue::FUNCTION_CALL;
	//				}
	//			}
	//			else
	//			exec_map_object_iterator mi;
	//			RCASSERT(self->map.next(mi));

	//			exec_tuple_object *oo;
	//			exec_object *o=vm->allocate_tuple(oo,2);
	//			oo->get(0)=self->map.getkey(mi).k;
	//			oo->get(1)=self->map.getval(mi);
	//			return_value->set_object(o);

	//			self->map.elem_remove(*vm,mi);

	//			return executionstackreturnvalue::RETURN;
	//		}
	//		else {
	//			return mp.next(this,p2->mode()!=VAR_NO_PARAM_GIVEN ? p2 : NULL);
	//		}
	//	}

	//	map_lookup<2,OBJECTTYPE> mp;
	//	unsigned char mode;
	//	void create_self(void) { mode=0; }
	//D_END

	D_SELF0(set,clone,exec_set_object*)
		{
			exec_set_object *oo;
			exec_object *o=vm->allocate_set(oo);
			oo->set.copy_from(*vm,self->set);
			return_value->set_object(o);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(set,clear,exec_set_object*)
		{
			self->set.clear(*vm);
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
	D_END

	D_SELF0(set,size,exec_set_object*)
		{
			return_value->set_int(self->set.size());
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(set,gen,exec_set_object*)
		{
			if (mode==0) {
				mode=1;
				return executionstackreturnvalue::CREATE_GENERATOR;
			}

			if (self->set.next(index)) {
				*return_value=self->set.getkey(index).k;
				return_value->gc_acquire();
				return executionstackreturnvalue::RETURN;
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
		exec_set_object_iterator index;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF0(set,values,exec_set_object*)
		{
			if (mode==0) {
				mode=1;
				return executionstackreturnvalue::CREATE_GENERATOR;
			}

			while(self->set.next(index)) {
				*return_value=self->set.getkey(index).k;
				return_value->gc_acquire();
				return executionstackreturnvalue::RETURN;
			}
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
		exec_set_object_iterator index;
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	void virtual_machine::initialize_set(internal_class &c)
	{
		c._setstructure<exec_set_object>();
		M_OPER_(c,set,init,(*this,"$init"));
		M_OPER0(c,set,str,(*this,"$str"));
		M_OPER0(c,set,bool,(*this,"$bool"));

		M_OPER1(c,set,in,(*this,"$in","value"));
		M_OPER1(c,set,binor,(*this,"$binor","value"));
		M_OPER_A(c,rbinor,set,binor,(*this,"$binor","value"));
		M_OPER1(c,set,sbinor,(*this,"$sbinor","value"));
		M_OPER1(c,set,binand,(*this,"$binand","value"));
		M_OPER_A(c,rbinand,set,binand,(*this,"$binand","value"));
		M_OPER1(c,set,sbinand,(*this,"$sbinand","value"));
		M_OPER1(c,set,binxor,(*this,"$binxor","value"));
		M_OPER_A(c,rbinxor,set,binxor,(*this,"$binxor","value"));
		M_OPER1(c,set,sbinxor,(*this,"$sbinxor","value"));

		//M_OPER2(c,map,write_1,(*this,"$write_1","index","value"));
		//M_OPER1(c,map,read_1,(*this,"$read_1","index"));
		M_OPER0(c,set,gen,(*this,"$gen"));

		M_OPER1(c,set,eq,(*this,"$eq","other"));
		M_OPER1(c,set,noteq,(*this,"$noteq","other"));

		M_FUNC0(c,set,clone,(*this,"clone"));
		M_FUNC0(c,set,clear,(*this,"clear"));
		M_FUNC0(c,set,size,(*this,"size"));

		M_FUNC0(c,set,values,(*this,"values"));
	}
} }












