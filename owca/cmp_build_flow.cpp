#include "stdafx.h"
#include "base.h"
#include "cmp_build_flow.h"
#include "cmp_build_expression.h"
#include "cmp_compiler.h"
#include "cmp_source.h"
#include "tree_function.h"
#include "tree_class.h"
#include "tree_for.h"
#include "tree_if.h"
#include "tree_eq.h"
#include "tree_while.h"
#include "tree_with.h"
#include "tree_try.h"
#include "tree_loop_control.h"
#include "tree_block.h"
#include "tree_noop.h"
#include "tree_return_yield_raise.h"
#include "tree_expr_2.h"
#include "tree_ident.h"
#include "tree_varspace.h"
#include "virtualmachine.h"
#include "message.h"

namespace owca {
	namespace __owca__ {
		extern std::string identificator_names[];

		RCLMFUNCTION tree_flow *build_tree_with(compiler *cmp, source &s)
		{
			tree_with *f=new tree_with(cmp,s.token_location());

			try {
				RCASSERT(s.is_keyword("with"));
				s.next();

				for(;;) {
					f->exprs.push_back(tree_with::tt());
					tree_with::tt &ptt=f->exprs.back();

					if (s.is_ident()) {
						source bp=s;
						std::string z=bp.token_text();
						bp.next();
						if (bp.token_text()=="=") {
							bp.next();
							s=bp;
							ptt.ident=z;
						}
					}
					ptt.expr=bt_oper_1(cmp,s);
					if (s.token_text()==":") break;
					if (s.token_text()!=",") throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
					s.next();
				}
				s.assert_oper(":");
				s.next();
				f->mainblock=build_tree_subblock(cmp,s);
			}
			catch(...) {
				delete f;
				throw;
			}
			return f;
		}

		static bool is_finally(source &s)
		{
			if (s.is_keyword("finally")) {
				source p=s;
				p.next();
				if (p.token_text()==":") return true;
			}
			return false;
		}

		RCLMFUNCTION tree_flow *build_tree_for(compiler *cmp, source &s, const std::string &name)
		{
			tree_for *f=new tree_for(cmp,s.token_location());

			try {
				f->name=name;
				RCASSERT(s.is_keyword("for"));
				s.next();
				f->vars=build_tree_expression(cmp,s);
				s.assert_oper("=");
				s.next();
				f->loop=build_tree_expression(cmp,s);
				s.assert_oper(":");
				s.next();
				f->mainblock=build_tree_subblock(cmp,s);
				if (s.is_keyword("else")) {
					s.next();
					s.assert_oper(":");
					s.next();
					f->elseblock=build_tree_subblock(cmp,s);
				}
				if (is_finally(s)) {
					s.next();
					s.assert_oper(":");
					s.next();
					f->finallyblock=build_tree_subblock(cmp,s);
				}
			}
			catch(...) {
				delete f;
				throw;
			}
			return f;
		}

		RCLMFUNCTION tree_flow *build_tree_while(compiler *cmp, source &s, const std::string &name)
		{
			tree_while *f=new tree_while(cmp,s.token_location());

			try {
				f->name=name;
				RCASSERT(s.is_keyword("while"));
				s.next();
				f->condition=build_tree_expression(cmp,s);
				s.assert_oper(":");
				s.next();
				f->mainblock=build_tree_subblock(cmp,s);
				if (s.is_keyword("else")) {
					s.next();
					s.assert_oper(":");
					s.next();
					f->elseblock=build_tree_subblock(cmp,s);
				}
				if (is_finally(s)) {
					s.next();
					s.assert_oper(":");
					s.next();
					f->finallyblock=build_tree_subblock(cmp,s);
				}
			}
			catch(...) {
				delete f;
				throw;
			}
			return f;
		}

		RCLMFUNCTION tree_flow *build_tree_if(compiler *cmp, source &s)
		{
			tree_if *fr=new tree_if(cmp,s.token_location());
			tree_expression *cond=NULL;

			try {
				RCASSERT(s.is_keyword("if"));
				do {
					s.next();
					cond=build_tree_expression(cmp,s);
					s.assert_oper(":");
					s.next();
					tree_flow *main=build_tree_subblock(cmp,s);
					fr->blocks.push_back(tree_if::pair(cond,main));
					cond=NULL;
				} while(s.is_keyword("elif"));
				if (s.is_keyword("else")) {
					s.next();
					s.assert_oper(":");
					s.next();
					fr->elseblock=build_tree_subblock(cmp,s);
				}
				if (is_finally(s)) {
					s.next();
					s.assert_oper(":");
					s.next();
					fr->finallyblock=build_tree_subblock(cmp,s);
				}
				return fr;
			}
			catch(...) {
				delete fr;
				delete cond;
				throw;
			}
		}

