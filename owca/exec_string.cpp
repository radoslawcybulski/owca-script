#include "stdafx.h"
#include "base.h"
#include "exec_string.h"
#include "virtualmachine.h"
#include "exec_class_int.h"
#include "vm_execution_stack_elem_external.h"
#include "exception.h"
#include <memory.h>

namespace owca {
	namespace __owca__ {

		bool owca_internal_string::is_ident() const
		{
			for (unsigned int j = 0; j<data_size(); ++j) {
				char c=data_pointer()[j];
				if (!((j>0 && c>='0' && c<='9') || (c>='a' && c<='z') || (c>='A' || c<='Z') || c=='_' || (j==0 && c=='$'))) return false;
			}
			return true;
		}

		std::string owca_internal_string::str() const
		{
			return std::string(data_pointer(), data_size());
		}

		bool owca_internal_string::equal(const owca_internal_string *s) const
		{
			if (this==s) return true;
			if (hash()!=s->hash()) return false;
			if (data_size() != s->data_size()) return false;
			unsigned int i=0;
			for(;;) {
				i+=sizeof(unsigned int);
				if (i>data_size()) break;
				if (*(unsigned int*)(data_pointer() + i - sizeof(unsigned int)) !=
					*(unsigned int*)(s->data_pointer() + i - sizeof(unsigned int))) return false;
			}
			i-=sizeof(unsigned int);
			for (; i<data_size(); ++i) if (data_pointer()[i] != s->data_pointer()[i]) return false;
			return true;
		}

		bool owca_internal_string::less_than(const owca_internal_string *s) const
		{
			if (this==s) return false;
			if (hash()!=s->hash()) return hash()<s->hash();
			if (data_size() != s->data_size()) return data_size()<s->data_size();
			for (unsigned int i = 0; i<data_size(); ++i) {
				if (data_pointer()[i] != s->data_pointer()[i]) return data_pointer()[i]<s->data_pointer()[i];
			}
			return false;
		}

		int owca_internal_string::compare(const char *d, unsigned int s) const
		{
			if (d != data_pointer()) {
				unsigned int mx = std::min(data_size(), s);
				for (unsigned int i = 0; i<mx; ++i) if (data_pointer()[i] != d[i]) return data_pointer()[i] - d[i];
			}
			if (data_size() == s) return 0;
			return data_size()<s ? -1 : 1;
		}

		char *owca_internal_string::__data()
		{
			return ((char*)this)+string_size+sizeof(string_pre);
		}

		owca_internal_string::owca_internal_string(unsigned int size__, unsigned int char_count__) {
			((string_pre*)(((char*)this)+string_size))->size=size__; // size
			((string_pre*)(((char*)this) + string_size))->char_count = char_count__; // size
			((string_pre*)(((char*)this)+string_size))->hash=0; // hash
#ifdef RCDEBUG
			value = data_pointer();
			value_pre = (string_pre*)(((char*)this) + string_size);
#endif
		}
		unsigned int owca_internal_string::hash() const
		{
			return ((string_pre*)(((char*)this)+string_size))->hash;
		}

		// from fox-toolkit FXString.cpp
		const signed char utf_bytes[256] = {
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
		};

		unsigned int owca_internal_string::calculate_char_count_and_missing_bytes_if_any(unsigned int &index, const char *ptr, unsigned int size)
		{
			unsigned int cnt = 0;
			while (index < size) {
				char c = ptr[index];
				unsigned char sz = get_byte_count_for_utf8_character(c);
				index += sz;
				++cnt;
			}
			index -= size;
			return cnt;
		}

		unsigned char owca_internal_string::get_byte_count_for_utf8_character(char p)
		{
			return utf_bytes[(unsigned char)p];
		}

		unsigned int owca_internal_string::character_count() const
		{
			return ((string_pre*)(((char*)this) + string_size))->char_count;
			//const char *str = data();
			//unsigned int n = data_size(), p = 0;
			//unsigned int len = 0;

			//while (p<n){
			//	unsigned char c = (unsigned char)str[p++];
			//	p += utf_bytes[c];
			//	++len;
			//}
			//return len;
		}

