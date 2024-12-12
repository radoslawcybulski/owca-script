#ifndef _RC_Y_STRUCTINFO_H
#define _RC_Y_STRUCTINFO_H

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		struct structinfo;
		class virtual_machine;
		class structid_type;
	}
}

namespace owca {
	namespace __owca__ {
		class object_base_user;
		class structid_type {
			char *c;
			typedef void (structid_type::*__bool_idiom_type__)() const;
			void __bool_idiom_func__() const { }
		public:
			operator __bool_idiom_type__() const { return c ? &structid_type::__bool_idiom_func__ : NULL; }
			structid_type(char *cc) : c(cc) { }
			bool operator == (const structid_type &a) const { return c==a.c; }
			bool operator != (const structid_type &a) const { return c!=a.c; }
			bool operator < (const structid_type &a) const { return c<a.c; }
			bool operator > (const structid_type &a) const { return c>a.c; }
			bool operator <= (const structid_type &a) const { return c<=a.c; }
			bool operator >= (const structid_type &a) const { return c>=a.c; }
		};
		struct structinfo {
			template <class A> static structid_type structid() { static char c; return structid_type(&c); }
			template <class A> struct T {
				typedef typename A::template __aliaser__<A>::T TT;
			};
			template <class A> struct T1 {
				template <typename Q> static void _constructor(void *p, unsigned int size, Q &vm) { (new (p) A(vm,size)); }
				template <typename Q> static void _destructor(void *p, Q &vm) { ((A*)p)->_destroy(vm); ((A*)p)->~A(); }
				static void _marker(const void *p, const gc_iteration &gc) { ((const A*)p)->_marker(gc); }
			};
			template <class A> struct T2 {
				template <typename Q> static void _constructor(void *p, unsigned int size, Q &vm) { (new (p) A(*vm.owner_vm,size)); }
				template <typename Q> static void _destructor(void *p, Q &vm) { ((A*)p)->destroy(*vm.owner_vm); ((A*)p)->~A(); }
				static void _marker(const void *p, const gc_iteration &gc) {
					((const A*)p)->marker(gc);
				}
			};
			template <class A> static structinfo make() {
				structinfo s;
				s.storagespace=sizeof(A);
				s.constructor=T<A>::TT::_constructor;
				s.destructor=T<A>::TT::_destructor;
				s.marker=T<A>::TT::_marker;
				s.structident=structid<A*>();
				return s;
			}
			unsigned int storagespace;
			void (*constructor)(void *, unsigned int oversize, virtual_machine &vm);
			void (*destructor)(void *, virtual_machine &vm);
			void (*marker)(const void *, const gc_iteration &gc);
			structid_type structident;
			structinfo() : structident(structid_type(NULL)),marker(NULL),constructor(NULL),destructor(NULL),storagespace(0) { }
		};
		struct object_base {
		private:
			template <typename A> friend struct structinfo::T;
			template <typename A> struct __aliaser__ {
				typedef typename structinfo::T1<A> T;
			};
		protected:
			virtual ~object_base() { }
		};
	}
}
#endif
