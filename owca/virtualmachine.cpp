#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_string.h"
#include "exec_object.h"
#include "exec_class_object.h"
#include "exec_tuple_object.h"
#include "cmp_compiler.h"
#include "exec_function_ptr.h"
#include "exec_array_object.h"
#include "exec_map_object.h"
#include "exec_set_object.h"
#include "exec_stack.h"
#include "exec_property.h"
#include "op_base.h"
#include "exec_class_int.h"
#include "exec_function_stack_data.h"
#include "namespace.h"
#include "class.h"
#include "exception.h"
#include "sourcefile.h"
#include "op_execute.h"
#include "string.h"
#include "vm_execution_stack_elem_base.h"
#include "vm_execution_stack_elem_internal.h"
#include "vm_execution_stack_elem_external.h"
#include "returnvalueflow.h"
#include "exec_callparams.h"
#include "operatorcodes.h"
#include "exec_namespace.h"
#include "op_validate.h"
#include <memory.h>

namespace owca {
	namespace __owca__ {
		std::string identificator_names[]={
				"$eq",
				"$noteq",
				"$lesseq",
				"$moreeq",
				"$less",
				"$more",

				"$req",
				"$rnoteq",
				"$rlesseq",
				"$rmoreeq",
				"$rless",
				"$rmore",

				"$add",
				"$sub",
				"$mul",
				"$div",

				"$mod",
				"$lshift",
				"$rshift",
				"$binand",
				"$binor",
				"$binxor",

				"$radd",
				"$rsub",
				"$rmul",
				"$rdiv",

				"$rmod",
				"$rlshift",
				"$rrshift",
				"$rbinand",
				"$rbinor",
				"$rbinxor",

				"$in",

				"$sadd",
				"$ssub",
				"$smul",
				"$sdiv",
				"$smod",
				"$slshift",
				"$srshift",
				"$sbinand",
				"$sbinor",
				"$sbinxor",
				"$read_1",
				"$write_1",
				"$read_2",
				"$write_2",

				"$binnot",
				"$bool",
				"$sign",
				"$call",
				"$gen",
				"$init",
				"$new",
				"$str",
				"$hash",
				"$withexit",
				"$withenter",
		};

		virtual_machine::virtual_machine(owca_vm *owner) : owner_vm(owner),internalclases(NULL),gciteration(*this),vislockers(NULL),
					vars(NULL),printfnc(NULL),class_class(NULL),
					execution_exception_object_thrown(NULL),
					execution_stack(NULL),allocated_object_waiting_for_delete(NULL),allocated_object_waiting_for_delete_in_progress(NULL)
		{
			allocated_objects=rootobject=new (allocate_memory(sizeof(exec_rootobject),typeid(exec_rootobject))) exec_rootobject();
			rootobject->_gc_next=rootobject->_gc_prev=rootobject;

			execution_exception_parameters[1].reset();
			execution_exception_object_temp.reset();

			for(unsigned int i=0;i<E_COUNT;++i) {
				if (identificator_names[i].empty()) operator_identificators[i]=NULL;
				else operator_identificators[i]=allocate_string(identificator_names[i]);
			}

			RCASSERT(sizeof(identificator_names)/sizeof(identificator_names[0])==E_COUNT);

			init_class(class_class,"$type",&virtual_machine::initialize_class);
			init_class(class_string,"$str",&virtual_machine::initialize_string);
			init_class(class_map,"$map",&virtual_machine::initialize_map);
			init_class(class_set,"$set",&virtual_machine::initialize_set);
			init_class(class_array,"$list",&virtual_machine::initialize_array);
			init_class(class_tuple,"$tuple",&virtual_machine::initialize_tuple);
			init_class(class_int,"$int",&virtual_machine::initialize_int);
			init_class(class_real,"$real",&virtual_machine::initialize_real);
			init_class(class_bool,"$bool",&virtual_machine::initialize_bool);
			init_class(class_function,"$function",&virtual_machine::initialize_function);
			init_class(class_coroutine,"$_coroutine",&virtual_machine::initialize_coroutine);
			init_class(class_null,"$null",&virtual_machine::initialize_null);
			init_class(class_generator,"$gen",&virtual_machine::initialize_generator);
			init_class(class_property,"$property",&virtual_machine::initialize_property);
			init_class(class_stack_element,"$stack_element",&virtual_machine::initialize_stack_element);
			init_class(class_exception,"$exception",&virtual_machine::initialize_exception);
			init_class(class_exception_math,"$exception_math",&virtual_machine::initialize_exception_std);
			init_class(class_exception_access,"$exception_access",&virtual_machine::initialize_exception_std);
			init_class(class_exception_operation,"$exception_operation",&virtual_machine::initialize_exception_std);
			init_class(class_exception_param,"$exception_param",&virtual_machine::initialize_exception_std);

			for(unsigned int i=0;i<VAR_COUNT;++i) basicmap[i]=NULL;
			basicmap[VAR_INT]=class_int;
			basicmap[VAR_REAL]=class_real;
			basicmap[VAR_BOOL]=class_bool;
			basicmap[VAR_NULL]=class_null;
			basicmap[VAR_STRING]=class_string;
			basicmap[VAR_FUNCTION]=class_function;
			basicmap[VAR_FUNCTION_FAST]=class_function;
			basicmap[VAR_GENERATOR]=class_generator;
			basicmap[VAR_PROPERTY]=class_property;

			init_builtin_functions();

			GC(this);
		}

		void virtual_machine::init_class(exec_object *&dst, const std::string &name, void (virtual_machine::*init)(internal_class &))
		{
			internal_class c(*this,name);
			(this->*init)(c);
			std::string z=c.create(dst);
			RCASSERT(z.empty());
		}

		virtual_machine::~virtual_machine()
		{
			execution_exception_object_temp.gc_release(*this);
			execution_exception_object_temp.reset();
			execution_exception_parameters[1].gc_release(*this);
			execution_exception_parameters[1].reset();

			while(!builtinfunctions.empty()) {
				exec_function_ptr *f=builtinfunctions.back();
				builtinfunctions.pop_back();
				f->gc_release(*this);
			}

			if (execution_exception_object_thrown!=NULL) {
				execution_exception_object_thrown->gc_release(*this);
				execution_exception_object_thrown=NULL;
			}

			while(vars) {
				owca_global *g=vars;
				g->null_set();
				g->_detach();
				g->_vm=NULL;
			};

			GC(this);

			if (execution_stack) {
				execution_stack->clear();
				execution_stack->gc_release(*this);
				execution_stack=NULL;
			}

#define Q(a) a->members.clear(*this);

			Q(class_string);
			Q(class_array);
			Q(class_tuple);
			Q(class_int);
			Q(class_real);
			Q(class_bool);
			Q(class_map);
			Q(class_set);
			Q(class_null);
			Q(class_function);
			Q(class_generator);
			Q(class_property);
			Q(class_coroutine);
			Q(class_stack_element);
			Q(class_exception);
			Q(class_exception_param);
			Q(class_exception_math);
			Q(class_exception_operation);
			Q(class_exception_access);
			Q(class_class);
#undef Q

#define Q(a) do { a->gc_release(*this); a=NULL; } while(0)
			Q(class_string);
			Q(class_array);
			Q(class_tuple);
			Q(class_int);
			Q(class_real);
			Q(class_bool);
			Q(class_map);
			Q(class_set);
			Q(class_null);
			Q(class_function);
			Q(class_generator);
			Q(class_property);
			Q(class_coroutine);
			Q(class_stack_element);
			Q(class_exception);
			Q(class_exception_param);
			Q(class_exception_math);
			Q(class_exception_operation);
			Q(class_exception_access);
			class_class->_refcnt|=0x80000000;
			class_class->_release_resources(*this);
			class_class->gc_release(*this);
			class_class=NULL;
			//Q(class_class);
#undef Q

			GC(this);

			for(unsigned int i=0;i<E_COUNT;++i) if (operator_identificators[i]) {
				operator_identificators[i]->gc_release(*this);
				operator_identificators[i]=NULL;
			}

			while (!opcode_datas.empty()) {
				opcode_datas[opcode_datas.size() - 1]->gc_release(*this);
				opcode_datas.pop_back();
			}

			for (auto it : strings_used_by_opcode_datas)
				it.second->gc_release(*this);
			strings_used_by_opcode_datas.clear();

			run_gc();

			rootobject->gc_release(*this);
			exec_ref_counter_finalize(*this);
		}

#ifdef RCDEBUG_GC
		unsigned int iteration;
#endif

		void virtual_machine::run_gc()
		{
			static bool inside=false;

			debug_check_memory();

			if (allocated_object_waiting_for_delete_in_progress==NULL) purge_objects();

			RCASSERT(!inside);
			inside=true;
#ifdef RCDEBUG_GC
			++iteration;

			unsigned int iter=iteration;
			if (iteration>=16747) {
				iteration=iteration;
			}
#endif

			gc_iteration &g=gciteration;
			g.value.increment();

			for (auto opc : opcode_datas)
				opc->gc_mark(g);
			for (auto it : strings_used_by_opcode_datas)
				it.second->gc_mark(g);

			rootobject->gc_mark(g);

			if (allocated_object_waiting_for_delete_in_progress!=NULL) {
				allocated_object_waiting_for_delete_in_progress->_gc_mark(g);
				exec_base *o=allocated_object_waiting_for_delete;
				while(o) {
					o->_gc_mark(g);
					o=o->_gc_next;
				}
			}

			execution_exception_object_temp.gc_mark(g);
			execution_exception_parameters[1].gc_mark(g);

			for(unsigned int i=0;i<E_COUNT;++i) if (operator_identificators[i]) operator_identificators[i]->gc_mark(g);

			if (execution_exception_object_thrown) execution_exception_object_thrown->gc_mark(g);

			if (internal_class *c=internalclases) {
				do {
					c->gc_mark(g);
				} while((c=c->next)!=internalclases);
			}
			for(unsigned int i=0;i<builtinfunctions.size();++i) builtinfunctions[i]->gc_mark(g);

#define Q(a) if (a) a->gc_mark(g)
			Q(class_string);
			Q(class_array);
			Q(class_tuple);
			Q(class_int);
			Q(class_real);
			Q(class_class);
			Q(class_bool);
			Q(class_map);
			Q(class_set);
			Q(class_null);
			Q(class_function);
			Q(class_generator);
			Q(class_property);
			Q(class_coroutine);
			Q(class_stack_element);
			Q(class_exception);
			Q(class_exception_param);
			Q(class_exception_math);
			Q(class_exception_operation);
			Q(class_exception_access);
#undef Q

			{
				vislocker *v=vislockers;
				while(v) {
					v->gc_mark(g);
					v=v->prev;
				}
			}

			if (execution_stack) {
				execution_stack->gc_mark(g);
			}

			if (vars) {
				owca_global *v=vars;
				do {
					v->_object.gc_mark(g);
					v=v->_next;
				} while(v!=vars);
			}

			exec_base *o=allocated_objects;

			o=allocated_objects;
#ifdef RCDEBUG_GC
			//unsigned int allocated_objects_id=0;
#endif
			do {
				if (o->_prev!=g.value && o->_refcnt < 0x80000000 && !o->is_type()) {
					o->gc_acquire();
					o->_refcnt|=0x80000000;
					o->_release_resources(*this);
					exec_base *o2=o->_gc_next;
#ifdef RCDEBUG_GC
					RCASSERT((o->_refcnt&0x7fffffff)>0);
#endif
					o->gc_release(*this);
					o=o2;
				}
				else o=o->_gc_next;
#ifdef RCDEBUG_GC
				//RCASSERT(o->get_debug_ident()>allocated_objects_id);
				//allocated_objects_id=o->get_debug_ident();
#endif
			} while(o!=rootobject);

			purge_memory();

#ifdef RCDEBUG_GC
			bool res=true;
			if (o=allocated_objects) {
				do {
					if (o->_prev!=g.value || (o->_refcnt&0x7fffffff)!=o->_debug_cnt_gc) {
						res=false;
						break;
					}
					o=o->_gc_next;
				} while(o!=allocated_objects);
			}
			if (!res) {
				RCPRINTF("\n\n_gc failure %d!\n",iteration);
				if (o=allocated_objects) {
					bool firsttime=true;
					do {
						if (o->_prev!=g.value || (o->_refcnt&0x7fffffff)!=o->_debug_cnt_gc) {
							if (firsttime) {
								firsttime=false;
								RCPRINTF("%3s %3s %6s %20s\n",
									"ref","dbg","dbgid","typename");
							}
							const std::type_info &res=typeid(*o);
							std::string name;
							if (exec_object *oo=dynamic_cast<exec_object*>(o)) {
								if (oo->type()) {
#ifdef RCDEBUG
									name=oo->type()->_co_->name->str();
#endif
								}
							}
							if (o->_prev!=g.value) 
								RCPRINTF("object skipped during gc phase %p %3d:    %s\n",o, (o->_refcnt&0x7fffffff),o->_debug_text().c_str());
							else
								RCPRINTF("object refcount mismatch       %p %3d:%3d %s\n",o, (o->_refcnt&0x7fffffff),o->_debug_cnt_gc,o->_debug_text().c_str());
						}
						o=o->_gc_next;
					} while(o!=allocated_objects);
				}
				RCASSERT(0);
				int zz;
				zz=4;
				zz=zz+2;
			}
#endif
			inside=false;
		}

		void virtual_machine::purge_objects()
		{
			RCASSERT(allocated_object_waiting_for_delete_in_progress==NULL);
			while(allocated_object_waiting_for_delete) {
				allocated_object_waiting_for_delete_in_progress=allocated_object_waiting_for_delete;
				allocated_object_waiting_for_delete=allocated_object_waiting_for_delete->_gc_next;

				allocated_object_waiting_for_delete_in_progress->_release_resources(*this);
				allocated_object_waiting_for_delete_in_progress->_destroy(*this);
			}
			allocated_object_waiting_for_delete_in_progress=NULL;
		}

		owca_internal_string *virtual_machine::identificator(operatorcodes oper)
		{
			owca_internal_string *s=operator_identificators[oper];
			s->gc_acquire();
			return s;
		}

		RCLMFUNCTION bool virtual_machine::calculate_iter_next(exec_variable *return_value, const exec_variable &param)
		{
			prepare_call_function(return_value,param,NULL,0);
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_generator(exec_variable *return_value, const exec_variable &param)
		{
			if (calculate_generator_nocall(return_value,param)) {
				return false;
			}
			prepare_call_operator(return_value,E_GENERATOR,&param);
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_str(exec_variable *return_value, const exec_variable &param)
		{
			owca_internal_string *s;
			if (calculate_str_nocall(this,s,param)) {
				return_value->set_string(s);
				return false;
			}
			prepare_call_operator(return_value,E_STR,&param);
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_hash(exec_variable *return_value, const exec_variable &param)
		{
			owca_int h;
			if (calculate_hash_nocall(this,h,param)) {
				return_value->set_int(h);
				return false;
			}
			prepare_call_operator(return_value,E_HASH,&param);
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_bool(exec_variable *return_value, const exec_variable &param)
		{
			bool b;
			if (calculate_bool_nocall(b,param)) {
				return_value->set_bool(b);
				return false;
			}
			prepare_call_operator(return_value,E_BOOL,&param);
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_eq(exec_variable *retval, const exec_variable *params)
		{
			if (calculate_eq_nocall(this,*retval,params[0],params[1])) return false;
			prepare_call_operator(retval,E_EQ,params);
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_noteq(exec_variable *retval, const exec_variable *params)
		{
			if (calculate_noteq_nocall(this,*retval,params[0],params[1])) return false;
			prepare_call_operator(retval,E_NOTEQ,params);
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_lesseq(exec_variable *retval, const exec_variable *params)
		{
			if (calculate_lesseq_nocall(this,*retval,params[0],params[1])) return false;
			prepare_call_operator(retval,E_LESSEQ,params);
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_moreeq(exec_variable *retval, const exec_variable *params)
		{
			if (calculate_moreeq_nocall(this,*retval,params[0],params[1])) return false;
			prepare_call_operator(retval,E_MOREEQ,params);
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_less(exec_variable *retval, const exec_variable *params)
		{
			if (calculate_less_nocall(this,*retval,params[0],params[1])) return false;
			prepare_call_operator(retval,E_LESS,params);
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_more(exec_variable *retval, const exec_variable *params)
		{
			if (calculate_more_nocall(this,*retval,params[0],params[1])) return false;
			prepare_call_operator(retval,E_MORE,params);
			return true;
		}


		RCLMFUNCTION bool virtual_machine::calculate_generator_nocall(exec_variable *retval, const exec_variable &param)
		{
			switch(param.mode()) {
			case VAR_GENERATOR:
				*retval=param;
				param.gc_acquire();
				return true;
			case VAR_FUNCTION:
			case VAR_FUNCTION_FAST:
			case VAR_NULL:
			case VAR_INT:
			case VAR_REAL:
			case VAR_BOOL:
			case VAR_STRING:
			case VAR_PROPERTY:
			case VAR_OBJECT:
			case VAR_WEAK_REF:
			case VAR_NAMESPACE:
				return false;
			default:
				RCASSERT(0);

			}
			return false;
		}

		RCLMFUNCTION bool virtual_machine::calculate_nocall(virtual_machine *self, exec_variable &retval, operatorcodes cmdoper, const exec_variable &left, const exec_variable &right)
		{
			switch(cmdoper) {
			case E_ADD_SELF: return calculate_add_nocall(self,retval,left,right);
			case E_SUB_SELF: return calculate_sub_nocall(self,retval,left,right);
			case E_MUL_SELF: return calculate_mul_nocall(self,retval,left,right);
			case E_DIV_SELF: return calculate_div_nocall(self,retval,left,right);
			case E_MOD_SELF: return calculate_mod_nocall(self,retval,left,right);
			case E_LSHIFT_SELF: return calculate_lshift_nocall(self,retval,left,right);
			case E_RSHIFT_SELF: return calculate_rshift_nocall(self,retval,left,right);
			case E_BIN_AND_SELF: return calculate_binand_nocall(self,retval,left,right);
			case E_BIN_OR_SELF: return calculate_binor_nocall(self,retval,left,right);
			case E_BIN_XOR_SELF: return calculate_binxor_nocall(self,retval,left,right);
			default:
				RCASSERT(0);
			}
			return false;
		}

		owca_internal_string *string_add(virtual_machine *vm, owca_internal_string *s1, owca_internal_string *s2);

		RCLMFUNCTION bool virtual_machine::calculate_add_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			switch(right.mode()==VAR_OBJECT ? VAR_OBJECT : left.mode()) {
			case VAR_INT:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_int(left.get_int()+right.get_int());
					break;
				case VAR_REAL:
					retval.set_real(left.get_int()+right.get_real());
					break;
				default:
					goto cont;
				}
				break;
			case VAR_REAL:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_real(left.get_real()+right.get_int());
					break;
				case VAR_REAL:
					retval.set_real(left.get_real()+right.get_real());
					break;
				default:
					goto cont;
				}
				break;
			case VAR_STRING:
				switch(right.mode()) {
				case VAR_STRING: {
					owca_internal_string *s=string_add(self,left.get_string(),right.get_string());
					retval.set_string(s);
					break; }
				default:
					goto cont;
				}
				break;
			default:
	cont:
				return false;
			}
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_sub_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			switch(right.mode()==VAR_OBJECT ? VAR_OBJECT : left.mode()) {
			case VAR_INT:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_int(left.get_int()-right.get_int());
					break;
				case VAR_REAL:
					retval.set_real(left.get_int()-right.get_real());
					break;
				default:
					goto cont;
				}
				break;
			case VAR_REAL:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_real(left.get_real()-right.get_int());
					break;
				case VAR_REAL:
					retval.set_real(left.get_real()-right.get_real());
					break;
				default:
					goto cont;
				}
				break;
			default:
	cont:
				return false;
			}
			return true;
		}

		owca_internal_string *string_mul(virtual_machine *vm, owca_internal_string *s, unsigned int i);

		RCLMFUNCTION bool virtual_machine::calculate_mul_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			owca_internal_string *ss;
			owca_int i;

			switch(right.mode()==VAR_OBJECT ? VAR_OBJECT : left.mode()) {
			case VAR_INT:
				i=left.get_int();
				switch(right.mode()) {
				case VAR_INT:
					retval.set_int(i*right.get_int());
					break;
				case VAR_REAL:
					retval.set_real(i*right.get_real());
					break;
				case VAR_STRING:
					ss=right.get_string();
process:
					if (i<0) {
neg:
						return false;
					}
					else {
						owca_internal_string *s=string_mul(self,ss,(unsigned int)i);
						retval.set_string(s);
					}
					break;
				default:
					goto cont;
				}
				break;
			case VAR_REAL:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_real(left.get_real()*right.get_int());
					break;
				case VAR_REAL:
					retval.set_real(left.get_real()*right.get_real());
					break;
				case VAR_STRING:
					i=(owca_int)left.get_real();
					if (i==left.get_real()) {
						ss=right.get_string();
						goto process;
					}
					else goto cont;
				default:
					goto cont;
				}
				break;
			case VAR_STRING:
				if (right.get_int(i)) {
					if (i<0) goto neg;
					owca_internal_string *s=string_mul(self,left.get_string(),(unsigned int)i);
					retval.set_string(s);
				}
				else goto cont;
				break;
			default:
	cont:
				return false;
			}
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_div_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			switch(right.mode()==VAR_OBJECT ? VAR_OBJECT : left.mode()) {
			case VAR_INT:
				switch(right.mode()) {
				case VAR_INT:
					if (right.get_int()==0) {
division_by_zero:
						return false;
					}
					else {
						if ((left.get_int()%right.get_int())==0) retval.set_int(left.get_int()/right.get_int());
						else retval.set_real((owca_real)left.get_int()/(owca_real)right.get_int());
					}
					break;
				case VAR_REAL:
					if (right.get_real()==0) goto division_by_zero;
					else retval.set_real(left.get_int()/right.get_real());
					break;
				default:
					goto cont;
				}
				break;
			case VAR_REAL:
				switch(right.mode()) {
				case VAR_INT:
					if (right.get_int()==0) goto division_by_zero;
					else retval.set_real(left.get_real()/right.get_int());
					break;
				case VAR_REAL:
					if (right.get_real()==0) goto division_by_zero;
					else retval.set_real(left.get_real()/right.get_real());
					break;
				default:
					goto cont;
				}
				break;

			default:
cont:
				return false;
			}
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_mod_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			owca_int i1,i2;
			if (left.get_int(i1) && right.get_int(i2)) {
				if (i2==0) return false;
				retval.set_int(i1%i2);
				return true;
			}
			else return false;
		}

		RCLMFUNCTION bool virtual_machine::calculate_lshift_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			owca_int i1,i2;
			if (left.get_int(i1) && right.get_int(i2)) {
				if (i2<0) return false;
				retval.set_int(i1 << i2);
				return true;
			}
			else return false;
		}

		RCLMFUNCTION bool virtual_machine::calculate_rshift_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			owca_int i1,i2;
			if (left.get_int(i1) && right.get_int(i2)) {
				if (i2<0) return false;
				retval.set_int(i1 >> i2);
				return true;
			}
			else return false;
		}

		RCLMFUNCTION bool virtual_machine::calculate_binxor_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			owca_int i1,i2;
			if (left.get_int(i1) && right.get_int(i2)) {
				retval.set_int(i1 ^ i2);
				return true;
			}
			return false;
		}

