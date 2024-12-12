#include "stdafx.h"
#include "base.h"
#include "cmp_compiler.h"
#include "cmp_source.h"
#include "cmp_build_expression.h"
#include "tree_expr_1.h"
#include "tree_expr_2.h"
#include "tree_expr_3.h"
#include "tree_const.h"
#include "tree_eq.h"
#include "tree_if.h"
#include "tree_for.h"
#include "tree_ident.h"
#include "tree_expr_list.h"
#include "tree_function.h"
#include "tree_return_yield_raise.h"
#include "message.h"
#include "operatorcodes.h"

namespace owca { namespace __owca__ {

	tree_expression *bt_oper_0(compiler *cmp, source &s);
	enum bt_oper_0_5_comprehension {
		C_NONE,
		C_SINGLE,
		C_DOUBLE
	};
	tree_expression *bt_oper_0_5(compiler *cmp, source &s, bt_oper_0_5_comprehension allowed=C_NONE, bt_oper_0_5_comprehension *result=NULL, tree_expression **o2ret=NULL);

	enum tokencode {
		T_UNKNOWN,
		T_TRUE,
		T_FALSE,
		T_NULL,
		T_SELF,
		T_CLASS,
	};

	static tokencode allowed_keyword(const std::string &s)
	{
		if (s=="true") return T_TRUE;
		if (s=="false") return T_FALSE;
		if (s=="null") return T_NULL;
		if (s=="self") return T_SELF;
		if (s=="class") return T_CLASS;
		return T_UNKNOWN;
	}

	RCLMFUNCTION tree_expression *bt_oper_lambda(compiler *cmp, source &s)
	{
		tree_function *func=new tree_function(cmp,s.token_location());
		func->functype=tree_function::FUNCTION;
		func->funcmode=tree_function::M_FUNCTION;
		func->generator=false;

		tree_function *prevfncowner=cmp->fncowner;
		cmp->fncowner=func;
		cmp->in_exception_handler.push_back(false);
		try {
			unsigned int loc=s.token_location();
			RCASSERT(s.is_keyword("lambda"));
			s.next();

			if (!s.is_oper(":")) func->tree_compile_params(s,":");
			s.assert_oper(":");
			s.next();

			tree_expression *expr=bt_oper_0_5(cmp,s);

			func->body=new tree_return_yield_raise(cmp,loc,tree_return_yield_raise::RETURN,expr);
		} catch(...) {
			cmp->fncowner=prevfncowner;
			cmp->in_exception_handler.pop_back();
			delete func;
			throw;
		}
		cmp->fncowner=prevfncowner;
		cmp->in_exception_handler.pop_back();
		func->location_end=s.token_location();

		return func;
	}

