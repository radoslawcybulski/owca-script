#include "stdafx.h"
#include "base.h"
#include "exec_base.h"
#include "virtualmachine.h"
#include "exec_object.h"
#include "exec_class_object.h"
#include "exec_string.h"
#include <stdarg.h>

#include <Windows.h>

namespace owca {
#ifdef RCDEBUG
    namespace __owca__ {
        void debug_printf(char *txt, ...) {
			va_list vl;
			va_start(vl,txt);
			char buf[256 * 64];
			vsprintf(buf,txt,vl);
			va_end(vl);
            debugprint(buf);
        }
    }
#endif
#ifdef RCDEBUG_GC
	namespace __owca__ {
				extern unsigned int iteration;
	}

	static std::vector<std::string> debug_states;
	const unsigned int min_iteration=99999999;

	gc_iteration::debug_info::debug_info(const char *name, ...)
	{
		if (__owca__::iteration>=min_iteration) {
			va_list vl;
			va_start(vl,name);
			char buf[2048];
			vsprintf(buf,name,vl);
			va_end(vl);

			if (debug_states.capacity()==0) {
				debug_states.reserve(256);
				RCASSERT(debug_states.capacity()==256);
			}
			else if (debug_states.size()==debug_states.capacity()) {
				debug_states.reserve(debug_states.capacity()*2);
			}
			debug_states.push_back(buf);
		}
	}

	gc_iteration::debug_info::~debug_info()
	{
		if (__owca__::iteration>=min_iteration) {
			debug_states.pop_back();
		}
	}

	static void dbg_print_stack(void)
	{
		for(unsigned int i=0;i<debug_states.size();++i) {
			RCPRINTF("        %3d: %s\n",i,debug_states[i].c_str());
		}
	}
	static void dbg(const __owca__::exec_base_refcount *b, bool acquire)
	{
		int z=__owca__::iteration;
#ifdef RCDEBUG
        const unsigned int id = b->get_debug_ident();
#endif
		if (__owca__::iteration>=min_iteration) {
			//RCPRINTF("iter: %4d   id: %5d   cnt: %3d   %c\n",iteration,b->get_debug_ident(),b->gc_count(),acquire ? '+' : '-');
			dbg_print_stack();
			z=1;
		}
		else
			z=1;
	}

	static void dbg_mrk(const __owca__::exec_base *b)
	{
		int z=__owca__::iteration; // id == 0x473 && z >= 1
#ifdef RCDEBUG
        const unsigned int id = b->get_debug_ident();
#endif
		if (__owca__::iteration>=min_iteration) {
			//RCPRINTF("iter: %4d   id: %5d   mark: %d\n",__owca__::iteration,b->get_debug_ident(),b->get_debug_gc_count());
			dbg_print_stack();
			z=1;
		}
		else
			z=1;
	}
#endif

	namespace __owca__ {

#ifdef RCDEBUG_MEMORY_BLOCKS
		static std::set<unsigned int> st;
		unsigned int memory_index=0;
#endif

#ifdef RCDEBUG
        static unsigned int debug_global_index = 0;
        static unsigned int generate_debug_ident()
        {
            unsigned int id = ++debug_global_index;
            return id;
        }
#endif

		exec_base_id::exec_base_id()
#ifdef RCDEBUG_MEMORY_BLOCKS
			: _memory_id(++memory_index)
#ifdef RCDEBUG
                ,debug_ident(generate_debug_ident())
#endif
#elif defined(RCDEBUG)
            : debug_ident(generate_debug_ident())
#endif
		{
#ifdef RCDEBUG_MEMORY_BLOCKS
			RCASSERT(st.find(_memory_id)==st.end());
			st.insert(_memory_id);
#endif
		}

		exec_base_id::~exec_base_id()
		{
			int v = 0;
#ifdef RCDEBUG_MEMORY_BLOCKS
			RCASSERT(st.find(_memory_id)!=st.end());
			st.erase(_memory_id);
#endif
		}

#ifdef RCDEBUG_MEMORY_BLOCKS
		void exec_base_id::check_blocks(void)
		{
			RCASSERT(st.empty());
		}
#endif

#ifdef RCDEBUG_GC
		static std::set<exec_base_exist*> objects;
		static std::map<unsigned int,exec_base_exist*> id_to_objects;
		bool objects_enabled=false;

		class mmm_type_2 {
			std::map<exec_base_exist*,std::list<unsigned int> > mp;
		public:
			std::list<unsigned int> &operator [] (exec_base_exist *bs) { return mp[bs]; }
		};

		typedef std::map<void *,mmm_type_2> mmm_type;
		static mmm_type mmm;

		void clear_mmm_map()
		{
			mmm.clear();
		}

		void exec_ref_counter_enable(bool z)
		{
			objects_enabled=z;
		}

		void exec_ref_counter_add(exec_base_exist *p)
		{
			RCASSERT((unsigned long long)p>=0x100);
			if (objects.find(p)!=objects.end()) {
				auto tmp = reinterpret_cast<ptrdiff_t>(p) & ~0xffff;
				std::list<unsigned int> &lst=mmm[reinterpret_cast<void*>(tmp)][p];
				RCASSERT(0);
			}
			//mmm[(void*)((unsigned int)reinterpret_cast<unsigned long long>((void*)p)&(~0xffff))][p].push_back(p->get_debug_ident());
			objects.insert(p);
		}