		static bool is_as_final(source &s)
		{
			if (s.token_text()=="as") {
				source ss=s;
				ss.next();
				if (ss.is_ident()) {
					ss.next();
					if (ss.is_oper(":")) return true;
					}
				}
			return false;
		}

		RCLMFUNCTION tree_flow *build_tree_try(compiler *cmp, source &s)
		{
			tree_try *acttry=new tree_try(cmp,s.token_location());
			try {
				RCASSERT(s.is_keyword("try"));
				s.next();
				s.assert_oper(":");
				s.next();
				cmp->in_exception_handler.push_back(false);
				acttry->mainblock=build_tree_subblock(cmp,s);
				cmp->in_exception_handler.back()=true;
				bool final=false;

				while(s.is_keyword("except")) {
					if (final) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
					acttry->blocks.push_back(tree_try::pair());
					tree_try::pair &p=acttry->blocks.back();

					p.location=s.token_location();
					s.next();
					try {
						if (!s.is_oper(":")) {
							for(;;) {
								if (p.types.size()!=0 || !is_as_final(s)) p.types.push_back(bt_oper_1(cmp,s));
								if (s.token_text()=="as") {
									s.next();
									if (!s.is_ident()) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
									p.asvar=s.token_text();
									p.location=s.token_location();
									s.next();
									break;
								}
								if (!s.is_oper(",")) break;
								s.next();
							}
						}
						else final=true;
						s.assert_oper(":");
						s.next();
						p.block=build_tree_subblock(cmp,s);
					} catch(...) {
						throw;
					}
				}
				cmp->in_exception_handler.back()=false;
				if (acttry->blocks.size()==0) throw error_information(owca::YERROR_NO_EXCEPT_CLAUSE,s.token_location());
				if (s.is_keyword("else")) {
					s.next();
					s.assert_oper(":");
					s.next();
					acttry->elseblock=build_tree_subblock(cmp,s);
				}
				if (is_finally(s)) {
					s.next();
					s.assert_oper(":");
					s.next();
					acttry->finallyblock=build_tree_subblock(cmp,s);
				}
			} catch(...) {
				delete acttry;
				cmp->in_exception_handler.pop_back();
				throw;
			}
			cmp->in_exception_handler.pop_back();
			return acttry;
		}

		RCLMFUNCTION tree_class *build_tree_class(compiler *cmp, source &s)
		{
			tree_class *actcls=new tree_class(cmp,s.token_location());
			tree_function *prevfncowner=cmp->fncowner;
			source sstart=s;

			cmp->in_exception_handler.push_back(false);
			try {
				RCASSERT(s.is_keyword("class"));
				s.next();
				if (!s.is_ident()) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
                if (s.token_text()[0]=='$') cmp->error(owca::YERROR_INVALID_IDENT,s.actual(),s.token_text());
				actcls->name=s.token_text();
				s.next();
				if (s.is_oper("(")) {
					s.next();
					for(;;) {
						actcls->inherited.push_back(bt_oper_1(cmp,s));
						if (s.is_oper(")")) break;
						s.assert_oper(",");
						s.next();
					}
					s.assert_oper(")");
					s.next();
				}
				s.assert_oper(":");
				s.next();

				actcls->body=new tree_function(cmp,sstart.token_location());
				cmp->fncowner=actcls->body;
				actcls->body->functype=tree_function::FUNCTION;
				actcls->body->funcmode=tree_function::M_CLASS_CREATOR;
				actcls->body->generator=false;
				actcls->body->name=actcls->name+" class creator";

				cmp->fncowner=actcls->body;
				actcls->body->body=build_tree_subblock(cmp,s);

			} catch(...) {
				delete actcls;
				cmp->fncowner=prevfncowner;
				cmp->in_exception_handler.pop_back();
				throw;
			}
			cmp->fncowner=prevfncowner;
			cmp->in_exception_handler.pop_back();
			actcls->location_end=s.token_location();
			return actcls;
		}

