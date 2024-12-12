#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "tree_expr_list.h"
#include "cmp_compiler.h"
#include "virtualmachine.h"
#include "exec_string.h"
#include "message.h"
#include "op_write.h"
#include "tree_varspace.h"
#include "tree_function.h"
#include "op_assign_tuple.h"
#include "op_compare.h"
#include "op_create_map.h"
#include "op_create_set.h"
#include "op_create_array.h"
#include "op_create_tuple.h"

namespace owca { namespace __owca__ {

	bool tree_expression_list::is_comprehension() const
	{
		if (elems.size()!=1) return false;
		tree_expression_list *l=dynamic_cast<tree_expression_list*>(elems.front().o);
		if (l==NULL || l->opc!=tree_expression_list::OPCODE_FUNCTION_CALL || l->elems.size()!=1) return false;
		tree_function *f=dynamic_cast<tree_function*>(l->elems.front().o);
		if (f==NULL) return false;
		return f->funcmode==tree_function::M_COMPREHENSION;
	}

	unsigned int tree_expression_list::get_first_location() const
	{
		for(std::list<node>::const_iterator it=elems.begin();it!=elems.end();++it) if (it->o) return it->o->get_first_location();
		return location;
	}
	unsigned int tree_expression_list::get_last_location() const
	{
		for(std::list<node>::const_reverse_iterator it=elems.rbegin();it!=elems.rend();++it) if (it->o) return it->o->get_last_location();
		return location;
	}

	tree_expression_list::tree_expression_list(compiler *cmp_, unsigned int l, Opcode opc_) : tree_expression(cmp_,l),opc(opc_),mapvalue(NULL),listvalue(NULL)
	{
		int z;
		z=2;
	}

	RCLMFUNCTION void tree_expression_list::compile_names(assignment_type assigned)
	{
		switch(opc) {
		case OPCODE_FUNCTION_CALL_LM:
			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) it->o->compile_names(AT_NONE);
			if (listvalue) listvalue->compile_names(AT_NONE);
			if (mapvalue) mapvalue->compile_names(AT_NONE);
			//cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_function_call_lm));
			//cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_function_call_lm));
			break;
		case OPCODE_CREATE_TUPLE:
			if (assigned!=AT_NONE) cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_assign_tuple));
			else if (assigned==AT_NONE && is_comprehension()) {
				cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_create_tuple));
			}
			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) if (it->o) it->o->compile_names(assigned);
			if (assigned!=AT_NONE) cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_assign_tuple));
			else if (assigned==AT_NONE && is_comprehension()) {
				cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_create_tuple));
			}
			break;
		case OPCODE_CREATE_SET:
		case OPCODE_CREATE_MAP:
		case OPCODE_CREATE_ARRAY:
		case OPCODE_FUNCTION_CALL:
			if (opc==OPCODE_CREATE_MAP && !elems.empty()) {
				cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_create_map));
			}
			else if (opc==OPCODE_CREATE_SET && !elems.empty()) {
				cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_create_set));
			}
			else if (opc==OPCODE_CREATE_ARRAY && assigned==AT_NONE && is_comprehension()) {
				cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_create_array));
			}
			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) it->o->compile_names(AT_NONE);
			if (opc==OPCODE_CREATE_MAP && !elems.empty()) {
				cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_create_map));
			}
			else if (opc==OPCODE_CREATE_SET && !elems.empty()) {
				cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_create_set));
			}
			else if (opc==OPCODE_CREATE_ARRAY && assigned==AT_NONE && is_comprehension()) {
				cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_create_array));
			}
			break;
		case OPCODE_CMP:
			if (elems.size()>2) cmp->actual_scope->add_stack_yield_size(sizeof(op_flow_compare));
			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) it->o->compile_names(AT_NONE);
			if (elems.size()>2) cmp->actual_scope->strip_stack_yield_size(sizeof(op_flow_compare));
		}
	}

	RCLMFUNCTION bool tree_expression_list::compile_write(opcode_writer &dst, expression_type et)
	{
		unsigned int index=0;

		if (opc!=OPCODE_CREATE_TUPLE && et!=ET_RVALUE) throw error_information(owca::YERROR_NOT_LVALUE,location);
		else if (opc==OPCODE_CREATE_TUPLE) {
			RCASSERT(et==ET_RVALUE || et==ET_LVALUE);
		}
		switch(opc) {
		case OPCODE_CREATE_TUPLE:
			if (et==ET_RVALUE) {
				for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) {
					if (it->o) RCASSERT(it->o->compile_write(dst,et));
					else {
						dst << EXEC_CREATE_NULL;
						cmp->actual_scope->push_temporary_variable();
					}
				}
				if (is_comprehension()) {
					dst << EXEC_CREATE_TUPLE_COMPREHENSION;
				}
				else {
					dst << EXEC_CREATE_TUPLE;
					dst << (unsigned int)elems.size();
				}
				cmp->actual_scope->pop_temporary_variables((unsigned int)elems.size());
				cmp->actual_scope->push_temporary_variable();
			}
			else {
				RCASSERT(et==ET_LVALUE);
				//cmp->actual_scope->pop_temporary_variables(1);
				dst << EXEC_ASSIGN_TUPLE;
				dst << (unsigned int)elems.size();
				for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) {
					if (it->o) RCASSERT(it->o->compile_write(dst,et));
					//else dst << EXEC_CREATE_NULL;
					dst << FLOW_FIN;
				}
				cmp->actual_scope->pop_temporary_variables(1);
				cmp->actual_scope->push_temporary_variable();
			}
			break;
		case OPCODE_CREATE_ARRAY:
			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) {
				RCASSERT(it->o->compile_write(dst,et));
			}
			if (is_comprehension()) {
				dst << EXEC_CREATE_ARRAY_COMPREHENSION;
			}
			else {
				dst << EXEC_CREATE_ARRAY;
				dst << (unsigned int)elems.size();
			}
			if (elems.empty()) cmp->actual_scope->push_temporary_variable();
			else cmp->actual_scope->pop_temporary_variables((unsigned int)elems.size()-1);
			break;
		case OPCODE_CREATE_SET:
			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) {
				RCASSERT(it->o->compile_write(dst,et));
			}
			if (is_comprehension()) {
				dst << EXEC_CREATE_SET_COMPREHENSION;
			}
			else {
				dst << EXEC_CREATE_SET;
				dst << (unsigned int)elems.size();
			}
			if (elems.empty()) cmp->actual_scope->push_temporary_variable();
			else cmp->actual_scope->pop_temporary_variables((unsigned int)elems.size()-1);
			break;
		case OPCODE_CREATE_MAP:
			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) {
				RCASSERT(it->o->compile_write(dst,et));
			}
			if (is_comprehension()) {
				dst << EXEC_CREATE_MAP_COMPREHENSION;
			}
			else {
				dst << EXEC_CREATE_MAP;
				dst << (unsigned int)elems.size();
			}
			if (elems.empty()) cmp->actual_scope->push_temporary_variable();
			else cmp->actual_scope->pop_temporary_variables((unsigned int)elems.size()-1);
			break;
		case OPCODE_FUNCTION_CALL:
