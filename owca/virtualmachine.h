#ifndef _RC_Y_VIRTUALMACHINE_H
#define _RC_Y_VIRTUALMACHINE_H

#include "gc_iterator.h"
#include "debug_memory_blocks.h"
#include "exec_base.h"
#include "exec_variable.h"
#include "structinfo.h"
#include "operatorcodes.h"
#include "exception.h"
#include "compile_visible_items.h"
#include "executionreturnvalue.h"
#include "debug_interface.h"
#include <memory>

namespace owca {
	class owca_global;
	class owca_message_list;
	class owca_source_file;
	class owca_string;
	class owca_vm;
	class returnvalue;
	namespace __owca__ {
		class opcode_data;
		struct callparams;
		class unifunction;
		template <unsigned int SIZE> class fastallocator;
		class opcode_executer;
		class stringbuffer;
		struct structinfo;
		class vislocker;
		struct vislocker_pair;
		class exec_array_object;
		class exec_base;
		class exec_class_data;
		class exec_class_object;
		class exec_function_bound;
		class exec_function_ptr;
		class exec_generator_function;
		class exec_map_object;
		class exec_set_object;
		class exec_object;
		class exec_property;
		class exec_stack_element_object;
		class exec_stack;
		class exec_stack_variables;
		class exec_tuple_object;
		class exec_variable;
		class exec_variable_location;
		class internal_class;
		class virtual_machine;
		class owca_internal_string;
		class owca_internal_string_nongc;
		class structid_type;
		class exec_function_stack_data;
		class op_flow_data_object;
		struct vm_execution_stack_elem_base;
		struct vm_execution_stack_elem_internal;
		struct vm_execution_stack_elem_external_base;
		class exec_namespace;
		class exec_weakref_object;
	}
}

#include "operatorcodes.h"


namespace owca {

	namespace __owca__ {
#ifdef RCDEBUG_MEMORY_BLOCKS
		extern unsigned int fastallocator_id;
		bool fastallocator_id_debug(unsigned int id);
#endif
		template <unsigned int SIZE> class fastallocator {
			enum { MULTI=sizeof(unsigned int)*8, MAGIC=0x12de34bf };
			struct elem;
			struct element {
				elem *owner;
				union {
					element *next;
					struct {
#ifdef RCDEBUG_MEMORY_BLOCKS
						unsigned int magic;
						unsigned int fa_id;
#endif
						char data[SIZE];
#ifdef RCDEBUG_MEMORY_BLOCKS
						unsigned int magic2;
#endif
					} data;
				} data;
				void *acquire() {
					const int index=owner->get_index(this);
					//const int index=(int)(this-owner->el);
					const unsigned int mask=(1<<index);
					RCASSERT(index>=0 && index<MULTI);
					owner->mode|=mask;
#ifdef RCDEBUG_MEMORY_BLOCKS
					if (fastallocator_id_debug(fastallocator_id)) {
						int z;
						z=4;
					}
					data.data.fa_id=fastallocator_id++;
					data.data.magic=MAGIC;
					data.data.magic2=MAGIC;
#endif
					return data.data.data;
				}
				void release() {
#ifdef RCDEBUG_MEMORY_BLOCKS
					RCASSERT(data.data.magic==MAGIC);
					RCASSERT(data.data.magic2==MAGIC);
					RCASSERT(data.data.fa_id>0);
#endif
					const int index=owner->get_index(this);
					//const int index=(int)(this-owner->el);
					const unsigned int mask=(1<<index);
					RCASSERT(index>=0 && index<MULTI);
#ifdef RCDEBUG_MEMORY_BLOCKS
					data.data.fa_id=0;
					data.data.magic=0;
					data.data.magic2=0;
#endif
					owner->mode&=~mask;
					data.next=owner->first;
					owner->first=this;
				}
			};
			struct elem {
				element el[MULTI],*first;
				elem *prev,*next,*next_free;
				unsigned int mode;
				static unsigned int &count() {
					static unsigned int c=0;
					return c;
				}
				int get_index(element *e) {
					const int index=(int)(e-el);
					RCASSERT(index>=0 && index<MULTI);
					RCASSERT(((((char*)e)-((char*)el)) % sizeof(element))==0);
					return index;
				}
				elem() {
					++count();
					for(unsigned int i=0;i<MULTI;++i) {
						el[i].owner=this;
						el[i].data.next=(i==MULTI-1 ? NULL : el+i+1);
					}
					first=el;
					mode=~0;
				}
				~elem() { --count(); }
				bool full() const { return first==NULL; }
				bool empty() const { return mode==0; }
				void *acquire() {
					element *nxt=first->data.next;
					void *p=first->acquire();
					first=nxt;
					return p;
				}
			};
			elem *first_free;
			elem root;
			unsigned int total;
		public:
			fastallocator() : total(0) {
				root.next_free=NULL;
				first_free=&root;
				root.next=root.prev=&root;
			}
			unsigned int count() const { return elem::count(); }
			const unsigned int blocksize() const { return SIZE; }
			void *alloc() {
				++total;
				if (first_free==NULL) {
					elem *e=new elem;
					e->next=root.next;
					e->prev=&root;
					e->next->prev=e->prev->next=e;
					e->next_free=first_free;
					first_free=e;
				}
				void *res=first_free->acquire();
				if (first_free->full()) first_free=first_free->next_free;
				return res;
			}
			void release(void *p) {
				--total;
				const int diff=(int)(((char*)(((element*)0)->data.data.data))-((char*)0));
				element *e=(element*)(((char*)p)-diff);
				e->release();
				if (e->owner->full()) {
					e->owner->next_free=first_free;
					first_free=e->owner;
				}
			}
			void purge() {
				elem *e=root.next;
				if (root.full()) first_free=NULL;
				else {
					root.next_free=NULL;
					first_free=&root;
				}
				while(e!=&root) {
					if (e->empty()) {
						e->next->prev=e->prev;
						e->prev->next=e->next;
						elem *q=e->prev;
						delete e;
						e=q;
					}
					else if (!e->full()) {
						e->next_free=first_free;
						first_free=e;
					}
					e=e->next;
				}
			}
		};

