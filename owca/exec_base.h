#ifndef _RC_Y_EXEC_BASE_H
#define _RC_Y_EXEC_BASE_H

#include "gc_iterator.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		class gc_iteration_value;
		template <class key, class value> class lineartree;
		class exec_base;
		class exec_base_exist;
		class exec_base_id;
		class exec_base_refcount;
		class virtual_machine;
		class exec_object;
	}
}

namespace owca {
	namespace __owca__ {
#ifdef RCDEBUG_GC
	#define GC(a) do { if (__owca__::debuggc) (a)->run_gc(); } while(0)
#else
	#define GC(a) do {} while(0)
#endif

#ifdef RCDEBUG_GC
		extern bool debuggc;
#endif
		class exec_base_id {
			friend class virtual_machine;
#ifdef RCDEBUG
			unsigned int debug_ident;
		public:
			DLLEXPORT exec_base_id();
			unsigned int get_debug_ident() const { return debug_ident; }
#else
		public:
			DLLEXPORT exec_base_id();
#endif
			DLLEXPORT virtual ~exec_base_id();

#ifdef RCDEBUG_MEMORY_BLOCKS
			const unsigned int _memory_id;
			static void check_blocks(void);
#endif
		};
		class exec_base_exist : public exec_base_id {
			friend void exec_ref_counter_add(exec_base_exist *);
			friend void exec_ref_counter_remove(exec_base_exist *);
			friend void exec_ref_counter_finalize();
		public:
			DLLEXPORT exec_base_exist();
			DLLEXPORT virtual ~exec_base_exist();
		};
		class exec_base_refcount : public exec_base_exist {
		private:
			friend class virtual_machine;
		protected:
			unsigned int _refcnt;

			DLLEXPORT virtual void _destroy(virtual_machine &vm);
			virtual void _release_resources(virtual_machine &vm)=0;
			DLLEXPORT virtual void _link_to_destroy(virtual_machine &vm);
			virtual ~exec_base_refcount() {
				RCASSERT(_refcnt==0 || _refcnt==0x80000000);
			}
		public:
#ifdef RCDEBUG_GC
			bool _memorygc;
#endif
			//exec_base_refcount(unsigned int initval);
			DLLEXPORT exec_base_refcount();
			DLLEXPORT void _gc_acquire();
			DLLEXPORT void _gc_release(virtual_machine &);
			void gc_acquire() {
#ifdef RCDEBUG_GC
				RCASSERT((_refcnt!=0 && _memorygc) || (_refcnt==0 && !_memorygc));
#endif
				if (_refcnt!=0) _gc_acquire();
			}
			void gc_release(virtual_machine &vm) {
#ifdef RCDEBUG_GC
				RCASSERT((_refcnt!=0 && _memorygc) || (_refcnt==0 && !_memorygc));
#endif
				if (_refcnt!=0) _gc_release(vm);
			}
			unsigned int gc_count() const { return _refcnt; }
		};
		class exec_base : public exec_base_refcount {
			friend class virtual_machine;
#ifdef RCDEBUG_GC
			mutable unsigned int _debug_cnt_gc;
#endif
			exec_base *_gc_next,*_gc_prev;
			mutable gc_iteration_value _prev;
		protected:
			DLLEXPORT void _destroy(virtual_machine &vm);
			DLLEXPORT void _delay_destruction(virtual_machine &vm);
			DLLEXPORT void _link_to_destroy(virtual_machine &vm);
			DLLEXPORT void _unlink(virtual_machine &vm);
			virtual void _mark_gc(const gc_iteration &gc) const=0;
			virtual bool is_type() const { return false; }
			virtual ~exec_base() {}
		public:
			exec_base() {
				_gc_next=_gc_prev=NULL;
			}
#ifdef RCDEBUG_GC
			unsigned int get_debug_gc_count() const { return _debug_cnt_gc; }
			DLLEXPORT std::string _debug_text() const;
#endif
			DLLEXPORT void _gc_mark(const gc_iteration &gi) const;
			void gc_mark(const gc_iteration &gi) const {
#ifdef RCDEBUG_GC
				RCASSERT((_refcnt!=0 && _memorygc) || (_refcnt==0 && !_memorygc));
#endif
				if (_refcnt!=0) return _gc_mark(gi);
			}
		};



		template <class key, class value> class lineartree {
			struct elem {
				key k;
				value v;
			};
			std::vector<elem> elems;
		public:
			unsigned int size() const { return (unsigned int)elems.size(); }
			void set(unsigned int size) { elems.resize(size); }
			void set(unsigned int index, const key &k, value &v) { elems[index].v=v; elems[index].k=k; }
			void set(std::map<key,value> &mp) {
				elems.resize(mp.size());
				unsigned int index=0;
				for(typename std::map<key,value>::iterator it=mp.begin();it!=mp.end();++it) {
					elems[index].k=it->first;
					elems[index].v=it->second;
					++index;
				}
			}
			void get(unsigned int index, key &k, value &v) { k=elems[index].k; v=elems[index].v; }
			value &get_value(unsigned int index) { return elems[index].v; }
			key &get_key(unsigned int index) { return elems[index].k; }
			const value &get_value(unsigned int index) const { return elems[index].v; }
			const key &get_key(unsigned int index) const { return elems[index].k; }
			value *operator [] (const key &k) {
				if (elems.empty()) return NULL;
				int beg=0,end=(int)elems.size()-1;
				do {
					int i=(beg+end)/2;
					if (elems[i].k>k) end=i-1;
					else if (elems[i].k<k) beg=i+1;
					else return &elems[i].v;
				} while(beg<=end);
				return NULL;
			}
		};
	}
}
#endif
