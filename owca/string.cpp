#include "stdafx.h"
#include "base.h"
#include "string.h"
#include "exec_variable.h"
#include "returnvalue.h"
#include "exception.h"
#include "global.h"
#include "exec_string.h"
#include "virtualmachine.h"
#include "vm.h"

namespace owca {
	using namespace __owca__;

	owca_string::~owca_string()
	{
		if (_destroy) {
			_ss->gc_release(*_vm);
		}
	}

	owca_string::owca_string(const owca_string &s) : _vm(s._vm),_destroy(s._destroy),_ss(s._ss)
	{
		if (_destroy) s._ss->gc_acquire();
	}

	owca_string::owca_string(const char *txt) : _vm(NULL), _destroy(true)
	{
		const char *t = txt;
		while (*t) ++t;
		unsigned int index = 0;
		unsigned int char_count = owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, txt, (unsigned int)(t - txt));
		if (index != 0)
			throw owca_exception("invalid utf8 string");
		_ss = owca_internal_string_nongc::allocate(txt, (unsigned int)(t - txt), char_count);
	}

	owca_string::owca_string(const char *txt, size_t size) : _vm(NULL),_destroy(true)
	{
		unsigned int index = 0;
		unsigned int char_count = (unsigned int)owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, txt, (unsigned int)size);
		if (index != 0)
			throw owca_exception("invalid utf8 string");
		_ss = owca_internal_string_nongc::allocate(txt, (unsigned int)size,char_count);
	}

	owca_string::owca_string(const std::string &s) : _vm(NULL),_destroy(true)
	{
		unsigned int index = 0;
		unsigned int char_count = owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, s.c_str(), (unsigned int)s.size());
		if (index != 0)
			throw owca_exception("invalid utf8 string");
		_ss = owca_internal_string_nongc::allocate(s.c_str(), (unsigned int)s.size(),char_count);
	}

	owca_string &owca_string::operator = (const owca_string &s)
	{
		if (s._destroy) {
			s._ss->gc_acquire();
		}
		if (_destroy) {
			_ss->gc_release(*_vm);
		}
		_ss=s._ss;
		_vm=s._vm;
		_destroy=s._destroy;
		return *this;
	}

	unsigned int owca_string::_update_index(owca_int index) const
	{
		owca_int sz = _ss ? _ss->character_count() : 0;
		if (index<0) index+=sz;
		if (index<0 || index>sz) throw owca_exception(OWCA_ERROR_FORMAT2("index %1 is invalid for string of size %2",int_to_string(index),int_to_string(sz)));
		return (unsigned int)index;
	}

	namespace __owca__ {
		void update_2index(owca_int &i1, owca_int &i2, owca_int size);
	}

	owca_string owca_string::get(owca_int from, owca_int to) const
	{
		if (_ss==NULL) return owca_string();
		update_2index(from, to, _ss->character_count());
		unsigned int p1 = _ss->calculate_byte_index_from_char_index((unsigned int)from);
		unsigned int p2 = _ss->calculate_byte_index_from_char_index((unsigned int)to);
		return owca_string(_ss->data_pointer() + p1,(size_t)(p2-p1));
	}

	owca_string owca_string::get(owca_int index) const
	{
		index=_update_index(index);
		unsigned int p1 = _ss->calculate_byte_index_from_char_index((unsigned int)index);
		unsigned int cnt = owca_internal_string::get_byte_count_for_utf8_character(_ss->data_pointer()[p1]);
		unsigned int max = std::min(p1 + cnt, _ss->data_size());
		return owca_string(_ss->data_pointer() + p1,(size_t)(max - p1));
	}

	unsigned int owca_string::operator [] (owca_int index) const
	{
		unsigned int p1 = _ss->calculate_byte_index_from_char_index(_update_index(index));
		unsigned int val = _ss->get_character_at_byte_index(p1);
		return val;
	}

	unsigned int owca_string::data_size() const
	{
		return _ss->data_size();
	}

	unsigned int owca_string::character_count() const
	{
		return _ss->character_count();
	}

	const char *owca_string::data() const
	{
		return _ss->data_pointer();
	}

	unsigned int owca_string::hash() const
	{
		return _ss->hash();
	}

	std::string owca_string::str() const
	{
		return std::string(data(),(size_t)data_size());
	}

	namespace __owca__ {
		int compare_strings(const char *d1, unsigned int s1, const char *d2, unsigned int s2)
		{
			unsigned int i;
			for(i=0;i<std::min(s1,s2);++i) if (d1[i]!=d2[i]) return d1[i]-(int)d2[i];
			if (s1<s2) return -1;
			if (s1>s2) return 1;
			return 0;
		}

	}

	bool owca_string::operator == (const std::string &s) const { return data_size() == (unsigned int)s.size() && __owca__::compare_strings(data(), data_size(), s.c_str(), (unsigned int)s.size()) == 0; }
	bool owca_string::operator != (const std::string &s) const { return data_size() != (unsigned int)s.size() || __owca__::compare_strings(data(), data_size(), s.c_str(), (unsigned int)s.size()) != 0; }
	bool owca_string::operator >= (const std::string &s) const { return __owca__::compare_strings(data(), data_size(), s.c_str(), (unsigned int)s.size()) >= 0; }
	bool owca_string::operator <= (const std::string &s) const { return __owca__::compare_strings(data(), data_size(), s.c_str(), (unsigned int)s.size()) <= 0; }
	bool owca_string::operator >  (const std::string &s) const { return __owca__::compare_strings(data(), data_size(), s.c_str(), (unsigned int)s.size())> 0; }
	bool owca_string::operator <  (const std::string &s) const { return __owca__::compare_strings(data(), data_size(), s.c_str(), (unsigned int)s.size())< 0; }
	bool operator == (const std::string &p, const owca_string &s) { return s.data_size() == (unsigned int)p.size() && __owca__::compare_strings(p.c_str(), (unsigned int)p.size(), s.data(), s.data_size()) == 0; }
	bool operator != (const std::string &p, const owca_string &s) { return s.data_size() != (unsigned int)p.size() || __owca__::compare_strings(p.c_str(), (unsigned int)p.size(), s.data(), s.data_size()) != 0; }
	bool operator >= (const std::string &p, const owca_string &s) { return __owca__::compare_strings(p.c_str(), (unsigned int)p.size(), s.data(), s.data_size()) >= 0; }
	bool operator <= (const std::string &p, const owca_string &s) { return __owca__::compare_strings(p.c_str(), (unsigned int)p.size(), s.data(), s.data_size()) <= 0; }
	bool operator >  (const std::string &p, const owca_string &s) { return __owca__::compare_strings(p.c_str(), (unsigned int)p.size(), s.data(), s.data_size())> 0; }
	bool operator <  (const std::string &p, const owca_string &s) { return __owca__::compare_strings(p.c_str(), (unsigned int)p.size(), s.data(), s.data_size())< 0; }

	bool operator == (const char *p, const owca_string &s)
	{
		const char *e=p;
		while(*e) ++e;
		return (unsigned int)(e - p) == s.data_size() && __owca__::compare_strings(p, (unsigned int)(e - p), s.data(), s.data_size()) == 0;
	}
	bool operator != (const char *p, const owca_string &s)
	{
		const char *e=p;
		while(*e) ++e;
		return (unsigned int)(e - p) != s.data_size() || __owca__::compare_strings(p, (unsigned int)(e - p), s.data(), s.data_size()) != 0;
	}
	bool operator <= (const char *p, const owca_string &s)
	{
		const char *e=p;
		while(*e) ++e;
		return __owca__::compare_strings(p, (unsigned int)(e - p), s.data(), s.data_size()) <= 0;
	}
	bool operator >= (const char *p, const owca_string &s)
	{
		const char *e=p;
		while(*e) ++e;
		return __owca__::compare_strings(p, (unsigned int)(e - p), s.data(), s.data_size()) >= 0;
	}
	bool operator <  (const char *p, const owca_string &s)
	{
		const char *e=p;
		while(*e) ++e;
		return __owca__::compare_strings(p, (unsigned int)(e - p), s.data(), s.data_size())< 0;
	}
	bool operator >  (const char *p, const owca_string &s)
	{
		const char *e=p;
		while(*e) ++e;
		return __owca__::compare_strings(p, (unsigned int)(e - p), s.data(), s.data_size())> 0;
	}

	bool owca_string::operator == (const char *s) const
	{
		const char *e=s;
		while(*e) ++e;
		return (unsigned int)(e - s) == data_size() && __owca__::compare_strings(data(), data_size(), s, (unsigned int)(e - s)) == 0;
	}
	bool owca_string::operator != (const char *s) const
	{
		const char *e=s;
		while(*e) ++e;
		return (unsigned int)(e - s) != data_size() || __owca__::compare_strings(data(), data_size(), s, (unsigned int)(e - s)) != 0;
	}
	bool owca_string::operator <= (const char *s) const
	{
		const char *e=s;
		while(*e) ++e;
		return __owca__::compare_strings(data(), data_size(), s, (unsigned int)(e - s)) <= 0;
	}
	bool owca_string::operator >= (const char *s) const
	{
		const char *e=s;
		while(*e) ++e;
		return __owca__::compare_strings(data(), data_size(), s, (unsigned int)(e - s)) >= 0;
	}
	bool owca_string::operator <  (const char *s) const
	{
		const char *e=s;
		while(*e) ++e;
		return __owca__::compare_strings(data(), data_size(), s, (unsigned int)(e - s))< 0;
	}
	bool owca_string::operator >  (const char *s) const
	{
		const char *e=s;
		while(*e) ++e;
		return __owca__::compare_strings(data(), data_size(), s, (unsigned int)(e - s))> 0;
	}
	bool owca_string::operator == (const owca_string &s) const
	{
		return _ss->hash() == s._ss->hash() && data_size() == s.data_size() && _ss->compare(s._ss) == 0;
	}

	bool owca_string::operator != (const owca_string &s) const
	{
		return _ss->hash() != s._ss->hash() || data_size() != s.data_size() || _ss->compare(s._ss) != 0;
	}

	bool owca_string::operator >= (const owca_string &s) const
	{
		return _ss->compare(s._ss)>=0;
	}

	bool owca_string::operator <= (const owca_string &s) const
	{
		return _ss->compare(s._ss)<=0;
	}

	bool owca_string::operator >  (const owca_string &s) const
	{
		return _ss->compare(s._ss)>0;
	}

	bool owca_string::operator <  (const owca_string &s) const
	{
		return _ss->compare(s._ss)<0;
	}

	owca_string_buffer::owca_string_buffer() : size(0)
	{
		act=&root;
		root.next=NULL;
		ptr=root.data;
	}

	owca_string_buffer::~owca_string_buffer()
	{
		while(elem *e=root.next) {
			root.next=e->next;
			delete e;
		}
	}

	owca_string_buffer &owca_string_buffer::operator << (char c)
	{
		_add(&c,1);
		return *this;
	}

	owca_string_buffer &owca_string_buffer::operator << (const char *c)
	{
		const char *e=c;
		while(*e) ++e;
		_add(c,(unsigned int)(e-c));
		return *this;
	}

	owca_string_buffer &owca_string_buffer::operator << (const std::string &s)
	{
		_add(s.c_str(),(unsigned int)s.size());
		return *this;
	}

	owca_string_buffer &owca_string_buffer::operator << (const owca_string &s)
	{
		_add(s._ss->data_pointer(), s._ss->data_size());
		return *this;
	}

	void owca_string_buffer::clear(void)
	{
		ptr=root.data;
		act=&root;
	}

	void owca_string_buffer::_add(const void *ptr_, unsigned int size)
	{
		const unsigned char *p=(const unsigned char*)ptr_;

		this->size+=size;
		while(size>0) {
			unsigned int max=(unsigned int)((act->data+SIZE)-ptr);
			if (max>size) max=size;
			for(unsigned int i=0;i<max;++i) ptr[i]=p[i];
			ptr+=max;
			p+=max;
			size-=max;
			if (ptr==act->data+SIZE) {
				if (act->next==NULL) {
					act->next=new elem;
					act->next->next=NULL;
				}
				act=act->next;
				ptr=act->data;
			}
			else RCASSERT(ptr>act->data && ptr<act->data+SIZE);
		}
	}

	owca_global owca_string_buffer::get(owca_vm &vm) const
	{
		unsigned int char_count = 0;
		const elem *e = &root;
		unsigned int index = 0;
		static char tmp[] = { (char)0x80, (char)0x80, (char)0x80, (char)0x80, (char)0x80, (char)0x80, (char)0x80, (char)0x80 };

		while (e != act) {
			char_count += owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, (const char*)e->data, SIZE);
			e = e->next;
		}
		char_count += owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, (const char*)act->data, size);

		owca_internal_string *s = vm.vm->allocate_string(NULL, size + index, char_count);
		string_filler sf(s);
		e=&root;
		while(e!=act) {
			sf.append((const char*)e->data,SIZE);
			e=e->next;
		}
		RCASSERT(ptr-act->data>=0 && ptr-act->data<SIZE);
		sf.append((const char*)act->data, (unsigned int)(ptr - act->data));
		sf.append(tmp, index);
		owca_global z(vm.vm);
		z._object.set_string(s);
		return z;
	}
}

