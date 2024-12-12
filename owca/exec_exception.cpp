#include "stdafx.h"
#include "base.h"
#include "exec_string.h"
#include "exec_class_int.h"
#include "virtualmachine.h"
#include "exec_tuple_object.h"
#include "exec_object.h"
#include "exec_class_object.h"
#include "exec_function_ptr.h"
#include "op_base.h"
#include "returnvalue.h"
#include "exception.h"
#include "vm.h"
#include "op_execute.h"
#include "location.h"
#include "vm_execution_stack_elem_internal.h"
#include "vm_execution_stack_elem_external.h"

namespace owca {
	const char *exceptioncode_text(exceptioncode code)
	{
		switch(code) {
	#define Q(a) case a: return #a;
		Q(YEXCEPTION_NONE)
		Q(YEXCEPTION_CLASSCREATION)
		Q(YEXCEPTION_PARAMASSIGNEDTWICE)
		Q(YEXCEPTION_PARAMNOTSET)
		Q(YEXCEPTION_UNUSEDKEYWORDPARAM)
		Q(YEXCEPTION_KEYWORDPARAMNOTSTRING)
		Q(YEXCEPTION_MISSINGKEYPARAMETER)
		Q(YEXCEPTION_MISSINGVALUEPARAMETER)
		Q(YEXCEPTION_NOCONSTRUCTOR)
		Q(YEXCEPTION_TOOMANYPARAMETERS)
		Q(YEXCEPTION_NOTENOUGHPARAMETERS)
		Q(YEXCEPTION_INVALIDLISTPARAM)
		Q(YEXCEPTION_INVALIDMAPPARAM)
		Q(YEXCEPTION_INVALIDPARAMTYPE)
		Q(YEXCEPTION_INTEGEROUTOFBOUNDS)
		Q(YEXCEPTION_UNSUPPORTEDKEYWORDPARAMETERS)
		Q(YEXCEPTION_INVALIDVM)
		Q(YEXCEPTION_DIVISIONBYZERO)
		Q(YEXCEPTION_OVERFLOW)
		Q(YEXCEPTION_INVALIDIDENT)
		Q(YEXCEPTION_NOTLVALUE)
		Q(YEXCEPTION_NOTRVALUE)
		Q(YEXCEPTION_MISSINGMEMBER)
		Q(YEXCEPTION_TOOMANYITEMSINITER)
		Q(YEXCEPTION_TOOLITTLEITEMSINITER)
		Q(YEXCEPTION_INVALIDRETURNTYPE)
		Q(YEXCEPTION_INVALIDOPERATORFUNCTION)
		Q(YEXCEPTION_MISSINGRETURNVALUE)
		Q(YEXCEPTION_STACKOVERFLOW)
		Q(YEXCEPTION_CANTINSERT)
		Q(YEXCEPTION_KEYNOTFOUND)
		Q(YEXCEPTION_NOCOROUTINETOSTOP)
		Q(YEXCEPTION_CANTSTOPFROMWITHINUSERFUNCTION)
		Q(YEXCEPTION_CANTRESUMEFROMCOROUTINE)
		Q(YEXCEPTION_CANTRESUMENORMALFUNCTION)
		Q(YEXCEPTION_CANTRESUMEFINISHEDCOROUTINE)
		Q(YEXCEPTION_CANTSTOPCOROUTINEFROMUSERFUNCTION)
		Q(YEXCEPTION_CANTCREATEGENERATORFROMUSERFUNCTION)
		Q(YEXCEPTION_LISTMODIFEDWHILEBEINGSORTED)
		Q(YEXCEPTION_MAPMODIFEDWHILEBEINGUSED)
		Q(YEXCEPTION_SETMODIFEDWHILEBEINGUSED)
		Q(YEXCEPTION_USER)
	#undef Q
		default:
			RCASSERT(0);
			return "";
		}
	}

	namespace __owca__ {

		struct stackelem {
			owca_location location;
			std::string funcname;
		};

		struct exceptiontype : public object_base {
			std::vector<stackelem> st;
			owca_internal_string *txt;
			exceptioncode code;

			exceptiontype(virtual_machine &vm, unsigned int oversize) {
				RCASSERT(oversize==0);
				txt=vm.allocate_string();
			}

			void _destroy(virtual_machine &vm) {
				txt->gc_release(vm);
			}
			void _marker(const gc_iteration &gc) const {
				gc_iteration::debug_info _d("exceptiontype %x: txt object",this);
				txt->gc_mark(gc);
			}

			void get(virtual_machine &vm, exec_variable &dst, unsigned int index) {
				RCASSERT(index<st.size());
				exec_tuple_object *t;
				exec_object *o=vm.allocate_tuple(t,3);
				dst.set_object(o);
				t->get(0).set_string(vm.allocate_string(st[(unsigned int)index].funcname));
				t->get(1).set_string(vm.allocate_string(st[(unsigned int)index].location.filename() ? st[(unsigned int)index].location.filename() : ""));
				t->get(2).set_int(st[(unsigned int)index].location.line());
			}
		};
	}

