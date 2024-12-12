#ifndef _RC_Y_EXEC_MAP_OBJECT_H
#define _RC_Y_EXEC_MAP_OBJECT_H

#include "structinfo.h"
#include "exec_variable.h"
#include "hash_map.h"
#include "exec_map_object_iterator.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		struct defval;
		class exec_map_object;
		//class exec_map_object_iterator;
		class exec_variable;
		class virtual_machine;
	}
}

namespace owca { namespace __owca__ {
	class exec_map_object : public object_base {
	public:
		struct key {
			exec_variable k;
			owca_int hash;
			key(const exec_variable &k_, owca_int hash_) : k(k_),hash(hash_) { }
			key(owca_internal_string *k_);
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
		typedef hash_map<key,exec_variable> hashmap;
		hashmap map;

		exec_map_object(virtual_machine &vm, unsigned int oversize) { }

		void _marker(const gc_iteration &gc) const;
		void _destroy(virtual_machine &vm) { map.clear(vm); }

		//bool used(const exec_map_object_iterator &index) const { return map.used(index); }
		//bool empty(const exec_map_object_iterator &index) const { return map.empty(index); }
		//bool deleted(const exec_map_object_iterator &index) const { return map.deleted(index); }

		//exec_map_object_iterator start_position(owca_int hash) const { return map.start_position(hash); }
		//exec_map_object_iterator write_position(owca_int hash) const { return map.write_position(hash); }
		//void next_position(exec_map_object_iterator &iter, unsigned int try_) const { map.next_position(iter,try_); }

		//void init_size(virtual_machine &vm) { map.init_size(vm); }
		//void update_size(virtual_machine &vm) { map.update_size(vm); }

		//unsigned int table_size() const { return tablesize; }
		//unsigned int size() const { return elems; }
		//void resize(virtual_machine &vm, unsigned int newsize);
		//bool next(exec_map_object_iterator &index) const;

		//const exec_variable &getkey(const exec_map_object_iterator &index) const { RCASSERT(used(index)); return table[index.index].key; }
		//const exec_variable &getval(const exec_map_object_iterator &index) const { RCASSERT(used(index)); return table[index.index].val; }
		//exec_variable &getkey(const exec_map_object_iterator &index) { RCASSERT(used(index)); return table[index.index].key; }
		//exec_variable &getval(const exec_map_object_iterator &index) { RCASSERT(used(index)); return table[index.index].val; }
		//owca_int gethash(const exec_map_object_iterator &index) const { RCASSERT(used(index)); return table[index.index].hash; }
		//exec_map_object_iterator getiterator(unsigned int index) const { exec_map_object_iterator m; m.index=index; return m; }
		//void set(const exec_map_object_iterator &index, const exec_variable &key, const exec_variable &val, owca_int hash);
		//void setdeleted(virtual_machine &vm, const exec_map_object_iterator &index);

		void ident_insert(virtual_machine &vm, owca_internal_string *ident, const exec_variable &val);
		exec_variable *ident_get(owca_internal_string *ident);
		//void clear(virtual_machine &vm) { map.clear
		//void copy(virtual_machine &vm, const exec_map_object *src);
	};
} }
#endif