		unsigned int owca_internal_string::get_character_at_byte_index(unsigned int index) const
		{
			const unsigned int n = data_size();
			const char *str = data_pointer() + index;

			// from fox-toolkit FXString.cpp
			unsigned int w = (unsigned char)str[0];
			if (0xc0 <= w){
				w = (w << 6) ^ (unsigned char)str[1] ^ 0x3080;
				if (0x800 <= w){
					w = (w << 6) ^ (unsigned char)str[2] ^ 0x20080;
					if (0x10000 <= w){
						w = (w << 6) ^ (unsigned char)str[3] ^ 0x400080;
						if (0x200000 <= w){
							w = (w << 6) ^ (unsigned char)str[4] ^ 0x8000080;
							if (0x4000000 <= w) { w = (w << 6) ^ (unsigned char)str[5] ^ 0x80; }
						}
					}
				}
			}
			return w;
		}

		unsigned int owca_internal_string::calculate_byte_index_from_char_index(unsigned int index) const
		{
			const unsigned int n = data_size();
			const char *str = data_pointer();
			unsigned int i;

			RCASSERT(index <= n);

			if (index <= n / 2) {
				i = 0;
				while (index > 0 && i < n) {
					unsigned char c = (unsigned char)str[i];
					i += utf_bytes[c];
					--index;
				}
			}
			else {
				i = n;
				index = n - index;
				while (i > 0 && index > 0) {
					while (--i > 0 && (((unsigned char)str[i]) & 0xc0) == 0x80) ;
					--index;
				}
			}
			return i;
		}

		unsigned int owca_internal_string::data_size() const
		{
			return ((string_pre*)(((char*)this) + string_size))->size;
		}

		const char *owca_internal_string::data_pointer() const
		{
			return ((const char*)this)+string_size+sizeof(string_pre);
		}

		owca_internal_string_nongc *owca_internal_string_nongc::allocate(const char *data1, unsigned int size1, const char *data2, unsigned int size2, unsigned int total_char_count)
		{
			owca_internal_string_nongc *c = new (new char[string_size + sizeof(string_pre)+sizeof(char)*(size1 + size2) + 1]) owca_internal_string_nongc(size1 + size2, total_char_count);
			string_filler sf(c);
			sf.append(data1,size1);
			sf.append(data2,size2);
			return c;
		}

		owca_internal_string_nongc *owca_internal_string_nongc::allocate(const char *data, unsigned int size, unsigned int char_count, unsigned int cnt)
		{
			owca_internal_string_nongc *c = new (new char[string_size + sizeof(string_pre)+sizeof(char)*size*cnt + 1]) owca_internal_string_nongc(size*cnt, char_count*cnt);
			string_filler sf(c);
			for(unsigned int j=0;j<cnt;++j) sf.append(data,size);
			return c;
		}

		owca_internal_string_nongc *owca_internal_string_nongc::allocate_nongc(const char *data)
		{
			const char *e=data;
			while(*e) ++e;
			return allocate_nongc(data,(unsigned int)(e-data));
		}

		owca_internal_string_nongc *owca_internal_string_nongc::allocate_nongc(const char *data, unsigned int size)
		{
			owca_internal_string_nongc *c=(owca_internal_string_nongc*)new char[string_size+sizeof(string_pre)+sizeof(char)*size+1];
			for(unsigned int i=0;i<string_size;++i) ((char*)c)[i]=0;
			((string_pre*)(((char*)c)+string_size))->size=size;
#ifdef RCDEBUG
			c->value = c->data_pointer();
#endif
			string_filler sf(c);
			sf.append(data,size);
			return c;
		}
		owca_internal_string_nongc *owca_internal_string_nongc::allocate(const char *data, unsigned int size, unsigned int char_count)
		{
			owca_internal_string_nongc *c = new (new char[string_size + sizeof(string_pre)+sizeof(char)*size + 1]) owca_internal_string_nongc(size, char_count);
			string_filler sf(c);
			sf.append(data, size);
			return c;
		}
		//owca_internal_string_nongc *owca_internal_string_nongc::allocate()
		//{
		//	return allocate((unsigned int)0);
		//}