		RCLMFUNCTION bool virtual_machine::calculate_binor_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			owca_int i1,i2;
			if (left.get_int(i1) && right.get_int(i2)) {
				retval.set_int(i1 | i2);
				return true;
			}
			return false;
		}

		RCLMFUNCTION bool virtual_machine::calculate_binand_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			owca_int i1,i2;
			if (left.get_int(i1) && right.get_int(i2)) {
				retval.set_int(i1 & i2);
				return true;
			}
			return false;
		}
















		RCLMFUNCTION bool virtual_machine::calculate_eq_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			switch(right.mode()==VAR_OBJECT ? VAR_OBJECT : left.mode()) {
			case VAR_NULL:
				retval.set_bool(right.mode()==VAR_NULL);
				break;
			case VAR_BOOL:
				retval.set_bool(right.mode()==VAR_BOOL && right.get_bool()==left.get_bool());
				break;
			case VAR_INT:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_bool(left.get_int()==right.get_int());
					break;
				case VAR_REAL:
					retval.set_bool(left.get_int()==right.get_real());
					break;
				default:
					retval.set_bool(false);
					break;
				}
				break;
			case VAR_REAL:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_bool(left.get_real()==right.get_int());
					break;
				case VAR_REAL:
					retval.set_bool(left.get_real()==right.get_real());
					break;
				default:
					retval.set_bool(false);
					break;
				}
				break;
			case VAR_STRING:
				retval.set_bool(right.mode()==VAR_STRING && left.get_string()->compare(right.get_string())==0);
				break;
			case VAR_GENERATOR:
				retval.set_bool(right.mode()==VAR_GENERATOR && left.get_generator()==right.get_generator());
				break;
			case VAR_PROPERTY:
				retval.set_bool(right.mode()==VAR_PROPERTY && left.get_property()==right.get_property());
				break;
			case VAR_WEAK_REF:
				retval.set_bool(right.mode()==VAR_WEAK_REF && left.get_weak_ref()==right.get_weak_ref());
				break;
			case VAR_NAMESPACE:
				retval.set_bool(right.mode()==VAR_NAMESPACE && left.get_namespace()==right.get_namespace());
				break;
			case VAR_FUNCTION:
				switch(right.mode()) {
				case VAR_FUNCTION:
					if (left.get_function()->function()==right.get_function()->function()) {
						return calculate_eq_nocall(self,retval,left.get_function()->slf,right.get_function()->slf);
					}
					break;
				case VAR_FUNCTION_FAST:
					if (left.get_function()->function()==right.get_function_fast().fnc) {
						exec_variable tmp;
						tmp.set_object_null(right.get_function_fast().slf);
						return calculate_eq_nocall(self,retval,left.get_function()->slf,tmp);
					}
					break;
				case VAR_NULL:
				case VAR_INT:
				case VAR_REAL:
				case VAR_BOOL:
				case VAR_STRING:
				case VAR_GENERATOR:
				case VAR_PROPERTY:
				case VAR_OBJECT:
				case VAR_NAMESPACE:
				case VAR_WEAK_REF:
				case VAR_COUNT:
				case VAR_HASH_DELETED:
				case VAR_HASH_EMPTY:
				case VAR_NO_DEF_VALUE:
				case VAR_NO_PARAM_GIVEN:
				case VAR_UNDEFINED:
					break;
				}
				retval.set_bool(false);
				break;
			case VAR_FUNCTION_FAST:
				switch(right.mode()) {
				case VAR_FUNCTION:
					if (left.get_function_fast().fnc==right.get_function()->function()) {
						exec_variable tmp;
						tmp.set_object_null(left.get_function_fast().slf);
						return calculate_eq_nocall(self,retval,tmp,right.get_function()->slf);
					}
					break;
				case VAR_FUNCTION_FAST:
					if (left.get_function_fast().fnc==right.get_function_fast().fnc) {
						exec_variable tmp[2];
						tmp[0].set_object_null(left.get_function_fast().slf);
						tmp[1].set_object_null(right.get_function_fast().slf);
						return calculate_eq_nocall(self,retval,tmp[0],tmp[1]);
					}
					break;
				case VAR_NULL:
				case VAR_INT:
				case VAR_REAL:
				case VAR_BOOL:
				case VAR_STRING:
				case VAR_GENERATOR:
				case VAR_PROPERTY:
				case VAR_OBJECT:
				case VAR_NAMESPACE:
				case VAR_WEAK_REF:
				case VAR_COUNT:
				case VAR_HASH_DELETED:
				case VAR_HASH_EMPTY:
				case VAR_NO_DEF_VALUE:
				case VAR_NO_PARAM_GIVEN:
				case VAR_UNDEFINED:
					break;
				}
				retval.set_bool(false);
				break;
			case VAR_OBJECT:
				return false;
			default:
				RCASSERT(0);
			}
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_noteq_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			if (calculate_eq_nocall(self,retval,left,right)) {
				retval.set_bool(!retval.get_bool());
				return true;
			}
			return false;
		}

		RCLMFUNCTION bool virtual_machine::calculate_less_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			switch(right.mode()==VAR_OBJECT ? VAR_OBJECT : left.mode()) {
			case VAR_INT:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_bool(left.get_int()<right.get_int());
					break;
				case VAR_REAL:
					retval.set_bool(left.get_int()<right.get_real());
					break;
				default:
					return false;
				}
				break;
			case VAR_REAL:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_bool(left.get_real()<right.get_int());
					break;
				case VAR_REAL:
					retval.set_bool(left.get_real()<right.get_real());
					break;
				default:
					return false;
				}
				break;
			case VAR_STRING:
				if (right.mode()==VAR_STRING) {
					retval.set_bool(left.get_string()->compare(right.get_string())<0);
					break;
				}
				return false;
			case VAR_NULL:
			case VAR_BOOL:
			case VAR_GENERATOR:
			case VAR_PROPERTY:
			case VAR_FUNCTION:
			case VAR_FUNCTION_FAST:
			case VAR_OBJECT:
			case VAR_WEAK_REF:
			case VAR_NAMESPACE:
				return false;
			default:
				RCASSERT(0);
			}
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_more_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			switch(right.mode()==VAR_OBJECT ? VAR_OBJECT : left.mode()) {
			case VAR_INT:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_bool(left.get_int()>right.get_int());
					break;
				case VAR_REAL:
					retval.set_bool(left.get_int()>right.get_real());
					break;
				default:
					return false;
				}
				break;
			case VAR_REAL:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_bool(left.get_real()>right.get_int());
					break;
				case VAR_REAL:
					retval.set_bool(left.get_real()>right.get_real());
					break;
				default:
					return false;
				}
				break;
			case VAR_STRING:
				if (right.mode()==VAR_STRING) {
					retval.set_bool(left.get_string()->compare(right.get_string())>0);
					break;
				}
				return false;
			case VAR_NULL:
			case VAR_BOOL:
			case VAR_GENERATOR:
			case VAR_PROPERTY:
			case VAR_FUNCTION:
			case VAR_FUNCTION_FAST:
			case VAR_OBJECT:
			case VAR_WEAK_REF:
			case VAR_NAMESPACE:
				return false;
			default:
				RCASSERT(0);
			}
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_lesseq_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			switch(right.mode()==VAR_OBJECT ? VAR_OBJECT : left.mode()) {
			case VAR_INT:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_bool(left.get_int()<=right.get_int());
					break;
				case VAR_REAL:
					retval.set_bool(left.get_int()<=right.get_real());
					break;
				default:
					return false;
				}
				break;
			case VAR_REAL:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_bool(left.get_real()<=right.get_int());
					break;
				case VAR_REAL:
					retval.set_bool(left.get_real()<=right.get_real());
					break;
				default:
					return false;
				}
				break;
			case VAR_STRING:
				if (right.mode()==VAR_STRING) {
					retval.set_bool(left.get_string()->compare(right.get_string())<=0);
					break;
				}
				return false;
			case VAR_NULL:
			case VAR_BOOL:
			case VAR_GENERATOR:
			case VAR_PROPERTY:
			case VAR_FUNCTION:
			case VAR_FUNCTION_FAST:
			case VAR_OBJECT:
			case VAR_WEAK_REF:
			case VAR_NAMESPACE:
				return false;
			default:
				RCASSERT(0);
			}
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_moreeq_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right)
		{
			switch(right.mode()==VAR_OBJECT ? VAR_OBJECT : left.mode()) {
			case VAR_INT:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_bool(left.get_int()>=right.get_int());
					break;
				case VAR_REAL:
					retval.set_bool(left.get_int()>=right.get_real());
					break;
				default:
					return false;
				}
				break;
			case VAR_REAL:
				switch(right.mode()) {
				case VAR_INT:
					retval.set_bool(left.get_real()>=right.get_int());
					break;
				case VAR_REAL:
					retval.set_bool(left.get_real()>=right.get_real());
					break;
				default:
					return false;
				}
				break;
			case VAR_STRING:
				if (right.mode()==VAR_STRING) {
					retval.set_bool(left.get_string()->compare(right.get_string())>=0);
					break;
				}
				return false;
			case VAR_NULL:
			case VAR_BOOL:
			case VAR_GENERATOR:
			case VAR_PROPERTY:
			case VAR_FUNCTION:
			case VAR_FUNCTION_FAST:
			case VAR_OBJECT:
			case VAR_WEAK_REF:
			case VAR_NAMESPACE:
				return false;
			default:
				RCASSERT(0);
			}
			return true;
		}

		RCLMFUNCTION bool virtual_machine::calculate_str_nocall(virtual_machine *self, owca_internal_string *&retval, const exec_variable &param)
		{
			switch(param.mode()) {
			case VAR_NULL:
				retval=self->allocate_string("null"); return true;
			case VAR_INT:
				retval=self->allocate_string(int_to_string(param.get_int())); return true;
			case VAR_REAL:
				retval=self->allocate_string(real_to_string(param.get_real())); return true;
			case VAR_BOOL:
				retval=self->allocate_string(param.get_bool() ? "true" : "false"); return true;
			case VAR_STRING:
				retval=param.get_string();
				retval->gc_acquire();
				return true;
			case VAR_GENERATOR:
			case VAR_PROPERTY:
			case VAR_FUNCTION:
			case VAR_FUNCTION_FAST:
			case VAR_OBJECT:
			case VAR_WEAK_REF:
			case VAR_NAMESPACE:
				break;
			default:
				RCASSERT(0);
			}
			return false;
		}

		RCLMFUNCTION bool virtual_machine::calculate_hash_nocall(virtual_machine *self, owca_int &retval, const exec_variable &param)
		{
			switch(param.mode()) {
			case VAR_NULL:
				retval=reinterpret_cast<owca_int>(self->class_null);
				return true;
			case VAR_INT:
				retval=param.get_int();
				return true;
			case VAR_REAL: {
				owca_real r=param.get_real();
				owca_int ii=(owca_int)r;
				if ((owca_real)ii==r) retval=ii;
				else if (sizeof(owca_int)<=sizeof(owca_real)) retval=*(owca_int*)&r;
				else {
					hashfunc f;
					f.process(&r,sizeof(owca_real));
					retval=f.value();
				}
				return true; }
			case VAR_BOOL:
				retval=param.get_bool() ? reinterpret_cast<owca_int>(self->class_bool) : ~reinterpret_cast<owca_int>(self->class_bool);
				return true;
			case VAR_STRING:
				retval=param.get_string()->hash();
				return true;
			case VAR_FUNCTION_FAST: {
				exec_variable v;
				v.set_object_null(param.get_function_fast().slf);
				if (!calculate_hash_nocall(self,retval,v)) return false;
				retval^=reinterpret_cast<owca_int>(param.get_function_fast().fnc);
				return true; }
			case VAR_FUNCTION: {
				if (!calculate_hash_nocall(self,retval,param.get_function()->slf)) return false;
				retval^=reinterpret_cast<owca_int>(param.get_function()->function());
				return true; }
			case VAR_GENERATOR:
			case VAR_PROPERTY:
			case VAR_OBJECT:
			case VAR_WEAK_REF:
			case VAR_NAMESPACE:
				break;
			default:
				RCASSERT(0);
			}
			return false;
		}

		RCLMFUNCTION bool virtual_machine::calculate_bool_nocall(bool &retval, const exec_variable &param)
		{
			switch(param.mode()) {
			case VAR_NULL:
				retval=false; return true;
			case VAR_INT:
				retval=param.get_int()!=0; return true;
			case VAR_REAL:
				retval=param.get_real()!=0; return true;
			case VAR_BOOL:
				retval=param.get_bool(); return true;
			case VAR_STRING:
				retval=param.get_string()->data_size()!=0; return true;
			case VAR_GENERATOR:
			case VAR_PROPERTY:
			case VAR_FUNCTION_FAST:
			case VAR_FUNCTION:
			case VAR_WEAK_REF:
			case VAR_NAMESPACE:
				retval=true;
				return true;
			case VAR_OBJECT:
				break;
			default:
				RCASSERT(0);
			}
			return false;
		}

		owca_int virtual_machine::id(const exec_variable &v) const
		{
			switch(v.mode()) {
			case VAR_NULL:
			case VAR_INT:
			case VAR_REAL:
			case VAR_FUNCTION_FAST:
			case VAR_BOOL: return reinterpret_cast<owca_int>(&v);
			case VAR_STRING: return reinterpret_cast<owca_int>(v.get_string());
			case VAR_GENERATOR: return reinterpret_cast<owca_int>(v.get_generator());
			case VAR_PROPERTY: return reinterpret_cast<owca_int>(v.get_property());
			case VAR_FUNCTION: return reinterpret_cast<owca_int>(v.get_function());
			case VAR_OBJECT: return reinterpret_cast<owca_int>(v.get_object());
			case VAR_NAMESPACE: return reinterpret_cast<owca_int>(v.get_namespace());
			case VAR_WEAK_REF: return reinterpret_cast<owca_int>(v.get_weak_ref());
			default:
				RCASSERT(0);
				return 0;
			}
		}

		void virtual_machine::set_self(exec_variable &dst, exec_function_ptr *fnc, const exec_variable &slf)
		{
			RCASSERT(dst.mode()==VAR_NULL);
			switch(fnc->mode()==exec_function_ptr::F_INTERNAL ? fnc->internal_self_type() : exec_function_ptr::SELF) {
			case exec_function_ptr::NONE:
				break;
			case exec_function_ptr::SELF: {
				dst=slf;
				break; }
			case exec_function_ptr::CLASS: {
				switch(slf.mode()) {
				case VAR_NULL:
					break;
				case VAR_OBJECT: {
					exec_object *o=slf.get_object();
					dst.set_object(o->is_type() ? o : o->type());
					break; }
				default:
					RCASSERT(basicmap[slf.mode()]);
					dst.set_object(basicmap[slf.mode()]);
				}
				break; }
			default:
				RCASSERT(0);
			}
		}

		void virtual_machine::set_self(exec_variable &dst, exec_function_ptr *fnc, exec_object *slf)
		{
			RCASSERT(dst.mode()==VAR_NULL);
			switch(fnc->mode()==exec_function_ptr::F_INTERNAL ? fnc->internal_self_type() : exec_function_ptr::SELF) {
			case exec_function_ptr::NONE:
				break;
			case exec_function_ptr::SELF: {
				dst.set_object_null(slf);
				break; }
			case exec_function_ptr::CLASS:
				if (slf) dst.set_object(slf->is_type() ? slf : slf->type());
				else dst.set_null();
				break;
			default:
				RCASSERT(0);
			}
		}

		void virtual_machine::_allocated(exec_base *b)
		{
			RCASSERT(allocated_objects);

			b->_gc_next=allocated_objects;
			b->_gc_prev=allocated_objects->_gc_prev;
			b->_gc_next->_gc_prev=b->_gc_prev->_gc_next=b;
		}

		void virtual_machine::purge_memory()
		{
			f2.purge();
			f4.purge();
			f6.purge();
			f8.purge();
			f12.purge();
			f16.purge();
			f24.purge();
			f32.purge();
			f48.purge();
			f64.purge();
		}

		void *virtual_machine::_allocate_memory(unsigned int size)
		{ // allocates memory block of size size
			unsigned int *c;
#ifdef RCDEBUG_MEMORY_BLOCKS
			unsigned int newsize=size+sizeof(unsigned int)*3;
#else
			unsigned int newsize=size+sizeof(unsigned int)*1;
#endif
			//c=(unsigned int*)new char[newsize];
			if (newsize>f64.blocksize()) c=(unsigned int*)new char[newsize];
			else if (newsize>f48.blocksize()) c=(unsigned int*)f64.alloc();
			else if (newsize>f32.blocksize()) c=(unsigned int*)f48.alloc();
			else if (newsize>f24.blocksize()) c=(unsigned int*)f32.alloc();
			else if (newsize>f16.blocksize()) c=(unsigned int*)f24.alloc();
			else if (newsize>f12.blocksize()) c=(unsigned int*)f16.alloc();
			else if (newsize>f8.blocksize()) c=(unsigned int*)f12.alloc();
			else if (newsize>f6.blocksize()) c=(unsigned int*)f8.alloc();
			else if (newsize>f4.blocksize()) c=(unsigned int*)f6.alloc();
			else if (newsize>f2.blocksize()) c=(unsigned int*)f4.alloc();
			else c=(unsigned int*)f2.alloc();

#ifdef RCDEBUG_MEMORY_BLOCKS
			c[0]=0x7ffc1234^(unsigned int)reinterpret_cast<unsigned long long>(c);
			c[1]=newsize;
			*(unsigned int*)(((char*)c)+newsize-sizeof(unsigned int))=0x5678c77f^(unsigned int)reinterpret_cast<unsigned long long>(c);
			unsigned char *ptr=(unsigned char*)(c+2);
			for(unsigned int i=0;i<size;++i) ptr[i]=0xfa;
			return (void*)(c+2);
#else
			c[0]=newsize;
			return (void*)(c+1);
#endif
		}

		void virtual_machine::free_memory(void *p)
		{
			RCASSERT(p != NULL);

			exec_ref_counter_check((exec_base_exist*)p);
#ifdef RCDEBUG_MEMORY_BLOCKS
			unsigned int *c=((unsigned int*)p)-2;
			RCASSERT(c[0]==(0x7ffc1234^(unsigned int)reinterpret_cast<unsigned long long>(c)));
			RCASSERT(*(unsigned int*)(((char*)c)+c[1]-sizeof(unsigned int))==(0x5678c77f^(unsigned int)reinterpret_cast<unsigned long long>(c)));
			const unsigned int size=c[1];
			const unsigned obj_size = size - sizeof(unsigned int)* 3;
#else
			unsigned int *c=((unsigned int*)p)-1;
			const unsigned int size = c[0];
			const unsigned obj_size = size - sizeof(unsigned int);
#endif

#ifdef RCDEBUG
            unsigned int idobject = ((exec_base_exist*)p)->debug_ident;
			memset(c, 0xfa, obj_size);
            ((exec_base_exist*)p)->debug_ident = idobject;
#endif

			if (size>f64.blocksize()) {
				delete [] (char*)c;
				return;
			}
			else {
				if (size>f48.blocksize()) f64.release(c);
				else if (size>f32.blocksize()) f48.release(c);
				else if (size>f24.blocksize()) f32.release(c);
				else if (size>f16.blocksize()) f24.release(c);
				else if (size>f12.blocksize()) f16.release(c);
				else if (size>f8.blocksize()) f12.release(c);
				else if (size>f6.blocksize()) f8.release(c);
				else if (size>f4.blocksize()) f6.release(c);
				else if (size>f2.blocksize()) f4.release(c);
				else f2.release(c);
			}
		}

		//void exec_ref_counter_check(exec_base_exist *);

		void *virtual_machine::_data_from_object(exec_object *o, const structid_type &type) const
		{
			exec_object *tp=find_type(type);
			if (tp==NULL) return NULL;
			exec_class_object &co=o->type_->CO();
			unsigned int *v=co.offsetmap[tp];
			if (!v) return NULL;
			return o->_getptr(*v);
		}







		bool virtual_machine::ensure_no_map_params(exec_map_object *mp)
		{
			if (mp) {
				exec_map_object_iterator mi;
				if (mp->map.next(mi)) {
					exec_variable &v=mp->map.getkey(mi).k;
					if (v.mode()==VAR_STRING) raise_unused_keyword_param(v.get_string());
					else raise_keowca_map_param_not_string();
					return false;
				}
			}
			return true;
		}

		exec_object *virtual_machine::_raise_get_exception_type(ExceptionCode code)
		{
			switch(code) {
			case ExceptionCode::INTEGER_OUT_OF_BOUNDS:
			case ExceptionCode::INVALID_LIST_PARAM:
			case ExceptionCode::INVALID_MAP_PARAM:
			case ExceptionCode::INVALID_PARAM_TYPE:
			case ExceptionCode::CLASS_CREATION:
			case ExceptionCode::PARAM_ASSIGNED_TWICE:
			case ExceptionCode::PARAM_NOT_SET:
			case ExceptionCode::UNUSED_KEYWORD_PARAM:
			case ExceptionCode::NO_CONSTRUCTOR:
			case ExceptionCode::TOO_MANY_PARAMETERS:
			case ExceptionCode::NOT_ENOUGH_PARAMETERS:
			case ExceptionCode::UNSUPPORTED_KEYWORD_PARAMETERS:
			case ExceptionCode::INVALID_VM:
			case ExceptionCode::KEYWORD_PARAM_NOT_STRING:
			case ExceptionCode::MISSING_VALUE_PARAMETER:
			case ExceptionCode::MISSING_KEY_PARAMETER:
				return class_exception_param;
			case ExceptionCode::DIVISION_BY_ZERO:
			case ExceptionCode::OVERFLOW:
				return class_exception_math;
			case ExceptionCode::TOO_MANY_ITEMS_IN_ITER:
			case ExceptionCode::TOO_LITTLE_ITEMS_IN_ITER:
			case ExceptionCode::INVALID_RETURN_TYPE:
			case ExceptionCode::INVALID_OPERATOR_FUNCTION:
			case ExceptionCode::MISSING_RETURN_VALUE:
			case ExceptionCode::STACK_OVERFLOW:
			case ExceptionCode::CANT_INSERT:
			case ExceptionCode::KEY_NOT_FOUND:
			case ExceptionCode::NO_COROUTINE_TO_STOP:
			case ExceptionCode::CANT_STOP_FROM_WITHIN_USER_FUNCTION:
			case ExceptionCode::CANT_RESUME_FROM_COROUTINE:
			case ExceptionCode::CANT_RESUME_NORMAL_FUNCTION:
			case ExceptionCode::CANT_RESUME_FINISHED_COROUTINE:
			case ExceptionCode::CANT_STOP_COROUTINE_FROM_USER_FUNCTION:
			case ExceptionCode::CANT_CREATE_GENERATOR_FROM_USER_FUNCTION:
			case ExceptionCode::LIST_MODIFED_WHILE_BEING_SORTED:
			case ExceptionCode::MAP_MODIFED_WHILE_BEING_USED:
			case ExceptionCode::SET_MODIFED_WHILE_BEING_USED:
				return class_exception_operation;
			case ExceptionCode::MISSING_MEMBER:
			case ExceptionCode::NOT_LVALUE:
			case ExceptionCode::NOT_RVALUE:
			case ExceptionCode::INVALID_IDENT:
				return class_exception_access;
			case ExceptionCode::USER:
			default:
				RCASSERT(code>=ExceptionCode::USER);
				return class_exception;
			}
		}

		void virtual_machine::_raise_from_user(const exec_variable &g)
		{
			if (g.mode()!=VAR_OBJECT || !g.get_object()->type()->inherit_from(class_exception)) {
				push_execution_stack();
				raise_invalid_param(g,class_exception);
				executionreturnvalue r=execute_stack();
				pop_execution_stack();
				RCASSERT(r==VME_EXCEPTION);
				execution_exception_object_thrown=execution_exception_object_temp.get_object();
				execution_exception_object_temp.reset();
			}
			else {
				RCASSERT(execution_exception_object_thrown==NULL);
				execution_exception_object_thrown=g.get_object();
				execution_exception_object_thrown->gc_acquire();
			}
		}

		void virtual_machine::_prepare_construct_exception(ExceptionCode code, exec_object *exctype, const std::string &txt)
		{
			if (exctype==NULL) exctype=_raise_get_exception_type(code);

			RCASSERT(exctype->is_type() && exctype->inherit_from(class_exception));

			execution_exception_parameters[0].set_object(exctype);
			execution_exception_parameters[1].set_string(allocate_string(txt));
			execution_exception_parameters[2].set_int((owca_int)code);
			execution_exception_object_temp.gc_release(*this);
			execution_exception_object_temp.reset();
			RCASSERT(prepare_call_function(&execution_exception_object_temp,execution_exception_parameters[0],&execution_exception_parameters[1],2));
		}

		RCLMFUNCTION void virtual_machine::_raise(ExceptionCode code, exec_object *exctype, const std::string &txt)
		{
			_prepare_construct_exception(code,exctype,txt);

			RCASSERT(execution_stack->peek_frame()->return_handling_mode==vm_execution_stack_elem_base::RETURN_HANDLING_NONE);
			execution_stack->peek_frame()->return_handling_mode=vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION;
		}

		void virtual_machine::raise_overflow()
		{
			_raise(ExceptionCode::OVERFLOW,NULL,OWCA_ERROR_FORMAT("an operation has caused numeric overlow"));
		}

		void virtual_machine::raise_division_by_zero()
		{
			_raise(ExceptionCode::DIVISION_BY_ZERO,NULL,OWCA_ERROR_FORMAT("division by zero"));
		}

		void virtual_machine::raise_invalid_list_param(const exec_variable &p)
		{
			_raise(ExceptionCode::INVALID_LIST_PARAM,NULL,OWCA_ERROR_FORMAT1("list parameter must be a list or tuple, not %1",to_stdstring_type(p)));
		}

		void virtual_machine::raise_invalid_map_param(const exec_variable &p)
		{
			_raise(ExceptionCode::INVALID_MAP_PARAM,NULL,OWCA_ERROR_FORMAT1("map parameter must be a map, not %1",to_stdstring_type(p)));
		}

		void virtual_machine::raise_invalid_integer(const std::string &txt)
		{
			_raise(ExceptionCode::INTEGER_OUT_OF_BOUNDS,NULL,txt);
		}

		void virtual_machine::raise_invalid_param(const std::string &txt)
		{
			_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,txt);
		}

		//void virtual_machine::raise_invalid_param(const exec_variable &o) // o has an invalid type
		//{
		//	_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("%1 is not of valid type",to_stdstring(o)));
		//}

		void virtual_machine::raise_invalid_param(const std::string &o, exec_object *p) // o is not a type of p
		{
			if (p==class_class) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("parameter '%1' is not a type",o));
			else if (p==class_generator) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("parameter '%1' is not a generator",o));
			else if (p==class_exception) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT2("parameter '%1' doesnt inherit from type ",o,class_exception->CO().name->str()));
			else if (p==class_int) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("parameter '%1' is not an integer",o));
			else if (p==class_real) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("parameter '%1' is not a real",o));
			else if (p==class_string) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("parameter '%1' is not a string",o));
			else if (p==class_function) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("parameter '%1' is not a function",o));
			else return _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT2("parameter '%1' is not of type %2",o,p->CO().name->str()));
		}

		void virtual_machine::raise_invalid_param(const exec_variable &o, exec_object *p) // o is not a type of p
		{
			if (p==class_class) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("%1 is not a type",to_stdstring_type(o)));
			else if (p==class_generator) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("%1 is not a generator",to_stdstring_type(o)));
			else if (p==class_exception) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT2("%1 doesnt inherit from type %2",to_stdstring_type(o),class_exception->CO().name->str()));
			else if (p==class_int) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("%1 is not an integer",to_stdstring_type(o)));
			else if (p==class_real) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("%1 is not a real",to_stdstring_type(o)));
			else if (p==class_string) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("%1 is not a string",to_stdstring_type(o)));
			else if (p==class_function) _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("%1 is not a function",to_stdstring_type(o)));
			else return _raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT2("%1 is not of type %2",to_stdstring_type(o),p->CO().name->str()));
		}

		void virtual_machine::raise_class_creation(const std::string &z)
		{
			_raise(ExceptionCode::CLASS_CREATION,NULL,z);
		}

		void virtual_machine::raise_param_not_set(owca_internal_string *id)
		{
			_raise(ExceptionCode::PARAM_NOT_SET,NULL,OWCA_ERROR_FORMAT1("parameter %1 not assigned",id->str()));
		}

		void virtual_machine::raise_keowca_map_param_not_string()
		{
			_raise(ExceptionCode::KEYWORD_PARAM_NOT_STRING,NULL,OWCA_ERROR_FORMAT("keys for map parameter must be strings"));
		}

		void virtual_machine::raise_missing_key_parameter()
		{
			_raise(ExceptionCode::MISSING_KEY_PARAMETER,NULL,OWCA_ERROR_FORMAT("missing key parameter in key - value pair"));
		}

		void virtual_machine::raise_missing_value_parameter()
		{
			_raise(ExceptionCode::MISSING_VALUE_PARAMETER,NULL,OWCA_ERROR_FORMAT("missing value parameter in key - value pair"));
		}

		void virtual_machine::raise_unused_keyword_param(owca_internal_string *id)
		{
			if (id) _raise(ExceptionCode::UNUSED_KEYWORD_PARAM,NULL,OWCA_ERROR_FORMAT1("unused keyword parameter %1",id->str()));
			else _raise(ExceptionCode::UNUSED_KEYWORD_PARAM,NULL,OWCA_ERROR_FORMAT("keyword parameter is not a string"));
		}

		void virtual_machine::raise_param_assigned_twice(owca_internal_string *id, int index)
		{
			if (id==NULL && index<0) _raise(ExceptionCode::PARAM_ASSIGNED_TWICE,NULL,OWCA_ERROR_FORMAT("keyword parameter assigned more than once"));
			else _raise(ExceptionCode::PARAM_ASSIGNED_TWICE,NULL,OWCA_ERROR_FORMAT1("parameter %1 assigned more than once",(id ? id->str() : "num "+int_to_string(index))));
		}

		void virtual_machine::raise_unsupported_keyword_parameters()
		{
			_raise(ExceptionCode::UNSUPPORTED_KEYWORD_PARAMETERS,NULL,OWCA_ERROR_FORMAT("unexpected keyword parameters"));
		}

		void virtual_machine::raise_not_enough_parameters(exec_function_ptr *fnc)
		{
			_raise(ExceptionCode::NOT_ENOUGH_PARAMETERS,NULL,OWCA_ERROR_FORMAT1("not enough parameters for function %1",fnc->name()->str()));
		}

		void virtual_machine::raise_no_constructor()
		{
			_raise(ExceptionCode::NO_CONSTRUCTOR,NULL,OWCA_ERROR_FORMAT("constructor is missing"));
		}

		void virtual_machine::raise_too_many_parameters(exec_function_ptr *fnc)
		{
			_raise(ExceptionCode::TOO_MANY_PARAMETERS,NULL,OWCA_ERROR_FORMAT1("too many parameters for function %1",fnc->name()->str()));
		}

		void virtual_machine::raise_too_many_parameters(const std::string &txt)
		{
			_raise(ExceptionCode::TOO_MANY_PARAMETERS,NULL,txt);
		}

		void virtual_machine::raise_invalid_ident(const std::string &id) // id is not valid string for an ident
		{
			_raise(ExceptionCode::INVALID_IDENT,NULL,OWCA_ERROR_FORMAT1("'%1' is not a valid identificator",id));
		}

		void virtual_machine::raise_not_rvalue_member(const exec_variable &v, const std::string &txt) // v.txt is not rvalue
		{
			_raise(ExceptionCode::NOT_RVALUE,NULL,OWCA_ERROR_FORMAT1("member %1 is not a rvalue",txt));
		}

		void virtual_machine::raise_not_lvalue_member(const exec_variable &v, const std::string &txt) // v.txt is not lvalue
		{
			_raise(ExceptionCode::NOT_LVALUE,NULL,OWCA_ERROR_FORMAT1("member %1 is not a lvalue",txt));
		}

		void virtual_machine::raise_missing_member(const exec_variable &o, const std::string &txt)
		{
			_raise(ExceptionCode::MISSING_MEMBER,NULL,OWCA_ERROR_FORMAT2("%1 is missing member %2",to_stdstring_type(o),txt));
		}

		void virtual_machine::raise_missing_member(exec_object *n, const std::string &txt)
		{
			_raise(ExceptionCode::MISSING_MEMBER,NULL,OWCA_ERROR_FORMAT2("%1 is missing member %2",to_stdstring_type(n),txt));
		}

		void virtual_machine::raise_invalid_operator_function(exec_function_ptr *fnc)
		{
			switch(fnc->mode()) {
			case exec_function_ptr::F_INTERNAL:
				RCASSERT(fnc->internal_map_param().location.valid() || fnc->internal_list_param().location.valid());
			case exec_function_ptr::F_FAST:
			case exec_function_ptr::F_SELF:
				_raise(ExceptionCode::INVALID_OPERATOR_FUNCTION,NULL,OWCA_ERROR_FORMAT("operator function cant take list / map parameters"));
				break;
			case exec_function_ptr::F_FAST_0:
			case exec_function_ptr::F_FAST_1:
			case exec_function_ptr::F_FAST_2:
			case exec_function_ptr::F_FAST_3:
				_raise(ExceptionCode::INVALID_OPERATOR_FUNCTION,NULL,OWCA_ERROR_FORMAT("operator function must accept self parameter"));
				break;
			case exec_function_ptr::F_SELF_0:
			case exec_function_ptr::F_SELF_1:
			case exec_function_ptr::F_SELF_2:
			case exec_function_ptr::F_SELF_3:
				break;
			default:
				RCASSERT(0);
			}
		}

		void virtual_machine::raise_missing_return_value()
		{
			_raise(ExceptionCode::MISSING_RETURN_VALUE,NULL,OWCA_ERROR_FORMAT("function failed to return any value"));
		}

		void virtual_machine::raise_map_modified_while_being_used()
		{
			_raise(ExceptionCode::MAP_MODIFED_WHILE_BEING_USED,NULL,OWCA_ERROR_FORMAT("map was modified while it was being used"));
		}

		void virtual_machine::raise_set_modified_while_being_used()
		{
			_raise(ExceptionCode::SET_MODIFED_WHILE_BEING_USED,NULL,OWCA_ERROR_FORMAT("set was modified while it was being used")); // correct eng gramar?
		}

		void virtual_machine::raise_list_modified_while_being_sorted()
		{
			_raise(ExceptionCode::LIST_MODIFED_WHILE_BEING_SORTED,NULL,OWCA_ERROR_FORMAT("list was modified while being sorted"));
		}

		void virtual_machine::raise_too_much_iter_obj()
		{
			_raise(ExceptionCode::TOO_MANY_ITEMS_IN_ITER,NULL,OWCA_ERROR_FORMAT("too many elements"));
		}

		void virtual_machine::raise_too_little_iter_obj()
		{
			_raise(ExceptionCode::TOO_LITTLE_ITEMS_IN_ITER,NULL,OWCA_ERROR_FORMAT("not enough elements"));
		}

		void virtual_machine::raise_invalid_return_type(const exec_variable &o, exec_object *p) // return type: o is not a type of p
		{
			if (p) _raise(ExceptionCode::INVALID_RETURN_TYPE,NULL,OWCA_ERROR_FORMAT2("return value %1 is not of type %2",to_stdstring_type(o),to_stdstring_type(p)));
			else _raise(ExceptionCode::INVALID_RETURN_TYPE,NULL,OWCA_ERROR_FORMAT1("unexpected %1 as return value",to_stdstring_type(o)));
		}

		void virtual_machine::raise_invalid_key()
		{
			_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT("key not found"));
		}

		void virtual_machine::raise_stack_overflow()
		{
			execution_stack->check_stack_overflow=false;
			_raise(ExceptionCode::STACK_OVERFLOW,NULL,OWCA_ERROR_FORMAT("maximum stack depth reached"));
		}

		void virtual_machine::raise_unsupported_operation()
		{
			_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT("unsupported operation"));
		}

		void virtual_machine::raise_unsupported_call_operation(const exec_variable &fnc)
		{
			_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,OWCA_ERROR_FORMAT1("%1 is not callable",to_stdstring_type(fnc)));
		}

		void virtual_machine::raise_cant_insert()
		{
			_raise(ExceptionCode::CANT_INSERT,NULL,OWCA_ERROR_FORMAT("cant insert into map (map possibly modified while inserting?)"));
		}

		void virtual_machine::raise_key_not_found()
		{
			_raise(ExceptionCode::KEY_NOT_FOUND,NULL,OWCA_ERROR_FORMAT("key not found"));
		}

		void virtual_machine::raise_cant_stop_from_within_user_function()
		{
			_raise(ExceptionCode::CANT_STOP_FROM_WITHIN_USER_FUNCTION,NULL,OWCA_ERROR_FORMAT("cant stop coroutine with user function call on stack"));
		}

		void virtual_machine::raise_no_coroutine_to_stop()
		{
			_raise(ExceptionCode::NO_COROUTINE_TO_STOP,NULL,OWCA_ERROR_FORMAT("no coroutine is running, nothing to stop"));
		}

		void virtual_machine::raise_cant_resume_from_coroutine()
		{
			_raise(ExceptionCode::CANT_RESUME_FROM_COROUTINE,NULL,OWCA_ERROR_FORMAT("cant resume from coroutine"));
		}

		void virtual_machine::raise_cant_resume_normal_function()
		{
			_raise(ExceptionCode::CANT_RESUME_NORMAL_FUNCTION,NULL,OWCA_ERROR_FORMAT("cant resume a function"));
		}

		void virtual_machine::raise_cant_resume_fnished_coroutine()
		{
			_raise(ExceptionCode::CANT_RESUME_FINISHED_COROUTINE,NULL,OWCA_ERROR_FORMAT("cant resume finished coroutine"));
		}

		void virtual_machine::raise_cant_stop_coroutine_from_user_function()
		{
			_raise(ExceptionCode::CANT_STOP_COROUTINE_FROM_USER_FUNCTION,NULL,OWCA_ERROR_FORMAT("cant yield coroutine from within simple user function"));
		}

		void virtual_machine::raise_cant_create_generator_from_user_function()
		{
			_raise(ExceptionCode::CANT_CREATE_GENERATOR_FROM_USER_FUNCTION,NULL,OWCA_ERROR_FORMAT("cant create generator out of simple user function"));
		}

		const char *operatorcodes_ident(operatorcodes opc);
		unsigned char operator_operand_count[]={
		/*	E_EQ             */ 2,
		/*	E_NOTEQ          */ 2,
		/*	E_LESSEQ         */ 2,
		/*	E_MOREEQ         */ 2,
		/*	E_LESS           */ 2,
		/*	E_MORE           */ 2,
		/*	E_EQ_R           */ 2,
		/*	E_NOTEQ_R        */ 2,
		/*	E_LESSEQ_R       */ 2,
		/*	E_MOREEQ_R       */ 2,
		/*	E_LESS_R         */ 2,
		/*	E_MORE_R         */ 2,
		/*	E_ADD            */ 2,
		/*	E_SUB            */ 2,
		/*	E_MUL            */ 2,
		/*	E_DIV            */ 2,
		/*	E_MOD            */ 2,
		/*	E_LSHIFT         */ 2,
		/*	E_RSHIFT         */ 2,
		/*	E_BIN_AND        */ 2,
		/*	E_BIN_OR         */ 2,
		/*	E_BIN_XOR        */ 2,
		/*	E_ADD_R          */ 2,
		/*	E_SUB_R          */ 2,
		/*	E_MUL_R          */ 2,
		/*	E_DIV_R          */ 2,
		/*	E_MOD_R          */ 2,
		/*	E_LSHIFT_R       */ 2,
		/*	E_RSHIFT_R       */ 2,
		/*	E_BIN_AND_R      */ 2,
		/*	E_BIN_OR_R       */ 2,
		/*	E_BIN_XOR_R      */ 2,
		/*	E_IN             */ 2,
		/*	E_ADD_SELF       */ 2,
		/*	E_SUB_SELF       */ 2,
		/*	E_MUL_SELF       */ 2,
		/*	E_DIV_SELF       */ 2,
		/*	E_MOD_SELF       */ 2,
		/*	E_LSHIFT_SELF    */ 2,
		/*	E_RSHIFT_SELF    */ 2,
		/*	E_BIN_AND_SELF   */ 2,
		/*	E_BIN_OR_SELF    */ 2,
		/*	E_BIN_XOR_SELF   */ 2,
		/*	E_ACCESS_1_READ  */ 2,
		/*	E_ACCESS_1_WRITE */ 3,
		/*	E_ACCESS_2_READ  */ 3,
		/*	E_ACCESS_2_WRITE */ 4,
		/*	E_BIN_NOT        */ 1,
		/*	E_BOOL           */ 1,
		/*	E_SIGN_CHANGE    */ 1,
		/*	E_CALL           */ 0,
		/*	E_GENERATOR      */ 1,
		/*	E_INIT           */ 0,
		/*	E_NEW            */ 0,
		/*	E_STR            */ 1,
		/*	E_HASH           */ 1,
		/*	E_WITH_EXIT      */ 1,
		/*	E_WITH_ENTER     */ 1,
			};

		void virtual_machine::raise_unsupported_operation(operatorcodes oper, const exec_variable *params)
		{
			switch(oper) {
			case E_SUB_SELF:
			case E_SUB_R:
			case E_SUB:
			case E_RSHIFT_SELF:
			case E_RSHIFT_R:
			case E_RSHIFT:
			case E_NOTEQ_R:
			case E_NOTEQ:
			case E_MUL_SELF:
			case E_MUL_R:
			case E_MUL:
			case E_MORE_R:
			case E_MOREEQ_R:
			case E_MOREEQ:
			case E_MORE:
			case E_MOD_SELF:
			case E_MOD_R:
			case E_MOD:
			case E_LSHIFT_SELF:
			case E_LSHIFT_R:
			case E_LSHIFT:
			case E_LESS_R:
			case E_LESSEQ_R:
			case E_LESSEQ:
			case E_LESS:
			case E_EQ_R:
			case E_EQ:
			case E_DIV_SELF:
			case E_DIV_R:
			case E_DIV:
			case E_BIN_XOR_SELF:
			case E_BIN_XOR_R:
			case E_BIN_XOR:
			case E_BIN_OR_SELF:
			case E_BIN_OR_R:
			case E_BIN_OR:
			case E_BIN_AND_SELF:
			case E_BIN_AND_R:
			case E_BIN_AND:
			case E_ADD_SELF:
			case E_ADD_R:
			case E_ADD: {
				const char *ident=operatorcodes_ident(oper);
				RCASSERT(ident);
				_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,to_stdstring_type_short(params[0])+" "+std::string(ident)+" "+to_stdstring_type_short(params[1])+" is not supported");
				break; }
			case E_IN: {
				const char *ident=operatorcodes_ident(oper);
				RCASSERT(ident);
				_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,to_stdstring_type_short(params[1])+" "+std::string(ident)+" "+to_stdstring_type_short(params[0])+" is not supported");
				break; }
			case E_SIGN_CHANGE:
			case E_BIN_NOT: {
				const char *ident=operatorcodes_ident(oper);
				RCASSERT(ident);
				_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,std::string(ident)+" "+to_stdstring_type_short(params[0])+" is not supported");
				break; }
			case E_NEW:
			case E_INIT:
				RCASSERT(0);
			case E_WITH_ENTER:
				_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,to_stdstring_type_short(params[0])+" doesnt support 'with' protocol, $withenter operator is missing");
				break;
			case E_WITH_EXIT:
				_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,to_stdstring_type_short(params[0])+" doesnt support 'with' protocol, $withexit operator is missing");
				break;
			case E_STR:
			case E_HASH:
			case E_GENERATOR:
			case E_BOOL:
			case E_CALL:
				_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,to_stdstring_type_short(params[0])+" doesnt support "+operator_identificators[oper]->str()+" operator");
				break;
			case E_ACCESS_2_WRITE:
				_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,to_stdstring_type_short(params[0])+" ["+to_stdstring_type_short(params[1])+" : "+to_stdstring_type_short(params[2])+"] = "+to_stdstring_type_short(params[3])+" is not supported");
				break;
			case E_ACCESS_2_READ:
				_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,to_stdstring_type_short(params[0])+" ["+to_stdstring_type_short(params[1])+" : "+to_stdstring_type_short(params[2])+"] is not supported");
				break;
			case E_ACCESS_1_WRITE:
				_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,to_stdstring_type_short(params[0])+" ["+to_stdstring_type_short(params[1])+"] = "+to_stdstring_type_short(params[2])+" is not supported");
				break;
			case E_ACCESS_1_READ:
				_raise(ExceptionCode::INVALID_PARAM_TYPE,NULL,to_stdstring_type_short(params[0])+" ["+to_stdstring_type_short(params[1])+"] is not supported");
				break;
			default:
				RCASSERT(0);
			}
		}







		std::vector<unsigned char> virtual_machine::compile(owca_message_list &errorswarnings, const owca_source_file &fs, const compile_visible_items &visible_names)
		{
			compiler c(*this,errorswarnings);

			return c.compile(fs,visible_names);
		}

		void virtual_machine::remove_type(structid_type type)
		{
			structid_to_type_object.erase(type);
		}

		exec_object *virtual_machine::find_type(structid_type type) const
		{
			std::map<structid_type,exec_object *>::const_iterator it=structid_to_type_object.find(type);
			RCASSERT(it!=structid_to_type_object.end());
			return it->second;
		}

		void virtual_machine::_registerstructid(structid_type structid, exec_object *type)
		{
			RCASSERT(structid_to_type_object.find(structid)==structid_to_type_object.end());
			structid_to_type_object[structid]=type;
			//type->gc_acquire();
		}

		exec_object *virtual_machine::allocate_object(exec_object *type, unsigned int oversize)
		{
			exec_object *oo=new (allocate_memory(sizeof(exec_object)+type->CO().total_storage_size+oversize,typeid(exec_object))) exec_object(type);
			_allocated(oo);
			for(unsigned int i=0;i<type->CO().offsetmap.size();++i) {
				void (*c)(void *, unsigned int oversize, virtual_machine &vm)=type->CO().offsetmap.get_key(i)->CO().constructor;
				if (c) {
					c(oo->_getptr(type->CO().offsetmap.get_value(i)),oversize,*this);
				}
			}
			return oo;
		}
		exec_stack_variables *virtual_machine::allocate_stack_variables(unsigned int size, exec_function_ptr *owner)
		{
			exec_stack_variables *s=new (allocate_memory(exec_stack_variables::calculate_size(size),typeid(exec_stack_variables))) exec_stack_variables(size,owner);
#ifdef RCDEBUG
			s->vars=size ? &s->get(0) : NULL;
#endif
			for(unsigned int i=0;i<size;++i) s->get(i).reset();
			return s;
		}
		exec_function_stack_data *virtual_machine::allocate_function_stackdata(unsigned int size)
		{
			exec_function_stack_data *s=new (allocate_memory(sizeof(exec_function_stack_data)+size,typeid(exec_function_stack_data))) exec_function_stack_data(size);
			_allocated(s);
			return s;
		}
		exec_function_ptr *virtual_machine::allocate_function_ptr(unsigned int size)
		{
			exec_function_ptr *p=new (allocate_memory(sizeof(exec_function_ptr)+size,typeid(exec_function_ptr))) exec_function_ptr();
			_allocated(p);
			return p;
		}
		exec_stack *virtual_machine::allocate_stack(exec_namespace *ns, unsigned int *level0map)
		{
			exec_stack *s=new (allocate_memory(sizeof(exec_stack),typeid(exec_stack))) exec_stack(ns,level0map);
			_allocated(s);
			return s;
		}
		exec_stack *virtual_machine::allocate_stack(exec_stack *prevst, unsigned int local_var_count, exec_function_ptr *owner)
		{
			exec_stack *s=new (allocate_memory(exec_stack::calculate_size(prevst),typeid(exec_stack))) exec_stack(prevst);
			_allocated(s);
			s->set_variables(s->size(),allocate_stack_variables(local_var_count,owner));
			return s;
		}
		exec_stack *virtual_machine::allocate_stack(exec_function_ptr *fnc)
		{
			exec_stack *s=new (allocate_memory(exec_stack::calculate_size(fnc),typeid(exec_stack))) exec_stack(fnc);
			_allocated(s);
			s->set_variables(fnc->internal_stack_variables_count()+1,allocate_stack_variables(fnc->internal_local_stack_size(),fnc));
			return s;
		}
		//exec_stack *virtual_machine::allocate_stack(exec_function_ptr *fnc)
		//{
		//	RCASSERT(fnc->mode()==exec_function_ptr::F);
		//	exec_stack *st=allocate_stack(fnc->internal_stack_variables_level0(),fnc->internal_stack_variables_count()+2,fnc->internal_local_stack_size());
		//	for(unsigned int i=1;i<=fnc->internal_stack_variables_count();++i) {
		//		st->set_variables(i,fnc->internal_stack_variables(i-1));
		//		fnc->internal_stack_variables(i-1)->gc_acquire();
		//	}
		//	return st;
		//}
		exec_object *virtual_machine::allocate_tuple(exec_tuple_object *&ptr, unsigned int member_count)
		{
			exec_object *o=allocate_object(class_tuple,member_count*sizeof(exec_variable));
			ptr=(exec_tuple_object*)o->_getptr(0);
			for(unsigned int i=0;i<member_count;++i) ptr->get(i).reset();
			return o;
		}
		exec_object *virtual_machine::allocate_map(exec_map_object *&ptr)
		{
			exec_object *o=allocate_object(class_map,0);
			ptr=(exec_map_object*)o->_getptr(0);
			return o;
		}
		exec_object *virtual_machine::allocate_set(exec_set_object *&ptr)
		{
			exec_object *o=allocate_object(class_set,0);
			ptr=(exec_set_object*)o->_getptr(0);
			return o;
		}
		exec_object *virtual_machine::allocate_array(exec_array_object *&ptr)
		{
			exec_object *o=allocate_object(class_array,0);
			ptr=(exec_array_object*)o->_getptr(0);
			return o;
		}
		exec_object *virtual_machine::allocate_stack_element(exec_stack_element_object *&ptr)
		{
			exec_object *o=allocate_object(class_stack_element,0);
			ptr=(exec_stack_element_object*)o->_getptr(0);
			return o;
		}
		exec_object *virtual_machine::allocate_type(exec_class_object *&ptr)
		{
			exec_object *o=new (allocate_memory(sizeof(exec_object)+sizeof(exec_class_object),typeid(exec_object))) exec_object();
			if (class_class==NULL) class_class=o;
			o->type_=class_class;
			class_class->gc_acquire();
			ptr=new (o->_getptr(0)) exec_class_object(*this,0);
			//ptr->_create(*this,0);
#ifdef RCDEBUG
			o->_co_=ptr;
#endif
			_allocated(o);
			return o;
		}

		exec_property *virtual_machine::allocate_property()
		{
			exec_property *p=new (allocate_memory(sizeof(exec_property),typeid(exec_property))) exec_property();
			_allocated(p);
			return p;
		}

		exec_namespace *virtual_machine::allocate_namespace(owca_internal_string *file_name)
		{
			exec_namespace *n=new (allocate_memory(sizeof(exec_namespace),typeid(exec_namespace))) exec_namespace(*this,file_name);
			_allocated(n);
			n->init();
			return n;
		}

		exec_function_bound *virtual_machine::allocate_function_bound(exec_function_ptr *fnc_, const exec_variable &slf)
		{
			exec_function_bound *p=new (allocate_memory(sizeof(exec_function_bound),typeid(exec_function_bound))) exec_function_bound();
			p->slf=slf;
			p->fnc=fnc_;
			_allocated(p);
			return p;
		}

		exec_weakref_object *virtual_machine::allocate_weakref_object(exec_object *par)
		{
			if (par->weakref!=NULL) {
				par->weakref->gc_acquire();
			}
			else {
				par->weakref=new (allocate_memory(sizeof(exec_weakref_object),typeid(exec_weakref_object))) exec_weakref_object(par);
				_allocated(par->weakref);
			}
			return par->weakref;
		}

		owca_internal_string *virtual_machine::allocate_string(std::string s)
		{
			unsigned int index = 0;
			unsigned int char_count = (unsigned int)owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, s.c_str(), (unsigned int)s.size());
			if (index > s.size())
				s.resize(index, (char)0x80);
			return allocate_string(s.c_str(), (unsigned int)s.size(), char_count);
		}

		owca_internal_string *virtual_machine::allocate_string(owca_internal_string_nongc *s)
		{
			return allocate_string(s->data_pointer(), s->data_size(), s->character_count());
		}

		owca_internal_string *virtual_machine::allocate_string(const char *data, unsigned int size, unsigned int char_count)
		{
			owca_internal_string *ret = new (allocate_memory(string_size + sizeof(string_pre) + sizeof(char)*size + 1, typeid(owca_internal_string))) owca_internal_string(size, char_count);
#ifdef RCDEBUG_DEBUG_STRINGS
			_allocated(ret);
#endif
			if (ret && (data || size==0)) {
				string_filler sf(ret);
				for(unsigned int i=0;i<size;++i) sf.append(data[i]);
			}
			return ret;
		}
