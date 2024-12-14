#include "stdafx.h"
#include "base.h"
#include "exec_namespace.h"
#include "exec_variable.h"
#include "virtualmachine.h"
#include "exec_function_ptr.h"
#include "exec_class_int.h"
#include "op_validate.h"
#include "exec_stack.h"
#include "exec_string.h"
#include "namespace.h"
#include "exec_map_object.h"
#include "exec_class_object.h"
#include "exec_coroutine.h"
#include "exec_object.h"
#include "returnvalue.h"

namespace owca {
	namespace __owca__ {
		exec_namespace::exec_namespace(virtual_machine &vm_, owca_internal_string *file_name) : vm(vm_),variables(NULL),file_name(file_name)
		{
            file_name->gc_acquire();
		}

		exec_namespace::~exec_namespace()
		{
			clear();
		}

        exec_namespace *exec_namespace::copy(owca_internal_string *filename)
        {
            auto n = vm.allocate_namespace(filename);
			hash_map_iterator it;

			while(hashindex.next(it)) {
                owca_internal_string *name = hashindex.getkey(it).k;
                unsigned int index = hashindex.getval(it).v;
                exec_variable &var = variables->get(index);
                n->insert_variable(name,var);
			}
            return n;
        }

		owca_namespace exec_namespace::generate()
		{
			owca_namespace n;
			n.ns=this;
			return n;
		}

		void exec_namespace::clear()
		{
            if (file_name) {
                file_name->gc_release(vm);
                file_name = NULL;
            }
			hashindex.clear(vm);
			if (variables) {
				variables->gc_release(vm);
				variables=NULL;
			}
		}

		void exec_namespace::_mark_gc(const gc_iteration &gc) const
		{
			{
				gc_iteration::debug_info _d("exec_namespace %x: hashindex",this);
				hashindex.gc_mark(gc);
			}
			if (file_name) {
				gc_iteration::debug_info _d("exec_namespace %x: file_name",this);
				file_name->gc_mark(gc);
			}
			if (variables) {
				gc_iteration::debug_info _d("exec_namespace %x: variables",this);
				variables->gc_mark(gc);
			}
		}

		void exec_namespace::_release_resources(virtual_machine &vm)
		{
			clear();
		}

		bool exec_namespace::validate_code_and_prepare_for_execution(const std::vector<unsigned char> &data, opcode_data *execution_data)
		{
			return bool(opcode_validator::validate(vm,data,execution_data));
		}

		owca_function_return_value exec_namespace::apply_code(exec_variable &result, opcode_data *opcodes)
		{
			vm.push_execution_stack();
			vm.prepare_call_opcodes_entry(&result,this,opcodes);
			return vm.owner_vm->resume_execution();
		}

		bool exec_namespace::get_variable(owca_internal_string *name, exec_variable &v)
		{
			hashmap::hash_map_finder mi(hashindex,name->hash());

			for(;mi.valid();mi.next()) {
				if (hashindex.getkey(mi.get()).k->equal(name)) {
					v=variables->get(hashindex.getval(mi.get()).v);
					v.gc_acquire();
					return true;
				}
			}
			return false;
		}

		int exec_namespace::insert_variable(owca_internal_string *name, const exec_variable &v)
		{
			hashmap::hash_map_finder mi(hashindex,name->hash());
			unsigned int index;

			for(;mi.valid();mi.next()) {
				if (hashindex.getkey(mi.get()).k->equal(name)) {
					index=hashindex.getval(mi.get()).v;
					variables->get(index).gc_release(vm);
					goto cont;
				}
			}
			name->gc_acquire();

			index=hashindex.size();
			hashindex.elem_insert(vm,hashindex.write_position(name->hash()),key(name),value(index));

			if (variables==NULL) {
				variables=vm.allocate_stack_variables(8,NULL);
			}
			else if (index>=variables->size()) {
				RCASSERT(index==variables->size());
				exec_stack_variables *vv=vm.allocate_stack_variables(variables->size()<<1,NULL);
				for(unsigned int i=0;i<variables->size();++i) {
					vv->get(i)=variables->get(i);
					variables->get(i).reset();
				}
				RCASSERT(variables->gc_count()==1);
				variables->gc_release(vm);
				variables=vv;
			}
cont:
			variables->get(index)=v;
			v.gc_acquire();

			return (int)index;
		}




