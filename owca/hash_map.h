#ifndef _RC_HASH_MAP
#define _RC_HASH_MAP

#define GROWPREC 0.8
#define SHRINKPREC (0.4*GROWPREC)

namespace owca {
	namespace __owca__ {
		class owca_internal_string;
		template <class KEY, class VALUE> class hash_map;

		class hash_map_iterator {
			template <class A, class B> friend class hash_map;
			unsigned int index;
		public:
			hash_map_iterator() : index(-1) { }
			explicit hash_map_iterator(unsigned int v) : index(v) { }

			unsigned int value() const { return index; }
			void clear() { index=-1; }
		};

		template <class KEY, class VALUE> class hash_map { // key must be inited to empty
		public:
			struct elem {
				KEY key;
				VALUE val;
				owca_int hash() const { return key.hashmap_hash(); }
				bool used() const { return key.hashmap_used(); }
				bool empty() const { return key.hashmap_empty(); }
				bool deleted() const { return key.hashmap_deleted(); }
				void setdeleted() { key.hashmap_setdeleted(); }
				void setempty() { key.hashmap_setempty(); }
			};
		private:
			elem *_table;
			unsigned int _elemcount,_tablesize,_minsize;

			template <typename VM> void _resize(VM &vm, unsigned int newsize) {
				RCASSERT(newsize==0 || newsize>=_minsize);
				RCASSERT(newsize>=_elemcount);
				RCASSERT((newsize & (newsize-1))==0);
				if (newsize>0) {
					unsigned int oldelemcount=_elemcount;
					elem *oldtable=_table;
					unsigned int oldsize=_tablesize;

					_table=(elem*)vm.allocate_memory((unsigned int)(newsize*sizeof(elem)),typeid(elem));
					_elemcount=0;
					_tablesize=newsize;

					for(owca_int i=0;i<_tablesize;++i) _table[i].setempty();

					if (oldtable) {
						_insertelems(oldtable,oldsize,false);
						RCASSERT(_elemcount==oldelemcount);
						vm.free_memory(oldtable);
					}
					RCASSERT(_elemcount==oldelemcount);
				}
				else {
					if (_table) {
						vm.free_memory(_table);
						_table=NULL;
						_tablesize=0;
					}
				}
			}
			void _insertelems(elem *table, unsigned int tablesize, bool acquire) {
				// this reinserts old table - we know all elements are distinct in respect to each other. so we will just find a free space for each.
				// also table must be of correct size

				for(unsigned int i=0;i<tablesize;++i) {
					if (table[i].used()) {
						elem &v=table[i];

						if (acquire) {
							v.key.gc_acquire();
							v.val.gc_acquire();
						}

						unsigned int j;
						hash_map_iterator mi=start_position(v.hash());
						for(j=1;j<_tablesize;++j) { RCLM1(973);
							if (empty(mi)) { RCLM1(974);
								_table[mi.index]=v;
								++_elemcount;
								break;
							}
							else RCLM1(975);
							next_position(mi,j);
						}
						RCASSERT(j<_tablesize);
					}
					else RCLM1(976);
				}
			}
			void _init_size(virtual_machine &vm) {
				if (_elemcount==0 && _tablesize==0) {
					_resize(vm,_minsize);
				}
			}
		public:
			hash_map() : _table(NULL),_elemcount(0),_tablesize(0),_minsize(8) { }
			~hash_map() { RCASSERT(_table==NULL); }

			const elem *table_pointer() const { return _table; }
			bool empty() const { return _elemcount==0; }
			//void inc_size() { ++_elemcount; }
			//void dec_size() { --_elemcount; }
			void gc_mark(const gc_iteration &gc) const {
				hash_map_iterator it;

				while(next(it)) {
					getkey(it).gc_mark(gc);
					getval(it).gc_mark(gc);
				}
			}

			bool used(const hash_map_iterator &index) const { RCASSERT(index.index<_tablesize); return _table[index.index].used(); }
			bool empty(const hash_map_iterator &index) const { RCASSERT(index.index<_tablesize); return _table[index.index].empty(); }
			bool deleted(const hash_map_iterator &index) const { RCASSERT(index.index<_tablesize); return _table[index.index].deleted(); }

			void update_size(virtual_machine &vm) {
				if (_elemcount==0 && _tablesize!=0) {
					_resize(vm,0);
				}
				else if (_tablesize>_minsize && _elemcount<SHRINKPREC*_tablesize) {
					unsigned int ts=_tablesize;
					while(ts>_minsize && _elemcount<SHRINKPREC*ts) { RCLM1(967); ts>>=1; }

					_resize(vm,ts);
				}
				else if (_elemcount>=_tablesize*GROWPREC) _resize(vm,_tablesize<<1);
			}

			hash_map_iterator start_position(owca_int hash) const {
				hash_map_iterator m;
				m.index=(unsigned int)hash&((_tablesize ? _tablesize : _minsize)-1);
				return m;
			}
			void next_position(hash_map_iterator &it, unsigned int try_) const {
				it.index=(it.index+try_)&(_tablesize-1);
			}
			hash_map_iterator write_position(owca_int hash) const { // assumes key was already looked for and not found
				hash_map_iterator m=start_position(hash);
				for(unsigned int i=1;i<=_tablesize;++i) {
					if (!used(m)) return m;
					next_position(m,i);
				}
				RCASSERT(_tablesize==0 && _elemcount==0 && _table==NULL);
				return m;
			}

			unsigned int table_size() const { return _tablesize; }
			unsigned int size() const { return _elemcount; }
			void setminsize(virtual_machine &vm, unsigned int msize) {
				unsigned char cnt=0;
				msize=msize<=8 ? 8 : msize-1;
				while(msize>0) {
					msize>>=1;
					++cnt;
				}
				cnt=std::min(sizeof(unsigned int)*8-4,(size_t)cnt);
				_minsize=1<<cnt;
				RCASSERT((_minsize & (_minsize-1))==0);
				if (_tablesize<_minsize) update_size(vm);
			}
			bool next(hash_map_iterator &index) const {
				while((++index.index)<_tablesize) {
					if (used(index)) return true;
				}
				return false;
			}

			const KEY &getkey(const hash_map_iterator &index) const { RCASSERT(used(index)); return _table[index.index].key; }
			const VALUE &getval(const hash_map_iterator &index) const { RCASSERT(used(index)); return _table[index.index].val; }
			KEY &getkey(const hash_map_iterator &index) { RCASSERT(used(index)); return _table[index.index].key; }
			VALUE &getval(const hash_map_iterator &index) { RCASSERT(used(index)); return _table[index.index].val; }
			owca_int gethash(const hash_map_iterator &index) const { RCASSERT(used(index)); return _table[index.index].key.hashmap_hash(); }
			//hash_map_iterator getiterator(unsigned int index) const { hash_map_iterator m; m.index=index; return m; }

			template <typename VM> void elem_insert(VM &vm, const hash_map_iterator &index, const KEY &key, const VALUE &val) {
				_init_size(vm);
				RCASSERT(!used(index));
				_table[index.index].key=key;
				_table[index.index].val=val;
				++_elemcount;
				update_size(vm);
			}
			void elem_update(const hash_map_iterator &index, const VALUE &val) {
				RCASSERT(used(index));
				_table[index.index].val=val;
			}
			void elem_remove(virtual_machine &vm, const hash_map_iterator &index, bool update=true) {
				RCASSERT(used(index));
				_table[index.index].key.hashmap_setdeleted();
				--_elemcount;
				if (update) update_size(vm);
			}
			template <typename VM> void clear(VM &vm) {
				if (_table) {
					hash_map_iterator iter;
					while(next(iter)) {
						getval(iter).gc_release(vm);
						getkey(iter).gc_release(vm);
						--_elemcount;
					}
					vm.free_memory(_table);
					_table=NULL;
					RCASSERT(_elemcount==0);
				}
				_tablesize=0;
				_elemcount=0;
			}
			template <typename VM> void copy_from(VM &vm, const hash_map &src) {
				clear(vm);
				_resize(vm,src._tablesize);
				_insertelems(src._table,src._tablesize,true);
				RCASSERT(_elemcount==src._elemcount);
			}
			class hash_map_finder {
				hash_map_iterator iter;
				hash_map &hashmap;
				owca_int hash;
				unsigned int step;
			public:
				hash_map_finder(hash_map &hashmap_, owca_int hashvalue) : hashmap(hashmap_),step(1),hash(hashvalue) {
					iter=hashmap.start_position(hashvalue);
				}

				hash_map_iterator get() const { return iter; }
				bool valid() {
					return step<=hashmap.table_size() && !hashmap.empty(iter);
				}
				void next() {
					do {
						hashmap.next_position(iter,step++);
					} while(step<hashmap.table_size() && hashmap.deleted(iter));
				}
			};
		};
	}
}

#undef GROWPREC
#undef SHRINKPREC

#endif