	namespace __owca__ {
		static void exception_init_set(stackelem &e, exec_object *classname, const std::string &funcname, const owca_location *l)
		{
			if (l) e.location=*l;
			e.funcname=(classname ? classname->CO().name->str()+"." : "")+funcname;
		}

		D_SELF2(exception,init,exceptiontype*,owca_internal_string*,owca_int)
			{
				self->txt->gc_release(*vm);

				std::vector<stackelem> &st=self->st;

#ifdef RCDEBUG
				RCPRINTF("%s\n",p1->str().c_str());
#endif

				self->code=(exceptioncode)p2;
				self->txt=p1;
				p1->gc_acquire();

				vm_execution_stack *stack=vm->execution_stack;
				int count=0;
				do {
					count+=stack->count();
				} while((stack=stack->prev())!=NULL);
				count-=1;
				RCASSERT(count>=0);
				st.reserve(count);
				// skip 2 first frames, connected to initialization of exception object
				// TODO: this is wrong?
				stack=vm->execution_stack;
				count=-1;
				do {
					for(int i=(int)stack->count()-1;i>=0;--i,++count) {
						if (count>=0) {
							vm_execution_stack_elem_base *frame=stack->peek_frame_indexed(i);

							if (frame->show_in_exception_stack()) {
								if (vm_execution_stack_elem_internal *inter=dynamic_cast<vm_execution_stack_elem_internal*>(frame)) {
									std::string func;
									exec_object *cn=NULL;

									if (frame->fnc && frame->fnc->name() && !frame->fnc->name()->empty()) {
										func=frame->fnc->name()->str();
										cn=frame->fnc->classobject();
									}
									else if (frame->fnc) func="unnamed function";
									else func="<code>";

									owca_location loc=inter->actual_location();
									st.push_back(stackelem());
									exception_init_set(st.back(),cn,func,&loc);
								}
								else {
									st.push_back(stackelem());
									exception_init_set(st.back(),frame->fnc->classobject(),frame->fnc->name()->str(),NULL);
								}
							}
						}
					}
				} while((stack=stack->prev())!=NULL);
				//RCASSERT(count==st.size());
				//std::reverse(st.begin(),st.end());

#ifdef RCDEBUG
				for(unsigned int i=0;i<st.size();++i) {
					owca_location &l=st[i].location;
					std::string loc=l.filename() && l.filename()[0] ? std::string(l.filename())+": "+int_to_string(l.line()) : "<built-in>";
					if (loc.size()<30) loc=std::string(30-loc.size(),' ')+loc;
					RCPRINTF("%s\n",(loc+": "+st[i].funcname).c_str());
				}
#endif

				return executionstackreturnvalue::RETURN_NO_VALUE;
			}
		D_END

		D_SELF0(exception,size,exceptiontype*)
			{
				return_value->set_int(self->st.size());
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF1(exception,read_1,exceptiontype*,owca_int)
			{
				if (p1<-(owca_int)self->st.size() || p1>=(owca_int)self->st.size()) {
					vm->raise_invalid_integer(OWCA_ERROR_FORMAT2("index %1 is invalid for stack depth of size %2",int_to_string(p1),int_to_string((owca_int)self->st.size())));
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				if (p1<0) p1+=(owca_int)self->st.size();
				self->get(*vm,*return_value,(unsigned int)p1);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF0(exception,text,exceptiontype*)
			{
				return_value->set_string(self->txt);
				self->txt->gc_acquire();
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF0(exception,code,exceptiontype*)
			{
				return_value->set_int(self->code);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF0(exception,gen,exceptiontype*)
			{
				if (mode==0) {
					index=0;
					mode=1;
					return executionstackreturnvalue::CREATE_GENERATOR;
				}
				if (index<self->st.size()) {
					self->get(*vm,*return_value,index++);
					return executionstackreturnvalue::RETURN;
				}
				return executionstackreturnvalue::RETURN_NO_VALUE;
			}

			unsigned int index;
			unsigned char mode;
			void create_self(void) { mode=0; }
		D_END

		D_SELF1(exception,eq,exceptiontype*,exceptiontype*)
			{
				return_value->set_bool(self==p1);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		D_SELF1(exception,noteq,exceptiontype*,exceptiontype*)
			{
				return_value->set_bool(self!=p1);
				return executionstackreturnvalue::RETURN;
			}
		D_END

		void virtual_machine::initialize_exception(internal_class &c)
		{
			c._setstructure<exceptiontype>();
			M_OPER2(c,exception,init,(*this,"$init","text","code","",YEXCEPTION_USER));
			M_OPER0(c,exception,gen,(*this,"$gen"));
			M_OPER1(c,exception,eq,(*this,"$eq","other"));
			M_OPER1(c,exception,noteq,(*this,"$noteq","other"));
			M_OPER1(c,exception,read_1,(*this,"$read_1","index"));

			M_FUNC0(c,exception,size,(*this,"size"));
			M_FUNC0(c,exception,text,(*this,"text"));
			M_FUNC0(c,exception,code,(*this,"code"));
		}

		void virtual_machine::initialize_exception_std(internal_class &c)
		{
			c._add_inherit(c.vm.class_exception);
		}
	}
}