		string_filler::string_filler(owca_internal_string *s_) : s(s_),index(0)
		{
			s->__data()[s->data_size()]=0;
			if (s->data_size()==0) ((string_pre*)(((char*)s)+string_size))->hash=0x14c72de3;
		}

		void string_filler::append(char c) {
			s->__data()[index]=c;
#ifdef RCDEBUG
			const_cast<char*>(s->value)[index]=c;
#endif
			f.process(c);
			++index;
			RCASSERT(index<=s->data_size());
			if (index==s->data_size()) ((string_pre*)(((char*)s)+string_size))->hash=f.value();
		}
		void string_filler::append(const char *ptr, unsigned int size)
		{
			for(unsigned int i=0;i<size;++i) {
				s->__data()[index+i]=ptr[i];
#ifdef RCDEBUG
				const_cast<char*>(s->value)[index+i]=ptr[i];
#endif
				f.process(ptr[i]);
			}
			index+=size;
			RCASSERT(index<=s->data_size());
			if (index==s->data_size()) ((string_pre*)(((char*)s)+string_size))->hash=f.value();
		}
		void string_filler::append(const char *ptr)
		{
			while(*ptr) {
				s->__data()[index]=*ptr;
#ifdef RCDEBUG
				const_cast<char*>(s->value)[index]=*ptr;
#endif
				++index;
				f.process(*ptr++);
			}
			RCASSERT(index<=s->data_size());
			if (index==s->data_size()) ((string_pre*)(((char*)s)+string_size))->hash=f.value();
		}

		bool string_compare::operator () (const owca_internal_string *p1, const owca_internal_string *p2) const
		{
			return p1->less_than(p2);
		}

		void stringbuffer::add(char c)
		{
			if (act == NULL) {
				act = root = new elem();
				offset = 0;
			}
			else if (offset >= SIZE) {
				act = act->next = new elem();
				offset = 0;
			}
			act->buf[offset++] = c;
		}

		void stringbuffer::add(const char *p, unsigned int size)
		{
			if (size == 0) return;
			if (act == NULL) {
				act = root = new elem();
				offset = 0;
			}
			unsigned int off = 0;
			while (off < size) {
				unsigned int acc = std::min(size - off, SIZE - offset);
				if (acc == 0) {
					RCASSERT(SIZE == offset);
					act = act->next = new elem();
					offset = 0;
					continue;
				}
				memcpy(act->buf + offset, p + off, acc);
				off += acc;
				offset += acc;
			}
		}

		unsigned int stringbuffer::size() const {
			if (root==NULL) return 0;
			unsigned int sz=0;
			if (elem *a=root) {
				while(a!=act) {
					sz+=SIZE;
					a=a->next;
				}
			}
			sz+=offset;
			return sz;
		}

		void stringbuffer::copy(char *dst) const
		{
			if (root) {
				elem *e=root;
				while(e!=act) {
					for(unsigned int i=0;i<SIZE;++i) dst[i]=e->buf[i];
					dst+=SIZE;
				}
				for(unsigned int i=0;i<offset;++i) dst[i]=act->buf[i];
			}
		}