		class vm_execution_stack : public exec_base {
			friend class virtual_machine;

			enum { MAX_EXECUTION_STACK_DEPTH=128,MAX_EXECUTION_STACK_SIZE=MAX_EXECUTION_STACK_DEPTH+5 };
			vm_execution_stack_elem_base *elems[MAX_EXECUTION_STACK_SIZE];
			vm_execution_stack *prev_stack;
			exec_object *_coroutine;
			bool check_stack_overflow;
			int index;
			bool execution_stacks_frame_gc_acquired_by_vm = false;
		protected:
			void _mark_gc(const gc_iteration &gc) const;
			void _release_resources(virtual_machine &vm);
		public:
			virtual_machine *vm;
			exec_variable *coretval;

			vm_execution_stack() : check_stack_overflow(true),index(-1),prev_stack(NULL),_coroutine(NULL),coretval(NULL) { }

			unsigned int count() const { return index+1; }
			exec_object *coroutine_object() const { return _coroutine; }
			void set_coroutine_object(exec_object *b) { _coroutine=b; }
			vm_execution_stack_elem_base *peek_frame_indexed(unsigned int ind) const { RCASSERT(index>=0 && ind<=(unsigned int)index); return elems[ind]; }
			vm_execution_stack *prev() const { return prev_stack; }
			void set_prev(vm_execution_stack *st) { prev_stack=st; }
			vm_execution_stack_elem_base *peek_frame() const { RCASSERT(index>=0); return elems[index]; }
			void pop_frame(vm_execution_stack_elem_base *);
			bool push_frame(virtual_machine &vm, vm_execution_stack_elem_base *);
			bool empty() { return index<0; }
			void clear(void);
		};

		struct null { };

		class virtual_machine {
			friend class owca_global;

			class exec_rootobject : public exec_base {
			public:
				void _mark_gc(const gc_iteration &gc) const { }
				void _release_resources(virtual_machine &vm) { }
			};
			exec_rootobject *rootobject;
		public:
			owca_internal_string *operator_identificators[E_COUNT];