	RCLMFUNCTION tree_expression *bt_oper_13(compiler *cmp, source &s)
	{
		tree_expression *o=NULL;
		tokencode tmp;

		try {
			if (s.is_keyword("lambda")) {
				return bt_oper_lambda(cmp,s);
			}
			else if (s.is_keyword() && (tmp=allowed_keyword(s.token_text()))!=T_UNKNOWN) {
				tree_function *f=dynamic_cast<tree_function*>(cmp->fncowner);
				tree_expression_const::type_ tp;

				switch(tmp) {
				case T_SELF:
					if (f==NULL || f->funcmode==tree_function::M_CLASS_CREATOR || f->funcmode==tree_function::M_SCOPE_CREATOR || f->funcmode==tree_function::M_CLASS) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
					f->funcmode=tree_function::M_SELF;
					tp=tree_expression_const::T_SELF;
					break;
				case T_CLASS:
					if (f==NULL || f->funcmode==tree_function::M_CLASS_CREATOR || f->funcmode==tree_function::M_SCOPE_CREATOR || f->funcmode==tree_function::M_SELF) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
					f->funcmode=tree_function::M_CLASS;
					tp=tree_expression_const::T_CLASS;
					break;
				case T_TRUE: tp=tree_expression_const::T_TRUE; break;
				case T_FALSE: tp=tree_expression_const::T_FALSE; break;
				case T_NULL: tp=tree_expression_const::T_NULL; break;
				default:
					RCASSERT(0);
				}
				/*else throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());*/
				o=new tree_expression_const(cmp,s.actual(),tp);
			}
			else if (s.is_ident()) o=new tree_expression_ident(cmp,s.token_location(),s.token_text(),false);
			else if (s.is_int()) o=new tree_expression_const(cmp,s.actual(),tree_expression_const::T_INT);
			else if (s.is_real()) o=new tree_expression_const(cmp,s.actual(),tree_expression_const::T_REAL);
			else if (s.is_string()) o=new tree_expression_const(cmp,s.actual(),tree_expression_const::T_STRING);
			else if (s.is_oper("(")) {
				unsigned int l=s.token_location();
				s.next();
				tree_expression_list *oo=NULL;

				if (s.is_oper(")")) {
					o=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_TUPLE);
				}
				else if (!s.is_oper(",")) {
					bt_oper_0_5_comprehension b;

					o=bt_oper_0_5(cmp,s,C_SINGLE,&b);
					if (s.is_oper(",")) {
						oo=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_TUPLE);
						oo->elems.push_back(tree_expression_list::node(o));
						s.next();
						goto tuple;
					}
					if (b!=C_NONE) {
						oo=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_TUPLE);
						oo->elems.push_back(tree_expression_list::node(o));
						o=oo;
					}
				}
				else {
					oo=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_TUPLE);
tuple:
					o=oo;
					while(!s.is_oper(")")) {
						if (s.is_oper(",")) oo->elems.push_back(tree_expression_list::node(NULL));
						else {
							oo->elems.push_back(tree_expression_list::node(bt_oper_0_5(cmp,s)));
						}
						if (s.is_oper(",")) s.next();
						else s.assert_oper(")");
					}
				}
				s.assert_oper(")");
			}
			else if (s.is_oper("[")) {
				unsigned int l=s.token_location();
				s.next();
				tree_expression_list *oo=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_ARRAY);
				o=oo;
				while(!s.is_oper("]")) {
					oo->elems.push_back(tree_expression_list::node(bt_oper_0_5(cmp,s,C_SINGLE)));
					if (s.is_oper(",")) s.next();
					else s.assert_oper("]");
				}
				s.assert_oper("]");
			}
			else if (s.is_oper("{")) {
				unsigned int l=s.token_location();
				s.next();
				if (s.is_oper("}")) {
					o=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_MAP);
				}
				else {
					bt_oper_0_5_comprehension compr;
					tree_expression *o2=NULL;
					tree_expression *r=bt_oper_0_5(cmp,s,C_DOUBLE,&compr,&o2);
					tree_expression_list *oo;

					switch(compr) {
					case C_NONE:
						if (o2) {
							o=oo=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_MAP);
							oo->elems.push_back(tree_expression_list::node(r));
							oo->elems.push_back(tree_expression_list::node(o2));

							while(!s.is_oper("}")) {
								s.assert_oper(",");
								s.next();
								oo->elems.push_back(tree_expression_list::node(bt_oper_0_5(cmp,s)));
								s.assert_oper(":");
								s.next();
								oo->elems.push_back(tree_expression_list::node(bt_oper_0_5(cmp,s)));
							}
							s.assert_oper("}");
						}
						else {
							o=oo=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_SET);
							oo->elems.push_back(tree_expression_list::node(r));

							while(!s.is_oper("}")) {
								s.assert_oper(",");
								s.next();
								oo->elems.push_back(tree_expression_list::node(bt_oper_0_5(cmp,s)));
							}
						}
						break;
					case C_DOUBLE:
						o=oo=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_MAP);
						oo->elems.push_back(tree_expression_list::node(r));
						s.assert_oper("}");
						break;
					case C_SINGLE:
						o=oo=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_SET);
						oo->elems.push_back(tree_expression_list::node(r));
						s.assert_oper("}");
						break;
					default:
						RCASSERT(0);
					}
				}
				s.assert_oper("}");
			}
			else {
				throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
			}
		}
		catch(...) {
			delete o;
			throw;
		}
		s.next();
		return o;
	}

	static token *is_param_keyword(compiler *cmp, source &s)
	{
		if (s.is_ident() || s.is_keyword("self")) {
			source ss=s;
			ss.next();
			if (ss.is_oper("=")) {
				ss.next();
				token *q=s.actual();
				s=ss;
				return q;
			}
		}
		return NULL;
	}

	static bool is_list_map_param(source &s, bool &mapobject)
	{
		if (s.is_oper("*")) {
			source ss=s;
			ss.next();
			mapobject=(ss.is_oper("*"));
			if (mapobject) ss.next();
			if (ss.is_ident()) {
				s=ss;
				return true;
			}
		}
		return false;
	}

	RCLMFUNCTION tree_expression *bt_oper_12(compiler *cmp, source &s)
	{
		tree_expression *o=bt_oper_13(cmp,s);

		try {
			for(;;) {
				if (s.is_oper("[")) {
					unsigned int l=s.token_location();
					s.next();
					tree_expression *o2=NULL,*o3=NULL;
					try {
						if (s.is_oper(":")) o2=new tree_expression_const(cmp,s.actual(),tree_expression_const::T_SPEC);
						else o2=bt_oper_0(cmp,s);
						if (s.is_oper(":")) {
							s.next();
							if (!s.is_oper("]")) o3=bt_oper_0(cmp,s);
							else o3=new tree_expression_const(cmp,s.actual(),tree_expression_const::T_SPEC);
						}
						s.assert_oper("]");
						s.next();
					}
					catch(...) {
						delete o2;
						delete o3;
						throw;
					}
					if (o3) o=new tree_expression_3(cmp,l,tree_expression_3::OPCODE_ACCESS_2,o,o2,o3);
					else o=new tree_expression_2(cmp,l,tree_expression_2::OPCODE_ACCESS_1,o,o2);
				}
				else if (s.is_oper("(")) {
					tree_expression_list *oo=new tree_expression_list(cmp,s.token_location(),tree_expression_list::OPCODE_FUNCTION_CALL);
					bool allow_params=true,mapobject,map_used=false,list_used=false;
					std::set<std::string> paramnames;

					oo->elems.push_back(tree_expression_list::node(o));
					o=oo;
					s.next();
					while(!s.is_oper(")")) {
						token *kname;

						if (s.is_oper("*")) {
							s.next();
							mapobject=(s.is_oper("*"));
							if (mapobject) s.next();
							allow_params=false;
							if (mapobject) {
								if (map_used) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
								map_used=true;
								oo->mapvalue=bt_oper_0_5(cmp,s);
							}
							else {
								if (list_used) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
								list_used=true;
								oo->listvalue=bt_oper_0_5(cmp,s);
							}
							oo->opc=tree_expression_list::OPCODE_FUNCTION_CALL_LM;
						}
						else if ((kname=is_param_keyword(cmp,s)) != NULL) {
							allow_params=false;
							if (paramnames.find(kname->text())!=paramnames.end()) throw error_information(owca::YERROR_KEYWORD_PARAM_USED_TWICE,s.actual(),kname->text());
							paramnames.insert(kname->text());
							tree_expression *res=bt_oper_0_5(cmp,s);
							oo->elems.push_back(tree_expression_list::node(res,kname->text()));
							oo->opc=tree_expression_list::OPCODE_FUNCTION_CALL_LM;
						}
						else if (!allow_params) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
						else oo->elems.push_back(bt_oper_0_5(cmp,s));
						if (s.is_oper(",")) s.next();
						else if (!s.is_oper(")")) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
					}
					s.assert_oper(")");
					s.next();
				}
				else if (s.is_oper(".")) {
					unsigned int l=s.token_location();
					s.next();
					if (!s.is_ident()) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
					o=new tree_expression_2(cmp,l,tree_expression_2::OPCODE_LOOKUP,o,new tree_expression_ident(cmp,s.token_location(),s.token_text(),false));
					s.next();
				}
				else break;
			}
		}
		catch(...) {
			delete o;
			throw;
		}
		return o;
	}

	RCLMFUNCTION tree_expression *bt_oper_11(compiler *cmp, source &s)
	{
		unsigned int l=s.token_location();

		if (s.is_oper("-")) { s.next(); return new tree_expression_1(cmp,l,tree_expression_1::OPCODE_SIGN_CHANGE,bt_oper_11(cmp,s)); }
		if (s.is_oper("~")) { s.next(); return new tree_expression_1(cmp,l,tree_expression_1::OPCODE_BIN_NOT,bt_oper_11(cmp,s)); }
		if (s.is_oper("+")) { s.next(); return bt_oper_11(cmp,s); }
		return bt_oper_12(cmp,s);
	}

	RCLMFUNCTION tree_expression *bt_oper_10(compiler *cmp, source &s)
	{
		tree_expression *o1,*o2;
		tree_expression_2::Opcode opc;

		o1=bt_oper_11(cmp,s);
		for(;;) {
			if (s.is_oper("*")) opc=tree_expression_2::OPCODE_MUL;
			else if (s.is_oper("/")) opc=tree_expression_2::OPCODE_DIV;
			else if (s.is_oper("%")) opc=tree_expression_2::OPCODE_MOD;
			else return o1;
			unsigned int l=s.token_location();
			s.next();
			try {
				o2=bt_oper_11(cmp,s);
			}
			catch(...) {
				delete o1;
				throw;
			}
			o1=new tree_expression_2(cmp,l,opc,o1,o2);
		}
		RCASSERT(0);
		return NULL;
	}

	RCLMFUNCTION tree_expression *bt_oper_9(compiler *cmp, source &s)
	{
		tree_expression *o1,*o2;
		tree_expression_2::Opcode opc;

		o1=bt_oper_10(cmp,s);
		for(;;) {
			if (s.is_oper("+")) opc=tree_expression_2::OPCODE_ADD;
			else if (s.is_oper("-")) opc=tree_expression_2::OPCODE_SUB;
			else return o1;
			unsigned int l=s.token_location();
			s.next();
			try {
				o2=bt_oper_10(cmp,s);
			}
			catch(...) {
				delete o1;
				throw;
			}
			o1=new tree_expression_2(cmp,l,opc,o1,o2);
		}
		RCASSERT(0);
		return NULL;
		return o1;
	}

	RCLMFUNCTION tree_expression *bt_oper_8(compiler *cmp, source &s)
	{
		tree_expression *o1,*o2;
		tree_expression_2::Opcode opc;

		o1=bt_oper_9(cmp,s);
		for(;;) {
			if (s.is_oper(">>")) opc=tree_expression_2::OPCODE_RSHIFT;
			else if (s.is_oper("<<")) opc=tree_expression_2::OPCODE_LSHIFT;
			else return o1;
			unsigned int l=s.token_location();
			s.next();
			try {
				o2=bt_oper_9(cmp,s);
			}
			catch(...) {
				delete o1;
				throw;
			}
			o1=new tree_expression_2(cmp,l,opc,o1,o2);
		}
		RCASSERT(0);
		return NULL;
	}

	RCLMFUNCTION tree_expression *bt_oper_7(compiler *cmp, source &s)
	{
		tree_expression *o1,*o2;

		o1=bt_oper_8(cmp,s);
		for(;;) {
			if (!s.is_oper("&")) return o1;
			unsigned int l=s.token_location();
			s.next();
			try {
				o2=bt_oper_8(cmp,s);
			}
			catch(...) {
				delete o1;
				throw;
			}
			o1=new tree_expression_2(cmp,l,tree_expression_2::OPCODE_BIN_AND,o1,o2);
		}
		RCASSERT(0);
		return NULL;
	}

	RCLMFUNCTION tree_expression *bt_oper_6(compiler *cmp, source &s)
	{
		tree_expression *o1,*o2;

		o1=bt_oper_7(cmp,s);
		for(;;) {
			if (!s.is_oper("^")) return o1;
			unsigned int l=s.token_location();
			s.next();
			try {
				o2=bt_oper_7(cmp,s);
			}
			catch(...) {
				delete o1;
				throw;
			}
			o1=new tree_expression_2(cmp,l,tree_expression_2::OPCODE_BIN_XOR,o1,o2);
		}
		RCASSERT(0);
		return NULL;
	}

	RCLMFUNCTION tree_expression *bt_oper_5(compiler *cmp, source &s)
	{
		tree_expression *o1,*o2;

		o1=bt_oper_6(cmp,s);
		for(;;) {
			if (!s.is_oper("|")) return o1;
			unsigned int l=s.token_location();
			s.next();
			try {
				o2=bt_oper_6(cmp,s);
			}
			catch(...) {
				delete o1;
				throw;
			}
			o1=new tree_expression_2(cmp,l,tree_expression_2::OPCODE_BIN_OR,o1,o2);
		}
		RCASSERT(0);
		return NULL;
	}

	static bool bt_oper_4_get_opcode(compiler *cmp, source &s, operatorcodes &opc)
	{
		if (s.is_oper("==")) opc=E_EQ;
		else if (s.is_oper("!=")) opc=E_NOTEQ;
		else if (s.is_oper("<=")) opc=E_LESSEQ;
		else if (s.is_oper(">=")) opc=E_MOREEQ;
		else if (s.is_oper("<"))  opc=E_LESS;
		else if (s.is_oper(">"))  opc=E_MORE;
		else return false;
		return true;
	}

	RCLMFUNCTION tree_expression *bt_oper_4(compiler *cmp, source &s)
	{
		unsigned int ll=s.token_location();
		tree_expression *o1=bt_oper_5(cmp,s),*o2;
		bool neg=false;

		if (s.token_text()=="not") {
			source z=s;
			neg=true;
			s.next();
			if (s.token_text()=="in") goto oper_in;
			s=z;
			goto expr;
		}
		if (s.token_text()=="in") {
oper_in:
			unsigned int l=s.token_location();
			s.next();

			try {
				o2=bt_oper_5(cmp,s);
			}
			catch(...) {
				delete o1;
				throw;
			}
			if (neg) return new tree_expression_1(cmp,l,tree_expression_1::OPCODE_LOG_NOT,new tree_expression_2(cmp,l,tree_expression_2::OPCODE_IN,o1,o2));
			return new tree_expression_2(cmp,l,tree_expression_2::OPCODE_IN,o1,o2);
		}
		if (s.token_text()=="is") {
			unsigned int l=s.token_location();
			s.next();
			if (s.token_text()=="not") {
				s.next();
				neg=true;
			}
			try {
				o2=bt_oper_5(cmp,s);
			}
			catch(...) {
				delete o1;
				throw;
			}
			if (neg) return new tree_expression_1(cmp,l,tree_expression_1::OPCODE_LOG_NOT,new tree_expression_2(cmp,l,tree_expression_2::OPCODE_IS,o1,o2));
			return new tree_expression_2(cmp,l,tree_expression_2::OPCODE_IS,o1,o2);
		}
		else {
expr:
			tree_expression_list *oo=NULL;
			operatorcodes opc;

			try {
				while(bt_oper_4_get_opcode(cmp,s,opc)) {
					if (oo==NULL) {
						oo=new tree_expression_list(cmp,ll,tree_expression_list::OPCODE_CMP);
						oo->elems.push_back(tree_expression_list::node(o1));
					}
					unsigned int l=s.token_location();
					s.next();
					o2=bt_oper_5(cmp,s);
					oo->elems.push_back(tree_expression_list::node(o2,opc));
				}
			} catch(...) {
				delete oo;
				throw;
			}
			return oo ? oo : o1;
		}
	}

	RCLMFUNCTION tree_expression *bt_oper_3_5(compiler *cmp, source &s)
	{
		if (s.is_ident("not")) {
			unsigned int l=s.token_location();
			s.next();
			if (s.is_ident("not")) {

				s.next();
				return bt_oper_3_5(cmp,s);
			}
			return new tree_expression_1(cmp,l,tree_expression_1::OPCODE_LOG_NOT,bt_oper_4(cmp,s));
		}
		return bt_oper_4(cmp,s);
	}

	RCLMFUNCTION tree_expression *bt_oper_3(compiler *cmp, source &s)
	{
		tree_expression *o1,*o2;

		o1=bt_oper_3_5(cmp,s);
		if (s.is_ident("and")) {
			unsigned int l=s.token_location();
			s.next();
			try {
				o2=bt_oper_3(cmp,s);
			}
			catch(...) {
				delete o1;
				throw;
			}
			return new tree_expression_2(cmp,l,tree_expression_2::OPCODE_LOG_AND,o1,o2);
		}
		return o1;
	}

	RCLMFUNCTION tree_expression *bt_oper_2(compiler *cmp, source &s)
	{
		tree_expression *o1,*o2;

		o1=bt_oper_3(cmp,s);
		if (s.is_ident("or")) {
			unsigned int l=s.token_location();
			s.next();
			try {
				o2=bt_oper_2(cmp,s);
			}
			catch(...) {
				delete o1;
				throw;
			}
			return new tree_expression_2(cmp,l,tree_expression_2::OPCODE_LOG_OR,o1,o2);
		}
		return o1;
	}

	RCLMFUNCTION tree_expression *bt_oper_1(compiler *cmp, source &s)
	{
		tree_expression *o1,*o2=NULL,*o3=NULL;

		o1=bt_oper_2(cmp,s);
		if (s.is_oper("?")) {
			unsigned int l=s.token_location();
			s.next();
			try {
				o2=bt_oper_1(cmp,s);
				s.assert_oper(":");
				s.next();
				o3=bt_oper_1(cmp,s);
			}
			catch(...) {
				delete o1;
				delete o2;
				throw;
			}
			return new tree_expression_3(cmp,l,tree_expression_3::OPCODE_QUE,o1,o2,o3);
		}
		return o1;
	}

	static tree_expression *build_comprehension(compiler *cmp, unsigned int l1, unsigned int l2, tree_expression *expr1, tree_expression *expr2, tree_expression *vars, tree_expression *loop, tree_expression *cond)
	{
		tree_function *func=new tree_function(cmp,l1);
		func->functype=tree_function::FUNCTION;
		func->funcmode=tree_function::M_COMPREHENSION;
		func->generator=true;

		tree_function *prevfncowner=cmp->fncowner;
		cmp->fncowner=func;
		cmp->in_exception_handler.push_back(false);

		tree_flow *_if;
		tree_return_yield_raise *_yield;

		if (expr2) {
			tree_expression_list *oo=new tree_expression_list(cmp,l1,tree_expression_list::OPCODE_CREATE_TUPLE);
			oo->elems.push_back(tree_expression_list::node(expr1));
			oo->elems.push_back(tree_expression_list::node(expr2));
			_yield=new tree_return_yield_raise(cmp,l1,tree_return_yield_raise::YIELD,oo);
		}
		else {
			_yield=new tree_return_yield_raise(cmp,l1,tree_return_yield_raise::YIELD,expr1);
		}

		if (cond) {
			tree_if *i=new tree_if(cmp,l1);
			i->blocks.push_back(tree_if::pair(
				cond,_yield));
			_if=i;
		}
		else _if=_yield;
		tree_for *_for=new tree_for(cmp,l1);
		_for->vars=vars;
		_for->loop=loop;
		_for->mainblock=_if;
		func->body=_for;

		cmp->fncowner=prevfncowner;
		cmp->in_exception_handler.pop_back();
		func->location_end=l2;

		tree_expression_list *oo=new tree_expression_list(cmp,l1,tree_expression_list::OPCODE_FUNCTION_CALL);
		oo->elems.push_back(tree_expression_list::node(func));
		return oo;
	}

	RCLMFUNCTION tree_expression *bt_oper_0_5(compiler *cmp, source &s, bt_oper_0_5_comprehension allowed, bt_oper_0_5_comprehension *result, tree_expression **o2ret)
	{
		tree_expression *o1=bt_oper_1(cmp,s);
		unsigned int colon_loc;

		if (result) {
			*result=C_NONE;
		}
		RCASSERT((allowed==C_DOUBLE && o2ret) || (allowed!=C_DOUBLE && o2ret==NULL));

		if (allowed==C_DOUBLE) {
			*o2ret=NULL;
			if (s.token_text()==":") {
				colon_loc=s.token_location();
				s.next();
				try {
					*o2ret=bt_oper_1(cmp,s);
				} catch(...) {
					delete o1;
					throw;
				}
			}
		}

		if (s.is_keyword("for")) {
			tree_expression *labels=NULL,*loop=NULL,*cond=NULL;
			unsigned int l=s.token_location();

			try {
				s.next();
				labels=build_tree_expression(cmp,s);
				s.assert_oper("=");
				s.next();
				loop=build_tree_expression(cmp,s);
				if (s.is_keyword("if")) {
					s.next();
					cond=build_tree_expression(cmp,s);
				}
				o1=build_comprehension(cmp,l,s.token_location(),o1,o2ret ? *o2ret : NULL,labels,loop,cond);
				if (result) {
					if (o2ret) {
						*result=*o2ret ? C_DOUBLE : C_SINGLE;
						*o2ret=NULL;
					}
					else {
						*result=C_SINGLE;
					}
				}
			}
			catch(...) {
				delete cond;
				delete loop;
				delete labels;
				delete o1;
				if (o2ret) delete *o2ret;
				throw;
			}
		}
		//else if (allowed!=C_DOUBLE && o2ret && *o2ret) {
		//	delete *o2ret;
		//	delete o1;
		//	throw error_information(owca::YERROR_SYNTAX_ERROR,colon_loc);
		//}
		return o1;
	}



	RCLMFUNCTION tree_expression *bt_oper_0(compiler *cmp, source &s)
	{
		tree_expression *o=NULL;
		tree_expression_list *oo=NULL;

		try {
			unsigned int l=s.token_location();
			if (!s.is_oper(",")) {
				o=bt_oper_0_5(cmp,s);
			}
			if (s.is_oper(",")) {
				bool empty=(o==NULL);

				oo=new tree_expression_list(cmp,l,tree_expression_list::OPCODE_CREATE_TUPLE);
				oo->elems.push_back(tree_expression_list::node(o));
				o=NULL;
				for(;;) {
					s.next();
					if (s.is_oper("]") || s.is_oper("}") || s.is_eo_l() || s.is_oper(":") || s.is_oper(")") || s.is_oper("=")) break;
					if (!s.is_oper(",")) {
						//unsigned int ll=s.token_location();
						oo->elems.push_back(tree_expression_list::node(bt_oper_0_5(cmp,s)));
						empty=false;
					}
					else oo->elems.push_back(tree_expression_list::node(NULL));
					if (!s.is_oper(",")) {
						if (s.is_oper("]") || s.is_oper("}") || s.is_eo_l() || s.is_oper(":") || s.is_oper(")") || s.is_oper("=")) break;
						throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
					}
				}
				if (empty) throw error_information(owca::YERROR_SYNTAX_ERROR,l);
				o=oo;
			}
			return o;
		} catch(...) {
			delete o;
			delete oo;
			throw;
		}
	}


	RCLMFUNCTION tree_expression *build_tree_expression(compiler *cmp, source &s)
	{
		return bt_oper_0(cmp,s);
	}

	RCLMFUNCTION tree_expression *build_tree_assignment(compiler *cmp, source &s, unsigned int mode)
	{
		tree_expression *left,*right;

		left=build_tree_expression(cmp,s);

		if (!s.is_eo_l()) {
			unsigned int l=s.token_location();
			tree_expression_2::Opcode opc;

			if (s.is_oper("=")) opc=tree_expression_2::OPCODE_ASSIGN;
			else if (s.is_oper(":=")) opc=tree_expression_2::OPCODE_ASSIGN_NON_LOCAL;
			else if (s.is_oper("+=")) opc=tree_expression_2::OPCODE_ASSIGN_ADD;
			else if (s.is_oper("-=")) opc=tree_expression_2::OPCODE_ASSIGN_SUB;
			else if (s.is_oper("*=")) opc=tree_expression_2::OPCODE_ASSIGN_MUL;
			else if (s.is_oper("/=")) opc=tree_expression_2::OPCODE_ASSIGN_DIV;
			else if (s.is_oper("%=")) opc=tree_expression_2::OPCODE_ASSIGN_MOD;
			else if (s.is_oper("<<=")) opc=tree_expression_2::OPCODE_ASSIGN_LSHIFT;
			else if (s.is_oper(">>=")) opc=tree_expression_2::OPCODE_ASSIGN_RSHIFT;
			else if (s.is_oper("&=")) opc=tree_expression_2::OPCODE_ASSIGN_BIN_AND;
			else if (s.is_oper("|=")) opc=tree_expression_2::OPCODE_ASSIGN_BIN_OR;
			else if (s.is_oper("^=")) opc=tree_expression_2::OPCODE_ASSIGN_BIN_XOR;
			else {
				delete left;
				throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
			}
			try {
				if (opc==tree_expression_2::OPCODE_ASSIGN || opc==tree_expression_2::OPCODE_ASSIGN_NON_LOCAL) {
					if (mode==2) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
					mode=1;
				}
				else {
					if (mode!=0) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
					mode=2;
				}
				s.next();
				right=build_tree_assignment(cmp,s,mode);
			}
			catch(...) {
				delete left;
				throw;
			}
			left=new tree_expression_2(cmp,l,opc,left,right);
		}
		return left;
	}

} }











