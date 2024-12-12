#include "stdafx.h"
#include "base.h"
#include "tree_varspace.h"
#include "tree_function.h"
#include "exec_base.h"
#include "exec_string.h"
#include "namespace.h"
#include "virtualmachine.h"
#include "message.h"
#include "cmp_base.h"

namespace owca { namespace __owca__ {

	tree_varspace::tree_varspace(virtual_machine &vm, tree_function *owner_, const compile_visible_items &visible_names) : depth_(0),owner(owner_),is_class_(false),children(NULL),parent(NULL),next(NULL),loopcount(0),temporaryvariables(0),maxtemporaryvariables(0)
	{
		stackyieldsize=maxstackyieldsize=0;
		const char *name;
		while((name=visible_names.builtinget()) != NULL) {
			if (variables.find(name)==variables.end()) {
				tree_varspace_location *tl=create_ident(name,0,false,true);
				tl->type=tree_varspace_location::VARIABLE;
				tl->required=true;
				tl->accessed=false;
				tl->loc=exec_variable_location(0x1234,0x1234);
			}
		}
		unsigned int size=0xffffffff;
		while((name=visible_names.get(size)) != NULL) {
			std::string z=size==0xffffffff ? name : std::string(name,size);
			if (variables.find(name)==variables.end()) {
				tree_varspace_location *tl=create_ident(name,0,false,true);
				tl->type=tree_varspace_location::VARIABLE;
				tl->required=true;
				tl->accessed=false;
				tl->loc=exec_variable_location(0x1234,0x1234);
			}
			size=0xffffffff;
		}
	}

	tree_varspace::tree_varspace(tree_varspace *parent_, tree_function *owner_) : parent(parent_),depth_(parent_->depth_+1),owner(owner_),
						is_class_(owner_->funcmode==tree_function::M_CLASS_CREATOR),children(NULL),loopcount(0),temporaryvariables(0),maxtemporaryvariables(0)
	{
		stackyieldsize=maxstackyieldsize=0;
		next=parent->children;
		parent->children=this;
	}

	unsigned int tree_varspace::create_variable_locations(bool root)
	{
		tree_varspace *p=children;
		while(p) {
			p->create_variable_locations(false);
			p=p->next;
		}

		unsigned int index=0;
		if (root) {
			RCASSERT(next==NULL);
			RCASSERT(parameters.empty());
			for(std::map<std::string,tree_varspace_location>::iterator it=variables.begin();it!=variables.end();++it) {
				if (it->second.accessed && it->second.required) {
					it->second.loc=exec_variable_location(index,depth_);
					++index;
				}
			}
			for(std::map<std::string,tree_varspace_location>::iterator it=variables.begin();it!=variables.end();++it) {
				if (it->second.accessed && !it->second.required) {
					it->second.loc=exec_variable_location(index,depth_);
					++index;
				}
			}
			//for(std::map<std::string,tree_varspace_location>::iterator it=variables.begin();it!=variables.end();++it) {
			//	if (!it->second.accessed) {
			//		it->second.loc=exec_variable_location(index,depth_);
			//		++index;
			//	}
			//}
		}
		else {
			for(unsigned int i=0;i<parameters.size();++i) {
				parameters[i]->loc=exec_variable_location(index++,depth_);
				parameters[i]->created=true;
			}
			for(std::map<std::string,tree_varspace_location>::iterator it=variables.begin();it!=variables.end();++it) {
				if (!it->second.created) it->second.loc=exec_variable_location(index++,depth_);
			}
		}
		return index;
	}

	tree_varspace_location *tree_varspace::create_ident(const std::string &name, unsigned int l, bool function_class, bool ignore_readonly)
	{
		//RCASSERT(name.value()!=0);
		RCASSERT(!ignore_readonly || !function_class);
		RCASSERT(!name.empty());
		bool read_only_ident=readonly_ident(name);

		if (!ignore_readonly && !function_class && read_only_ident) {
			throw error_information(owca::YERROR_NOT_LVALUE,l);
		}

		std::map<std::string,tree_varspace_location>::iterator it=variables.find(name);
		if (it!=variables.end()) {
			RCASSERT(!ignore_readonly);
			if (!it->second.accessed) {
				it->second.accessed=true;
			}
			if (read_only_ident && (!is_class_ || !it->second.is_function_class)) { // function_class is true here if readonly_ident is true
				throw error_information(owca::YERROR_NOT_LVALUE,l);
			}
			if (!function_class && it->second.readonly) {
				it->second.readonly=false;
				throw error_information(owca::YERROR_NOT_LVALUE,l);
			}
			it->second.is_function_class=function_class;
			return &it->second;
		}

		tree_varspace_location *tl=&(variables[name]=tree_varspace_location(this));
		tl->accessed=true;
		tl->readonly = read_only_ident;
		tl->is_function_class=function_class;
		tl->loc=exec_variable_location(0x1234,0x1234);
		return tl;
	}

	tree_varspace_location *tree_varspace::lookup_ident(const std::string &name, bool no_recursion)
	{
		RCASSERT(!name.empty());

		tree_varspace *act=this;

		while(act) {
			if (act==this || act->owner->funcmode!=tree_function::M_CLASS_CREATOR) {
				std::map<std::string,tree_varspace_location>::iterator it=act->variables.find(name);
				if (it!=act->variables.end()) {
					if (!it->second.accessed) {
						it->second.accessed=true;
					}
					return &it->second;
				}
			}
			if (no_recursion) break;
			act=act->parent;
		}
		return NULL;
	}

} }