			std::vector<exec_function_ptr*> builtinfunctions;

			void init_builtin_functions(void);

			fastallocator<sizeof(exec_variable)*2+sizeof(int)*3> f2;
			fastallocator<sizeof(exec_variable)*4+sizeof(int)*3> f4;
			fastallocator<sizeof(exec_variable)*6+sizeof(int)*3> f6;
			fastallocator<sizeof(exec_variable)*8+sizeof(int)*3> f8;
			fastallocator<sizeof(exec_variable)*12+sizeof(int)*3> f12;
			fastallocator<sizeof(exec_variable)*16+sizeof(int)*3> f16;
			fastallocator<sizeof(exec_variable)*24+sizeof(int)*3> f24;
			fastallocator<sizeof(exec_variable)*32+sizeof(int)*3> f32;
			fastallocator<sizeof(exec_variable)*48+sizeof(int)*3> f48;
			fastallocator<sizeof(exec_variable)*64+sizeof(int)*3> f64;
			owca_vm *owner_vm;
			//tree_varspace *globalvarspace;
			internal_class *internalclases;
			gc_iteration gciteration;
			vislocker *vislockers;
			exec_base *allocated_objects,*allocated_object_waiting_for_delete,*allocated_object_waiting_for_delete_in_progress;
			std::set<std::string> readonly_idents;
			std::map<std::string,owca_internal_string*> strings_used_by_opcode_datas;
			std::vector<opcode_data*> opcode_datas;
			owca_global *vars;
			//// check for failed allocation!
			void purge_memory();
			DLLEXPORT void _allocated(exec_base *b);
			DLLEXPORT void *_allocate_memory(unsigned int size);
#ifdef RCDEBUG
			DLLEXPORT void *allocate_memory(unsigned int size, const std::type_info &ti);
#else
			DLLEXPORT void *allocate_memory(unsigned int size, const std::type_info &ti) { return _allocate_memory(size); }
#endif
			void free_memory(void *);
			template <class A> A *allocate_template(unsigned int oversize)
			{
				A *p=new (allocate_memory(sizeof(A)+oversize,typeid(A))) A(*this,oversize);
				exec_base *b=dynamic_cast<exec_base*>(p);
				if (b) _allocated(b);
				return p;
			}
			//exec_generator_function *allocate_generator_function(exec_stack *st, exec_function_ptr *fd, exec_function_stack_data *fncstackdata);
			exec_property *allocate_property();
			//exec_function_data *allocate_function_data(unsigned char *&bodyptr, unsigned int paramcount, unsigned int observedcount, unsigned int bodysize);
			//exec_class_data *allocate_class_data(unsigned char *&bodyptr, unsigned int inheritedcount, unsigned int variablescount, unsigned int observedcount, unsigned int bodysize);
			exec_function_ptr *allocate_function_ptr(unsigned int oversize);
			exec_function_bound *allocate_function_bound(exec_function_ptr *fnc_, const exec_variable &slf);
			exec_function_stack_data *allocate_function_stackdata(unsigned int size);
			exec_stack *allocate_stack(exec_namespace *, unsigned int *);
			exec_stack *allocate_stack(exec_function_ptr *fd);
			exec_stack *allocate_stack(exec_stack *prevst, unsigned int local_var_count, exec_function_ptr *owner);
			vm_execution_stack_elem_internal *allocate_stack_elem_internal(unsigned int temp_variables_count);