		static owca_internal_string_nongc *s_join=owca_internal_string_nongc::allocate_nongc("join",4);

		D_FUNC_(compiler,print)
			{
				switch(mode) {
				CASE(0)
					if (cp.map && cp.map->map.size()>0) {
						exec_map_object_iterator mi;

						while(cp.map->map.next(mi)) {
							exec_variable &key=cp.map->map.getkey(mi).k;
							RCASSERT(key.mode()==VAR_STRING);

							if (key.get_string()->equal(s_join)) {
								exec_variable &val=cp.map->map.getval(mi);
								if (val.mode()==VAR_STRING) {
									join=val.get_string();
									join->gc_acquire();
								}
								else {
									vm->raise_invalid_param("join",vm->class_string);
									return executionstackreturnvalue::FUNCTION_CALL;
								}
							}
							else {
								vm->raise_unused_keyword_param(key.get_string());
								return executionstackreturnvalue::FUNCTION_CALL;
							}
						}
					}
					index=0;
					GOTO(1);
				CASE(1)
					for(;index<cp.normal_params_count;++index) {
						CALC(2,vm->calculate_str(&tmp,cp.normal_params[index]));
				CASE(2)
						sb.add(tmp.get_string());
						tmp.gc_release(*vm);
						tmp.reset();
						if (index+1<cp.normal_params_count+cp.list_params_count) {
							if (join) sb.add(join);
							else sb.add(" ",1);
						}
					}
					index=0;
					GOTO(3);
				CASE(3)
					for(;index<cp.list_params_count;++index) {
						CALC(4,vm->calculate_str(&tmp,cp.list_params[index]));
				CASE(4)
						sb.add(tmp.get_string());
						tmp.gc_release(*vm);
						tmp.reset();
						if (cp.normal_params_count+index+1<cp.normal_params_count+cp.list_params_count) {
							if (join) sb.add(join);
							else sb.add(" ",1);
						}
					}
					return_value->set_string(sb.to_string(*vm));
					if (vm->printfnc) {
						vm->printfnc(return_value->get_string()->str()+"\n");
					}
					return executionstackreturnvalue::RETURN;
				}
				RCASSERT(0);
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
			exec_variable tmp;
			owca_internal_string *join;
			stringbuffer sb;
			unsigned int index;
			unsigned char mode;

			void create_self()
			{
				mode=0;
				tmp.reset();
				join=NULL;
			}
			void _mark_gc(const gc_iteration &gc) const
			{
				BASE::_mark_gc(gc);
				{
					gc_iteration::debug_info _d("compiler print: tmp variable");
					tmp.gc_mark(gc);
				}
				if (join) {
					gc_iteration::debug_info _d("compiler print: join object");
					join->gc_mark(gc);
				}
			}
			void _release_resources(virtual_machine &vm)
			{
				BASE::_release_resources(vm);
				tmp.gc_release(vm);
				if (join) join->gc_release(vm);
			}
		D_END

		D_FUNC0(compiler,debugbreak)
			{
				if (vm->debug_interface)
					vm->debug_interface->debug_break();
				return executionstackreturnvalue::RETURN;
			}
			void create_self(void) {}
		D_END

		D_FUNC1(compiler,id,const exec_variable*)
			{
				return_value->set_int(vm->id(*p1));
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_FUNC1(compiler,hash,const exec_variable*)
			{
				switch(mode) {
				CASE(0)
					CALC(1,vm->calculate_hash(return_value,*p1));
				CASE(1)
					break;
				default:
					RCASSERT(0);
				}
				return executionstackreturnvalue::RETURN;
			}
			unsigned char mode;
			void create_self(void) { mode=0; }
		D_END

		D_FUNC2(compiler,cmp,const exec_variable*,const exec_variable*)
			{
				switch(mode) {
				CASE(0)
					CALC(1,vm->calculate_eq(&tmp,v_params));
				CASE(1)
					if (tmp.get_bool()) {
						return_value->set_int(0);
						break;
					}
					GOTO(2);
				CASE(2)
					CALC(3,vm->calculate_less(&tmp,v_params));
				CASE(3)
					return_value->set_int(tmp.get_bool() ? -1 : 1);
					break;
				default:
					RCASSERT(0);
				}
				return executionstackreturnvalue::RETURN;
			}

			exec_variable tmp;
			unsigned char mode;
			void create_self(void) { mode=0; }
		D_END

		D_FUNC3(compiler,range,owca_int,const exec_variable *,const exec_variable *)
			{
				switch(mode) {
				CASE(0)
					start=p1;
					if (p2->mode()==VAR_NO_PARAM_GIVEN) {
						if (p3->mode()!=VAR_NO_PARAM_GIVEN) {
							vm->raise_invalid_param(OWCA_ERROR_FORMAT("parameter 'end' not given"));
							return executionstackreturnvalue::FUNCTION_CALL;
						}
						end=start;
						start=0;
						step=end>start ? 1 : -1;
						goto done;
					}
					else if (!vm->convert(end,*p2)) {
						vm->raise_invalid_param("end",vm->class_int);
						return executionstackreturnvalue::FUNCTION_CALL;
					}
					if (p3->mode()==VAR_NO_PARAM_GIVEN) {
						step=end>start ? 1 : -1;
						goto done;
					}
					else if (!vm->convert(step,*p3)) {
						vm->raise_invalid_param("step",vm->class_int);
						return executionstackreturnvalue::FUNCTION_CALL;
					}
					else if (step==0) {
						vm->raise_invalid_param(OWCA_ERROR_FORMAT("parameter 'step' cant be 0"));
						return executionstackreturnvalue::FUNCTION_CALL;
					}
done:
					mode=1;
					return executionstackreturnvalue::CREATE_GENERATOR;
				CASE(1)
					if ((step<0 && end<start) || (step>0 && end>start)) {
						return_value->set_int(start);
						start+=step;
					}
					else return executionstackreturnvalue::RETURN_NO_VALUE;
					break;
				default:
					RCASSERT(0);
				}
				return executionstackreturnvalue::RETURN;
			}
			owca_int start,step,end;
			unsigned char mode;
			void create_self(void) { mode=0; }
		D_END

		D_FUNC1(compiler,class_name,exec_class_object*)
		{
			return_value->set_string(vm->allocate_string(p1->name->str()));
			return executionstackreturnvalue::RETURN;
		}
		D_END

		D_FUNC0(compiler,trap)
		{
			return executionstackreturnvalue::RETURN_NO_VALUE;
		}
		D_END

		D_FUNC3(compiler,get,const exec_variable *, owca_internal_string *, const exec_variable *)
			{
				switch(mode) {
				CASE(0) {
					lookupreturnvalue l=p1->lookup_read(*return_value,*vm,p2);
					switch(l.type()) {
					case lookupreturnvalue::LOOKUP_FOUND:
						return executionstackreturnvalue::RETURN;
					case lookupreturnvalue::LOOKUP_NOT_FOUND:
						if (p3->mode()==VAR_NO_PARAM_GIVEN) {
							vm->raise_missing_member(*p1,p2->str());
							return executionstackreturnvalue::FUNCTION_CALL;
						}
						*return_value=*p3;
						p3->gc_acquire();
						break;
					case lookupreturnvalue::LOOKUP_FUNCTION_CALL:
						mode=1;
						return executionstackreturnvalue::FUNCTION_CALL;
					default:
						RCASSERT(0);
					}
					break; }
				CASE(1)
					break;
				default:
					RCASSERT(0);
				}
				return executionstackreturnvalue::RETURN;
			}
			unsigned char mode;
			void create_self(void) { mode=0; }
		D_END

		D_FUNC3(compiler,set,const exec_variable *, owca_internal_string *, const exec_variable *)
			{
				switch(mode) {
				CASE(0) {
					lookupreturnvalue l=p1->lookup_write(*return_value,*vm,p2,*p3);
					switch(l.type()) {
					case lookupreturnvalue::LOOKUP_FOUND:
						return executionstackreturnvalue::RETURN;
					case lookupreturnvalue::LOOKUP_NOT_FOUND:
						vm->raise_missing_member(*p1,p2->str());
						return executionstackreturnvalue::FUNCTION_CALL;
					case lookupreturnvalue::LOOKUP_FUNCTION_CALL:
						mode=1;
						return executionstackreturnvalue::FUNCTION_CALL;
					default:
						RCASSERT(0);
					}
					break; }
				CASE(1)
					break;
				default:
					RCASSERT(0);
				}
				return executionstackreturnvalue::RETURN;
			}
			unsigned char mode;
			void create_self(void) { mode=0; }
		D_END

		D_FUNC3(compiler,pop,const exec_variable *, owca_internal_string *, const exec_variable *)
			{
				if (p1->mode()==VAR_OBJECT) {
					exec_object::hashmap::hash_map_finder mi(p1->get_object()->members,p2->hash());
					for(;mi.valid() && !p1->get_object()->members.getkey(mi.get()).k->equal(p2);mi.next()) ;
					if (mi.valid()) {
						*return_value=p1->get_object()->members.getval(mi.get());
						p1->get_object()->members.getkey(mi.get()).k->gc_release(*vm);

						p1->get_object()->members.elem_remove(*vm,mi.get());
						return executionstackreturnvalue::RETURN;
					}
				}
				if (p3->mode()==VAR_NO_PARAM_GIVEN) {
					vm->raise_missing_member(*p1,p2->str());
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				*return_value=*p3;
				p3->gc_acquire();
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_FUNC1(compiler,construct,const exec_variable *)
			{
				if (p1->mode()!=VAR_OBJECT || !p1->get_object()->is_type()) {
					vm->raise_invalid_param(*p1,vm->class_class);
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				return_value->set_object(vm->allocate_object(p1->get_object(),0));
				return executionstackreturnvalue::RETURN;
			}
		D_END

		static unsigned int m_z=173921749,m_w=5632847;

		unsigned int random()
		{
			m_z = 36969 * (m_z & 65535) + (m_z >> 16);
			m_w = 18000 * (m_w & 65535) + (m_w >> 16);
			return (m_z << 16) + m_w;
		}

		D_FUNC1(compiler,random,owca_int)
			{
				if (p1) {
					m_z=(unsigned int)((p1>>32)&0xffffffff);
					m_w=(unsigned int)(p1&0xffffffff);
					return executionstackreturnvalue::RETURN_NO_VALUE;
				}
				else {
					return_value->set_int(random());
					return executionstackreturnvalue::RETURN;
				}
			}
		D_END

		D_FUNC1(compiler,weakref,exec_object*)
			{
				if (p1->weakref==NULL) {
					p1->weakref=vm->allocate_weakref_object(p1);
				}
				else {
					p1->weakref->gc_acquire();
				}
				return_value->set_weak_ref(p1->weakref);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_FUNC1(compiler,object,exec_weakref_object*)
			{
				if (p1->ptr) {
					return_value->set_object(p1->ptr);
					p1->ptr->gc_acquire();
				}
				else return_value->set_null();
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_FUNC1(compiler,costop,const exec_variable*)
			{
				switch(mode) {
				CASE(0) {
					if (p1->mode()!=VAR_NO_PARAM_GIVEN) {
						*return_value=*p1;
						p1->gc_acquire();
					}
					else {
						return_value->set_null();
					}
					mode=1;
					return executionstackreturnvalue::CO_STOP; }
				CASE(1)
					*return_value=retval;
					retval.reset();
					return executionstackreturnvalue::RETURN;
				default:
					RCASSERT(0);
					return executionstackreturnvalue::RETURN_NO_VALUE;
				}
			}

			exec_variable retval;
			unsigned char mode;

			void create_self()
			{
				mode=0;
				retval.reset();
			}
			void _mark_gc(const gc_iteration &gc) const
			{
				BASE::_mark_gc(gc);
				gc_iteration::debug_info _d("compiler costop: retval variable");
				retval.gc_mark(gc);
			}
			void _release_resources(virtual_machine &vm)
			{
				BASE::_release_resources(vm);
				retval.gc_release(vm);
			}
		D_END

		D_FUNC2(compiler,coresume,exec_coroutine_object*,const exec_variable *)
			{
				switch(mode) {
				case 0: {
					vm_execution_stack *st=vm->execution_stack;
					while(st) {
						if (vm->execution_stack->coroutine_object()) {
							vm->raise_cant_resume_from_coroutine();
							return executionstackreturnvalue::FUNCTION_CALL;
						}
						st=st->prev();
					}

					if (p1->coroutine()->count()==0) {
						vm->raise_cant_resume_fnished_coroutine();
						return executionstackreturnvalue::FUNCTION_CALL;
					}

					RCASSERT(p1->coroutine()->peek_frame_indexed(0)->return_value==NULL);

					p1->coroutine()->peek_frame_indexed(0)->return_value=return_value;
					p1->coroutine()->set_prev(vm->execution_stack);
					if (p2->mode()!=VAR_NO_PARAM_GIVEN) {
						compiler_costop_object *cco=dynamic_cast<compiler_costop_object*>(p1->coroutine()->peek_frame());
						if (cco) {
							cco->retval=*p2;
							p2->gc_acquire();
						}
					}
					vm->execution_stack=p1->coroutine();
					p1->coroutine()->gc_acquire();
					mode=1;
					return executionstackreturnvalue::CO_START; }
				case 1: // coresume returned - coroutine exited via function done, clean up
					//RCASSERT(p1->coroutine()->empty());
					//p1->peek_frame_indexed(0)->return_value=NULL;
					break;
				default:
					RCASSERT(0);
				}
				return executionstackreturnvalue::RETURN;
			}
			unsigned char mode;
			void create_self(void) { mode=0; }
		D_END

		D_FUNC1(compiler,coalive,exec_coroutine_object*)
			{
				return_value->set_bool(p1->coroutine()->count()>0);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_FUNC0(compiler,copeek)
			{
				if (!vm->execution_stack->coroutine_object()) return_value->set_null();
				else {
					return_value->set_object(vm->execution_stack->coroutine_object());
					vm->execution_stack->coroutine_object()->gc_acquire();
				}
				return executionstackreturnvalue::RETURN;
			}
		D_END

		const char *builtinnamesarray[]={
		"$typename",
		"$print",
		"$hash",
		"$id",
		"$range",
		"$cmp",
		"$memberget",
		"$memberset",
		"$memberpop",
		"$construct",
		"$type",
		"$set",
		"$str",
		"$map",
		"$list",
		"$tuple",
		"$int",
		"$real",
		"$bool",
		"$function",
		"$null",
		"$gen",
		"$property",
		"$debugbreak",
		"$exception",
		"$exception_math",
		"$exception_access",
		"$exception_operation",
		"$exception_param",
		"$coroutine",
		"$coresume",
		"$copeek",
		"$costop",
		"$coalive",
		};
		const char *compile_visible_items::builtinget() const
		{
			if (builtinindex<sizeof(builtinnamesarray)/sizeof(builtinnamesarray[0])) {
				return builtinnamesarray[builtinindex++];
			}
			return NULL;
		}

		void virtual_machine::init_builtin_functions(void)
		{
			RCASSERT(builtinfunctions.empty());
			builtinfunctions.push_back(exec_function_ptr::generate_function_1<__owca__::compiler_class_name_object>(*this,"$typename","type"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_0<__owca__::compiler_debugbreak_object>(*this,"$debugbreak"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_0<__owca__::compiler_trap_object>(*this,"$trap"));
			builtinfunctions.push_back(exec_function_ptr::generate_function__<__owca__::compiler_print_object>(*this,"$print"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_1<__owca__::compiler_hash_object>(*this,"$hash","object"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_1<__owca__::compiler_id_object>(*this,"$id","object"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_3<__owca__::compiler_range_object>(*this,"$range","start","end","step",__owca__::dv_no_param_given,__owca__::dv_no_param_given));
			builtinfunctions.push_back(exec_function_ptr::generate_function_2<__owca__::compiler_cmp_object>(*this,"$cmp","left","right"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_3<__owca__::compiler_get_object>(*this,"$memberget","object","ident","default_value",__owca__::dv_no_param_given));
			builtinfunctions.push_back(exec_function_ptr::generate_function_3<__owca__::compiler_set_object>(*this,"$memberset","object","ident","value"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_3<__owca__::compiler_pop_object>(*this,"$memberpop","object","ident","default_value",__owca__::dv_no_param_given));
			builtinfunctions.push_back(exec_function_ptr::generate_function_1<__owca__::compiler_construct_object>(*this,"$construct","type"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_1<__owca__::compiler_random_object>(*this,"$random","seed",0));
			builtinfunctions.push_back(exec_function_ptr::generate_function_1<__owca__::compiler_weakref_object>(*this,"$weakref","object"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_1<__owca__::compiler_object_object>(*this,"$object","weakref"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_2<__owca__::compiler_coresume_object>(*this,"$coresume","coobject","value",__owca__::dv_no_param_given));
			builtinfunctions.push_back(exec_function_ptr::generate_function_0<__owca__::compiler_copeek_object>(*this,"$copeek"));
			builtinfunctions.push_back(exec_function_ptr::generate_function_1<__owca__::compiler_costop_object>(*this,"$costop","stop_value",__owca__::dv_no_param_given));
			builtinfunctions.push_back(exec_function_ptr::generate_function_1<__owca__::compiler_coalive_object>(*this,"$coalive","coobject"));
		}

		void exec_namespace::init()
		{
			//(*this)["$typename"]._set_function_1<__owca__::compiler_class_name_object>("type");
			//(*this)["$trap"]._set_function_0<__owca__::compiler_trap_object>();
			//(*this)["$print"]._set_function__<__owca__::compiler_print_object>();
			//(*this)["$hash"]._set_function_1<__owca__::compiler_hash_object>("object");
			//(*this)["$id"]._set_function_1<__owca__::compiler_id_object>("object");
			//(*this)["$range"]._set_function_3<__owca__::compiler_range_object>("start","end","step",__owca__::dv_no_param_given,__owca__::dv_no_param_given);
			//(*this)["$cmp"]._set_function_2<__owca__::compiler_cmp_object>("left","right");
			//(*this)["$get"]._set_function_3<__owca__::compiler_get_object>("object","ident","default_value",__owca__::dv_no_param_given);
			//(*this)["$set"]._set_function_3<__owca__::compiler_set_object>("object","idend","value");
			//(*this)["$pop"]._set_function_3<__owca__::compiler_pop_object>("object","idendt","default_value",__owca__::dv_no_param_given);
			//(*this)["$random"]._set_function_1<__owca__::compiler_random_object>("seed",0);
			//(*this)["$_coresume"]._set_function_2<__owca__::compiler_coresume_object>("coobject","value",__owca__::dv_no_param_given);
			//(*this)["$_copeek"]._set_function_0<__owca__::compiler_copeek_object>();
			//(*this)["$_costop"]._set_function_1<__owca__::compiler_costop_object>("stop_value");
			//(*this)["$_coalive"]._set_function_1<__owca__::compiler_coalive_object>("coobject");

			for(unsigned int i=0;i<vm.builtinfunctions.size();++i) {
				exec_variable v;
				v.set_function_fast(vm.builtinfunctions[i]);
				RCASSERT(insert_variable(vm.builtinfunctions[i]->name(),v)>=0);
			}
#define A(a,b) do { \
				owca_internal_string *i=vm.allocate_string(b); \
				exec_variable v; \
				v.set_object(vm.a); \
				RCASSERT(insert_variable(i,v)>=0); \
				i->gc_release(vm); } while(0)
			//(*this)[b].set(_vm.a); (_vm.a)->gc_acquire()
			A(class_class,"$type");
			A(class_string,"$str");
			A(class_map,"$map");
			A(class_set,"$set");
			A(class_array,"$list");
			A(class_tuple,"$tuple");
			A(class_int,"$int");
			A(class_real,"$real");
			A(class_bool,"$bool");
			A(class_function,"$function");
			A(class_null,"$null");
			A(class_generator,"$gen");
			A(class_property,"$property");
			A(class_coroutine,"$coroutine");
			A(class_stack_element,"$stack_element");
			A(class_exception,"$exception");
			A(class_exception_math,"$exception_math");
			A(class_exception_access,"$exception_access");
			A(class_exception_operation,"$exception_operation");
			A(class_exception_param,"$exception_param");
#undef A
		}
	}
}
