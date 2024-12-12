#ifndef _RC_Y_EXEC_STRING_H
#define _RC_Y_EXEC_STRING_H

#include "debug_strings.h"
#include "exec_base.h"
#include "exec_string_compare.h"
#include "hashfunc.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		class hashfunc;
		class stringbuffer;
		class exec_base;
		class virtual_machine;
		class owca_internal_string;
		class string_compare;
		class string_filler;
		class owca_internal_string_nongc;
	}
}

namespace owca { namespace __owca__ {
	struct string_pre {
		unsigned int size;
		unsigned int hash;
		unsigned int char_count;
	};
#ifdef RCDEBUG_DEBUG_STRINGS
	class owca_internal_string : public exec_base /* exec_base_refcount */
#else
	class owca_internal_string : public exec_base_refcount
#endif
	{
		friend class string_filler;
		friend class virtual_machine;
		friend class owca_internal_string_nongc;
		owca_internal_string(unsigned int size__, unsigned int char_count__);
	protected:
		char *__data();
		void _release_resources(virtual_machine &vm) { }
		void _mark_gc(const gc_iteration &gc) const { }
#ifdef RCDEBUG
	public:
		const char *value;
		string_pre *value_pre;
#endif
	public:
		unsigned int hash() const;
		unsigned int data_size() const;
		unsigned int character_count() const;
		const char *data_pointer() const;
		unsigned int calculate_byte_index_from_char_index(unsigned int index) const;
		unsigned int get_character_at_byte_index(unsigned int index) const;
		std::string str() const;
		bool is_ident() const;
		bool is_readonly() const;
		bool equal(const owca_internal_string *s) const;
		bool less_than(const owca_internal_string *s) const;
		bool empty(void) const { return data_size() == 0; }
		int compare(const char *d, unsigned int s) const;
		int compare(const owca_internal_string *s) const { return compare(s->data_pointer(), s->data_size()); }
		static unsigned int object_size(unsigned int stringsize);

#ifndef RCDEBUG_DEBUG_STRINGS
		void gc_mark(const gc_iteration &gi) const { }
#endif

		static unsigned char get_byte_count_for_utf8_character(char p);
		static unsigned int calculate_char_count_and_missing_bytes_if_any(unsigned int &index, const char *ptr, unsigned int size);

		struct fast_comparer {
			bool operator () (const owca_internal_string *a, const owca_internal_string *b) const {
				return a->less_than(b);
			}
		};
	};
	class owca_internal_string_nongc : public owca_internal_string {
	public:
		owca_internal_string_nongc(unsigned int size, unsigned int char_count) : owca_internal_string(size, char_count) {}

	static owca_internal_string_nongc *allocate_nongc(const char *data); // allocate_nongc returns owca_internal_string out of gc completely, its safe in multithreading app and with multiple vms
	static owca_internal_string_nongc *allocate_nongc(const char *data, unsigned int size);
	static owca_internal_string_nongc *allocate(const char *data, unsigned int size, unsigned int char_count);
	static owca_internal_string_nongc *allocate(const char *data1, unsigned int size1, const char *data2, unsigned int size2, unsigned int total_char_count);
	static owca_internal_string_nongc *allocate(const char *data1, unsigned int size1, unsigned int char_count, unsigned int cnt);
	protected:
		void _mark_gc(const gc_iteration &gc) { }
		void _destroy(virtual_machine &vm);
		void _release_resources(virtual_machine &vm) { }
	};

	class string_filler {
		hashfunc f;
		owca_internal_string *s;
		unsigned int index;
	public:
		string_filler(owca_internal_string *s_);
		~string_filler() { RCASSERT(index==s->data_size()); }

		void append(char c);
		void append(const char *ptr, unsigned int size);
		void append(const char *ptr);
		void append(owca_internal_string *s) { append((char*)s->data_pointer(), s->data_size()); }
	};
	class stringbuffer {
		enum { SIZE=1024*128-sizeof(unsigned int)-sizeof(void*) };
		struct elem {
			char buf[SIZE];
			elem *next;
			elem() : next(NULL) { }
			~elem() { delete next; }
			//unsigned int fit(unsigned int maxsize) const { return maxsize>SIZE-offset ? SIZE-offset : maxsize; }
		};
		elem *root,*act;
		unsigned int offset;
	public:
		stringbuffer() : root(NULL),act(NULL) { }
		~stringbuffer() { delete root; }
		void add(char c);
		void add(const char *p, unsigned int size);
		void add(const char *p) { while(*p) add(*p++); }
		void add(owca_internal_string *c) { add(c->data_pointer(), c->data_size()); }
		unsigned int size() const;
		void copy(char *dst) const;
		owca_internal_string *to_string(virtual_machine &vm) const;
	};
	enum {
		string_size=sizeof(owca_internal_string)>sizeof(owca_internal_string_nongc) ? sizeof(owca_internal_string) : sizeof(owca_internal_string_nongc)
	};
} }
#endif
