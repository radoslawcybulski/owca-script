#ifndef _RC_Y_EXEC_NAMESPACE_H
#define _RC_Y_EXEC_NAMESPACE_H

#include "exec_base.h"
#include "exec_string_compare.h"
#include "hash_map.h"
#include "exec_string.h"
#include "executionreturnvalue.h"

namespace owca {
	class owca_vm;
	class owca_namespace;
	class returnvalue;
	namespace __owca__ {
		class virtual_machine;
		class exec_stack_variables;
		class exec_stack;
		class virtual_machine;
		class opcode_data;
	}
}

namespace owca {
	namespace __owca__ {
		class exec_namespace : public exec_base {
			friend class owca::owca_vm;
			friend class exec_stack;
			friend class virtual_machine;
		public:
			struct key {
				owca_internal_string *k;

				key(owca_internal_string *kk) : k(kk) { }
				key() : k(NULL) { }

				void gc_acquire() { k->gc_acquire(); }
				void gc_mark(const gc_iteration &gc) const { k->gc_mark(gc); }
				void gc_release(virtual_machine &vm) { k->gc_release(vm); }
				owca_int hashmap_hash() const { return k->hash(); }
				bool hashmap_used() const { return k>(owca_internal_string*)1; }
				bool hashmap_empty() const { return k==(owca_internal_string*)0; }
				bool hashmap_deleted() const { return k==(owca_internal_string*)1; }
				void hashmap_setdeleted() { k=(owca_internal_string*)1; }
				void hashmap_setempty() { k=(owca_internal_string*)0; }
			};
			struct value {
				unsigned int v;
				value() { }
				value(unsigned int q) : v(q) { }
				void gc_acquire() { }
				void gc_mark(const gc_iteration &gc) const { }
				void gc_release(virtual_machine &vm) { }
			};
		private:
			typedef hash_map<key,value> hashmap;
			hashmap hashindex;
			//std::map<owca_internal_string*,unsigned int,string_compare> mapindex;
			exec_stack_variables *variables;
            owca_internal_string *file_name;

			DLLEXPORT unsigned int _insert_variable(owca_internal_string *);
		protected:
			DLLEXPORT void _release_resources(virtual_machine &vm);
			DLLEXPORT void _mark_gc(const gc_iteration &gc) const;
		public:
			virtual_machine &vm;

            DLLEXPORT exec_namespace(virtual_machine &vm_, owca_internal_string *file_name);
			DLLEXPORT ~exec_namespace();

			void init();
			int insert_variable(owca_internal_string *ident, const exec_variable &v); // <0 - ident is readonly and value already exists
			bool get_variable(owca_internal_string *ident, exec_variable &v); // false - not found
			void clear();
			void apply_code(opcode_data *opc);
			bool validate_code_and_prepare_for_execution(const std::vector<unsigned char> &data, opcode_data *execution_data = NULL);

            exec_namespace *copy(owca_internal_string *file_name);
			DLLEXPORT owca_namespace generate();
            owca_internal_string *get_file_name() { return file_name; }

			void gc_acquire() { _gc_acquire(); }
			void gc_release(virtual_machine &vm) { _gc_release(vm); }
		};
	}
}

#endif