			exec_stack_variables *allocate_stack_variables(unsigned int size, exec_function_ptr *owner);
			owca_internal_string *allocate_string() { return allocate_string(NULL,0,0); } // empty string
			owca_internal_string *allocate_string(const char *data, unsigned int size, unsigned int char_count);
			DLLEXPORT owca_internal_string *allocate_string(std::string s);
			owca_internal_string *allocate_string(owca_internal_string_nongc *s);
			//owca_internal_string *allocate_string(const owca_string &s);
			owca_internal_string *identificator(operatorcodes oper);
			exec_namespace *allocate_namespace(owca_internal_string *file_name);
			exec_object *allocate_type(exec_class_object *&);
			DLLEXPORT exec_object *allocate_tuple(exec_tuple_object *&, unsigned int varcount);
			DLLEXPORT exec_object *allocate_array(exec_array_object *&);
			exec_object *allocate_map(exec_map_object *&);
			exec_object *allocate_set(exec_set_object *&);
			exec_object *allocate_stack_element(exec_stack_element_object *&);
			exec_object *allocate_object(exec_object *type, unsigned int oversize);
			exec_weakref_object *allocate_weakref_object(exec_object *par);
			//exec_object *allocate_typeobject();
			DLLEXPORT void *_data_from_object(exec_object *o, const structid_type &type) const;
			template <class A> A *data_from_object(exec_object *o) const { return (A*)_data_from_object(o,structinfo::structid<A*>()); }
			void _registerstructid(structid_type structid, exec_object *type);
			exec_object *find_type(structid_type type) const;
			void remove_type(structid_type type);
	private:
			std::map<structid_type,exec_object*> structid_to_type_object;
			void init_class(exec_object *&dst, const std::string &name, void (virtual_machine::*init)(internal_class &));
			void release_class(exec_object *&ds);
			void initialize_string(internal_class &c);
			void initialize_array(internal_class &c);
			void initialize_tuple(internal_class &c);
			void initialize_int(internal_class &c);
			void initialize_real(internal_class &c);
			void initialize_class(internal_class &c);
			void initialize_bool(internal_class &c);
			void initialize_function(internal_class &c);
			void initialize_null(internal_class &c);
			void initialize_generator(internal_class &c);
			void initialize_property(internal_class &c);
			void initialize_map(internal_class &c);
			void initialize_set(internal_class &c);
			void initialize_coroutine(internal_class &c);
			void initialize_stack_element(internal_class &c);
			void initialize_exception(internal_class &c);
			void initialize_exception_std(internal_class &c);
			void initialize_namespace(internal_class &c);
	public:
			virtual_machine(owca_vm *owner);
			~virtual_machine();
			exec_object *class_string,*class_array,*class_tuple,*class_int,*class_real,*class_class,*class_bool,*class_map,*class_set,
				*class_null,*class_function,*class_generator,*class_property,*class_namespace,*class_stack_element;
			exec_object *class_exception,*class_exception_param,*class_exception_math,*class_exception_operation,*class_exception_access,*class_coroutine;
			exec_object *basicmap[VAR_COUNT];
			bool can_purge_memory;

			void (*printfnc)(const std::string &);
			void set_print_function(void (*fnc)(const std::string &)) { printfnc=fnc; }
			owca_int id(const exec_variable &v) const;
			DLLEXPORT std::vector<unsigned char> compile(owca_message_list &errorswarnings, const owca_source_file &fs, const compile_visible_items &visible_names);
			static std::string to_stdstring_type(const exec_variable &r);
			static std::string to_stdstring_type_short(const exec_variable &r);
			std::string to_stdstring_type(exec_object *type);

			vm_execution_stack *execution_stack;
			bool execution_self_oper;
			//exec_variable tmpretval;
			DebugInterface* debug_interface = nullptr;

			void register_opcode_data(opcode_data *opc) { opcode_datas.push_back(opc); }
			executionreturnvalue execute_stack(void);

			void push_execution_stack(vm_execution_stack *st=NULL);
			void pop_execution_stack();
			bool has_execution_stack(void) const { return execution_stack != NULL; }
			vm_execution_stack_elem_internal *push_execution_stack_frame_internal(unsigned int stack_data_size, unsigned int temporary_variables);
			DLLEXPORT bool push_execution_stack_frame(vm_execution_stack_elem_base *sf);
			void push_frame_returning_bool_value(bool value, exec_variable *return_value, exec_function_ptr *fnc);

