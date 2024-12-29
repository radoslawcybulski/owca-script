#ifndef _RC_Y_STRING_H
#define _RC_Y_STRING_H

namespace owca {
	class owca_local;
	class owca_class;
	class owca_parameters;
	class owca_string;
	class owca_global;
	class owca_string_buffer;
	class owca_namespace;
	class owca_vm;
	namespace __owca__ {
		class virtual_machine;
		class internal_class;
		class exec_variable;
		class owca_internal_string;
		class exec_function_ptr;
		class obj_constructor_function;
		class local_obj_constructor;
	}
}

namespace owca {

	class owca_string {
		friend class owca_local;
		friend class __owca__::virtual_machine;
		friend class __owca__::exec_function_ptr;
		friend class __owca__::obj_constructor_function;
		friend class __owca__::internal_class;
		friend class __owca__::local_obj_constructor;
		friend class owca_parameters;
		friend class owca_string_buffer;
		friend class owca_namespace;
		friend class owca_class;

		__owca__::owca_internal_string *_ss;
		__owca__::virtual_machine *_vm;
		bool _destroy;
		OWCA_SCRIPT_DLLEXPORT unsigned int _update_index(owca_int index) const;
		owca_string(__owca__::virtual_machine *vm, __owca__::owca_internal_string *s) : _vm(vm),_ss(s),_destroy(false) { }
	public:
		owca_string() : _ss(NULL),_vm(NULL),_destroy(false) { }
		OWCA_SCRIPT_DLLEXPORT owca_string(const char *);
		OWCA_SCRIPT_DLLEXPORT owca_string(const char *, size_t size);
		OWCA_SCRIPT_DLLEXPORT owca_string(const std::string &s);
		OWCA_SCRIPT_DLLEXPORT owca_string(const owca_string &);
		OWCA_SCRIPT_DLLEXPORT ~owca_string();
		bool not_bound() const { return _ss == NULL; }

		bool empty(void) const { return data_size() == 0; }

		OWCA_SCRIPT_DLLEXPORT owca_string &operator = (const owca_string &);
		OWCA_SCRIPT_DLLEXPORT bool operator == (const owca_string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator != (const owca_string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator >= (const owca_string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator <= (const owca_string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator >  (const owca_string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator <  (const owca_string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator == (const char *s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator != (const char *s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator >= (const char *s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator <= (const char *s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator >  (const char *s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator <  (const char *s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator == (const std::string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator != (const std::string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator >= (const std::string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator <= (const std::string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator >  (const std::string &s) const;
		OWCA_SCRIPT_DLLEXPORT bool operator <  (const std::string &s) const;
		OWCA_SCRIPT_DLLEXPORT friend bool operator == (const std::string &p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator != (const std::string &p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator >= (const std::string &p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator <= (const std::string &p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator >  (const std::string &p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator <  (const std::string &p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator == (const char *p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator != (const char *p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator >= (const char *p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator <= (const char *p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator >  (const char *p, const owca_string &s);
		OWCA_SCRIPT_DLLEXPORT friend bool operator <  (const char *p, const owca_string &s);

		OWCA_SCRIPT_DLLEXPORT unsigned int operator [] (owca_int index) const;

		OWCA_SCRIPT_DLLEXPORT unsigned int character_count() const;
		OWCA_SCRIPT_DLLEXPORT unsigned int data_size() const;
		OWCA_SCRIPT_DLLEXPORT const char *data() const;
		OWCA_SCRIPT_DLLEXPORT unsigned int hash() const;

		OWCA_SCRIPT_DLLEXPORT owca_string get(owca_int from, owca_int to) const;
		OWCA_SCRIPT_DLLEXPORT owca_string get(owca_int index) const;

		OWCA_SCRIPT_DLLEXPORT std::string str() const;
	};

	class owca_string_buffer {
		enum { SIZE=4096 };
		struct elem {
			unsigned char data[SIZE];
			elem *next;
		};
		unsigned char *ptr;
		elem root,*act;
		unsigned int size;

		OWCA_SCRIPT_DLLEXPORT void _add(const void *ptr, unsigned int size);
	public:
		OWCA_SCRIPT_DLLEXPORT owca_string_buffer();
		OWCA_SCRIPT_DLLEXPORT ~owca_string_buffer();

		OWCA_SCRIPT_DLLEXPORT owca_string_buffer &operator << (char c);
		OWCA_SCRIPT_DLLEXPORT owca_string_buffer &operator << (const char *c);
		OWCA_SCRIPT_DLLEXPORT owca_string_buffer &operator << (const std::string &);
		OWCA_SCRIPT_DLLEXPORT owca_string_buffer &operator << (const owca_string &);

		OWCA_SCRIPT_DLLEXPORT void add(const char *ptr, unsigned int size) { _add(ptr,size); }
		OWCA_SCRIPT_DLLEXPORT owca_global get(owca_vm &vm) const;
		OWCA_SCRIPT_DLLEXPORT void clear(void);
	};
}

#endif
