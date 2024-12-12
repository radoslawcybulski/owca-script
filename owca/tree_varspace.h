#ifndef _RC_Y_TREE_VARSPACE_H
#define _RC_Y_TREE_VARSPACE_H

#include "exec_base.h"
#include "exec_variablelocation.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class exec_variable_location;
		class tree_function;
		class tree_varspace;
		class tree_varspace_location;
		class virtual_machine;
		class compile_visible_items;
	}
}

namespace owca {
	namespace __owca__ {
		class tree_varspace_location {
			friend class std::map<std::string,tree_varspace_location>;
			friend class tree_varspace;
		public:
			enum {
				VARIABLE,LOOP,NONE
			} type;
			bool readonly,accessed,required,created,is_function_class;
			exec_variable_location loc;
			tree_varspace *owner;
			tree_varspace_location() : type(NONE), readonly(false), owner(NULL), created(false) { }
			tree_varspace_location(tree_varspace *owner_) : type(NONE), readonly(false), owner(owner_), accessed(false), required(false), created(false), is_function_class(false) { }
		};
		class tree_varspace {
		private:
			unsigned int depth_;
			bool is_class_;
			unsigned int stackyieldsize,maxstackyieldsize;
			unsigned int temporaryvariables,maxtemporaryvariables;
			unsigned int index1,index2;
		public:
			tree_varspace(virtual_machine &vm, tree_function *owner_, const compile_visible_items &visible_names);
			tree_varspace(tree_varspace *parent_, tree_function *owner_);

			unsigned int loopcount;
			tree_varspace *parent,*next,*children;
			tree_function *owner;
			std::map<std::string,tree_varspace_location> variables;
			std::vector<tree_varspace_location*> parameters;

			bool is_class() const { return is_class_; }
			unsigned int depth() const { return depth_; }
			tree_varspace_location *lookup_ident(const std::string &name, bool no_recursion=false);
			tree_varspace_location *create_ident(const std::string &name, unsigned int l, bool function_class, bool ignore_readonly);
			static bool readonly_ident(const std::string &ident) {
				if (ident=="true" || ident=="false" || ident=="null" || ident=="self" || ident=="class") return true;
				return !ident.empty() && ident[0]=='$';
			}
			unsigned int used_local_variables() { return (unsigned int)variables.size(); }
			unsigned int create_variable_locations(bool root=true);
			void add_stack_yield_size(unsigned int sz) {
				stackyieldsize+=sz;
				if (stackyieldsize>maxstackyieldsize) maxstackyieldsize=stackyieldsize;
			}
			void strip_stack_yield_size(unsigned int sz) {
				stackyieldsize-=sz;
			}
			unsigned int max_stack_yield_size() const { return maxstackyieldsize; }
			unsigned int max_temporary_variables() const { return maxtemporaryvariables; }
			void push_temporary_variable() { ++temporaryvariables; if (temporaryvariables>maxtemporaryvariables) maxtemporaryvariables=temporaryvariables; }
			void pop_temporary_variables(unsigned int cnt) { RCASSERT(temporaryvariables>=cnt); temporaryvariables-=cnt; }
			void clear_temporary_variables(unsigned int newcnt) { temporaryvariables=newcnt; }
			unsigned int count_temporary_variables() const { return temporaryvariables; }
		};
	}
}
#endif