			template <class A> bool convert(A &ptr, const exec_variable &v);
			template <class A> bool convert(A *&ptr, const exec_variable &v) {
				return v.mode()==VAR_OBJECT && (ptr=data_from_object<A>(v.get_object()))!=NULL;
			}
		private:
			DLLEXPORT bool _convert(owca_string &s, const exec_variable &v);
			DLLEXPORT bool _convert(owca_internal_string *&s, const exec_variable &v);
			DLLEXPORT bool _convert(owca_int &s, const exec_variable &v);
			DLLEXPORT bool _convert(bool &s, const exec_variable &v);
			DLLEXPORT bool _convert(owca_real &s, const exec_variable &v);
			DLLEXPORT bool _convert(null &s, const exec_variable &v);
			DLLEXPORT bool _convert(unifunction &s, const exec_variable &v);
			DLLEXPORT bool _convert(vm_execution_stack_elem_base *&s, const exec_variable &v);
			DLLEXPORT bool _convert(exec_class_object *&s, const exec_variable &v);
			DLLEXPORT bool _convert(exec_property *&s, const exec_variable &v);
			DLLEXPORT bool _convert(owca_local &s, const exec_variable &v);
			DLLEXPORT bool _convert(owca_global &s, const exec_variable &v);
			DLLEXPORT bool _convert(exec_weakref_object *&s, const exec_variable &v);
			DLLEXPORT bool _convert(exec_object *&s, const exec_variable &v);
		public:
			bool convert(owca_string &s, const exec_variable &v) { return _convert(s,v); }
			bool convert(owca_internal_string *&s, const exec_variable &v) { return _convert(s,v); }
			bool convert(owca_int &s, const exec_variable &v) { return _convert(s,v); }
			bool convert(bool &s, const exec_variable &v) { return _convert(s,v); }
			bool convert(owca_real &s, const exec_variable &v) { return _convert(s,v); }
			bool convert(null &s, const exec_variable &v) { return _convert(s,v); }
			bool convert(unifunction &s, const exec_variable &v) { return _convert(s,v); }
			bool convert(vm_execution_stack_elem_base *&s, const exec_variable &v) { return _convert(s,v); }
			bool convert(exec_class_object *&s, const exec_variable &v) { return _convert(s,v); }
			bool convert(exec_property *&s, const exec_variable &v) { return _convert(s,v); }
			bool convert(owca_local &s, const exec_variable &v) { return _convert(s,v); }
			bool convert(owca_global &s, const exec_variable &v) { return _convert(s,v); }
			bool convert(exec_weakref_object *&s, const exec_variable &v) { return _convert(s,v); }
			bool convert(exec_object *&s, const exec_variable &v) { return _convert(s,v); }
			bool convert(const exec_variable *&s, const exec_variable &v) { s=&v; return true; }

			// those will either initialize new frame with function call or with exception $init function
			bool operator_get_function_right_over_left(operatorcodes oper, const exec_variable *operands);
			void operator_get_next_function(vm_execution_stack_elem_base *);
			const exec_variable *operator_get_function_2(exec_variable &retfnc, unsigned char &oper_mode, unsigned char oper_index, const exec_variable *operands);
			void prepare_call_opcodes_entry(exec_variable *retval, exec_namespace *ns, opcode_data *opcodes);
			void prepare_call_operator(exec_variable *retval, operatorcodes oper, const exec_variable *operands);
			//void prepare_call_operator(exec_variable *retval, operatorcodes oper, callparams &cp, bool replace_current_frame);

			struct keyword_param_iterator {
				virtual bool next(owca_internal_string *&, const exec_variable *&) { return false; }
				virtual unsigned int count() { return 0; }
			};

			bool prepare_call_function(exec_variable *retval, const exec_variable &fnc, const exec_variable *listvar, const exec_variable *mapvar, keyword_param_iterator &kpi, const exec_variable *params, unsigned int paramcnt, const exec_variable *self_override);
			bool prepare_call_function(exec_variable *retval, const exec_variable &fnc, const exec_variable *params, unsigned int paramcnt);
			bool prepare_call_function(exec_variable *retval, const exec_variable &fnc, const callparams &cp, const exec_variable *self_override);

			void set_self(exec_variable &dst, exec_function_ptr *fnc, const exec_variable &slf);
			void set_self(exec_variable &dst, exec_function_ptr *fnc, exec_object *slf);

			exec_object *execution_exception_object_thrown; // an exception is thrown and is (yet) uncaught
			exec_variable execution_exception_object_temp;
			exec_variable execution_exception_parameters[3];

