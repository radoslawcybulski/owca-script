#ifndef _RC_Y_EXEC_OBJECT_H
#define _RC_Y_EXEC_OBJECT_H

#include "exec_base.h"
#include "exec_string_compare.h"
#include "hash_map.h"
#include "exec_string.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		struct defval;
		class exec_base;
		class exec_class_object;
		class exec_object;
		class exec_variable;
		class internal_class;
		class virtual_machine;
		class owca_internal_string;
	}
}

namespace owca {
	namespace __owca__ {
		class exec_weakref_object : public exec_base {
			friend class virtual_machine;
			friend class internal_class;

			exec_weakref_object(exec_object *obj) : ptr(obj) { }
		public:
			exec_object *ptr;
		protected:
			void _mark_gc(const gc_iteration &gc) const { }
			void _release_resources(virtual_machine &vm);
		public:
			void gc_acquire() { _gc_acquire(); }
			void gc_release(virtual_machine &vm) { _gc_release(vm); }
			void gc_mark(const gc_iteration &gi) const { _gc_mark(gi); }
		};

		class exec_object : public exec_base {
			friend class virtual_machine;
			friend class internal_class;

	#ifdef RCDEBUG
			exec_object() : _co_(NULL),type_(NULL),weakref(NULL) { }
			exec_object(exec_object *type__) : type_(type__),_co_(NULL),weakref(NULL) { type_->gc_acquire(); }
	#else
			exec_object() : type_(NULL),weakref(NULL) { }
			exec_object(exec_object *type__) : type_(type__),weakref(NULL) { type_->gc_acquire(); }
	#endif
			~exec_object() { }
			exec_object *type_;
			void *_getptr(unsigned int off) { return ((char*)this+sizeof(exec_object))+off; }
			const void *_getptr(unsigned int off) const { return ((char*)this+sizeof(exec_object))+off; }
		public:
			exec_weakref_object *weakref;
	#ifdef RCDEBUG
			exec_class_object *_co_;
	#endif

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

			typedef hash_map<key,exec_variable> hashmap;
			hashmap members;
			void set_member(virtual_machine &vm, owca_internal_string *id, const exec_variable &val);
			bool pop_member(virtual_machine &vm, owca_internal_string *id, exec_variable &retval);
			void pop_member(virtual_machine &vm, owca_internal_string *id, exec_variable &retval, const exec_variable &defval);
			exec_class_object &CO() {
	#ifdef RCDEBUG
				RCASSERT(type_->type()==type_->type()->type());
	#endif
				//return *(exec_class_object*)dataptr();
				return *(exec_class_object*)(((char*)this)+sizeof(exec_object));
			}
			bool inherit_from(exec_object *t);
			bool is_type() const { return type()->type()==type(); }
			exec_object *type() const { return type_; }
		protected:
			void _mark_gc(const gc_iteration &gc) const;
			void _release_resources(virtual_machine &vm);
		public:
			void gc_acquire() { _gc_acquire(); }
			void gc_release(virtual_machine &vm) { _gc_release(vm); }
			void gc_mark(const gc_iteration &gi) const { _gc_mark(gi); }
		};
	}
}
#endif
