#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_map_object.h"
#include "exec_tuple_object.h"
#include "op_compare.h"
#include "returnvalue.h"
#include "exec_string.h"

//#define GROWPREC 0.8
//#define SHRINKPREC (0.4*GROWPREC)

namespace owca { namespace __owca__ {

	exec_map_object::key::key(owca_internal_string *k_) : hash(k_->hash()) { k.set_string(k_); }

	void exec_map_object::_marker(const gc_iteration &gc) const
	{
		gc_iteration::debug_info _d("exec_map_object %x: map hashmap %d",this);
		map.gc_mark(gc);
	}

	//void exec_map_object::_create(virtual_machine &vm, unsigned int oversize)
	//{
	//}

	//void exec_map_object::_marker(const gc_iteration &gc) const
	//{
	//	exec_map_object_iterator it;

	//	while(next(it)) {
	//		getkey(it).gc_mark(gc);
	//		getval(it).gc_mark(gc);
	//	}
	//}

	//void exec_map_object::_destroy(virtual_machine &vm)
	//{
	//	clear(vm);
	//}

	//bool exec_map_object::used(const exec_map_object_iterator &index) const { RCASSERT(index.index<tablesize); return table[index.index].used(); }
	//bool exec_map_object::empty(const exec_map_object_iterator &index) const { RCASSERT(index.index<tablesize); return table[index.index].empty(); }
	//bool exec_map_object::deleted(const exec_map_object_iterator &index) const { RCASSERT(index.index<tablesize); return table[index.index].deleted(); }

	//exec_map_object_iterator exec_map_object::start_position(owca_int hash) const
	//{
	//	exec_map_object_iterator m;
	//	m.index=((unsigned int)hash&(tablesize-1));
	//	return m;
	//}

	//void exec_map_object::next_position(exec_map_object_iterator &m, unsigned int try_) const
	//{
	//	m.index=(m.index+try_)&(tablesize-1);
	//}

	exec_variable *exec_map_object::ident_get(owca_internal_string *ident)
	{
		hashmap::hash_map_finder mi(map,ident->hash());
		for(;mi.valid();mi.next()) {
			exec_map_object::key &k=map.getkey(mi.get());
			if (k.hashmap_hash()==ident->hash() && k.k.mode()==VAR_STRING && k.k.get_string()->equal(ident)) return &map.getval(mi.get());
		}
		return NULL;
	}