			DLLEXPORT bool exception_thrown() const;
			DLLEXPORT exec_object *_raise_get_exception_type(exceptioncode code);

			DLLEXPORT bool ensure_no_map_params(exec_map_object *mp);
			DLLEXPORT void _raise_from_user(const exec_variable &);
			//void _raise_from_user(exceptioncode code, const std::string &txt);
			//void _raise_from_user(exceptioncode code);
			DLLEXPORT void _raise(exceptioncode code, exec_object *exctype, const std::string &txt);
			DLLEXPORT void _prepare_construct_exception(exceptioncode code, exec_object *exctype, const std::string &txt);
			// params

			void raise_class_creation(const std::string &);
			void raise_param_assigned_twice(owca_internal_string *id, int index=-1);
			void raise_param_not_set(owca_internal_string *id);
			void raise_unused_keyword_param(owca_internal_string *id);
			void raise_keowca_map_param_not_string();
			void raise_missing_key_parameter();
			void raise_missing_value_parameter();
			void raise_no_constructor();
			void raise_too_many_parameters(exec_function_ptr *);
			void raise_too_many_parameters(const std::string &txt);
			void raise_not_enough_parameters(exec_function_ptr *);
			void raise_invalid_list_param(const exec_variable &);
			void raise_invalid_map_param(const exec_variable &);
			void raise_invalid_param(const std::string &txt);
			void raise_invalid_param(const std::string &txt, exec_object *req);
			void raise_invalid_param(const exec_variable &o, exec_object *req);
			void raise_invalid_integer(const std::string &txt);
			void raise_unsupported_keyword_parameters();
			void raise_unsupported_operation();
			void raise_unsupported_operation(operatorcodes oper, const exec_variable &fnc);
			void raise_unsupported_operation(operatorcodes oper, const exec_variable *params);
			void raise_unsupported_call_operation(const exec_variable &fnc);
			void raise_invalid_key();
			// math
			void raise_division_by_zero();
			void raise_overflow();
			// access
			void raise_invalid_ident(const std::string &ident);
			void raise_not_lvalue_member(const exec_variable &o, const std::string &txt);
			void raise_not_rvalue_member(const exec_variable &o, const std::string &txt);
			void raise_missing_member(const exec_variable &o, const std::string &txt);
			void raise_missing_member(exec_object *o, const std::string &txt);
			// operation
			void raise_list_modified_while_being_sorted();
			void raise_map_modified_while_being_used();
			void raise_set_modified_while_being_used();
			void raise_too_much_iter_obj();
			void raise_too_little_iter_obj();
			void raise_invalid_return_type(const exec_variable &o, exec_object *req);
			void raise_missing_return_value();
			void raise_invalid_operator_function(exec_function_ptr *fnc);
			void raise_stack_overflow();
			void raise_cant_insert();
			void raise_key_not_found();
			void raise_no_coroutine_to_stop();
			DLLEXPORT void raise_cant_stop_from_within_user_function();
			void raise_cant_resume_from_coroutine();
			void raise_cant_resume_normal_function();
			void raise_cant_resume_fnished_coroutine();
			DLLEXPORT void raise_cant_stop_coroutine_from_user_function();
			DLLEXPORT void raise_cant_create_generator_from_user_function();

			bool calculate_iter_next(exec_variable *retval, const exec_variable &param);
			bool calculate_eq(exec_variable *retval, const exec_variable *params);
			bool calculate_noteq(exec_variable *retval, const exec_variable *params);
			bool calculate_lesseq(exec_variable *retval, const exec_variable *params);
			bool calculate_moreeq(exec_variable *retval, const exec_variable *params);
			bool calculate_less(exec_variable *retval, const exec_variable *params);
			bool calculate_more(exec_variable *retval, const exec_variable *params);
			bool calculate_str(exec_variable *retval, const exec_variable &param);
			bool calculate_hash(exec_variable *retval, const exec_variable &param);
			bool calculate_bool(exec_variable *retval, const exec_variable &param);
			bool calculate_generator(exec_variable *retval, const exec_variable &param);

