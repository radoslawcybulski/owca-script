#include "stdafx.h"
#include "base.h"
#include "tree_function.h"
#include "tree_block.h"
#include "tree_varspace.h"
#include "op_base.h"
#include "op_write.h"
#include "op_write.h"
#include "cmp_base.h"
#include "cmp_compiler.h"
#include "virtualmachine.h"
#include "exec_function_ptr.h"
#include "cmp_build_expression.h"
#include "cmp_source.h"
#include "message.h"
#include "op_function.h"

namespace owca { namespace __owca__ {

	RCLMFUNCTION void tree_function::tree_compile_params(source &s, const std::string &finaltoken)
	{
		bool defvalues=false;

		for(;;) {
			if (!s.is_ident()) {
				if (s.is_oper("*")) {
					s.next();
					if (s.is_oper("*")) {
						s.next();
						if (!s.is_ident()) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
						if (!mapparam.name.empty()) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
						mapparam.name=s.token_text();
						mapparam.location=s.token_location();
					}
					else {
						if (!s.is_ident()) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
						if (!listparam.name.empty()) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
						listparam.name=s.token_text();
						listparam.location=s.token_location();
					}
					s.next();
				}
				else throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
			}
			else if (!mapparam.name.empty() || !listparam.name.empty()) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
			else {
				params.push_back(tree_function::paraminfo());
				tree_function::paraminfo &p=params.back();
				p.name=s.token_text();
				p.location=s.token_location();
				p.defval=NULL;
				s.next();
				if (s.is_oper("=")) {
					s.next();
					p.defval=bt_oper_1(cmp,s);
					defvalues=true;
				}
				else if (defvalues) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
			}
			if (s.is_oper(finaltoken)) break;
			if (!s.is_oper(",")) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
			s.next();
		}
	}

	RCLMFUNCTION void tree_function::compile_names(assignment_type assigned)
	{
		tree_varspace *prevscope=NULL;


		if (funcmode!=tree_function::M_SCOPE_CREATOR) {
			//cmp->actual_scope->add_stack_yield_size((unsigned int)(sizeof(op_flow_function)));

			prevscope=cmp->actual_scope;
			scope=new tree_varspace(cmp->actual_scope,this);
			cmp->actual_scope=scope;
			RCASSERT(scope->variables.size()==0);
		}

		scope->parameters.resize(params.size());
		unsigned int paramindex=0;

		for(std::list<paraminfo>::iterator it=params.begin();it!=params.end();++it) {
			it->vi=scope->create_ident(it->name,it->location,false,false);
			scope->parameters[paramindex++]=it->vi;
            if (it->vi->type!=tree_varspace_location::NONE) cmp->error(owca::YERROR_ALREADY_DEFINED,it->location,it->name);
			else it->vi->type=tree_varspace_location::VARIABLE;
			if (it->defval) {
				TRY_(it->defval->compile_names(AT_NONE));
			}
		}

		if (!listparam.name.empty()) {
			listparam.vi=scope->create_ident(listparam.name,listparam.location,false,false);
			if (listparam.vi->type!=tree_varspace_location::NONE) cmp->error(owca::YERROR_ALREADY_DEFINED,listparam.location,listparam.name);
			else listparam.vi->type=tree_varspace_location::VARIABLE;
		}
		if (!mapparam.name.empty()) {
			mapparam.vi=scope->create_ident(mapparam.name,mapparam.location,false,false);
			if (mapparam.vi->type!=tree_varspace_location::NONE) cmp->error(owca::YERROR_ALREADY_DEFINED,mapparam.location,mapparam.name);
			else mapparam.vi->type=tree_varspace_location::VARIABLE;
		}

		switch(funcmode) {
		case M_SELF:
			selfvar=scope->create_ident("self",location,false,true);
			RCASSERT(selfvar->type==tree_varspace_location::NONE);
			selfvar->type=tree_varspace_location::VARIABLE;
			RCASSERT(selfvar->readonly);
			break;
		case M_CLASS:
			selfvar=scope->create_ident("class",location,false,true);
			RCASSERT(selfvar->type==tree_varspace_location::NONE);
			selfvar->type=tree_varspace_location::VARIABLE;
			RCASSERT(selfvar->readonly);
			break;
		case M_FUNCTION:
		case M_COMPREHENSION:
		case M_CLASS_CREATOR:
		case M_SCOPE_CREATOR:
			break;
		default:
			RCASSERT(0);
		}

		TRY_(body->compile_names());

		if (funcmode!=tree_function::M_SCOPE_CREATOR) {
			cmp->actual_scope=prevscope;
			//cmp->actual_scope->strip_stack_yield_size((unsigned int)(sizeof(op_flow_function)));
		}
	}