		RCLMFUNCTION tree_function *build_tree_function(compiler *cmp, source &s)
		{
			tree_function *func=new tree_function(cmp,s.token_location());
			func->functype=tree_function::FUNCTION;
			func->funcmode=tree_function::M_FUNCTION;
			func->generator=false;

			tree_function *prevfncowner=cmp->fncowner;
			cmp->fncowner=func;
			cmp->in_exception_handler.push_back(false);
			try {
				RCASSERT(s.is_keyword("function"));
				s.next();

				if (!s.is_ident()) throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
				func->name=s.token_text();
				if (s.token_text()[0]=='$') {
					unsigned int i;
					for(i=0;i<E_COUNT;++i) if (identificator_names[i]==func->name) break;
					if (i==E_COUNT) {
                        cmp->error(owca::YERROR_INVALID_IDENT,s.actual(),s.token_text());
					}
				}
				s.next();
				if (s.is_ident()) {
					if (func->name=="read") func->functype=tree_function::READ;
					else if (func->name=="write") func->functype=tree_function::WRITE;
					else throw error_information(owca::YERROR_SYNTAX_ERROR,s.actual());
					func->name=s.token_text();
					if (s.token_text()[0]=='$') {
                        cmp->error(owca::YERROR_INVALID_IDENT,s.actual(),s.token_text());
					}
					s.next();
				}
				s.assert_oper("(");
				s.next();
				if (!s.is_oper(")")) {
					func->tree_compile_params(s,")");
				}
				s.assert_oper(")");
				s.next();
				s.assert_oper(":");
				s.next();
				func->body=build_tree_subblock(cmp,s);
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

		static bool is_name(source &s, std::string &name)
		{
			if (s.is_ident()) {
				source ss=s;
				ss.next();
				if (ss.is_oper(":")) {
					ss.next();
					if (ss.is_keyword("for") || ss.is_keyword("while")) {
						name=s.token_text();
						s=ss;
						return true;
					}
				}
			}
			return false;
		}

		static bool is_classdef(source &s)
		{
			if (s.is_keyword("class")) {
				source ss=s;
				ss.next();
				if (ss.is_ident() || ss.is_oper(":")) return true;
			}
			return false;
		}

		RCLMFUNCTION tree_flow *build_tree_subblock(compiler *cmp, source &s)
		{
			if (s.is_eo_l()) {
				s.next();
				s.assert_up();
				tree_block *block=new tree_block(cmp,s.token_location());
				s.next();

				while(!s.is_down()) block->elements.push_back(build_tree_line(cmp,s));

				RCASSERT(!block->elements.empty());

				s.next();
				return block;
			}
			return build_tree_statement(cmp,s);
		}

		RCLMFUNCTION tree_flow *build_tree_return_yield_raise(compiler *cmp, source &s)
		{
			tree_return_yield_raise::Type opc;
			unsigned int l=s.token_location();

			if (s.is_keyword("return")) opc=tree_return_yield_raise::RETURN;
			else if (s.is_keyword("yield")) opc=tree_return_yield_raise::YIELD;
			else if (s.is_keyword("raise")) opc=tree_return_yield_raise::RAISE;
			else RCASSERT(0);

			tree_function *f=dynamic_cast<tree_function*>(cmp->fncowner);
			token *sact=s.actual();
			s.next();
			if ((cmp->fncowner->funcmode==tree_function::M_SCOPE_CREATOR || cmp->fncowner->funcmode==tree_function::M_CLASS_CREATOR) &&
						((opc==tree_return_yield_raise::RETURN && !s.is_eo_l()) || opc==tree_return_yield_raise::YIELD)) {

				throw error_information(owca::YERROR_UNEXPECTED_RETURN_YIELD,sact,opc==tree_return_yield_raise::RETURN ? "return" : "yield");
			}
			if (s.is_eo_l() && (opc==tree_return_yield_raise::YIELD || (!cmp->in_exception_handler.back() && opc==tree_return_yield_raise::RAISE))) throw error_information(owca::YERROR_MISSING_VALUE_FOR_YIELD_RAISE,s.actual(),opc==tree_return_yield_raise::RAISE ? "raise" : "yield");
			if (opc==tree_return_yield_raise::YIELD) {
				if (f==NULL) throw error_information(owca::YERROR_SYNTAX_ERROR,l);
				f->generator=true;
			}
			return new tree_return_yield_raise(cmp,l,opc,s.is_eo_l() ? NULL : build_tree_expression(cmp,s));
		}

		RCLMFUNCTION tree_flow *build_tree_loop_control(compiler *cmp, source &s)
		{
			tree_loop_control::Type opc;
			unsigned int l=s.token_location();
			tree_expression *oo=NULL;

			if (s.is_keyword("break")) opc=tree_loop_control::BREAK;
			else if (s.is_keyword("continue")) opc=tree_loop_control::CONTINUE;
			else if (s.is_keyword("restart")) opc=tree_loop_control::RESTART;
			else if (s.is_keyword("finally")) opc=tree_loop_control::FINALLY;
			else RCASSERT(0);

			s.next();
			std::string ident;
			if (s.is_ident()) {
				ident=s.actual()->text();
				s.next();
			}
			return new tree_loop_control(cmp,opc,l,s.token_location(),ident);
		}

		RCLMFUNCTION tree_flow *build_tree_statement(compiler *cmp, source &s)
		{
			tree_flow *b=NULL;

			try {
				if (s.is_keyword("return") || s.is_keyword("yield") || s.is_keyword("raise")) b=build_tree_return_yield_raise(cmp,s);
				else if (s.is_keyword("break") || s.is_keyword("continue") || s.is_keyword("restart") || s.is_keyword("finally")) b=build_tree_loop_control(cmp,s);
				else if (s.is_keyword("pass")) {
					b=new tree_noop(cmp,s.token_location());
					s.next();
				}
				else {
					unsigned int l=s.token_location();
					tree_expression *e=build_tree_assignment(cmp,s);
					b=new tree_eq(cmp,l,s.token_location(),e);
				}
				s.assert_eo_l();
			}
			catch(error_information &e) {
				while(s.valid() && !s.is_eo_l()) s.next();
				cmp->error(e);
			}
			if (s.is_eo_l()) s.next();
			else RCASSERT(!s.valid());

			return b;
		}

		RCLMFUNCTION tree_flow *build_tree_line(compiler *cmp, source &s)
		{
			tree_flow *b=NULL;
			std::list<token>::iterator act=s.iter_actual();
			std::string name;

			RCASSERT(s.valid());

			try {
				unsigned int l=s.token_location();

				if (s.is_keyword("if")) b=build_tree_if(cmp,s);
				else if (s.is_keyword("with")) b=build_tree_with(cmp,s);
				else if (s.is_keyword("function")) {
					tree_function *fnc=build_tree_function(cmp,s);
					tree_expression_2::Opcode opc;
					switch(fnc->functype) {
					case tree_function::FUNCTION: opc=tree_expression_2::OPCODE_ASSIGN; break;
					case tree_function::READ:
					case tree_function::WRITE: opc=tree_expression_2::OPCODE_ASSIGN_PROPERTY; break;
					default:
						RCASSERT(0);
					}
					tree_expression *e1=new tree_expression_2(cmp,l,opc,new tree_expression_ident(cmp,l,fnc->name,true),fnc);
					b=new tree_eq(cmp,l,s.token_location(),e1);
				}
				else if (is_classdef(s)) {
					tree_executable *ex=build_tree_class(cmp,s);
					tree_expression *e1=new tree_expression_2(cmp,l,tree_expression_2::OPCODE_ASSIGN,new tree_expression_ident(cmp,l,ex->name,true),ex);
					b=new tree_eq(cmp,l,s.token_location(),e1);
				}
				else if (s.is_keyword("try")) b=build_tree_try(cmp,s);
				else if (s.is_up()) throw error_information(owca::YERROR_UNEXPECTED_INDENT,s.actual());
				else if (s.is_keyword("while") || (is_name(s,name) && s.is_keyword("while"))) b=build_tree_while(cmp,s,name);
				else if (s.is_keyword("for") || (is_name(s,name) && s.is_keyword("for"))) b=build_tree_for(cmp,s,name);
				else b=build_tree_statement(cmp,s);
			}
			catch(error_information &e) {
				cmp->error(e);
				int dpth=0;

				for(;;) {
					if (act->type()==TOKEN_DOWN) {
						if (dpth==0) break;
						--dpth;
					}
					else if (act->type()==TOKEN_UP) ++dpth;
					++act;
				}
				s.iter_actual(act);
			}
			return b;
		}

		RCLMFUNCTION tree_function *build_tree(compiler *cmp, const compile_visible_items &visible_names)
		{
			cmp->in_exception_handler.clear();
			cmp->in_exception_handler.push_back(false);

			source s(cmp->tokens.begin(),--cmp->tokens.end());
			tree_function *fnc=new tree_function(cmp,s.token_location());
			cmp->fncowner=fnc;
			fnc->funcmode=tree_function::M_SCOPE_CREATOR;
			fnc->name="scope creator";
			cmp->actual_scope=fnc->scope=new tree_varspace(cmp->vm,fnc,visible_names);

			s.next();
			tree_block *bl=new tree_block(cmp,s.token_location());
			fnc->body=bl;
			while(!s.is_down()) bl->elements.push_back(build_tree_line(cmp,s));
			RCASSERT(!s.valid());

			return fnc;
		}

	}
}