#ifdef RCDEBUG
		void *virtual_machine::allocate_memory(unsigned int size, const std::type_info &ti)
		{
			//++mm_mp[std::pair<unsigned int, const type_info*>(size,&ti)];
			return _allocate_memory(size);
		}
#endif

		vm_execution_stack_elem_internal *virtual_machine::allocate_stack_elem_internal(unsigned int temp_variables_count)
		{
			vm_execution_stack_elem_internal *s=new (allocate_memory(vm_execution_stack_elem_internal::calculate_size(temp_variables_count),typeid(exec_function_bound))) vm_execution_stack_elem_internal();
			_allocated(s);
			s->tempstackactpos=0;
			s->tempstackmaxsize=temp_variables_count;
			return s;
		}

		//void virtual_machine::to_string(exec_variable &ret, const owca_string &txt)
		//{
		//	ret.set_string(txt.ss);
		//	txt.ss->gc_acquire();
		//}

		static std::string property_fnc_str(exec_function_ptr *p)
		{
			if (p==NULL) return "null";
			if (p->name()) {
				RCASSERT(p->name()->data_size()>0);
				return "function "+p->name()->str();
			}
			return "unnamed function";
		}

		std::string virtual_machine::to_stdstring_type(exec_object *type)
		{
			RCASSERT(type->is_type());
			if (type==class_null) return "a null";
			if (type==class_int) return "an integer";
			if (type==class_real) return "a real";
			if (type==class_bool) return "a boolean";
			if (type==class_string) return "a string";
			if (type==class_generator) return "a generator object";
			if (type==class_property) return "a property";
			if (type==class_function) return "a function";
			if (type==class_array) return "an array";
			if (type==class_tuple) return "a tuple";
			if (type==class_stack_element) return "a stack element";
			if (type==class_map) return "a map";
			return "an object of type "+type->CO().name->str();
		}

		std::string virtual_machine::to_stdstring_type_short(const exec_variable &r)
		{
			switch(r.mode()) {
			case VAR_NULL: return "null";
			case VAR_INT: return "integer";
			case VAR_REAL: return "real";
			case VAR_BOOL: return "boolean";
			case VAR_STRING: return "string";
			case VAR_GENERATOR: return "generator object";
			case VAR_PROPERTY: return "property";
			case VAR_FUNCTION:
			case VAR_FUNCTION_FAST: return "function";
			case VAR_NAMESPACE: return "namespace object";
			case VAR_WEAK_REF: return "weakref object";
			case VAR_UNDEFINED: return "undefined";
			case VAR_OBJECT: {
				std::string type=r.get_object()->type()->CO().name->str();
				if (type=="$tuple") return "tuple";
				if (type=="$map") return "map";
				if (type=="$array") return "array";
				return "type "+type; }
			case VAR_COUNT:
			case VAR_HASH_DELETED:
			case VAR_HASH_EMPTY:
			case VAR_NO_DEF_VALUE:
			case VAR_NO_PARAM_GIVEN: RCASSERT(0);
			}
			return "";
		}

		std::string virtual_machine::to_stdstring_type(const exec_variable &r)
		{
			switch(r.mode()) {
			case VAR_UNDEFINED: return "undefined";
			case VAR_NULL: return "a null";
			case VAR_INT: return "an integer";
			case VAR_REAL: return "a real";
			case VAR_BOOL: return "a boolean";
			case VAR_STRING: return "a string";
			case VAR_GENERATOR: return "a generator object";
			case VAR_PROPERTY: return "a property";
			case VAR_FUNCTION:
			case VAR_FUNCTION_FAST: return "a function";
			case VAR_WEAK_REF: return "a weakref object";
			case VAR_NAMESPACE: return "a namespace";
			case VAR_OBJECT: {
				std::string type=r.get_object()->type()->CO().name->str();
				if (type=="$tuple") return "a tuple";
				if (type=="$map") return "a map";
				if (type=="$array") return "an array";
				return "an object of type "+type; }
			case VAR_COUNT:
			case VAR_HASH_DELETED:
			case VAR_HASH_EMPTY:
			case VAR_NO_DEF_VALUE:
			case VAR_NO_PARAM_GIVEN: RCASSERT(0);
			}
			return "";
		}

		bool virtual_machine::exception_thrown() const
		{
			return execution_exception_object_thrown!=NULL;
		}

		executionreturnvalue virtual_machine::execute_stack()
		{
			RCASSERT(execution_stack && execution_stack->index>=0);

			vm_execution_stack *finishstack=execution_stack;
			if (finishstack->coroutine_object()) {
				finishstack=finishstack->prev();
				RCASSERT(finishstack);
			}

			executionstackreturnvalue r=executionstackreturnvalue::OK;
			exec_variable *returnval=finishstack->peek_frame_indexed(0)->return_value;
			RCASSERT(returnval->mode()==VAR_NULL);
			bool no_value = false;

			while (finishstack != execution_stack || !execution_stack->empty()) {
				vm_execution_stack_elem_base* q = execution_stack->peek_frame();

#ifdef RCDEBUG
				int cindex=execution_stack->index;
				vm_execution_stack *cstack=execution_stack;
#endif
#ifdef RCDEBUG
				static unsigned int _exec_cnt=0;
				unsigned int _exec_cnt_this=++_exec_cnt;
				if (_exec_cnt_this>=1) {
					_exec_cnt_this=_exec_cnt_this;
				}
#endif
				GC(this);
#ifdef RCDEBUG_EXECUTION
				char dbuf[256];
				sprintf(dbuf,"%6d %3d  ",q->get_debug_ident(),_exec_cnt_this);
				debugprint("executing stackframe ");
				debugprint(dbuf);
				if (q->fnc && q->fnc->name() && q->fnc->name()->data_size()>0) debugprint(q->fnc->name()->data_pointer());
				else if (q->fnc) debugprint("unnamed function");
				else debugprint("<code>");
				debugprint("\n");
#endif
				if (q->finalized()) {
					goto pop_frame;
				}
				r=q->first_time_run() ? q->first_time_execute(r) : q->execute(r);
				
				// static unsigned int index = 0;
				// auto qii = ++index;
				// debugprint("%d: frame %p (%s) -> %s\n",
				// 	qii, q, (q->fnc ? q->fnc->name()->str().c_str() : "<empty>"),
				// 	to_string(r).c_str()
				// 	);
				
				switch(r.type()) {
				case executionstackreturnvalue::CREATE_GENERATOR:
#ifdef RCDEBUG_EXECUTION
					sprintf(dbuf,"executing stackframe %6d      CREATE_GENERATOR\n",q->get_debug_ident()); debugprint(dbuf);
#endif
#ifdef RCDEBUG
					RCASSERT(execution_stack->index==cindex && execution_stack==cstack);
#endif
					q->return_value->set_generator(q);
					q->gc_acquire();

					if (q->return_handling_mode==vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED) {
						q->return_handling_mode=vm_execution_stack_elem_base::RETURN_HANDLING_NONE;
					}
					else {
						RCASSERT(q->return_handling_mode==vm_execution_stack_elem_base::RETURN_HANDLING_NONE);
					}
					goto handle_return;
				case executionstackreturnvalue::OK:
#ifdef RCDEBUG_EXECUTION
					sprintf(dbuf,"executing stackframe %6d      OK\n",q->get_debug_ident()); debugprint(dbuf);
#endif
#ifdef RCDEBUG
					RCASSERT(execution_stack->index==cindex && execution_stack==cstack);
#endif
					break;
				case executionstackreturnvalue::CO_START:
#ifdef RCDEBUG_EXECUTION
					sprintf(dbuf,"executing stackframe %6d      CO_START\n",q->get_debug_ident()); debugprint(dbuf);
#endif
#ifdef RCDEBUG
					RCASSERT(execution_stack!=cstack && execution_stack->coroutine_object());
#endif
					r=executionstackreturnvalue::OK;
					break;
				case executionstackreturnvalue::CO_STOP:
					if (execution_stack->coroutine_object()) {
						RCASSERT(execution_stack->prev());
						RCASSERT(execution_stack->peek_frame_indexed(0)->return_value);

						for(unsigned int i=0;i<execution_stack->count()-1;++i) {
							if (!execution_stack->peek_frame_indexed(i)->internal_frame()) {
								raise_cant_stop_from_within_user_function();
								//q->return_value->gc_release(*this);
								goto function_call;
							}
						}

						*execution_stack->peek_frame_indexed(0)->return_value=*q->return_value;
						no_value = q->return_value->is_no_return_value();
						//q->return_value->gc_acquire();
						q->return_value->set_null();
						execution_stack->peek_frame_indexed(0)->return_value=NULL;

						vm_execution_stack *pp=execution_stack->prev();
						RCASSERT(pp);
						execution_stack->set_prev(NULL);
						execution_stack->gc_release(*this);
						execution_stack=pp;
					}
					else {
						raise_no_coroutine_to_stop();
						//q->return_value->gc_release(*this);
						goto function_call;
					}

#ifdef RCDEBUG_EXECUTION
					sprintf(dbuf,"executing stackframe %6d      CO_STOP\n",q->get_debug_ident()); debugprint(dbuf);
#endif
#ifdef RCDEBUG
					RCASSERT(execution_stack!=cstack && !execution_stack->coroutine_object());
#endif
					r=executionstackreturnvalue::OK;
					break;
				case executionstackreturnvalue::FUNCTION_CALL:
function_call:
#ifdef RCDEBUG_EXECUTION
					sprintf(dbuf,"executing stackframe %6d      FUNCTION_CALL\n",q->get_debug_ident()); debugprint(dbuf);
#endif
#ifdef RCDEBUG
					RCASSERT(execution_stack!=cstack || (execution_stack->index>=cindex+1 && execution_stack->index<=cindex+2));
					// you failed to prepare_call function or did it more than once
#endif
					r=executionstackreturnvalue::OK;
					break;
//				case executionstackreturnvalue::REPLACE_CALL:
//#ifdef RCDEBUG_EXECUTION
//					sprintf(dbuf,"executing stackframe %6d      REPLACE_CALL\n",q->get_debug_ident()); debugprint(dbuf);
//#endif
//#ifdef RCDEBUG
//					RCASSERT(execution_stack==cstack && execution_stack->index>=cindex && execution_stack->index<=(cindex+1) && execution_stack->peek_frame()!=q);
//#endif
//					r=executionstackreturnvalue::OK;
//					break;
				case executionstackreturnvalue::EXCEPTION:
#ifdef RCDEBUG_EXECUTION
					sprintf(dbuf,"executing stackframe %6d      EXCEPTION\n",q->get_debug_ident()); debugprint(dbuf);
#endif
#ifdef RCDEBUG
					RCASSERT(execution_stack->index==cindex && execution_stack==cstack);
#endif
					execution_stack->pop_frame(q);
handle_exception:
					while(!execution_stack->empty()) {
						q=execution_stack->peek_frame();
						if (!q->catch_exceptions()) execution_stack->pop_frame(q);
						else break;
					}
					if (execution_stack->empty() && execution_stack->coroutine_object()) {
						RCASSERT(finishstack!=execution_stack);

						vm_execution_stack *st=execution_stack->prev();
						RCASSERT(st);
						execution_stack->set_prev(NULL);
						execution_stack->gc_release(*this);
						execution_stack=st;
						goto handle_exception;
					}

					break;
				case executionstackreturnvalue::RETURN:
					execution_self_oper=(q->operator_mode==10);
handle_return:
#ifdef RCDEBUG_EXECUTION
					sprintf(dbuf,"executing stackframe %6d      RETURN\n",q->get_debug_ident()); debugprint(dbuf);
#endif
#ifdef RCDEBUG
					RCASSERT(execution_stack->index==cindex && execution_stack==cstack);
#endif
					r=executionstackreturnvalue::OK;
					switch(q->return_handling_mode) {
					case vm_execution_stack_elem_base::RETURN_HANDLING_NONE_FORCE_RETURN:
					case vm_execution_stack_elem_base::RETURN_HANDLING_NONE:
pop_frame:
						if (q->return_value) {
							no_value = q->return_value->is_no_return_value();
						}
						else {
							no_value = true;
						}
						if (execution_stack->count()==1 && execution_stack->coroutine_object()) {
							vm_execution_stack *st=execution_stack->prev();
							RCASSERT(st);
							RCASSERT(execution_stack->peek_frame_indexed(0)==q);

							execution_stack->pop_frame(q);
							execution_stack->set_prev(NULL);
							execution_stack->gc_release(*this);
							execution_stack=st;
						}
						else {
							execution_stack->pop_frame(q);
						}
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION:
						RCASSERT(q->return_value->is_no_return_value());
						execution_exception_parameters[1].gc_release(*this);
						execution_exception_parameters[1].reset();
						if (execution_exception_object_thrown) execution_exception_object_thrown->gc_release(*this);
						execution_exception_object_thrown=q->init_object;
						q->init_object=NULL;

						RCASSERT(execution_exception_object_thrown);
						execution_stack->pop_frame(q);
						r=executionstackreturnvalue::EXCEPTION;
						goto handle_exception;
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INIT:
						if (!q->return_value->is_no_return_value()) {
							raise_invalid_return_type(*q->return_value,NULL);
#ifdef RCDEBUG
							RCASSERT(execution_stack->index>cindex && execution_stack==cstack);
#endif
						}
						else {
							q->return_value->set_object(q->init_object);
							q->init_object=NULL;
							goto pop_frame;
						}
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL:
						if (q->return_value->is_no_return_value()) goto unsupported;
						else if (q->return_value->mode()!=VAR_BOOL) {
							raise_invalid_return_type(*q->return_value,class_bool);
#ifdef RCDEBUG
							RCASSERT(execution_stack->index>cindex && execution_stack==cstack);
#endif
						}
						else goto pop_frame;
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INT:
						if (q->return_value->is_no_return_value()) goto unsupported;
						else if (q->return_value->mode()!=VAR_INT) {
							raise_invalid_return_type(*q->return_value,class_int);
#ifdef RCDEBUG
							RCASSERT(execution_stack->index>cindex && execution_stack==cstack);
#endif
						}
						else goto pop_frame;
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_CANT_BE_UNUSED:
						if (q->return_value->is_no_return_value()) {
							raise_missing_return_value();
#ifdef RCDEBUG
							RCASSERT(execution_stack->index>cindex && execution_stack==cstack);
#endif
						}
						else goto pop_frame;
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_STR:
						if (q->return_value->is_no_return_value()) goto unsupported;
						else if (q->return_value->mode()!=VAR_STRING) {
							raise_invalid_return_type(*q->return_value,class_string);
#ifdef RCDEBUG
							RCASSERT(execution_stack->index>cindex && execution_stack==cstack);
#endif
						}
						else goto pop_frame;
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED:
						if (q->return_value->is_no_return_value()) {
unsupported:
							q->gc_acquire();
							//vislocker _v(*this,q);
							execution_stack->pop_frame(q);
							raise_unsupported_operation((operatorcodes)q->operator_index,q->oper_2_values);
							q->gc_release(*this);
						}
						else goto pop_frame;
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER:
						if (q->return_value->is_no_return_value()) {
update_oper:
							q->gc_acquire();
							//vislocker _v(*this,q);
							execution_stack->pop_frame(q);
							operator_get_next_function(q);
							q->gc_release(*this);
						}
						else goto pop_frame;
						break;
					case vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER:
						if (q->return_value->is_no_return_value()) goto update_oper;
						switch(q->return_value->mode()) {
						case VAR_BOOL:
							goto pop_frame;
							break;
						default:
							raise_invalid_return_type(*q->return_value,class_bool);
#ifdef RCDEBUG
							RCASSERT(execution_stack->index>cindex && execution_stack==cstack);
#endif
						}
						break;
					default:
						RCASSERT(0);
					}
					break;
				default: RCASSERT(0);
				}
			}
			switch(r.type()) {
			case executionstackreturnvalue::OK:
				return no_value ? VME_NO_VALUE : VME_VALUE;
			case executionstackreturnvalue::RETURN:
				return VME_VALUE;
			case executionstackreturnvalue::EXCEPTION:
				returnval->gc_release(*this);
				returnval->set_object(execution_exception_object_thrown);
				execution_exception_object_thrown=NULL;
				return VME_EXCEPTION;
			case executionstackreturnvalue::FUNCTION_CALL:
				RCASSERT(0);
				break;
			default:
				RCASSERT(0);
			}
			return VME_VALUE;
		}

		//vm_execution_stack *virtual_machine::create_execution_stack()
		//{
		//	vm_execution_stack *s=new (allocate_memory(sizeof(vm_execution_stack),typeid(vm_execution_stack))) vm_execution_stack();
		//	_allocated(s);
		//	return s;
		//}

		void virtual_machine::push_execution_stack(vm_execution_stack *st)
		{
			if (st==NULL) {
				st=new (allocate_memory(sizeof(vm_execution_stack),typeid(vm_execution_stack))) vm_execution_stack();
				st->vm=this;
				_allocated(st);
			}
			else st->gc_acquire();
			st->prev_stack=execution_stack;
			execution_stack=st;
		}

		void virtual_machine::pop_execution_stack()
		{
			vm_execution_stack *st=execution_stack;
			execution_stack=execution_stack->prev_stack;
			st->prev_stack=NULL;
			st->gc_release(*this);
		}

		void vm_execution_stack::clear(void)
		{
			while(!empty()) {
				pop_frame(peek_frame());
			}
		}

		void vm_execution_stack::_release_resources(virtual_machine &vm)
		{
			for(int i=0;i<=index;++i) elems[i]->gc_release(vm);
			index=-1;
			if (prev_stack) prev_stack->gc_release(vm);
		}

		void vm_execution_stack::_mark_gc(const gc_iteration &gc) const
		{
			for(int i=0;i<=index;++i) {
				gc_iteration::debug_info _d("vm_execution_stack: frame object %d",i);
				elems[i]->gc_mark(gc);
			}
			if (prev_stack) {
				gc_iteration::debug_info _d("vm_execution_stack: prev stack object");
				prev_stack->gc_mark(gc);
			}
		}

		void vm_execution_stack::pop_frame(vm_execution_stack_elem_base *el)
		{
			RCASSERT(elems[index]==el);
			el->gc_release(*vm);
			--index;
		}

		bool vm_execution_stack::push_frame(virtual_machine &vm, vm_execution_stack_elem_base *el)
		{
			if (!check_stack_overflow || index<(int)vm_execution_stack::MAX_EXECUTION_STACK_DEPTH) {
				++index;
				RCASSERT(index<MAX_EXECUTION_STACK_SIZE);
				elems[index]=el;
				return true;
			}
			vm.raise_stack_overflow();
			return false;
		}

		bool virtual_machine::push_execution_stack_frame(vm_execution_stack_elem_base *sf)
		{
			return execution_stack->push_frame(*this,sf);
			//execution_stack->elems[++execution_stack->index]=sf;
		}

		D_FUNC0(virtualmachine,bool_return)
			{
				return_value->set_bool(value);
				return executionstackreturnvalue::RETURN;
			}

			bool value;
		D_END

		void virtual_machine::push_frame_returning_bool_value(bool value, exec_variable *return_value, exec_function_ptr *fnc)
		{
			virtualmachine_bool_return_object *sb=static_cast<virtualmachine_bool_return_object*>(virtualmachine_bool_return_object::create<virtualmachine_bool_return_object>(*this,NULL));
			if (sb) {
				sb->value=value;
				sb->return_value=return_value;
				sb->fnc=NULL;
				sb->show_in_exception_stack(true);
			}
		}

		vm_execution_stack_elem_internal *virtual_machine::push_execution_stack_frame_internal(unsigned int stack_data_size, unsigned int temporary_variables)
		{
			vm_execution_stack_elem_internal *sf=allocate_stack_elem_internal(temporary_variables);
			sf->vm=this;
			sf->fncstackdata=allocate_function_stackdata(stack_data_size);
			RCPRINTF("allocating %p\n", sf->fncstackdata);
			sf->tempstackactpos=0;
			if (execution_stack->push_frame(*this,sf)) return sf;
			sf->gc_release(*this);
			return NULL;
		}

		static owca_internal_string *_get_string(const unsigned char *data, unsigned int &offset)
		{
			owca_internal_string *s=(owca_internal_string*)(data+offset);
			offset+=string_size+sizeof(string_pre)+s->data_size()+1;
			return s;
		}

		void virtual_machine::prepare_call_opcodes_entry(exec_variable *retval, exec_namespace *result, opcode_data *opcodes)
		{
			const unsigned char *dt = opcodes->get_opcodes();

			vm_execution_stack_elem_internal *stackframe = push_execution_stack_frame_internal(opcodes->get_top_level_stack_data_size(), opcodes->get_top_level_max_variable_count());
			RCASSERT(stackframe && execution_stack->index == 0 && execution_stack->peek_frame() == stackframe);

			//stackframe->return_handling_mode=vm_execution_stack_elem_internal::RETURN_HANDLING_NONE;
			stackframe->return_value = retval;
			stackframe->opcodes = opcodes;
			stackframe->opcodes_offset = 0;

			vm_execution_stack_elem_internal &oe = *stackframe;

			unsigned int *level0map;
			level0map = new unsigned int[opcodes->get_required_variable_name_index().size() + opcodes->get_not_required_variable_name_index().size() + 1];
			level0map[0] = 1;
			stackframe->stack = allocate_stack(result, level0map);
			unsigned int level0map_index = 1;
			for (auto a : opcodes->get_required_variable_name_index()) {
				owca_internal_string *ident = opcodes->get_string_by_index(a);
				exec_namespace::hashmap::hash_map_finder mi(result->hashindex, ident->hash());

				for (; mi.valid(); mi.next()) {
					if (result->hashindex.getkey(mi.get()).k->equal(ident)) break;
				}
				if (!mi.valid()) {
					delete[] level0map;
					throw owca_exception(ExceptionCode::MISSING_MEMBER, OWCA_ERROR_FORMAT1("namespace object is missing member %1", ident->str()));
				}
				level0map[level0map_index] = result->hashindex.getval(mi.get()).v;
				++level0map_index;
			}
			exec_variable ttt;
			ttt.reset();
			for (auto a : opcodes->get_not_required_variable_name_index()) {
				owca_internal_string *ident = opcodes->get_string_by_index(a);
				int index = result->insert_variable(ident, ttt);
				RCASSERT(index >= 0);
				level0map[level0map_index] = index;
				++level0map_index;
			}

			oe.r = returnvalueflow::CONTINUE_OPCODES;
			oe.show_in_exception_stack(true);
		}

		unsigned char operator_operand_next[]={
		/*	E_EQ             */ E_EQ_R,
		/*	E_NOTEQ          */ E_NOTEQ_R,
		/*	E_LESSEQ         */ E_LESSEQ_R,
		/*	E_MOREEQ         */ E_MOREEQ_R,
		/*	E_LESS           */ E_LESS_R,
		/*	E_MORE           */ E_MORE_R,
		/*	E_EQ_R           */ E_EQ,
		/*	E_NOTEQ_R        */ E_NOTEQ,
		/*	E_LESSEQ_R       */ E_LESSEQ,
		/*	E_MOREEQ_R       */ E_MOREEQ,
		/*	E_LESS_R         */ E_LESS,
		/*	E_MORE_R         */ E_MORE,
		/*	E_ADD            */ E_ADD_R,
		/*	E_SUB            */ E_SUB_R,
		/*	E_MUL            */ E_MUL_R,
		/*	E_DIV            */ E_DIV_R,
		/*	E_MOD            */ E_MOD_R,
		/*	E_LSHIFT         */ E_LSHIFT_R,
		/*	E_RSHIFT         */ E_RSHIFT_R,
		/*	E_BIN_AND        */ E_BIN_AND_R,
		/*	E_BIN_OR         */ E_BIN_OR_R,
		/*	E_BIN_XOR        */ E_BIN_XOR_R,
		/*	E_ADD_R          */ E_ADD,
		/*	E_SUB_R          */ E_SUB,
		/*	E_MUL_R          */ E_MUL,
		/*	E_DIV_R          */ E_DIV,
		/*	E_MOD_R          */ E_MOD,
		/*	E_LSHIFT_R       */ E_LSHIFT,
		/*	E_RSHIFT_R       */ E_RSHIFT,
		/*	E_BIN_AND_R      */ E_BIN_AND,
		/*	E_BIN_OR_R       */ E_BIN_OR,
		/*	E_BIN_XOR_R      */ E_BIN_XOR,
		/*	E_IN             */ E_COUNT,
		/*	E_ADD_SELF       */ E_ADD,
		/*	E_SUB_SELF       */ E_SUB,
		/*	E_MUL_SELF       */ E_MUL,
		/*	E_DIV_SELF       */ E_DIV,
		/*	E_MOD_SELF       */ E_MOD,
		/*	E_LSHIFT_SELF    */ E_LSHIFT,
		/*	E_RSHIFT_SELF    */ E_RSHIFT,
		/*	E_BIN_AND_SELF   */ E_BIN_AND,
		/*	E_BIN_OR_SELF    */ E_BIN_OR,
		/*	E_BIN_XOR_SELF   */ E_BIN_XOR,
		/*	E_ACCESS_1_READ  */ E_COUNT,
		/*	E_ACCESS_1_WRITE */ E_COUNT,
		/*	E_ACCESS_2_READ  */ E_COUNT,
		/*	E_ACCESS_2_WRITE */ E_COUNT,
		/*	E_BIN_NOT        */ E_COUNT,
		/*	E_BOOL           */ E_COUNT,
		/*	E_SIGN_CHANGE    */ E_COUNT,
		/*	E_CALL           */ E_COUNT,
		/*	E_GENERATOR      */ E_COUNT,
		/*	E_INIT           */ E_COUNT,
		/*	E_NEW            */ E_COUNT,
		/*	E_STR            */ E_COUNT,
		/*	E_HASH           */ E_COUNT,
		/*	E_WITH_EXIT      */ E_COUNT,
		/*	E_WITH_ENTER     */ E_COUNT,
			};

		unsigned char operator_return_is_operator_multi[]={
		/*	E_EQ             */ 1,
		/*	E_NOTEQ          */ 1,
		/*	E_LESSEQ         */ 1,
		/*	E_MOREEQ         */ 1,
		/*	E_LESS           */ 1,
		/*	E_MORE           */ 1,
		/*	E_EQ_R           */ 0,
		/*	E_NOTEQ_R        */ 0,
		/*	E_LESSEQ_R       */ 0,
		/*	E_MOREEQ_R       */ 0,
		/*	E_LESS_R         */ 0,
		/*	E_MORE_R         */ 0,
		/*	E_ADD            */ 1,
		/*	E_SUB            */ 1,
		/*	E_MUL            */ 1,
		/*	E_DIV            */ 1,
		/*	E_MOD            */ 1,
		/*	E_LSHIFT         */ 1,
		/*	E_RSHIFT         */ 1,
		/*	E_BIN_AND        */ 1,
		/*	E_BIN_OR         */ 1,
		/*	E_BIN_XOR        */ 1,
		/*	E_ADD_R          */ 0,
		/*	E_SUB_R          */ 0,
		/*	E_MUL_R          */ 0,
		/*	E_DIV_R          */ 0,
		/*	E_MOD_R          */ 0,
		/*	E_LSHIFT_R       */ 0,
		/*	E_RSHIFT_R       */ 0,
		/*	E_BIN_AND_R      */ 0,
		/*	E_BIN_OR_R       */ 0,
		/*	E_BIN_XOR_R      */ 0,
		/*	E_IN             */ 0,
		/*	E_ADD_SELF       */ 1,
		/*	E_SUB_SELF       */ 1,
		/*	E_MUL_SELF       */ 1,
		/*	E_DIV_SELF       */ 1,
		/*	E_MOD_SELF       */ 1,
		/*	E_LSHIFT_SELF    */ 1,
		/*	E_RSHIFT_SELF    */ 1,
		/*	E_BIN_AND_SELF   */ 1,
		/*	E_BIN_OR_SELF    */ 1,
		/*	E_BIN_XOR_SELF   */ 1,
		/*	E_ACCESS_1_READ  */ 0,
		/*	E_ACCESS_1_WRITE */ 0,
		/*	E_ACCESS_2_READ  */ 0,
		/*	E_ACCESS_2_WRITE */ 0,
		/*	E_BIN_NOT        */ 0,
		/*	E_BOOL           */ 0,
		/*	E_SIGN_CHANGE    */ 0,
		/*	E_CALL           */ 0,
		/*	E_GENERATOR      */ 0,
		/*	E_INIT           */ 0,
		/*	E_NEW            */ 0,
		/*	E_STR            */ 0,
		/*	E_HASH           */ 0,
		/*	E_WITH_EXIT      */ 0,
		/*	E_WITH_ENTER     */ 0,
			};

		unsigned char operator_return_handling_mode[]={
		/*	E_EQ             */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_NOTEQ          */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_LESSEQ         */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_MOREEQ         */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_LESS           */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_MORE           */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_EQ_R           */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_NOTEQ_R        */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_LESSEQ_R       */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_MOREEQ_R       */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_LESS_R         */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_MORE_R         */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER,
		/*	E_ADD            */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_SUB            */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_MUL            */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_DIV            */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_MOD            */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_LSHIFT         */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_RSHIFT         */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_BIN_AND        */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_BIN_OR         */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_BIN_XOR        */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_ADD_R          */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_SUB_R          */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_MUL_R          */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_DIV_R          */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_MOD_R          */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_LSHIFT_R       */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_RSHIFT_R       */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_BIN_AND_R      */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_BIN_OR_R       */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_BIN_XOR_R      */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_IN             */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_ADD_SELF       */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_SUB_SELF       */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_MUL_SELF       */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_DIV_SELF       */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_MOD_SELF       */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_LSHIFT_SELF    */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_RSHIFT_SELF    */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_BIN_AND_SELF   */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_BIN_OR_SELF    */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_BIN_XOR_SELF   */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER,
		/*	E_ACCESS_1_READ  */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED,
		/*	E_ACCESS_1_WRITE */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED,
		/*	E_ACCESS_2_READ  */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED,
		/*	E_ACCESS_2_WRITE */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED,
		/*	E_BIN_NOT        */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED,
		/*	E_BOOL           */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_BOOL,
		/*	E_SIGN_CHANGE    */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED,
		/*	E_CALL           */ vm_execution_stack_elem_base::RETURN_HANDLING_NONE,
		/*	E_GENERATOR      */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED,
		/*	E_INIT           */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INIT,
		/*	E_NEW            */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED,
		/*	E_STR            */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_STR,
		/*	E_HASH           */ vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INT,
		/*	E_WITH_EXIT      */ vm_execution_stack_elem_base::RETURN_HANDLING_NONE,
		/*	E_WITH_ENTER     */ vm_execution_stack_elem_base::RETURN_HANDLING_NONE,
			};

		//bool virtual_machine::operator_get_function_right_over_left(unsigned char &oper, const exec_variable *opers)
		//{
		//	//if (left.mode()==VAR_OBJECT && right.mode()==VAR_OBJECT &&
		//	//		left.has_operator(*this,(operatorcodes)oper) &&
		//	//		right.has_operator(*this,(operatorcodes)operator_operand_next_if_unsupported[oper])) {
		//	//		// TODO: check, if right operands inherits from left
		//	//		// this is tricky, as base classes have to be considered as well (?)

		//	//		//exec_object *o1=left.get_object(),*o2=right.get_object();
		//	//		//exec_class_object &c1=o1->type()->CO();
		//	//		//exec_class_object &c2=o2->type()->CO();
		//	//		//
		//	//		//for(unsigned int i=0;i<lookup_order.size();++i) {
		//	//		//	exec_object *p=c1.lookup_order[i];
		//	//		//	if (p->CO().operators[
		//	//		//}
		//	//}
		//	return false;
		//}

		void virtual_machine::operator_get_next_function(vm_execution_stack_elem_base *q)
		{
			exec_variable fnc;

			const exec_variable *other=operator_get_function_2(fnc,q->operator_mode,q->operator_index,q->oper_2_values);
			if (other) {
				RCASSERT(operator_return_is_operator_multi[q->operator_index]);

				if (prepare_call_function(q->return_value,fnc,other,1)) {
					vm_execution_stack_elem_base *qq=execution_stack->elems[execution_stack->index];
					qq->oper_2_values=q->oper_2_values;
					q->oper_2_values=NULL;
					qq->operator_mode=q->operator_mode;
					qq->operator_index=q->operator_index;
					qq->return_handling_mode=(vm_execution_stack_elem_base::return_handling_mode_)operator_return_handling_mode[q->operator_index];
				}
				fnc.gc_release(*this);
			}
			else {
				switch((operatorcodes)q->operator_index) {
				case E_EQ:
				case E_EQ_R:
					q->return_value->set_bool(
						q->oper_2_values[0].mode()==VAR_OBJECT && q->oper_2_values[1].mode()==VAR_OBJECT &&
						q->oper_2_values[0].get_object()==q->oper_2_values[1].get_object());
					break;
				case E_NOTEQ:
				case E_NOTEQ_R:
					q->return_value->set_bool(!(
						q->oper_2_values[0].mode()==VAR_OBJECT && q->oper_2_values[1].mode()==VAR_OBJECT &&
						q->oper_2_values[0].get_object()==q->oper_2_values[1].get_object()));
					break;
				case E_LESSEQ:
				case E_MOREEQ:
				case E_LESS:
				case E_MORE:
				case E_LESSEQ_R:
				case E_MOREEQ_R:
				case E_LESS_R:
				case E_MORE_R:
				case E_ADD:
				case E_SUB:
				case E_MUL:
				case E_DIV:
				case E_MOD:
				case E_LSHIFT:
				case E_RSHIFT:
				case E_BIN_AND:
				case E_BIN_OR:
				case E_BIN_XOR:
				case E_ADD_R:
				case E_SUB_R:
				case E_MUL_R:
				case E_DIV_R:
				case E_MOD_R:
				case E_LSHIFT_R:
				case E_RSHIFT_R:
				case E_BIN_AND_R:
				case E_BIN_OR_R:
				case E_BIN_XOR_R:
				case E_IN:
				case E_ADD_SELF:
				case E_SUB_SELF:
				case E_MUL_SELF:
				case E_DIV_SELF:
				case E_MOD_SELF:
				case E_LSHIFT_SELF:
				case E_RSHIFT_SELF:
				case E_BIN_AND_SELF:
				case E_BIN_OR_SELF:
				case E_BIN_XOR_SELF:
				case E_ACCESS_1_READ:
				case E_ACCESS_1_WRITE:
				case E_ACCESS_2_READ:
				case E_ACCESS_2_WRITE:
				case E_BIN_NOT:
				case E_BOOL:
				case E_SIGN_CHANGE:
				case E_CALL:
				case E_GENERATOR:
				case E_INIT:
				case E_NEW:
				case E_STR:
				case E_HASH:
				case E_WITH_EXIT:
				case E_WITH_ENTER:
					raise_unsupported_operation((operatorcodes)q->operator_index,q->oper_2_values);
					break;
				default:
					RCASSERT(0);
				}
			}
		}

		bool virtual_machine::operator_get_function_right_over_left(operatorcodes oper, const exec_variable *operands)
		{
			if (operands[1].mode()==VAR_OBJECT) {
				if (operands[0].mode()!=VAR_OBJECT) return true;
				// TODO: check, if right inherits from left
				exec_object *l=operands[0].get_object();
				exec_object *r=operands[1].get_object();
				if (l->type() != r->type() && r->type()->inherit_from(l->type())) return true;
			}
			return false;
		}

		const exec_variable *virtual_machine::operator_get_function_2(exec_variable &retfnc, unsigned char &oper_mode, unsigned char oper_index, const exec_variable *operands)
		{
			unsigned int ttt=sizeof(operator_operand_count)/sizeof(operator_operand_count[0]);
			RCASSERT(sizeof(operator_operand_count)/sizeof(operator_operand_count[0])==E_COUNT);

			switch(oper_index) {
			case E_EQ:
			case E_NOTEQ:
			case E_LESSEQ:
			case E_MOREEQ:
			case E_LESS:
			case E_MORE:
			case E_ADD:
			case E_SUB:
			case E_MUL:
			case E_DIV:
			case E_MOD:
			case E_LSHIFT:
			case E_RSHIFT:
			case E_BIN_AND:
			case E_BIN_OR:
			case E_BIN_XOR:
				for(;;) {
					switch(oper_mode) {
					case 0:
						if (operator_get_function_right_over_left((operatorcodes)oper_index,operands)) {
							oper_mode=3;
							break;
						}
						oper_mode=1;
						if (operands[0].lookup_operator(retfnc,*this,(operatorcodes)oper_index)) return &operands[1];
						break;
					case 1:
						oper_mode=2;
						if (operands[1].lookup_operator(retfnc,*this,(operatorcodes)operator_operand_next[oper_index])) return &operands[0];
					case 2:
						return NULL;
					case 3:
						oper_mode=4;
						if (operands[1].lookup_operator(retfnc,*this,(operatorcodes)operator_operand_next[oper_index])) return &operands[0];
						break;
					case 4:
						oper_mode=2;
						if (operands[0].lookup_operator(retfnc,*this,(operatorcodes)oper_index)) return &operands[1];
						return NULL;
					default:
						RCASSERT(0);
					}
				}
			case E_ADD_SELF:
			case E_SUB_SELF:
			case E_MUL_SELF:
			case E_DIV_SELF:
			case E_MOD_SELF:
			case E_LSHIFT_SELF:
			case E_RSHIFT_SELF:
			case E_BIN_AND_SELF:
			case E_BIN_OR_SELF:
			case E_BIN_XOR_SELF:
				for(;;) {
					unsigned char opc=operator_operand_next[oper_index];

					switch(oper_mode) {
					case 0:
						oper_mode=10;
						if (operands[0].lookup_operator(retfnc,*this,(operatorcodes)oper_index)) {
							return &operands[1];
						}
						break;
					case 10: // this value is important
						if (operator_get_function_right_over_left((operatorcodes)opc,operands)) {
							oper_mode=20;
							break;
						}
						oper_mode=11;
						if (operands[0].lookup_operator(retfnc,*this,(operatorcodes)opc)) return &operands[1];
						break;
					case 11:
						oper_mode=12;
						if (operands[1].lookup_operator(retfnc,*this,(operatorcodes)operator_operand_next[opc])) return &operands[0];
					case 12:
						return NULL;
					case 20:
						oper_mode=21;
						if (operands[1].lookup_operator(retfnc,*this,(operatorcodes)operator_operand_next[opc])) return &operands[0];
						break;
					case 21:
						oper_mode=12;
						if (operands[0].lookup_operator(retfnc,*this,(operatorcodes)opc)) return &operands[1];
						return NULL;
					//case 0xff:
					//	oper_mode=0xff;
					//	return operands[0].lookup_operator(retfnc,*this,(operatorcodes)oper_index) ? operands[1] : NULL;
					default:
						RCASSERT(0);
					}
				}
			default:
				RCASSERT(0);
			}
			return NULL;
		}

		void virtual_machine::prepare_call_operator(exec_variable *retval, operatorcodes oper, const exec_variable *operands)
		{
			exec_variable fnc;

			RCASSERT(operator_operand_count[oper]!=0);
			if (operator_return_is_operator_multi[oper]) {
				unsigned char oper_index=oper;
				unsigned char oper_mode=0;

				RCASSERT(operator_operand_count[oper]==2);

				const exec_variable *other=operator_get_function_2(fnc,oper_mode,oper_index,operands);

				if (other) {
					if (prepare_call_function(retval,fnc,other,1)) {
						vm_execution_stack_elem_base *frame=execution_stack->peek_frame();
						// this is needed because function might modify parameters in function and then return without value to force continuing looking

						frame->oper_2_values=new exec_variable[2];
						frame->oper_2_values[0]=operands[0];
						frame->oper_2_values[1]=operands[1];
						frame->oper_2_values[0].gc_acquire();
						frame->oper_2_values[1].gc_acquire();
						frame->operator_mode=oper_mode;
						frame->operator_index=oper_index;
						frame->return_handling_mode=(vm_execution_stack_elem_base::return_handling_mode_)operator_return_handling_mode[oper];
					}
					fnc.gc_release(*this);
				}
				else {
					switch(oper) {
					case E_EQ:
					case E_EQ_R:
						push_frame_returning_bool_value(
							operands[0].mode()==VAR_OBJECT && operands[1].mode()==VAR_OBJECT &&
							operands[0].get_object()==operands[1].get_object(),retval,NULL);
						break;
					case E_NOTEQ:
					case E_NOTEQ_R:
						push_frame_returning_bool_value(!(
							operands[0].mode()==VAR_OBJECT && operands[1].mode()==VAR_OBJECT &&
							operands[0].get_object()==operands[1].get_object()),retval,NULL);
						break;
					case E_LESSEQ:
					case E_MOREEQ:
					case E_LESS:
					case E_MORE:
					case E_LESSEQ_R:
					case E_MOREEQ_R:
					case E_LESS_R:
					case E_MORE_R:
					case E_ADD:
					case E_SUB:
					case E_MUL:
					case E_DIV:
					case E_MOD:
					case E_LSHIFT:
					case E_RSHIFT:
					case E_BIN_AND:
					case E_BIN_OR:
					case E_BIN_XOR:
					case E_ADD_R:
					case E_SUB_R:
					case E_MUL_R:
					case E_DIV_R:
					case E_MOD_R:
					case E_LSHIFT_R:
					case E_RSHIFT_R:
					case E_BIN_AND_R:
					case E_BIN_OR_R:
					case E_BIN_XOR_R:
					case E_ADD_SELF:
					case E_SUB_SELF:
					case E_MUL_SELF:
					case E_DIV_SELF:
					case E_MOD_SELF:
					case E_LSHIFT_SELF:
					case E_RSHIFT_SELF:
					case E_BIN_AND_SELF:
					case E_BIN_OR_SELF:
					case E_BIN_XOR_SELF:
					case E_ACCESS_1_READ:
					case E_ACCESS_1_WRITE:
					case E_ACCESS_2_READ:
					case E_ACCESS_2_WRITE:
					case E_BIN_NOT:
					case E_BOOL:
					case E_SIGN_CHANGE:
					case E_CALL:
					case E_GENERATOR:
					case E_INIT:
					case E_NEW:
					case E_STR:
					case E_HASH:
					case E_WITH_EXIT:
					case E_WITH_ENTER:
						goto raise_unsupported;
						break;
					default:
						RCASSERT(0);
					}
				}
			}
			else if (operands[0].lookup_operator(fnc,*this,oper)) {
				unsigned int cnt=operator_operand_count[oper];

				if (prepare_call_function(retval,fnc,operands+1,cnt-1)) {
					vm_execution_stack_elem_base *frame=execution_stack->peek_frame();

					frame->return_handling_mode=(vm_execution_stack_elem_base::return_handling_mode_)operator_return_handling_mode[oper];
					frame->oper_2_values=new exec_variable[cnt];
					switch(cnt) {
					case 4: frame->oper_2_values[3]=operands[3]; frame->oper_2_values[3].gc_acquire();
					case 3: frame->oper_2_values[2]=operands[2]; frame->oper_2_values[2].gc_acquire();
					case 2: frame->oper_2_values[1]=operands[1]; frame->oper_2_values[1].gc_acquire();
					case 1: frame->oper_2_values[0]=operands[0]; frame->oper_2_values[0].gc_acquire();
						break;
					default:
						RCASSERT(0);
					}
					frame->operator_index=oper;
				}
				fnc.gc_release(*this);
			}
			else {
raise_unsupported:
				raise_unsupported_operation(oper,operands);
			}
		}

		//void virtual_machine::prepare_call_operator(exec_variable *retval, operatorcodes oper, callparams &cp, const exec_variable *self_override, bool replace_current_frame)
		//{
		//	RCASSERT(operator_operand_next[oper]==E_COUNT);
		//	RCASSERT(cp.normal_params_count>0);
		//	exec_variable fnc;
		//	if (cp.normal_params[0].lookup_operator(fnc,*this,oper)) {
		//		cp.normal_params++;
		//		cp.normal_params_count--;
		//		prepare_call_function(retval,fnc,cp,oper);
		//		fnc.gc_release(*this);
		//		execution_stack->elems[execution_stack->index]->operator_mode=0xff;
		//		execution_stack->elems[execution_stack->index]->operator_index=oper;
		//		execution_stack->elems[execution_stack->index]->return_handling_mode=operator_return_handling_mode[oper];
		//	}
		//	else {
		//		raise_unsupported_operation(oper,cp.normal_params[0]);
		//	}
		//}

		class applyparamobject {
			exec_variable *retval;
			virtual_machine &vm;
			union {
				vm_execution_stack_elem_external__ *f__;
				vm_execution_stack_elem_external_0 *f_0;
				vm_execution_stack_elem_external_1 *f_1;
				vm_execution_stack_elem_external_2 *f_2;
				vm_execution_stack_elem_external_3 *f_3;
				vm_execution_stack_elem_external_self__ *s__;
				vm_execution_stack_elem_external_self_0 *s_0;
				vm_execution_stack_elem_external_self_1 *s_1;
				vm_execution_stack_elem_external_self_2 *s_2;
				vm_execution_stack_elem_external_self_3 *s_3;
			} fncptr;
			exec_map_object *mo;
			exec_array_object *ao;
			exec_variable *parameters;
			unsigned int parameterscount;
		public:
			applyparamobject(exec_variable *retval_, virtual_machine &vm_) : retval(retval_),vm(vm_) { }

			exec_function_ptr *fnc;
			//vm_execution_stack_elem_internal *sf;
			//vm_execution_stack_elem_external_base *se;

			vm_execution_stack_elem_base *sb;

			RCLMFUNCTION bool init(const exec_variable &fncvar, unsigned int total_linear_param_count, const exec_variable *self_override)
			{
				exec_variable *self;
				unsigned int paramcnt;

				switch(fnc->mode()) {
				case exec_function_ptr::F_INTERNAL: {
					sb=vm.push_execution_stack_frame_internal(fnc->internal_stack_yield_size(),fnc->internal_temporary_variables_count());
					vm_execution_stack_elem_internal *sf=static_cast<vm_execution_stack_elem_internal*>(sb);
					if (sf==NULL) return false;
					(sf->fnc=fnc)->gc_acquire();
					sf->stack=vm.allocate_stack(fnc);
					sf->opcodes=fnc->internal_opcodes();
					sf->set_code_position(fnc->internal_jump_start());
					sf->tempstackactpos=0;
					sf->return_value=retval;
					if (fnc->internal_self_location().valid()) {
						exec_variable &slf=sf->get_local(fnc->internal_self_location());
						if (self_override) {
							slf=*self_override;
						}
						else {
							switch(fncvar.mode()) {
							case VAR_FUNCTION_FAST:
								vm.set_self(slf,fnc,fncvar.get_function_fast().slf);
								break;
							case VAR_FUNCTION:
								vm.set_self(slf,fnc,fncvar.get_function()->slf);
								break;
							case VAR_OBJECT:
								vm.set_self(slf,fnc,fncvar);
								break;
							default:
								RCASSERT(0);
							}
						}
						slf.gc_acquire();
					}
					sf->r=returnvalueflow::CONTINUE_OPCODES;
					if ((parameterscount=fnc->internal_param_count())>0) {
						const exec_function_ptr::internal_param_info *ipi=&fnc->internal_param(0);
						parameters=&sf->get_local(ipi[0].var->location);
					}
					if (total_linear_param_count>fnc->internal_param_count()) {
						if (fnc->internal_list_param().location.valid()) {
							exec_variable &v=sf->get_local(fnc->internal_list_param().location);
							v.set_object(vm.allocate_array(ao));
							ao->resize(vm,total_linear_param_count-fnc->internal_param_count());
						}
						else {
							vm.raise_too_many_parameters(fnc);
							return false;
						}
					}
					else if (fnc->internal_list_param().location.valid()) {
						exec_variable &v=sf->get_local(fnc->internal_list_param().location);
						v.set_object(vm.allocate_array(ao));
					}

					return true; }
				case exec_function_ptr::F_SELF:
					if ((sb=fncptr.s__=fnc->external_function().s__(vm,fnc->external_user_pointer()))==NULL) return false;
					fncptr.s__->return_value=retval;
					self=&fncptr.s__->v_self;
					paramcnt=0xffffffff;
f_self:
					if (self_override) *self=*self_override;
					else {
						switch(fncvar.mode()) {
						case VAR_FUNCTION_FAST:
							self->set_object_null(fncvar.get_function_fast().slf);
							break;
						case VAR_FUNCTION:
							*self=fncvar.get_function()->slf;
							break;
						case VAR_OBJECT:
							*self=fncvar;
							break;
						default:
							RCASSERT(0);
						}
					}
					self->gc_acquire();
f_fast:
					if (total_linear_param_count>paramcnt) {
						(static_cast<vm_execution_stack_elem_external_base*>(sb)->fnc=fnc)->gc_acquire();
						vm.raise_too_many_parameters(fnc);
						return false;
					}
f_fast_done:
					(static_cast<vm_execution_stack_elem_external_base*>(sb)->fnc=fnc)->gc_acquire();
					return true;
				case exec_function_ptr::F_FAST:
					if ((sb=fncptr.f__=fnc->external_function().f__(vm,fnc->external_user_pointer()))==NULL) return false;
					fncptr.f__->return_value=retval;
					goto f_fast_done;
				case exec_function_ptr::F_SELF_0:
					paramcnt=0;
					if ((sb=fncptr.s_0=fnc->external_function().s_0(vm,fnc->external_user_pointer()))==NULL) return false;
					parameterscount=0;
					self=&fncptr.s_0->v_self;
					fncptr.s_0->return_value=retval;
					goto f_self;
				case exec_function_ptr::F_SELF_1:
					paramcnt=1;
					if ((sb=fncptr.s_1=fnc->external_function().s_1(vm,fnc->external_user_pointer()))==NULL) return false;
					parameters=fncptr.s_1->v_params;
					parameterscount=1;
					self=&fncptr.s_1->v_self;
					fncptr.s_1->return_value=retval;
self_1_reset:
					parameters[0].reset();
					goto f_self;
				case exec_function_ptr::F_SELF_2:
					paramcnt=2;
					if ((sb=fncptr.s_2=fnc->external_function().s_2(vm,fnc->external_user_pointer()))==NULL) return false;
					parameters=fncptr.s_2->v_params;
					parameterscount=2;
					self=&fncptr.s_2->v_self;
					fncptr.s_2->return_value=retval;
self_2_reset:
					parameters[1].reset();
					goto self_1_reset;
				case exec_function_ptr::F_SELF_3:
					paramcnt=3;
					if ((sb=fncptr.s_3=fnc->external_function().s_3(vm,fnc->external_user_pointer()))==NULL) return false;
					parameters=fncptr.s_3->v_params;
					parameterscount=3;
					self=&fncptr.s_3->v_self;
					fncptr.s_3->return_value=retval;
					parameters[2].reset();
					goto self_2_reset;
				case exec_function_ptr::F_FAST_0:
					paramcnt=0;
					if ((sb=fncptr.f_0=fnc->external_function().f_0(vm,fnc->external_user_pointer()))==NULL) return false;
					parameterscount=0;
					fncptr.f_0->return_value=retval;
					goto f_fast;
				case exec_function_ptr::F_FAST_1:
					paramcnt=1;
					if ((sb=fncptr.f_1=fnc->external_function().f_1(vm,fnc->external_user_pointer()))==NULL) return false;
					parameters=fncptr.f_1->v_params;
					parameterscount=1;
					fncptr.f_1->return_value=retval;
fast_1_reset:
					parameters[0].reset();
					goto f_fast;
				case exec_function_ptr::F_FAST_2:
					paramcnt=2;
					if ((sb=fncptr.f_2=fnc->external_function().f_2(vm,fnc->external_user_pointer()))==NULL) return false;
					parameters=fncptr.f_2->v_params;
					parameterscount=2;
					fncptr.f_2->return_value=retval;
fast_2_reset:
					parameters[1].reset();
					goto fast_1_reset;
				case exec_function_ptr::F_FAST_3:
					paramcnt=3;
					if ((sb=fncptr.f_3=fnc->external_function().f_3(vm,fnc->external_user_pointer()))==NULL) return false;
					parameters=fncptr.f_3->v_params;
					parameterscount=3;
					fncptr.f_3->return_value=retval;
					parameters[2].reset();
					goto fast_2_reset;
				default:
					RCASSERT(0);
				}
				return true;
			}

			RCLMFUNCTION bool apply_parameters_list(const exec_variable *params, unsigned int count, unsigned int start_index, bool islistparams)
			{
				switch(fnc->mode()) {
				case exec_function_ptr::F_INTERNAL:
					if (count>0) {
						vm_execution_stack_elem_internal *sf=static_cast<vm_execution_stack_elem_internal*>(sb);

#ifdef RCDEBUG
						for(unsigned int i=0;i<fnc->internal_param_count();++i) {
							RCASSERT(fnc->internal_param(i).var->location.offset()==i);
						}
#endif
						if (start_index<fnc->internal_param_count()) {
							const exec_function_ptr::internal_param_info *ipi=&fnc->internal_param(start_index);
							exec_variable *dst=&sf->get_local(ipi[0].var->location);
							unsigned int cnt=std::min(fnc->internal_param_count()-start_index,count);
							for(unsigned int i=0;i<cnt;++i) {
								RCASSERT(dst[i].mode()==VAR_NULL);
								dst[i]=params[i];
								dst[i].gc_acquire();
							}
							params+=cnt;
							start_index+=cnt;
							count-=cnt;
						}
						if (count>0) {
							RCASSERT(ao);
							RCASSERT(start_index>=fnc->internal_param_count());
							RCASSERT(start_index+count<=fnc->internal_param_count()+ao->size());

							exec_variable *dst=&ao->get(start_index-fnc->internal_param_count());
							for(unsigned int i=0;i<count;++i) {
								RCASSERT(dst[i].mode()==VAR_NULL);
								dst[i]=params[i];
								dst[i].gc_acquire();
							}
						}
					}
					return true;
				case exec_function_ptr::F_FAST:
					if (islistparams) {
						fncptr.f__->cp.list_params=params;
						fncptr.f__->cp.list_params_count=count;
					}
					else {
						fncptr.f__->cp.normal_params=params;
						fncptr.f__->cp.normal_params_count=count;
					}
					return true;
				case exec_function_ptr::F_SELF:
					if (islistparams) {
						fncptr.s__->cp.list_params=params;
						fncptr.s__->cp.list_params_count=count;
					}
					else {
						fncptr.s__->cp.normal_params=params;
						fncptr.s__->cp.normal_params_count=count;
					}
					return true;
				case exec_function_ptr::F_FAST_0:
				case exec_function_ptr::F_FAST_1:
				case exec_function_ptr::F_FAST_2:
				case exec_function_ptr::F_FAST_3:
				case exec_function_ptr::F_SELF_0:
				case exec_function_ptr::F_SELF_1:
				case exec_function_ptr::F_SELF_2:
				case exec_function_ptr::F_SELF_3:
					for(unsigned int i=start_index;i<start_index+count;++i) {
						parameters[i]=params[i-start_index];
						parameters[i].gc_acquire();
					}
					return true;
				default:
					RCASSERT(0);
					return true;
				}
			}

			RCLMFUNCTION bool apply_parameters_map_prepare(unsigned int normal_list_params)
			{
				mo=NULL;

				switch(fnc->mode()) {
				case exec_function_ptr::F_INTERNAL: {
					if (normal_list_params<fnc->internal_param_count()) {
						const exec_function_ptr::internal_param_info *ipi=&fnc->internal_param(0);
						exec_variable *dst=&static_cast<vm_execution_stack_elem_internal*>(sb)->get_local(ipi[0].var->location);

						for(unsigned int i=normal_list_params;i<fnc->internal_param_count();++i) dst[i].setmode(VAR_NO_PARAM_GIVEN);
					}
					return true; }
				case exec_function_ptr::F_FAST:
				case exec_function_ptr::F_SELF:
					return true;
				case exec_function_ptr::F_FAST_0:
				case exec_function_ptr::F_SELF_0:
				case exec_function_ptr::F_FAST_1:
				case exec_function_ptr::F_SELF_1:
				case exec_function_ptr::F_FAST_2:
				case exec_function_ptr::F_SELF_2:
				case exec_function_ptr::F_FAST_3:
				case exec_function_ptr::F_SELF_3:
					for(unsigned int i=normal_list_params;i<parameterscount;++i) {
						parameters[i].setmode(VAR_NO_PARAM_GIVEN);
					}
					return true;
				default:
					RCASSERT(0);
					return true;
				}
			}

			RCLMFUNCTION bool apply_parameters_keyword_param(owca_internal_string *ident, const exec_variable &value)
			{
				switch(fnc->mode()) {
				case exec_function_ptr::F_FAST:
					if (!mo) {
						fncptr.f__->mapobject=vm.allocate_map(mo);
					}
cont:
					mo->ident_insert(vm,ident,value);
					return true;
				case exec_function_ptr::F_SELF:
					if (!mo) {
						fncptr.s__->mapobject=vm.allocate_map(mo);
					}
					goto cont;
				case exec_function_ptr::F_INTERNAL:
					for(unsigned int i=0;i<fnc->internal_param_count();++i) {
						if (fnc->internal_param(i).var->ident->equal(ident)) {
							if (parameters[i].mode()!=VAR_NO_PARAM_GIVEN) {
								vm.raise_param_assigned_twice(ident);
								return false;
							}
							parameters[i]=value;
							parameters[i].gc_acquire();
							return true;
						}
					}
					if (!mo) {
						if (fnc->internal_map_param().location.valid()) {
							exec_variable &v=static_cast<vm_execution_stack_elem_internal*>(sb)->get_local(fnc->internal_map_param().location);
							v.set_object(vm.allocate_map(mo));
						}
						else {
							vm.raise_unused_keyword_param(ident);
							return false;
						}
					}
					goto cont;
				case exec_function_ptr::F_FAST_1:
				case exec_function_ptr::F_SELF_1:
				case exec_function_ptr::F_FAST_2:
				case exec_function_ptr::F_SELF_2:
				case exec_function_ptr::F_FAST_3:
				case exec_function_ptr::F_SELF_3:
					for(unsigned int i=0;i<parameterscount;++i) {
						if (fnc->external_param_name(i)->equal(ident)) {
							if (parameters[i].mode()!=VAR_NO_PARAM_GIVEN) {
								vm.raise_param_assigned_twice(ident);
								return false;
							}
							parameters[i]=value;
							parameters[i].gc_acquire();
							return true;
						}
					}
				case exec_function_ptr::F_FAST_0:
				case exec_function_ptr::F_SELF_0:
					vm.raise_unused_keyword_param(ident);
					return false;
				default:
					RCASSERT(0);
					return true;
				}
			}
			RCLMFUNCTION bool apply_parameters_keyword_params(virtual_machine::keyword_param_iterator *iterator)
			{
				owca_internal_string *ident;
				const exec_variable *value;

				while(iterator->next(ident,value)) {
					if (!apply_parameters_keyword_param(ident,*value)) return false;
				}
				return true;
			}

			RCLMFUNCTION bool apply_parameters_map_object(exec_map_object *map_param)
			{
				exec_map_object_iterator iter;

				while(map_param->map.next(iter)) {
					exec_variable &key=map_param->map.getkey(iter).k;
					if (key.mode()!=VAR_STRING) {
						vm.raise_keowca_map_param_not_string();
						return false;
					}
					exec_variable &value=map_param->map.getval(iter);
					if (!apply_parameters_keyword_param(key.get_string(),value)) return false;
				}
				return true;
			}

			RCLMFUNCTION bool apply_parameters_finalize(unsigned int normal_and_list_params, bool simple_mode_params)
			{
				switch(fnc->mode()) {
				case exec_function_ptr::F_INTERNAL: {
					vm_execution_stack_elem_internal *sf=static_cast<vm_execution_stack_elem_internal*>(sb);

					unsigned int mxparams=fnc->internal_param_count();
					if (mxparams>0) {
						const exec_function_ptr::internal_param_info *ipi=&fnc->internal_param(0);
						exec_variable *dst=&sf->get_local(ipi[0].var->location);
						for(unsigned int i=normal_and_list_params;i<mxparams;++i) {
							if (simple_mode_params || dst[i].mode()==VAR_NO_PARAM_GIVEN) {
								if (fnc->internal_param(i).defaultvalue.mode()!=VAR_NO_DEF_VALUE) {
									dst[i]=fnc->internal_param(i).defaultvalue;
									dst[i].gc_acquire();
								}
								else {
									vm.raise_not_enough_parameters(fnc);
									return false;
								}
							}
						}
					}
					if ((mo==NULL || simple_mode_params) && fnc->internal_map_param().location.valid()) {
						exec_variable &v=sf->get_local(fnc->internal_map_param().location);
						v.set_object(vm.allocate_map(mo));
					}
					if (simple_mode_params && fnc->internal_list_param().location.valid()) {
						exec_variable &v=sf->get_local(fnc->internal_list_param().location);
						if (v.mode()==VAR_NULL) {
							exec_object *o;
							o=vm.allocate_array(ao);
							v.set_object(o);
						}
					}
					sb->show_in_exception_stack(true);
					return true; }
				case exec_function_ptr::F_FAST:
					if (simple_mode_params) fncptr.f__->cp.set_simple();
					else fncptr.f__->cp.map=mo;
					break;
				case exec_function_ptr::F_SELF:
					if (simple_mode_params) fncptr.s__->cp.set_simple();
					else fncptr.s__->cp.map=mo;
					break;
				case exec_function_ptr::F_SELF_0:
				case exec_function_ptr::F_FAST_0:
				case exec_function_ptr::F_SELF_1:
				case exec_function_ptr::F_FAST_1:
				case exec_function_ptr::F_SELF_2:
				case exec_function_ptr::F_FAST_2:
				case exec_function_ptr::F_SELF_3:
				case exec_function_ptr::F_FAST_3:
					for(unsigned int i=normal_and_list_params;i<parameterscount;++i) {
						if (simple_mode_params || parameters[i].mode()==VAR_NO_PARAM_GIVEN) {
							if (fnc->external_param_default_value(i).mode()!=VAR_NO_DEF_VALUE) {
								parameters[i]=fnc->external_param_default_value(i);
								parameters[i].gc_acquire();
							}
							else {
								vm.raise_not_enough_parameters(fnc);
								return false;
							}
						}
					}
					break;
				default:
					RCASSERT(0);
				}
				sb->show_in_exception_stack(true);
				return true;
			}
		};

		RCLMFUNCTION bool virtual_machine::prepare_call_function(exec_variable *retval, const exec_variable &fncvar, const exec_variable *params, unsigned int paramcnt)
		{
			exec_function_ptr *fnc;

			switch(fncvar.mode()) {
			case VAR_FUNCTION_FAST:
				fnc=fncvar.get_function_fast().fnc;
				break;
			case VAR_FUNCTION:
				fnc=fncvar.get_function()->function();
				break;
			case VAR_GENERATOR:
				if (paramcnt!=0) {
					raise_too_many_parameters(OWCA_ERROR_FORMAT("resuming a generator requires no parameters"));
					return false;
				}
				if (push_execution_stack_frame(fncvar.get_generator())) {
					fncvar.get_generator()->gc_acquire();
					fncvar.get_generator()->return_value=retval;
					return true;
				}
				return false;
			case VAR_OBJECT:
				if ((fnc=fncvar.get_operator_function(*this,E_CALL))==NULL) {
unsupported:
					raise_unsupported_call_operation(fncvar);
					return false;
				}
				break;
			default:
				goto unsupported;
			}

			applyparamobject apo(retval,*this);
			apo.fnc=fnc;
			return apo.init(fncvar,paramcnt,NULL) && apo.apply_parameters_list(params,paramcnt,0,false) && apo.apply_parameters_finalize(paramcnt,true);
		}

		RCLMFUNCTION bool virtual_machine::prepare_call_function(exec_variable *retval, const exec_variable &fncvar, const exec_variable *listvar, const exec_variable *mapvar, keyword_param_iterator &kpi, const exec_variable *params, unsigned int paramscnt, const exec_variable *self_override)
		{
			exec_function_ptr *fnc;

			switch(fncvar.mode()) {
			case VAR_FUNCTION_FAST:
				fnc=fncvar.get_function_fast().fnc;
				break;
			case VAR_FUNCTION:
				fnc=fncvar.get_function()->function();
				break;
			case VAR_GENERATOR:
				if (paramscnt!=0) {
generator_params:
					raise_too_many_parameters(OWCA_ERROR_FORMAT("resuming a generator requires no parameters"));
					return false;
				}
				if (listvar) {
					if (listvar->mode()!=VAR_OBJECT) goto generator_params;
					if (exec_array_object *o=data_from_object<exec_array_object>(listvar->get_object())) {
						if (o->size()>0) goto generator_params;
					}
					else if (exec_tuple_object *o=data_from_object<exec_tuple_object>(listvar->get_object())) {
						if (o->size()>0) goto generator_params;
					}
					else goto generator_params;
				}
				if (mapvar) {
					exec_map_object *mo;

					if (mapvar->mode()!=VAR_OBJECT || (mo=data_from_object<exec_map_object>(mapvar->get_object()))==NULL || mo->map.size()>0) goto generator_params;
				}
				if (kpi.count()>0) goto generator_params;
				if (push_execution_stack_frame(fncvar.get_generator())) {
					fncvar.get_generator()->gc_acquire();
					fncvar.get_generator()->return_value=retval;
					return true;
				}
				return false;
			case VAR_OBJECT:
				if ((fnc=fncvar.get_operator_function(*this,E_CALL))==NULL) {
unsupported:
					raise_unsupported_call_operation(fncvar);
					return false;
				}
				break;
			default:
				goto unsupported;
			}

			applyparamobject apo(retval,*this);
			apo.fnc=fnc;

			exec_variable *listarr=NULL;
			unsigned int size;
			unsigned int totalparams=paramscnt;
			bool listexception=false;

			if (listvar) {
				if (listvar->mode()==VAR_OBJECT) {
					if (exec_array_object *o=data_from_object<exec_array_object>(listvar->get_object())) {
						if ((size=o->size())>0) listarr=&o->get(0);
						totalparams+=size;
						goto cont_list;
					}
					if (exec_tuple_object *o=data_from_object<exec_tuple_object>(listvar->get_object())) {
						if ((size=o->size())>0) listarr=&o->get(0);
						totalparams+=size;
						goto cont_list;
					}
				}
				listexception=true;
			}
cont_list:

			if (!apo.init(fncvar,totalparams,self_override)) return false;

			if (!apo.apply_parameters_list(params,paramscnt,0,false)) return false;
			if (listvar) {
				if (listexception) {
					execution_stack->pop_frame(apo.sb);
					raise_invalid_list_param(*listvar);
					return false;
				}
				if (!apo.apply_parameters_list(listarr,size,paramscnt,true)) return false;
			}
			else if (!apo.apply_parameters_list(NULL,0,0,true)) return false;

			if (!apo.apply_parameters_map_prepare(totalparams)) return false;
			if (!apo.apply_parameters_keyword_params(&kpi)) return false;

			if (mapvar) {
				exec_map_object *mo;

				if (mapvar->mode()!=VAR_OBJECT || (mo=data_from_object<exec_map_object>(mapvar->get_object()))==NULL) {
					execution_stack->pop_frame(apo.sb);
					raise_invalid_map_param(*mapvar);
					return false;
				}
				if (!apo.apply_parameters_map_object(mo)) return false;
			}

			if (!apo.apply_parameters_finalize(totalparams,false)) return false;

			return true;
		}

		RCLMFUNCTION bool virtual_machine::prepare_call_function(exec_variable *retval, const exec_variable &fncvar, const callparams &cp, const exec_variable *self_override)
		{
			exec_function_ptr *fnc;

			switch(fncvar.mode()) {
			case VAR_FUNCTION_FAST:
				fnc=fncvar.get_function_fast().fnc;
				break;
			case VAR_FUNCTION:
				fnc=fncvar.get_function()->function();
				break;
			case VAR_GENERATOR:
				if (cp.normal_params_count+cp.list_params_count!=0 || (cp.map && cp.map->map.size()>0)) {
					raise_too_many_parameters(OWCA_ERROR_FORMAT("resuming a generator requires no parameters"));
					return false;
				}
				if (push_execution_stack_frame(fncvar.get_generator())) {
					fncvar.get_generator()->gc_acquire();
					fncvar.get_generator()->return_value=retval;
					return true;
				}
				return false;
			case VAR_OBJECT:
				if ((fnc=fncvar.get_operator_function(*this,E_CALL))==NULL) {
unsupported:
					raise_unsupported_call_operation(fncvar);
					return false;
				}
				break;
			default:
				goto unsupported;
			}

			applyparamobject apo(retval,*this);
			apo.fnc=fnc;
			unsigned int paramcnt=cp.normal_params_count+cp.list_params_count;

			if (!apo.init(fncvar,paramcnt,self_override)) return false;
			if (!apo.apply_parameters_list(cp.normal_params,cp.normal_params_count,0,false) || !apo.apply_parameters_list(cp.list_params,cp.list_params_count,cp.normal_params_count,true)) return false;
			if (!apo.apply_parameters_map_prepare(paramcnt)) return false;
			if (cp.map && !apo.apply_parameters_map_object(cp.map)) return false;
			if (!apo.apply_parameters_finalize(paramcnt,false)) return false;
			return true;
		}


		bool virtual_machine::_convert(owca_local &s, const exec_variable &v)
		{
			s._object.gc_release(*s._vm);
			s._object=v;
			s._object.gc_acquire();
			s._update_vm(this);
			return true;
		}

		bool virtual_machine::_convert(owca_global &s, const exec_variable &v)
		{
			s._object.gc_release(*s._vm);
			s._object=v;
			s._object.gc_acquire();
			s._update_vm(this);
			return true;
		}

		bool virtual_machine::_convert(exec_property *&s, const exec_variable &v)
		{
			if (v.mode()==VAR_PROPERTY) {
				s=v.get_property();
				return true;
			}
			return false;
		}

		bool virtual_machine::_convert(exec_class_object *&s, const exec_variable &v)
		{
			return v.mode()==VAR_OBJECT && (s=data_from_object<exec_class_object>(v.get_object()))!=NULL;
		}

		bool virtual_machine::_convert(vm_execution_stack_elem_base *&s, const exec_variable &v)
		{
			if (v.mode()==VAR_GENERATOR) {
				s=v.get_generator();
				return true;
			}
			return false;
		}

		bool virtual_machine::_convert(unifunction &s, const exec_variable &v) {
			switch(v.mode()) {
			case VAR_FUNCTION:
				s.fnc_=v.get_function()->function();
				s.slf_=v.get_function()->slf;
				return true;
			case VAR_FUNCTION_FAST:
				s.fnc_=v.get_function_fast().fnc;
				s.slf_.set_object_null(v.get_function_fast().slf);
				return true;
			default:
				return false;
			}
		}

		bool virtual_machine::_convert(owca_internal_string *&s, const exec_variable &v) {
			if (v.mode()!=VAR_STRING) return false;
			s=v.get_string();
			return true;
		}

		bool virtual_machine::_convert(owca_string &s, const exec_variable &v) {
			if (v.mode()!=VAR_STRING) return false;
			s._ss=v.get_string();
			s._vm=this;
			return true;
		}

		bool virtual_machine::_convert(owca_int &s, const exec_variable &v)
		{
			switch(v.mode()) {
			case VAR_INT:
				s=v.get_int();
				return true;
			case VAR_REAL:
				s=(owca_int)v.get_real();
				return (owca_real)s==v.get_real();
			default:
				return false;
			}
		}

		bool virtual_machine::_convert(bool &s, const exec_variable &v)
		{
			if (v.mode()==VAR_BOOL) {
				s=v.get_bool();
				return true;
			}
			return false;
		}

		bool virtual_machine::_convert(owca_real &s, const exec_variable &v)
		{
			switch(v.mode()) {
			case VAR_INT:
				s=(owca_real)v.get_int();
				return (owca_int)s==v.get_int();
			case VAR_REAL:
				s=v.get_real();
				return true;
			default:
				return false;
			}
		}

		bool virtual_machine::_convert(null &s, const exec_variable &v)
		{
			return v.mode()==VAR_NULL;
		}

		bool virtual_machine::_convert(exec_weakref_object *&s, const exec_variable &v)
		{
			if (v.mode()!=VAR_WEAK_REF) return false;
			s=v.get_weak_ref();
			return true;
		}

		bool virtual_machine::_convert(exec_object *&s, const exec_variable &v)
		{
			if (v.mode()!=VAR_OBJECT) return false;
			s=v.get_object();
			return true;
		}


		void vislocker::gc_mark(gc_iteration &g)
		{
			switch(mode) {
			case VARIABLE_ARRAY:
				for(unsigned int i=0;i<data.variable_array.cnt;++i) data.variable_array.v[i].gc_mark(g);
				break;
			case VARIABLE:
				data.variable.v->gc_mark(g);
				break;
			case EXEC_BASE:
				data.execbase.v->gc_mark(g);
				break;
			case EXEC_OBJECT_PTR:
				if (*data.execobjectptr.v) (*data.execobjectptr.v)->gc_mark(g);
				break;
			case EXEC_OBJECT_LIST:
				for(std::list<exec_object*>::const_iterator it=data.execobjectlist.v->begin();it!=data.execobjectlist.v->end();++it) {
					(*it)->gc_mark(g);
				}
				break;
			case EXEC_VARIABLE_LIST:
				for(std::list<exec_variable>::const_iterator it=data.variablelist.v->begin();it!=data.variablelist.v->end();++it) {
					it->gc_mark(g);
				}
				break;
			case EXEC_VARIABLE_PAIR_ARRAY:
				for(std::vector<vislocker_pair>::const_iterator it=data.variablepairarray.v->begin();it!=data.variablepairarray.v->end();++it) {
					it->key.gc_mark(g);
					it->val.gc_mark(g);
				}
				break;
			default:
				RCASSERT(0);
			}
		}

