#ifndef _RC_Y_TREE_BASE_H
#define _RC_Y_TREE_BASE_H

#include "stdafx.h"
#include "debug_memory_blocks.h"
#include "location.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_base;
		class tree_executable;
		class tree_expression;
		class tree_flow;
		class tree_varspace;
	}
}

namespace owca { namespace __owca__ {
	class tree_base {
	public:
		unsigned int location;
		unsigned int size;
		compiler *cmp;
		tree_base(compiler *cmp_, unsigned int l);
		virtual ~tree_base();
		virtual unsigned int get_first_location() const=0;
		virtual unsigned int get_last_location() const=0;
#ifdef RCDEBUG_MEMORY_BLOCKS
		const unsigned int _memory_id;
		static void check_blocks(void);
#endif
	};
	class tree_expression : public tree_base {
	public:
		enum Expression_type_type {
			ET_RVALUE,
			ET_LVALUE,
			ET_LVALUE_OPER,
		};
		class expression_type {
		public:
			Expression_type_type tp;
			int opcode;

			expression_type(Expression_type_type tp_, int opcode_=0) : tp(tp_),opcode(opcode_) { }

			bool operator == (const expression_type &et) const { return tp==et.tp; }
			bool operator != (const expression_type &et) const { return tp!=et.tp; }
		};

		enum assignment_type {
			AT_NONE,
			AT_INTERNAL,
			AT_SELF,
			AT_EXTERNAL
		};
		tree_expression(compiler *cmp_, unsigned int l) : tree_base(cmp_,l) { }
		virtual void compile_names(assignment_type assigned)=0; // assigned = true if part of the tree is on the left of assignment operator ('=')
		virtual bool compile_write(opcode_writer &dst, expression_type type)=0;

		virtual bool is_tuple() const { return false; }
	};
	class tree_flow : public tree_base {
	public:
		tree_flow(compiler *cmp_, unsigned int l) : tree_base(cmp_,l) { }
		virtual bool compile_names()=0;
		virtual bool compile_write(opcode_writer &dst)=0;
	};
	class tree_executable : public tree_expression {
	public:
		std::string name;
		tree_varspace *scope;
		unsigned int location_end;
		tree_executable(compiler *cmp_, unsigned int l) : tree_expression(cmp_,l),location_end(l) { }
		~tree_executable() { }

		unsigned int get_first_location() const { return location; }
		unsigned int get_last_location() const { return location_end; }
	};
	#define TRY_(expr) do { try { expr; } catch(error_information &e) { cmp->error(e); } } while(0)
	#define TRY1(expr) do { try { expr; RCASSERT(cmp->actual_scope->count_temporary_variables()==1); } catch(error_information &e) { cmp->error(e); cmp->actual_scope->clear_temporary_variables(1); } } while(0)
} }
#endif
