#ifndef _RC_Y_GC_ITERATION_H
#define _RC_Y_GC_ITERATION_H

namespace owca {
	namespace __owca__ {
		class exec_base_exist;
		class virtual_machine;
		class exec_object;

#ifdef RCDEBUG_GC
		void exec_ref_counter_add(exec_base_exist *);
		void exec_ref_counter_remove(exec_base_exist *);
		void exec_ref_counter_check(exec_base_exist *);
		void exec_ref_counter_finalize(virtual_machine &);
		void exec_ref_counter_enable(bool z);
#else
		inline void exec_ref_counter_add(exec_base_exist *) { }
		inline void exec_ref_counter_remove(exec_base_exist *) { }
		inline void exec_ref_counter_check(exec_base_exist *) { }
		inline void exec_ref_counter_finalize(virtual_machine &) { }
		inline void exec_ref_counter_enable(bool z) { }
#endif

		class gc_iteration_value {
			friend class virtual_machine;
			unsigned int index;
			void increment(void) { ++index; if (index<2) index=2; }
		public:
			gc_iteration_value() : index(0) { }
			bool operator == (const gc_iteration_value &g) const { return index==g.index; }
			bool operator != (const gc_iteration_value &g) const { return index!=g.index; }
		};
	}
	class gc_iteration {
		friend void __owca__::exec_ref_counter_finalize(__owca__::virtual_machine &);
		friend class __owca__::virtual_machine;
		friend class __owca__::exec_object;
		__owca__::virtual_machine &vm;
		gc_iteration(__owca__::virtual_machine &vm_) : vm(vm_) { }
	public:
		__owca__::gc_iteration_value value;

#ifdef RCDEBUG_GC
		struct debug_info {
			debug_info(const char *name, ...);
			~debug_info();
		};
#else
		struct debug_info {
			debug_info(const char *name, ...) { }
		};
#endif
	};
}

#endif