#ifdef RCDEBUG
		unsigned int vislocker::_id_cnt=0;
#endif

#ifdef RCDEBUG_MEMORY_BLOCKS
		unsigned int fastallocator_id=1;

		bool fastallocator_id_debug(unsigned int id)
		{
			return false;
		}
#endif

#ifdef RCDEBUG_LINE_NUMBERS
		std::vector<std::pair<std::string,unsigned int> > _rc_linemarker_index_to_file;
		std::vector<unsigned int> _rc_linemarker_index_value;

		void _rc_linemarker_print()
		{
			unsigned int total=0;
			FILE *f;

			f=fopen("todoupd.txt","wb");
			for(unsigned int i=0;i<_rc_linemarker_index_value.size();++i) {
				if (_rc_linemarker_index_value[i]==0) {
					if (f) fprintf(f,"%d %s %d\n",i,_rc_linemarker_index_to_file[i].first.c_str(),_rc_linemarker_index_to_file[i].second);
					//sprintf(buf,"! %d %s %d\n",i,_rc_linemarker_index_to_file[i].first.c_str(),_rc_linemarker_index_to_file[i].second);
					//debugprint(buf);
					++total;
				}
			}
			if (f) fclose(f);
			//sprintf(buf,"total %d\n",total);
			//debugprint(buf);
		}

		void _rc_linemarker_mark(unsigned int index)
		{
			_rc_linemarker_index_value[index]++;
		}
