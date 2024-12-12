#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_sort_array.h"

namespace owca {
	namespace __owca__ {

		bool exec_sort_array::create(virtual_machine *vm_, exec_sort_array_getter_base &getter, unsigned int size_)
		{
			if (size_<=1) return true;

			vm=vm_;
			size=size_;
			unsigned char *data=new unsigned char[sizeof(elem)*size+sizeof(recursiveinfo)*(size+2)];
			sortarray=(elem*)data;
			recarray=(recursiveinfo*)(data+sizeof(elem)*size);
			recmax=recarray+size+2;
			recact=recarray-1;

			for(unsigned int i=0;i<size;++i) {
				sortarray[i].ptr=&getter.next(sortarray[i].hash);
				sortarray[i].next=sortarray+i+1;
			}
			sortarray[size-1].next=NULL;
			root=sortarray;
			RCASSERT(push_new_sort_array(&root,NULL));
			mode=MODE_COMPARE;
			prepare_compare();
			return false;
		}

		unsigned char exec_sort_array::push_new_sort_array(elem **array, elem **last_ptr)
		{
			if ((*array) && (*array)->next) {
				++recact;

				recact->array=array;
				recact->pivot=*array;
				*array=(*array)->next;
				recact->last_ptr=last_ptr;

				//{
				//	char buf[2048];
				//	sprintf(buf,"new subarray %d\n",(unsigned int)(recact-recarray));
				//	elem *e;
				//	char *b=buf;
				//	e=*array;
				//	while(e) {
				//		b+=sprintf(b,"%3d ",(unsigned int)e->ptr->get_int());
				//		e=e->next;
				//	}
				//	debugprint("    data  ");
				//	debugprint(buf);
				//	debugprint("\n");
				//}

#ifdef RCDEBUG
				{
					unsigned int cnt=1;
					elem *e=*array;
					while(e) {
						++cnt;
						e=e->next;
					}
					recact->count=cnt;
				}
#endif
				recact->left=recact->right=NULL;
				return 1;
			}
			else {
				*last_ptr=*array;
			}
			return 0;
		}

		void exec_sort_array::prepare_compare()
		{
			left_last=right_last=NULL;
		}

		RCLMFUNCTION bool exec_sort_array::next()
		{
			//char buf[2048];

			switch(mode) {
			case MODE_COMPARE: {
mode_compare:
				RCASSERT(recact>=recarray);
				RCASSERT((*recact->array)!=NULL);
				if ((*recact->array)->hash==recact->pivot->hash) {
					exec_variable tmp[2]={*(*recact->array)->ptr,*recact->pivot->ptr};
					if (vm->calculate_less(&ret,tmp)) {
						mode=MODE_DECIDE;
						return true;
					}
				}
				else {
					if ((*recact->array)->hash<recact->pivot->hash) goto decide_less;
					goto decide_moreeq;
				} }
			case MODE_DECIDE: {
				if (ret.get_bool()) {
decide_less:
					if (left_last==NULL) recact->left=*recact->array;
					else {
						left_last->next=*recact->array;
					}
					left_last=*recact->array;
				}
				else {
decide_moreeq:
					if (right_last==NULL) recact->right=*recact->array;
					else {
						right_last->next=*recact->array;
					}
					right_last=*recact->array;
				}
				elem *nxt=(*recact->array)->next;
				(*recact->array)->next=NULL;
				if (nxt==NULL) {
					recursiveinfo *ra=recact;
					unsigned char c1=push_new_sort_array(&ra->left,&ra->left_last);
					unsigned char c2=push_new_sort_array(&ra->right,&ra->right_last);
					if (c1 | c2) {
						prepare_compare();
						goto mode_compare;
					}

merge_arrays:
					//sprintf(buf,"merging %d\n",(unsigned int)(recact-recarray));
					//debugprint(buf);
					//
					//{
					//	elem *e;
					//	char *b=buf;
					//	e=recact->left;
					//	*b=0;
					//	while(e) {
					//		b+=sprintf(b,"%3d ",(unsigned int)e->ptr->get_int());
					//		e=e->next;
					//	}
					//	debugprint("    left  ");
					//	debugprint(buf);
					//	debugprint("\n");

					//	sprintf(buf,"    pivot %3d\n",(unsigned int)recact->pivot->ptr->get_int());
					//	debugprint(buf);

					//	b=buf;
					//	e=recact->right;
					//	*b=0;
					//	while(e) {
					//		b+=sprintf(b,"%3d ",(unsigned int)e->ptr->get_int());
					//		e=e->next;
					//	}
					//	debugprint("    right ");
					//	debugprint(buf);
					//	debugprint("\n");
					//}

					*recact->array=NULL;
					if (recact->left) {
						*recact->array=recact->left;
						recact->left_last->next=recact->pivot;
					}
					else *recact->array=recact->pivot;
					recact->pivot->next=recact->right;
					if (recact->last_ptr) {
						*recact->last_ptr=recact->right ? recact->right_last : recact->pivot;
					}
#ifdef RCDEBUG
					{
						unsigned int cnt=0;
						elem *e=*recact->array;
						while(e) {
							++cnt;
							e=e->next;
						}
						RCASSERT(cnt==recact->count);
					}
#endif

					--recact;

					if (recact<recarray) {
						// done

						//elem *e=*recarray->array;
						//std::vector<exec_variable> tmp;
						//tmp.reserve(size);
						//while(e) {
						//	tmp.push_back(*e->ptr);
						//	e=e->next;
						//}
						//RCASSERT(tmp.size()==size);
						//for(unsigned int i=0;i<size;++i) realarray[i]=tmp[i];
						return false;
					}

					//sprintf(buf,"trying index %d\n",(unsigned int)(recact-recarray));
					//debugprint(buf);

					if (recact->left || recact->right) goto merge_arrays;
					prepare_compare();
					goto mode_compare;
				}
				else (*recact->array)=nxt;
				goto mode_compare; }
			default:
				RCASSERT(0);
				return false;
			}
		}

		exec_sort_array_result exec_sort_array::result()
		{
			return exec_sort_array_result(sortarray,*recarray->array);
		}
	}
}

