#include "stdafx.h"
#include "debug_opcodes.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_execute.h"
#include "tree_function.h"
#include "tree_varspace.h"
#include "exec_string.h"
#include "vm_execution_stack_elem_internal.h"
#include <memory.h>

namespace owca {
	namespace __owca__ {
		extern const char *opcodetxtarray[];

		//opcode_writer &opcode_writer::operator << (unsigned char val)
		//{
		//	write_type(OST_UCHAR8);
		//	_add(&val,sizeof(val));
		//	return *this;
		//}

		opcode_writer &opcode_writer::operator << (const std::string &s)
		{
			write_type(OST_STRING);
			//unsigned int t=totalsize;
			store_uint_or_ptr ptr;
			(*this) << ptr;
			//unsigned int *ptr = (unsigned int*)_add(NULL, std::max(sizeof(unsigned int), sizeof(owca_internal_string**)));
			//stringoffsets.push_back(std::pair<unsigned int, store_uint_or_ptr>(t, ptr));
			ptr.set_uint(_register_string(s));

			return *this;
		}

		opcode_writer &opcode_writer::operator << (owca_int i)
		{
			RCASSERT(sizeof(owca_int_unsigned) == sizeof(owca_int));
			write_type(OST_INT);
			store_int si;
			si.init_ptr(_add(sizeof(store_int::data_type)));
			si.set_integer(i);
			return *this;
		}

		opcode_writer &opcode_writer::operator << (owca_real r)
		{
			write_type(OST_REAL);
			store_real sr;
			sr.init_ptr(_add(sizeof(store_int::data_type)));
			sr.set_real(r);
			return *this;
		}

		opcode_writer &opcode_writer::operator << (exec_variable_location &vl)
		{
			write_type(OST_VARIABLELOCATION);
			store_variable_location svl;
			svl.init_ptr(_add(sizeof(store_variable_location::data_type)));
			variablelocations.push_back(std::pair<exec_variable_location*, store_variable_location>(&vl,svl));
			return *this;
		}

		opcode_writer &opcode_writer::operator << (operatorcodes opc)
		{
			unsigned char c=opc;
			RCASSERT(c == opc);
			write_type(OST_OPERATOROPCODE);
			_add(&c,sizeof(c));
			return *this;
		}

		opcode_writer &opcode_writer::operator << (execopcode opc)
		{
			unsigned char c=opc;
			RCASSERT(c == opc);
//#ifdef RCDEBUG
//			char buf[1024];
//			sprintf(buf,"%4d %s\n",totalsize,opcodetxtarray[opc]);
//			debugprint(buf);
//#endif
			write_type(OST_EXECOPCODE);
			_add(&c,sizeof(c));
			return *this;
		}

		opcode_writer &opcode_writer::operator << (unsigned int val)
		{
			write_type(OST_UINT32);
			_add(&val,sizeof(val));
			return *this;
		}

		opcode_writer &opcode_writer::operator << (store_uint_or_ptr &val)
		{
			write_type(OST_UINT32_OR_PTR);
			val.init_ptr(_add(sizeof(store_uint_or_ptr::data_type)));
			return *this;
		}
		//opcode_writer &opcode_writer::operator << (unsigned short val)
		//{
		//	write_type(OST_UINT16);
		//	_add(&val,sizeof(val));
		//	return *this;
		//}

		opcode_writer &opcode_writer::operator << (opcode_writer_jump &wj)
		{
			unsigned int v = 0;

			write_type(OST_JUMP);
			wj.off = totalsize;
			wj.ptr = (unsigned int*)_add(&v, sizeof(v));
			return *this;
		}

#ifdef RCDEBUG_OPCODES
		void opcode_writer::write_type(opcodestreamtype ost)
		{
			unsigned char c=ost;
			_add(&c,sizeof(c));
		}
#endif