	RCLMFUNCTION bool tree_function::compile_write(opcode_writer &dst, expression_type et)
	{
		RCASSERT(et==ET_RVALUE);
		unsigned int defvalcount=0;

		for(std::list<paraminfo>::iterator it=params.begin();it!=params.end();++it) {
			if (it->defval) {
				opcode_writer_location owl(dst,it->defval->get_first_location());
				TRY_(it->defval->compile_write(dst,ET_RVALUE));
				++defvalcount;
			}
		}

		cmp->actual_scope->pop_temporary_variables(defvalcount);
		cmp->actual_scope->push_temporary_variable();

		dst << EXEC_CREATE_FUNCTION;

		tree_varspace *prevscope=cmp->actual_scope;
		cmp->actual_scope=scope;
		store_uint_or_ptr uintptr;

		dst << scope->depth() << (unsigned int)scope->variables.size() << uintptr << scope->max_stack_yield_size();
		switch(funcmode) {
		case M_SELF:
		case M_CLASS:
			dst << selfvar->loc;
			break;
		default:
			RCASSERT(0);
		case M_FUNCTION:
		case M_COMPREHENSION:
		case M_SCOPE_CREATOR:
		case M_CLASS_CREATOR:
			dst << exec_variable_location::invalid;
		}
		dst << name;

		unsigned char mode;
		switch(funcmode) {
		case M_SELF: mode=exec_function_ptr::SELF; break;
		case M_CLASS: mode=exec_function_ptr::CLASS; break;
		default:
			RCASSERT(0);
		case M_FUNCTION:
		case M_COMPREHENSION:
		case M_SCOPE_CREATOR:
		case M_CLASS_CREATOR:
			mode=exec_function_ptr::NONE;
			break;
		}
		switch(functype) {
		case FUNCTION: mode|=exec_function_ptr::FUNCTION<<2; break;
		case READ: mode|=exec_function_ptr::PROPERTY_READ<<2; break;
		case WRITE: mode|=exec_function_ptr::PROPERTY_WRITE<<2; break;
		default:
			RCASSERT(0);
		}
		if (generator) mode|=(1<<4);
		if (!listparam.name.empty()) mode|=(1<<5);
		if (!mapparam.name.empty()) mode|=(1<<6);
		dst << (unsigned int)mode;

		dst << (unsigned int)params.size();
		//for(std::list<paraminfo>::iterator it=params.begin();it!=params.end();++it) {
		//	if (it->defval) ++defvalcount;
		//}
		dst << defvalcount;

		std::set<tree_varspace_location*> tmpset;
		for(std::list<paraminfo>::iterator it=params.begin();it!=params.end();++it) {
			tmpset.insert(it->vi);
			dst << it->name << it->vi->loc;
		}
		for(std::map<std::string,tree_varspace_location>::iterator it=scope->variables.begin();it!=scope->variables.end();++it) {
			if (tmpset.find(&it->second)==tmpset.end()) {
				dst << it->first << it->second.loc;
			}
		}
		//for(std::map<unsigned int,std::map<std::string,tree_varspace_location>::iterator >::iterator it=tmpmap.begin();it!=tmpmap.end();++it) {
		//	dst << it->second->first;
		//	dst << it->second->second.loc;
		//}

		if (!listparam.name.empty()) {
			dst << listparam.name << listparam.vi->loc;
		}
		if (!mapparam.name.empty()) {
			dst << mapparam.name << mapparam.vi->loc;
		}

		opcode_writer_jump jmp;
		dst << jmp;

		bool lastb=false;
		try {
			lastb=body->compile_write(dst);
			RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		} catch(error_information &e) {
			cmp->error(e);
			cmp->actual_scope->clear_temporary_variables(0);
		}
		if (lastb) {
			opcode_writer_location owl(dst,get_last_location());
			dst << FLOW_RETURN_NO_VALUE;
		}
		dst.finalize_jump(jmp);

		uintptr.set_uint((unsigned int)scope->max_temporary_variables());

		cmp->actual_scope=prevscope;

		return true;
	}

	void tree_function::compile_write_root(owca::__owca__::opcode_writer &dst)
	{
		RCASSERT(cmp->actual_scope==scope);
		bool lastb=false;
		try {
			lastb=body->compile_write(dst);
			RCASSERT(cmp->actual_scope->count_temporary_variables()==0);
		} catch(error_information &e) {
			cmp->error(e);
			cmp->actual_scope->clear_temporary_variables(0);
		}
		if (lastb) {
			opcode_writer_location owl(dst,get_last_location());
			dst << FLOW_RETURN_NO_VALUE;
		}
		RCASSERT(cmp->actual_scope==scope);
		//dst << FLOW_FIN_0;
	}
} }












