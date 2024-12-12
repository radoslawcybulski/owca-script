#ifndef _RC_Y_OP_WRITE_H
#define _RC_Y_OP_WRITE_H

#include "debug_opcodes.h"
#include "vm_execution_stack_elem_internal.h"
#include "operatorcodes.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		enum execopcode;
		class opcode_executer;
		class opcode_writer;
		class opcode_writer_jump;
		class exec_function_stack_data;
		class exec_variable;
		class exec_variable_location;
		template <class A> class op_flow_data_pointer;
		class virtual_machine;
		class owca_internal_string;
		class owca_internal_string_nongc;
		class tree_function;
	}
}

namespace owca {
	namespace __owca__ {
		enum {
			VERSION_MAJOR = 1,
			VERSION_MINOR = 1,
		};
		enum {
			VERSION_MAGIC = 0xee000000 | (VERSION_MAJOR << 16) | 0x0000aa00 | VERSION_MINOR
		};
		class opcode_writer_jump {
			friend class opcode_writer;
			unsigned int *ptr;
			unsigned int off;
		public:
			opcode_writer_jump() : ptr(NULL),off(0) { }
		};
		class opcode_writer_location {
			opcode_writer &ow;
			unsigned int loc;
		public:
			opcode_writer_location(opcode_writer &ow_, unsigned int loc_);
			~opcode_writer_location();
		};
		class store_uint_or_ptr {
		public:
			typedef union {
				unsigned int val;
				void *ptr;
			} data_type;
		private:
			data_type *val;
		public:
			store_uint_or_ptr() : val(NULL) { }
			void init_ptr(void *v) { val = (data_type*)v; }
			void set_uint(unsigned int q) { val->val = q; }
			void set_pointer(void *q) { val->ptr = q; }
			unsigned int get_uint(void) const { return val->val; }
			void *get_pointer(void) const { return val->ptr; }
		};
		class store_real {
		public:
			typedef union {
				owca_real val;
			} data_type;
		private:
			data_type *val;
		public:
			store_real() : val(NULL) { }
			void init_ptr(void *v) { val = (data_type*)v; }
			void set_real(owca_real r) { val->val = r; }
			owca_real get_real(void) const { return val->val; }
		};
		class store_int {
		public:
			typedef union {
				owca_int val;
			} data_type;
		private:
			data_type *val;
		public:
			store_int() : val(NULL) { }
			void init_ptr(void *v) { val = (data_type*)v; }
			void set_integer(owca_int i) { val->val = i; }
			owca_int get_integer(void) const { return val->val; }
		};
		class store_variable_location {
		public:
			typedef union {
				unsigned short val[2];
			} data_type;
		private:
			data_type *val;
		public:
			store_variable_location() : val(NULL) { }
			void init_ptr(void *v) { val = (data_type*)v; }
			void set_location(unsigned short depth, unsigned short offset) { val->val[0] = depth; val->val[1] = offset; }
			unsigned short get_depth(void) const { return val->val[0]; }
			unsigned short get_offset(void) const { return val->val[1]; }
		};
		class opcode_writer {
			friend class opcode_writer_location;
			//struct locationinfo {
			//	unsigned int loc;
			//	unsigned int offset_begin,offset_end;
			//	locationinfo() : offset_begin(0),offset_end(0) { }
			//};
			struct locationinfotemp {
				unsigned int loc;
				unsigned int begin;
			};
			struct memory_elem {
				enum { SIZE = 1024 * 64 };
				unsigned int pos;
				memory_elem *next;
				memory_elem() : pos(0), next(NULL) { }
				unsigned char *get() { return (unsigned char*)this + sizeof(*this); }

				static memory_elem *alloc(unsigned int size = SIZE) {
					unsigned char *q = new unsigned char[size + sizeof(memory_elem)];
					return new (q) memory_elem;
				}
			};
			memory_elem *first, *last;

			//std::list<unsigned char*> data;
			//std::list<std::pair<unsigned int, store_uint_or_ptr> > stringoffsets;
			std::map<std::string,unsigned int> stringmap;
			std::list<vm_execution_stack_elem_internal::locationinfo> locinfos;
			std::list<locationinfotemp> locinfotemp;
			std::vector<std::pair<exec_variable_location*, store_variable_location> > variablelocations;
			unsigned int totalsize;
			void *_add(unsigned int size);
			void *_add(const void *buf, unsigned int size);
			void _add_string(const std::string &);
			unsigned int _register_string(const std::string &);
#ifdef RCDEBUG_OPCODES
			void write_type(opcodestreamtype);
#else
			void write_type(opcodestreamtype) { }
#endif
			void finalize_location();
			void push_location(unsigned int l);
			void pop_location(unsigned int l);
		public:
			opcode_writer() : totalsize(0), first(NULL),last(NULL) {}
			~opcode_writer() {
				while (first) {
					memory_elem *e = first->next;
					delete [] (unsigned char*)first;
					first = e;
				}
				//for(std::list<unsigned char*>::iterator it=data.begin();it!=data.end();++it) {
				//	delete [] (*it);
				//}
			}
			std::vector<unsigned char> compile(tree_function *root);
			void finalize_jump(opcode_writer_jump &);
			opcode_writer &operator << (opcode_writer_jump &);
			opcode_writer &operator << (unsigned int val);
			opcode_writer &operator << (store_uint_or_ptr &val);
			opcode_writer &operator << (const std::string &);
			opcode_writer &operator << (owca_int);
			opcode_writer &operator << (owca_real);
			opcode_writer &operator << (exec_variable_location &);
			opcode_writer &operator << (operatorcodes opc);
			opcode_writer &operator << (execopcode opc);
		};

	}
}

#endif