function_call:
			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) {
				RCASSERT(it->o->compile_write(dst,et));
			}
			dst << EXEC_FUNCTION_CALL;
			dst << (unsigned int)elems.size();
			cmp->actual_scope->pop_temporary_variables((unsigned int)elems.size()-1);
			break;
		case OPCODE_FUNCTION_CALL_LM: {
			unsigned int i1=0,i2=0,cnt=0;

			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) {
				if (it->ident.empty()) ++i1;
				else if (it->ident!="self") ++i2;
			}

			if (i1==elems.size() && mapvalue==NULL && listvalue==NULL) {
				RCASSERT(0);
				goto function_call;
			}

			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) if (it->ident.empty()) {
				RCASSERT(it->o->compile_write(dst,et));
				++cnt;
			}
			RCASSERT(cnt==i1);
			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) if (!it->ident.empty() && it->ident!="self") {
				RCASSERT(it->o->compile_write(dst,et));
				++cnt;
			}
			RCASSERT(cnt==i1+i2);

			if (listvalue) {
				RCASSERT(listvalue->compile_write(dst,ET_RVALUE));
				++cnt;
			}
			if (mapvalue) {
				RCASSERT(mapvalue->compile_write(dst,ET_RVALUE));
				++cnt;
			}
			if (i1+i2!=elems.size()) {
				std::list<node>::iterator it;
				for(it=elems.begin();it!=elems.end();++it) if (it->ident=="self") {
					RCASSERT(it->o->compile_write(dst,et));
					++cnt;
					break;
				}
				RCASSERT(it!=elems.end());
			}
			dst << EXEC_FUNCTION_CALL_LIST_MAP;
			dst << i1;
			dst << i2;
			dst << (unsigned int)(
				(listvalue ? 1 : 0) | (mapvalue ? 2 : 0) | (i1+i2!=elems.size() ? 4 : 0)
				);
			for(std::list<node>::iterator it=elems.begin();it!=elems.end();++it) if (!it->ident.empty() && it->ident!="self") {
				dst << it->ident;
			}
			cmp->actual_scope->pop_temporary_variables(cnt-1);
			break; }
		case OPCODE_CMP: {
			std::list<node>::iterator it=elems.begin();
			RCASSERT(it->o->compile_write(dst,ET_RVALUE));
			++it;
			if (elems.size()==2) {
				RCASSERT(it->o->compile_write(dst,ET_RVALUE));
				dst << EXEC_COMPARE_SIMPLE;
				dst << it->opc;
				cmp->actual_scope->pop_temporary_variables(1);
			}
			else {
				opcode_writer_jump done;

				dst << EXEC_COMPARE;
				dst << (unsigned int)(elems.size()-1);
				dst << done;

				for(;it!=elems.end();++it) {
					RCASSERT(it->o->compile_write(dst,ET_RVALUE));
					dst << FLOW_FIN;
					dst << it->opc;
				}
				dst.finalize_jump(done);
				cmp->actual_scope->pop_temporary_variables((unsigned int)elems.size()-1);
			}
			break; }
		default:
			RCASSERT(0);
		}
		RCASSERT(cmp->actual_scope->count_temporary_variables()>0);
		return true;
	}

} }












