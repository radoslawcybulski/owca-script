#ifndef _RC_Y_EXEC_SET_OBJECT_H
#define _RC_Y_EXEC_SET_OBJECT_H

#include "structinfo.h"
#include "exec_variable.h"
#include "hash_map.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		struct defval;
		class exec_set_object;
		//class exec_map_object_iterator;
		class exec_variable;
		class virtual_machine;
	}
}

namespace owca { namespace __owca__ {
	typedef hash_map_iterator exec_set_object_iterator;

	class exec_set_object : public object_base {
	public:
		struct key {
			exec_variable k;
			owca_int hash;
			key(const exec_variable &k_, owca_int hash_) : k(k_),hash(hash_) { }
			void gc_acquire() { k.gc_acquire(); }
			void gc_mark(const gc_iteration &gc) const { k.gc_mark(gc); }
			void gc_release(virtual_machine &vm) { k.gc_release(vm); }
			owca_int hashmap_hash() const { return hash; }
			bool hashmap_used() const { return k.mode()<VAR_COUNT; }
			bool hashmap_empty() const { return k.mode()==VAR_HASH_EMPTY; }
			bool hashmap_deleted() const { return k.mode()==VAR_HASH_DELETED; }
			void hashmap_setdeleted() { k.setmode(VAR_HASH_DELETED); }
			void hashmap_setempty() { k.setmode(VAR_HASH_EMPTY); }
		};
		struct value {
			void gc_acquire() { }
			void gc_mark(const gc_iteration &gc) const { }
			void gc_release(virtual_machine &vm) { }
		};
		typedef hash_map<key,value> hashmap;
		hashmap set;

		exec_set_object(virtual_machine &vm, unsigned int oversize) { }

		void _marker(const gc_iteration &gc) const {
			gc_iteration::debug_info _d("exec_set_object %x: set hashmap %d",this);
			set.gc_mark(gc);
		}
		void _destroy(virtual_machine &vm) { set.clear(vm); }
	};
} }
#endif
