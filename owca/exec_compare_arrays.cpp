#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_compare_arrays.h"

namespace owca {
	namespace __owca__ {

		RCLMFUNCTION void exec_compare_arrays::next(bool order)
		{
			switch(result) {
			case START:
				if (!order && s1!=s2) {
					result=LESS;
					break;
				}
				goto start;
			case CALL_1:
				RCASSERT(ret.mode()==VAR_BOOL);
				if (ret.get_bool()) {
call_1_true:
					++index;
start:
					if (index<std::min(s1,s2)) {
						exec_variable tmp[2]={a1[index],a2[index]};
						if (vm->calculate_eq(&ret,tmp)) {
							result=CALL_1;
							break;
						}
						if (ret.get_bool()) goto call_1_true;
						goto call_1_false;
					}
					else {
						if (s1==s2) result=EQ;
						else if (s1<s2) result=LESS;
						else if (s1>s2) result=MORE;
						else RCASSERT(0);
					}
					break;
				}
				else {
call_1_false:
					exec_variable tmp[2]={a1[index],a2[index]};
					if (order && vm->calculate_less(&ret,tmp)) {
						result=CALL_2;
						break;
					}
					goto call_2;
				}
			case CALL_2:
call_2:
				if (ret.get_bool()) result=LESS;
				else result=MORE;
				break;
			case EQ:
			case LESS:
			case MORE:
				RCASSERT(0);
			}
		}

		//exec_compare_arrays::exec_compare_arrays()
		//{
		//	tmp[0].reset();
		//	tmp[1].reset();
		//}

		//void exec_compare_arrays::_mark_gc(const gc_iteration &gc) const
		//{
		//	tmp[0].gc_mark(gc);
		//	tmp[1].gc_mark(gc);
		//}

		//void exec_compare_arrays::_release_resources(virtual_machine &vm)
		//{
		//	tmp[0].gc_release(vm);
		//	tmp[1].gc_release(vm);
		//}

//		RCLMFUNCTION void exec_compare_arrays::next(bool order)
//		{
//			switch(result) {
//			case START:
//				goto start;
//			case CALL_1:
//				RCASSERT(ret.mode()==VAR_BOOL);
//				tmp[0].gc_release(*vm);
//				tmp[1].gc_release(*vm);
//				if (ret.get_bool()) {
//call_1_true:
//					++index;
//start:
//					if (index<std::min(s1,s2)) {
//						tmp[0]=a1[index];
//						tmp[1]=a2[index];
//						if (vm->calculate_eq(&ret,tmp)) {
//							tmp[0].gc_acquire();
//							tmp[1].gc_acquire();
//							result=CALL_1;
//							break;
//						}
//						if (ret.get_bool()) goto call_1_true;
//						goto call_1_false;
//					}
//					else {
//						tmp[0].reset();
//						tmp[1].reset();
//						if (s1==s2) result=EQ;
//						else if (s1<s2) result=LESS;
//						else if (s1>s2) result=MORE;
//						else RCASSERT(0);
//					}
//					break;
//				}
//				else {
//call_1_false:
//					//tmp[0]=a1[index];
//					//tmp[1]=a2[index];
//					if (order && vm->calculate_less(&ret,tmp)) {
//						result=CALL_2;
//						tmp[0].gc_acquire();
//						tmp[1].gc_acquire();
//						break;
//					}
//					goto call_2;
//				}
//			case CALL_2:
//				tmp[0].gc_release(*vm);
//				tmp[1].gc_release(*vm);
//call_2:
//				tmp[0].reset();
//				tmp[1].reset();
//				if (ret.get_bool()) result=LESS;
//				else result=MORE;
//				break;
//			case EQ:
//			case LESS:
//			case MORE:
//				RCASSERT(0);
//			}
//		}

	}
}