		owca_internal_string *stringbuffer::to_string(virtual_machine &vm) const
		{
			unsigned int index = 0;
			unsigned int chars = 0;
			static char tmp[] = { (char)0x80, (char)0x80, (char)0x80, (char)0x80, (char)0x80, (char)0x80, (char)0x80, (char)0x80 };

			if (root != NULL) {
				elem *e = root;
				while (e != act) {
					chars += owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, e->buf, SIZE);
					e = e->next;
				}
				chars += owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, act->buf, offset);
			}
			owca_internal_string *s = vm.allocate_string(NULL, size() + index, chars);
			if (s) {
				string_filler sf(s);
				if (root) {
					elem *e = root;
					while(e!=act) {
						sf.append(e->buf,SIZE);
						e=e->next;
					}
					sf.append(act->buf,offset);
					sf.append(tmp, index);
				}
			}
			return s;
		}

		void owca_internal_string_nongc::_destroy(virtual_machine &vm)
		{
			this->~owca_internal_string_nongc();
			delete [] (char*)this;
		}









		bool string_read_1(exec_variable &dst, owca_internal_string *s, virtual_machine &vm, owca_int i1)
		{
			owca_int size = s->character_count();
			if (i1<-size || i1>=size) {
				vm.raise_invalid_integer(OWCA_ERROR_FORMAT2("index %1 is invalid for string of size %2",int_to_string(i1),int_to_string(size)));
				return false;
			}
			if (i1<0) i1+=size;

			unsigned int p1 = s->calculate_byte_index_from_char_index((unsigned int)i1);
			unsigned int cnt = owca_internal_string::get_byte_count_for_utf8_character(s->data_pointer()[p1]);
			unsigned int max = std::min(p1 + cnt, s->data_size());
			dst.set_string(vm.allocate_string(s->data_pointer() + p1, (size_t)(max - p1),1));
			return true;
		}

		void update_2index(owca_int &i1, owca_int &i2, owca_int size)
		{
			if (i1<0) i1+=size;
			if (i2<0) {
				i2+=size;
				if (i2<0) {
					i1=i2=0;
					return;
				}
			}
			if (i2>size) i2=size;
			if (i1>i2) i1=i2;
			else if (i1<0) i1=0;
			RCASSERT(i1>=0 && i1<=i2 && i2<=size);
		}

		owca_internal_string *string_read_2(owca_internal_string *s, virtual_machine *vm, owca_int i1, owca_int i2)
		{
			owca_int size=s->character_count();
			update_2index(i1,i2,size);
			unsigned int p1 = s->calculate_byte_index_from_char_index((unsigned int)i1);
			unsigned int p2 = s->calculate_byte_index_from_char_index((unsigned int)i2);
			return !vm ? owca_internal_string_nongc::allocate(s->data_pointer() + p1, (unsigned int)(p2 - p1), (unsigned int)(i2 - i1)) : vm->allocate_string(s->data_pointer() + p1, (unsigned int)(p2 - p1), (unsigned int)(i2 - i1));
		}

		owca_internal_string *string_add(virtual_machine *vm, owca_internal_string *s1, owca_internal_string *s2)
		{
			if (!vm) return owca_internal_string_nongc::allocate(s1->data_pointer(), s1->data_size(), s2->data_pointer(), s2->data_size(),s1->character_count()+s2->character_count());
			owca_internal_string *ss = vm->allocate_string(NULL, s1->data_size() + s2->data_size(), s1->character_count() + s2->character_count());
			string_filler fc(ss);
			fc.append(s1);
			fc.append(s2);
			return ss;
		}

		owca_internal_string *string_add(virtual_machine *vm, const char *d1, unsigned int l1, const char *d2, unsigned int l2, unsigned int char_count)
		{
			if (!vm) return owca_internal_string_nongc::allocate(d1, l1, d2, l2, char_count);
			owca_internal_string *ss = vm->allocate_string(NULL, l1 + l2, char_count);
			string_filler fc(ss);
			fc.append(d1, l1);
			fc.append(d2, l2);
			return ss;
		}

		owca_internal_string *string_mul(virtual_machine *vm, owca_internal_string *s, unsigned int mul)
		{
			RCASSERT(mul >= 0);
			if (!vm) return owca_internal_string_nongc::allocate(s->data_pointer(), s->data_size(), s->character_count(), mul);
			owca_internal_string *ss = vm->allocate_string(NULL, s->data_size()*mul, s->character_count() * mul);
			string_filler fc(ss);
			for (unsigned int i = 0; i < mul; ++i) {
				fc.append(s);
			}
			return ss;
		}





		D_FUNC1(string, new, const exec_variable *)
			{
				switch(mode) {
				CASE(0)
					if (p1->mode()==VAR_NO_PARAM_GIVEN) {
						return_value->set_string(vm->allocate_string());
						return executionstackreturnvalue::RETURN;
					}
					CALC(1,vm->calculate_str(return_value,*p1));
				CASE(1)
					break;
				default:
					RCASSERT(0);
				}
				return executionstackreturnvalue::RETURN;
			}
			unsigned char mode;
			void create_self(void) { mode=0; }
		D_END

		D_SELF0(string,hash,owca_internal_string*)
			{
				return_value->set_int(self->hash());
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF0(string,str,owca_internal_string*)
			{
				return_value->set_string(self);
				self->gc_acquire();
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF0(string,bool,owca_internal_string*)
			{
				unsigned int ds = self->data_size();
				return_value->set_bool(!self->empty());
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF1(string,add,owca_internal_string*,owca_internal_string*)
			{
				return_value->set_string(string_add(vm,self,p1));
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF1(string,mul,owca_internal_string*,owca_int)
			{
				if (p1<0) {
					vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant multiply by negative amount"));
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				return_value->set_string(string_mul(vm,self,(unsigned int)p1));
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF0(string,size,owca_internal_string*)
			{
				return_value->set_int(self->character_count());
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF1(string,eq,owca_internal_string*,owca_internal_string*)
			{
				return_value->set_bool(self->compare(p1)==0);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF1(string,noteq,owca_internal_string*,owca_internal_string*)
			{
				return_value->set_bool(self->compare(p1)!=0);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF1(string,less,owca_internal_string*,owca_internal_string*)
			{
				return_value->set_bool(self->compare(p1)<0);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF1(string,more,owca_internal_string*,owca_internal_string*)
			{
				return_value->set_bool(self->compare(p1)>0);
				return executionstackreturnvalue::RETURN;
			}
		D_END


		D_SELF1(string,lesseq,owca_internal_string*,owca_internal_string*)
			{
				return_value->set_bool(self->compare(p1)<=0);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF1(string,moreeq,owca_internal_string*,owca_internal_string*)
			{
				return_value->set_bool(self->compare(p1)>=0);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF1(string,read_1,owca_internal_string*,owca_int)
			{
				return string_read_1(*return_value,self,*vm,p1) ? executionstackreturnvalue::RETURN : executionstackreturnvalue::FUNCTION_CALL;
			}
		D_END

		D_SELF2(string,read_2,owca_internal_string*,const exec_variable *, const exec_variable *)
			{
				owca_int i1,i2;

				if (p1->get_int_min(i1) && p2->get_int_max(i2)) {
					return_value->set_string(string_read_2(self,vm,i1,i2));
					return executionstackreturnvalue::RETURN;
				}
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
		D_END

		void virtual_machine::initialize_string(internal_class &c)
		{
			c._setinheritable(false);

			M_OPER1(c,string,new,(*this,"$new","value",dv_no_param_given));
			M_OPER0(c,string,str,(*this,"$str"));
			M_OPER0(c,string,hash,(*this,"$hash"));
			M_OPER0(c,string,bool,(*this,"$bool"));

			M_OPER1(c,string,read_1,(*this,"$read_1","index"));
			M_OPER2(c,string,read_2,(*this,"$read_2","from","to"));

			M_OPER1(c,string,add,(*this,"$add","other"));
			M_OPER1(c,string,mul,(*this,"$mul","other"));
			M_OPER1(c,string,eq,(*this,"$eq","other"));
			M_OPER1(c,string,noteq,(*this,"$noteq","other"));
			M_OPER1(c,string,less,(*this,"$less","other"));
			M_OPER1(c,string,more,(*this,"$more","other"));
			M_OPER1(c,string,lesseq,(*this,"$lesseq","other"));
			M_OPER1(c,string,moreeq,(*this,"$moreeq","other"));

			M_FUNC0(c,string,size,(*this,"size"));
		}
	}
}