#endif

		const char *__find(const char *p)
		{
			while(*p && *p!='%') ++p;
			return p;
		}

		std::string __exception_format(const char *msg)
		{
			RCASSERT(*__find(msg)==0);
			return msg;
		}

		std::string __exception_format1(const char *msg, const std::string &p1)
		{
			std::string z;
			bool used[1]={false};

			for(;;) {
				const char *ptr=__find(msg);
				z+=std::string(msg,ptr-msg);
				if (*ptr==0) return z;
				msg=ptr+1;
				switch(*msg) {
				case '1': z+=p1; used[0]=true; break;
				case '%': z+='%'; break;
				default:
					RCASSERT(0);
				}
				++msg;
			}
			RCASSERT(used[0] && used[1]);
		}

		std::string __exception_format2(const char *msg, const std::string &p1, const std::string &p2)
		{
			std::string z;
			bool used[2] = { false, false };

			for (;;) {
				const char *ptr = __find(msg);
				z += std::string(msg, ptr - msg);
				if (*ptr == 0) return z;
				msg = ptr + 1;
				switch (*msg) {
				case '1': z += p1; used[0] = true; break;
				case '2': z += p2; used[1] = true; break;
				case '%': z += '%'; break;
				default:
					RCASSERT(0);
				}
				++msg;
			}
			RCASSERT(used[0] && used[1]);
		}

		vmstack::vmstack(virtual_machine &vm_) : vm(vm_)
		{
			vm.push_execution_stack();
		}

		vmstack::~vmstack()
		{
			vm.pop_execution_stack();
		}
	}
}