		void opcode_writer::finalize_jump(opcode_writer_jump &wj)
		{
			RCASSERT(totalsize >= wj.off + sizeof(unsigned int));
			*wj.ptr = totalsize - wj.off;
		}

		void *opcode_writer::_add(unsigned int size)
		{
			unsigned int alloc_size = (size + sizeof(unsigned int)-1) & ~(sizeof(unsigned int) - 1);

			if (first == NULL || first->pos + alloc_size > memory_elem::SIZE) {
				memory_elem *q = memory_elem::alloc(std::max((unsigned int)memory_elem::SIZE, alloc_size));
				if (first == NULL) first = last = q;
				else last = (last->next = q);
			}
			unsigned char *dst = last->get() + last->pos;
			last->pos += alloc_size;
			totalsize += alloc_size;
			return dst;
		}

		void *opcode_writer::_add(const void *buf, unsigned int size)
		{
			void *dst = _add(size);
			memcpy(dst, buf, size);
			return dst;

			//unsigned char *dt=new unsigned char[sizeof(unsigned int)+size];
			//*(unsigned int*)dt=size;
			//if (buf) {
			//	for(unsigned int i=0;i<size;++i) (dt+sizeof(unsigned int))[i]=((unsigned char*)buf)[i];
			//}
			//else {
			//	for(unsigned int i=0;i<size;++i) (dt+sizeof(unsigned int))[i]=0xfe;
			//}
			//data.push_back(dt);
			//totalsize+=size;
			//return dt+sizeof(unsigned int);
		}

		void opcode_writer::_add_string(const std::string &str)
		{
			(*this) << (unsigned int)str.size();
			write_type(OST_STRING_BODY);
			_add(str.c_str(), (unsigned int)str.size());
		}

		unsigned int opcode_writer::_register_string(const std::string &s)
		{
			std::pair<std::map<std::string, unsigned int>::iterator, bool> it = stringmap.insert(
				std::pair<std::string, unsigned int>(s, (unsigned int)stringmap.size()));
			return it.first->second;
		}