			static bool calculate_nocall(virtual_machine *self, exec_variable &retval, operatorcodes cmpoper, const exec_variable &left, const exec_variable &right);
			static bool calculate_add_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_sub_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_mul_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_div_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_mod_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_lshift_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_rshift_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_binxor_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_binor_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_binand_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);

			static bool calculate_eq_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_noteq_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_less_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_more_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_lesseq_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			static bool calculate_moreeq_nocall(virtual_machine *self, exec_variable &retval, const exec_variable &left, const exec_variable &right);
			//static bool calculate_cmp_nocall(exec_variable &retval, operatorcodes cmpoper, const exec_variable &left, const exec_variable &right);
			static bool calculate_str_nocall(virtual_machine *self, owca_internal_string *&retval, const exec_variable &param);
			static bool calculate_hash_nocall(virtual_machine *self, owca_int &retval, const exec_variable &param);
			static bool calculate_bool_nocall(bool &retval, const exec_variable &param);
			static bool calculate_generator_nocall(exec_variable *retval, const exec_variable &param);

			void run_gc();
			void purge_objects();
		};
		struct vislocker_pair {
			exec_variable key;
			exec_variable val;
			owca_int hash;
			vislocker_pair(const exec_variable &key_, const exec_variable &val_, owca_int hash_) : key(key_),val(val_),hash(hash_) { }
		};
		class vislocker {
			friend class virtual_machine;
			union {
				struct { const exec_variable *v; unsigned int cnt; } variable_array;
				struct { const exec_variable *v; } variable;
				struct { const exec_base *v; } execbase;
				struct { exec_object **v; } execobjectptr;
				struct { const std::list<exec_object*> *v; } execobjectlist;
				struct { const std::list<exec_variable> *v; } variablelist;
				struct { const std::vector<vislocker_pair> *v; } variablepairarray;
			} data;
			vislocker *prev;
			enum {
				VARIABLE_ARRAY,
				VARIABLE,
				EXEC_BASE,
				EXEC_OBJECT_PTR,
				EXEC_OBJECT_LIST,
				EXEC_VARIABLE_LIST,
				EXEC_VARIABLE_PAIR_ARRAY,
			} mode;
			virtual_machine &vm;
			void init() {
				prev=vm.vislockers;
				vm.vislockers=this;
#ifdef RCDEBUG
				_id_this=_id_cnt++;
#endif
			}
		public:
#ifdef RCDEBUG
			unsigned int _id_this;
			static unsigned int _id_cnt;
#endif
			vislocker(virtual_machine &vm_, const exec_variable *v, size_t cnt) : vm(vm_),mode(VARIABLE_ARRAY) { init(); data.variable_array.v=v; data.variable_array.cnt=(unsigned int)cnt; }
			vislocker(virtual_machine &vm_, const exec_variable &v) : vm(vm_),mode(VARIABLE) { init(); data.variable.v=&v; }
			vislocker(virtual_machine &vm_, const exec_base *v) : vm(vm_),mode(EXEC_BASE) { init(); data.execbase.v=v; }
			vislocker(virtual_machine &vm_, exec_object **v) : vm(vm_),mode(EXEC_OBJECT_PTR) { init(); data.execobjectptr.v=v; }
			vislocker(virtual_machine &vm_, const std::list<exec_object*> &v) : vm(vm_),mode(EXEC_OBJECT_LIST) { init(); data.execobjectlist.v=&v; }
			vislocker(virtual_machine &vm_, const std::list<exec_variable> &v) : vm(vm_),mode(EXEC_VARIABLE_LIST) { init(); data.variablelist.v=&v; }
			vislocker(virtual_machine &vm_, const std::vector<vislocker_pair> &v) : vm(vm_),mode(EXEC_VARIABLE_PAIR_ARRAY) { init(); data.variablepairarray.v=&v; }
			~vislocker() {
				RCASSERT(vm.vislockers==this);
				vm.vislockers=vm.vislockers->prev;
			}
			void gc_mark(gc_iteration &);
		};

		class vmstack {
			virtual_machine &vm;
		public:
			vmstack(virtual_machine &vm_);
			~vmstack();
		};
	}
}
#endif