	void exec_map_object::ident_insert(virtual_machine &vm, owca_internal_string *ident, const exec_variable &val)
	{
		exec_variable *v=ident_get(ident);
		if (v) {
			v->gc_release(vm);
			*v=val;
		}
		else {
			map.elem_insert(vm,map.write_position(ident->hash()),exec_map_object::key(ident),val);
			ident->gc_acquire();
		}
		val.gc_acquire();

		//if (tablesize==0) _resize(vm,minsize);
		//exec_map_object_iterator mi=start_position(ident->hash());
		//for(unsigned int i=1;i<tablesize;++i) {
		//	if (empty(mi)) {
		//		exec_variable v;
		//		v.set_string(ident);
		//		set(mi,v,val,ident->hash());
		//		ident->gc_acquire();
		//		val.gc_acquire();
		//		++elems;
		//		update_size(vm);
		//		return;
		//	}
		//	else if (used(mi) && gethash(mi)==ident->hash() && getkey(mi).mode()==VAR_STRING && getkey(mi).get_string()->equal(ident)) {
		//		getval(mi).gc_release(vm);
		//		getval(mi)=val;
		//		val.gc_acquire();
		//		return;
		//	}
		//	RCASSERT(deleted(mi) || getkey(mi).mode()==VAR_STRING);
		//	next_position(mi,i);
		//}
		//RCASSERT(0);
	}

//	bool exec_map_object::next(exec_map_object_iterator &index) const
//	{
//		while((++index.index)<tablesize) {
//			if (used(index)) return true;
//		}
//		return false;
//	}
//
//	RCLMFUNCTION void exec_map_object::resize(virtual_machine &vm, unsigned int newsize)
//	{
//		if (newsize==0) {
//			RCASSERT(elems==0);
//			_resize(vm,0);
//		}
//		else {
//			RCASSERT(newsize>=elems);
//			RCASSERT(newsize>=minsize);
//			unsigned int v=newsize;
//			//unsigned int c=0;
//			//while(v>1) {
//			//	++c;
//			//	v>>=1;
//			//}
//			//RCASSERT(v==1);
//			//while(c>0) {
//			//	v<<=1;
//			//	--c;
//			//}
//			//if (v<newsize) v<<=1;
//			RCASSERT(v>=newsize);
//			RCASSERT(v>=elems);
//			RCASSERT(v>=minsize);
//			_resize(vm,v);
//		}
//	}
//
//	RCLMFUNCTION void exec_map_object::_resize(virtual_machine &vm, unsigned int newsize)
//	{
//		unsigned int old_elems=elems;
//		value *oldtable=table;
//		unsigned int oldsize=tablesize;
//
//		RCASSERT(newsize==0 || newsize>=minsize);
//#ifdef RCDEBUG
//		if (newsize) {
//			owca_int v=newsize;
//
//			while (v) {
//				RCASSERT(v==1 || (v&1)==0);
//				v=v>>1;
//			}
//		}
//#endif
//		elems=0;
//		tablesize=newsize;
//		if (tablesize>0) {
//			table=(value*)vm.allocate_memory((unsigned int)(tablesize*sizeof(value)),typeid(value));
//			for(owca_int i=0;i<tablesize;++i) table[i].key.setmode(VAR_HASH_EMPTY);
//			if (oldtable) {
//				insertelems(oldtable,oldsize);
//				RCASSERT(elems==old_elems);
//				vm.free_memory(oldtable);
//				//debug_check_memory();
//			}
//			else
//		}
//		else {
//			RCASSERT(old_elems==0);
//			if (oldtable) vm.free_memory(oldtable);
//			else
//			table=NULL;
//		}
//		RCASSERT(elems==old_elems);
//	}
//
//	RCLMFUNCTION void exec_map_object::update_size(virtual_machine &vm)
//	{
//		if (size()==0) {
//			resize(vm,0);
//		}
//		else if (tablesize>minsize && size()<SHRINKPREC*tablesize) {
//			unsigned int ts=tablesize;
//			while(ts>minsize && size()<SHRINKPREC*ts) ts>>=1;
//
//			_resize(vm,ts);
//		}
//		else if (size()>=tablesize*GROWPREC) _resize(vm,tablesize<<1);
//		else
//	}
//
//	RCLMFUNCTION void exec_map_object::insertelems(value *tbl, unsigned int tsize)
//	{
//		// this reinserts old table - we know all elements are distinct in respect to each other. so we will just find a free space for each.
//		// also table must be of correct size
//
//		for(unsigned int i=0;i<tsize;++i) {
//			if (tbl[i].used()) {
//				value &v=tbl[i];
//				exec_map_object_iterator mi=start_position(v.hash);
//
//				unsigned int i;
//				for(i=1;i<tablesize;++i) {
//					if (empty(mi)) {
//						table[mi.index]=v;
//						++elems;
//						break;
//					}
//					else
//					next_position(mi,i);
//				}
//				RCASSERT(i<tablesize);
//			}
//			else
//		}
//	}
//
//	RCLMFUNCTION void exec_map_object::clear(virtual_machine &vm)
//	{
//		if (table) {
//			exec_map_object_iterator iter;
//			while(next(iter)) {
//				getval(iter).gc_release(vm);
//				getkey(iter).gc_release(vm);
//			}
//			vm.free_memory(table);
//			table=NULL;
//		}
//		else
//		tablesize=0;
//		elems=0;
//	}
//
//	void exec_map_object::set(const owca::__owca__::exec_map_object_iterator &index, const owca::__owca__::exec_variable &key, const owca::__owca__::exec_variable &val, owca_int hash)
//	{
//		RCASSERT(index.index<tablesize);
//		value &v=table[index.index];
//		v.key=key;
//		v.val=val;
//		v.hash=hash;
//	}
//
//	void exec_map_object::setdeleted(virtual_machine &vm, const exec_map_object_iterator &index)
//	{
//		table[index.index].key.setmode(VAR_HASH_DELETED);
//		--elems;
//		update_size(vm);
//	}
//
//	RCLMFUNCTION void exec_map_object::copy(owca::__owca__::virtual_machine &vm, const owca::__owca__::exec_map_object *src)
//	{
//		clear(vm);
//		resize(vm,src->tablesize);
//
//		exec_map_object_iterator iter;
//		while(src->next(iter)) {
//			src->getkey(iter).gc_acquire();
//			src->getval(iter).gc_acquire();
//		}
//
//		insertelems(src->table,src->tablesize);
//	}

} }