		std::vector<unsigned char> opcode_writer::compile(tree_function *root)
		{
			RCASSERT(totalsize == 0);
			RCASSERT(stringmap.size() == 0);

			//store_uint_or_ptr header[5];
			root->compile_write_root(*this);
			memory_elem *memory_opcodes = first;
			unsigned int opcodes_size = totalsize;

			first = last = NULL;
			totalsize = 0;

			unsigned int index1 = 0, index2 = 0;

			for (std::map<std::string, tree_varspace_location>::iterator it = root->scope->variables.begin(); it != root->scope->variables.end(); ++it) {
				if (it->second.accessed && it->second.required) {
					++index1;
					_register_string(it->first);
				}
			}
			for (std::map<std::string, tree_varspace_location>::iterator it = root->scope->variables.begin(); it != root->scope->variables.end(); ++it) {
				if (it->second.accessed && !it->second.required) {
					++index2;
					_register_string(it->first);
				}
			}

			unsigned int count_12 = root->scope->create_variable_locations();

			RCASSERT(index1 + index2 == count_12);

			for (auto it : variablelocations) {
				it.second.set_location(it.first->depth(), it.first->offset());
				//unsigned short c[2]={vl.depth(),vl.offset()};
			}

			//vm_execution_stack_elem_internal::locationinfo *p=(vm_execution_stack_elem_internal::locationinfo *)(dest+sizeof(unsigned int)*5);
			(*this) << (unsigned int)locinfos.size();

			for (std::list<vm_execution_stack_elem_internal::locationinfo>::iterator it = locinfos.begin(); it != locinfos.end(); ++it) {
				(*this) << it->line << it->offset_begin << it->offset_end;
			}

			(*this) << index1 << index2 << (unsigned int)root->scope->max_stack_yield_size() << (unsigned int)root->scope->max_temporary_variables();

			(*this) << (unsigned int)stringmap.size();
			std::vector<std::pair<std::string, unsigned int> > tmp;
			tmp.reserve(stringmap.size());
			for (auto a : stringmap) tmp.push_back(a);
			std::sort(tmp.begin(), tmp.end(), [](std::pair<std::string, unsigned int> a, std::pair<std::string, unsigned int> b) { return a.second < b.second; });

			for (auto it : tmp)
				_add_string(it.first);

			for (std::map<std::string, tree_varspace_location>::iterator it = root->scope->variables.begin(); it != root->scope->variables.end(); ++it) {
				if (it->second.accessed && it->second.required)
					(*this) << stringmap[it->first];
			}
			for (std::map<std::string, tree_varspace_location>::iterator it = root->scope->variables.begin(); it != root->scope->variables.end(); ++it) {
				if (it->second.accessed && !it->second.required)
					(*this) << stringmap[it->first];
			}

			std::vector<unsigned char> dest(totalsize + opcodes_size + 16);
			dest[ 0] = 'O';
			dest[ 1] = 'W';
			dest[ 2] = 'C';
			dest[ 3] = 'A';
			dest[ 4] = 'S';
			dest[ 5] = 'C';
			dest[ 6] = 'R';
			dest[ 7] = 'I';
			dest[ 8] = 'P';
			dest[ 9] = 'T';
			dest[10] = '!';
			dest[11] = '\0';
			*(uint32_t*)&dest[12] = VERSION_MAGIC;

			//vm_execution_stack_elem_internal::locationinfo *p=(vm_execution_stack_elem_internal::locationinfo *)(dest+sizeof(unsigned int)*5);
			//for(std::list<vm_execution_stack_elem_internal::locationinfo>::iterator it=locinfos.begin();it!=locinfos.end();++it,++p) {
			//	*p=*it;
			//}

			if (first == NULL)
				first = memory_opcodes;
			else
				last->next = memory_opcodes;

			unsigned char *off = &dest[16];
			memory_elem *q = first;
			while (q != NULL) {
				memcpy(off, q->get(), q->pos);
				off += q->pos;
				q = q->next;
			}
			//for(std::list<unsigned char*>::iterator it=data.begin();it!=data.end();++it) {
			//	unsigned int sz=*(unsigned int*)(*it);
			//	unsigned char *buf=(*it)+sizeof(unsigned int);
			//	for(unsigned int i=0;i<sz;++i) off[i]=buf[i];
			//	off+=sz;
			//}

			return dest;
		}

		void opcode_writer::finalize_location()
		{
			if (!locinfotemp.empty() && locinfotemp.back().begin!=totalsize) {
				RCASSERT(locinfotemp.back().begin<totalsize);

				if (!locinfos.empty() && locinfotemp.back().loc==locinfos.back().line) {
					vm_execution_stack_elem_internal::locationinfo &ll=locinfos.back();
					ll.offset_end=totalsize;
				}
				else {
					locinfos.push_back(vm_execution_stack_elem_internal::locationinfo());
					vm_execution_stack_elem_internal::locationinfo &ll=locinfos.back();
					ll.line=locinfotemp.back().loc;
					ll.offset_begin=locinfotemp.back().begin;
					ll.offset_end=totalsize;
				}
			}
		}

		void opcode_writer::push_location(unsigned int l)
		{
			finalize_location();
			locinfotemp.push_back(locationinfotemp());
			locinfotemp.back().loc=l;
			locinfotemp.back().begin=totalsize;
		}

		void opcode_writer::pop_location(unsigned int l)
		{
			RCASSERT(locinfotemp.back().loc==l);
			finalize_location();
			locinfotemp.pop_back();
			if (!locinfotemp.empty()) locinfotemp.back().begin=totalsize;
		}

		opcode_writer_location::opcode_writer_location(opcode_writer &ow_, unsigned int loc_) : ow(ow_),loc(loc_)
		{
			ow.push_location(loc);
		}

		opcode_writer_location::~opcode_writer_location()
		{
			ow.pop_location(loc);
		}

	}
}