		void exec_ref_counter_remove(exec_base_exist *p)
		{
#ifdef RCDEBUG
            if (p->get_debug_ident()==0x13f) {
				int z=1+1;
			}
#endif
			RCASSERT(objects.find(p)!=objects.end());
			objects.erase(p);
		}

		void exec_ref_counter_check(exec_base_exist *p)
		{
			if (objects_enabled) RCASSERT(objects.find(p)==objects.end());
		}

		void clear_mmm_map();

		void exec_ref_counter_finalize(virtual_machine &vm)
		{
			if (objects_enabled) {
				if (!objects.empty()) {
					RCPRINTF("last iteration: %d\n",iteration);
#ifdef RCDEBUG
					for(std::set<exec_base_exist*>::iterator it=objects.begin();it!=objects.end();++it) {
						const std::type_info &res=typeid(*(*it));
						RCPRINTF("object 0x%x, %s not deallocated!\n",(*it)->get_debug_ident(),res.name());
					}
#endif
					RCASSERT(0);
				}
			}
			clear_mmm_map();
			id_to_objects.clear();
		}

#endif

		exec_base_exist::exec_base_exist()
		{
			exec_ref_counter_add(this);
#ifdef RCDEBUG_GC
			//RCASSERT(id_to_objects.find(get_debug_ident())==id_to_objects.end());
			//id_to_objects[get_debug_ident()]=this;
#endif
		}

		exec_base_exist::~exec_base_exist()
		{
			exec_ref_counter_remove(this);
		}

		//exec_base_refcount::exec_base_refcount(unsigned int initval) : _refcnt(initval) { dbg(this,true); }
		exec_base_refcount::exec_base_refcount() {
            _refcnt = 1;
#ifdef RCDEBUG_GC
			_memorygc = true;
            dbg(this,true);
#endif
        }

		void exec_base_refcount::_gc_acquire()
		{
			RCASSERT(_refcnt>0 && _refcnt<0x80000000);
			++_refcnt;
#ifdef RCDEBUG_GC
			RCASSERT(_memorygc);
			dbg(this,true);
#endif
		}

		void exec_base_refcount::_gc_release(virtual_machine &vm)
		{
			RCASSERT((_refcnt&0x7fffffff)>0);
			--_refcnt;
#ifdef RCDEBUG_GC
			RCASSERT(_memorygc);
			dbg(this,false);
#endif
			if ((_refcnt&0x7fffffff)==0) {
				_link_to_destroy(vm);
			}
		}

		void exec_base::_gc_mark(const gc_iteration &gi) const
		{
#ifdef RCDEBUG_GC
			RCASSERT(_memorygc);
#endif
			if (_prev!=gi.value) {
				_prev=gi.value;
#ifdef RCDEBUG_GC
				_debug_cnt_gc=1;
				dbg_mrk(this);
#endif
				_mark_gc(gi);
			}
			else {
#ifdef RCDEBUG_GC
				++_debug_cnt_gc;
				dbg_mrk(this);
#endif
			}
		}

#ifdef RCDEBUG_GC
		std::string exec_base::_debug_text() const
		{
			char buf[2048];
			const exec_object *oo=dynamic_cast<const exec_object*>(this);
			const std::type_info &res=typeid(*this);

			sprintf(buf,"0x%04x %s %s",
#ifdef RCDEBUG
				get_debug_ident(),
#else
				0,
#endif
				res.name(),
#ifdef RCDEBUG
				oo && oo->type() && oo->type()->_co_->name ? oo->type()->_co_->name->str().c_str() : ""
#else
				""
#endif
			);
			return buf;
		}
#endif

		void exec_base_refcount::_link_to_destroy(virtual_machine &vm)
		{
			_release_resources(vm);
			_destroy(vm);
		}

		void exec_base_refcount::_destroy(virtual_machine &vm)
		{
			this->~exec_base_refcount();
			vm.free_memory(this);
		}

		void exec_base::_unlink(virtual_machine &vm)
		{
			RCASSERT(_gc_prev);
			if (vm.allocated_objects==this) {
				vm.allocated_objects=vm.allocated_objects==_gc_next ? NULL : _gc_next;
			}
			_gc_next->_gc_prev=_gc_prev;
			_gc_prev->_gc_next=_gc_next;
			_gc_prev=NULL;
		}

		void exec_base::_link_to_destroy(virtual_machine &vm)
		{
			if (_gc_prev) _unlink(vm);
			if (_refcnt==0) _release_resources(vm);
			_destroy(vm);
		}

		void exec_base::_delay_destruction(virtual_machine &vm)
		{
			RCASSERT(_refcnt==0);
			if (_gc_prev) {
				_unlink(vm);
				_gc_next=vm.allocated_object_waiting_for_delete;
				vm.allocated_object_waiting_for_delete=this;
			}
		}

		void exec_base::_destroy(virtual_machine &vm)
		{
			this->~exec_base();
			vm.free_memory(this);
		}
	}
}












