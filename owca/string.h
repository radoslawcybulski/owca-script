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
	}
}

namespace owca {

	class owca_string {
		friend class owca_local;
		friend class __owca__::virtual_machine;
		friend class __owca__::exec_function_ptr;
		friend class __owca__::obj_constructor_function;
		friend class __owca__::internal_class;
		friend class owca_parameters;
		friend class owca_string_buffer;
		friend class owca_namespace;
		friend class owca_class;

		__owca__::owca_internal_string *_ss;
		__owca__::virtual_machine *_vm;
		bool _destroy;
		DLLEXPORT unsigned int _update_index(owca_int index) const;
		owca_string(__owca__::virtual_machine *vm, __owca__::owca_internal_string *s) : _vm(vm),_ss(s),_destroy(false) { }
	public:
		owca_string() : _ss(NULL),_vm(NULL),_destroy(false) { }
		DLLEXPORT owca_string(const char *);
		DLLEXPORT owca_string(const char *, size_t size);
		DLLEXPORT owca_string(const std::string &s);
		DLLEXPORT owca_string(const owca_string &);
		DLLEXPORT ~owca_string();
		bool not_bound() const { return _ss == NULL; }

		bool empty(void) const { return data_size() == 0; }

		DLLEXPORT owca_string &operator = (const owca_string &);
		DLLEXPORT bool operator == (const owca_string &s) const;
		DLLEXPORT bool operator != (const owca_string &s) const;
		DLLEXPORT bool operator >= (const owca_string &s) const;
		DLLEXPORT bool operator <= (const owca_string &s) const;
		DLLEXPORT bool operator >  (const owca_string &s) const;
		DLLEXPORT bool operator <  (const owca_string &s) const;
		DLLEXPORT bool operator == (const char *s) const;
		DLLEXPORT bool operator != (const char *s) const;
		DLLEXPORT bool operator >= (const char *s) const;
		DLLEXPORT bool operator <= (const char *s) const;
		DLLEXPORT bool operator >  (const char *s) const;
		DLLEXPORT bool operator <  (const char *s) const;
		DLLEXPORT bool operator == (const std::string &s) const;
		DLLEXPORT bool operator != (const std::string &s) const;
		DLLEXPORT bool operator >= (const std::string &s) const;
		DLLEXPORT bool operator <= (const std::string &s) const;
		DLLEXPORT bool operator >  (const std::string &s) const;
		DLLEXPORT bool operator <  (const std::string &s) const;
		DLLEXPORT friend bool operator == (const std::string &p, const owca_string &s);
		DLLEXPORT friend bool operator != (const std::string &p, const owca_string &s);
		DLLEXPORT friend bool operator >= (const std::string &p, const owca_string &s);
		DLLEXPORT friend bool operator <= (const std::string &p, const owca_string &s);
		DLLEXPORT friend bool operator >  (const std::string &p, const owca_string &s);
		DLLEXPORT friend bool operator <  (const std::string &p, const owca_string &s);
		DLLEXPORT friend bool operator == (const char *p, const owca_string &s);
		DLLEXPORT friend bool operator != (const char *p, const owca_string &s);
		DLLEXPORT friend bool operator >= (const char *p, const owca_string &s);
		DLLEXPORT friend bool operator <= (const char *p, const owca_string &s);
		DLLEXPORT friend bool operator >  (const char *p, const owca_string &s);
		DLLEXPORT friend bool operator <  (const char *p, const owca_string &s);

		DLLEXPORT unsigned int operator [] (owca_int index) const;

		DLLEXPORT unsigned int character_count() const;
		DLLEXPORT unsigned int data_size() const;
		DLLEXPORT const char *data() const;
		DLLEXPORT unsigned int hash() const;

		DLLEXPORT owca_string get(owca_int from, owca_int to) const;
		DLLEXPORT owca_string get(owca_int index) const;

		DLLEXPORT std::string str() const;
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

		DLLEXPORT void _add(const void *ptr, unsigned int size);
	public:
		DLLEXPORT owca_string_buffer();
		DLLEXPORT ~owca_string_buffer();

		DLLEXPORT owca_string_buffer &operator << (char c);
		DLLEXPORT owca_string_buffer &operator << (const char *c);
		DLLEXPORT owca_string_buffer &operator << (const std::string &);
		DLLEXPORT owca_string_buffer &operator << (const owca_string &);

		DLLEXPORT void add(const char *ptr, unsigned int size) { _add(ptr,size); }
		DLLEXPORT owca_global get(owca_vm &vm) const;
		DLLEXPORT void clear(void);
	};
}

#endif
