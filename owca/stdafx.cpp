// stdafx.cpp : source file that includes just the standard includes
// rc_q.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "base.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#if defined WIN32 || defined WIN64
#include <windows.h>
#endif

#ifdef RCDEBUG_TIME_IT

#undef min
#undef max


namespace owca {
	namespace __owca__ {
		static timer *roottimer=NULL;
		static unsigned __int64 freq;

		static unsigned __int64 timeval() {
			LARGE_INTEGER li;

			Query_performance_counter(&li);
			return li.Quad_part;
		}
		timer::timer(const std::string &ident_) : ident(ident_),value(0),count(0)
		{
			if (roottimer==NULL) {
				LARGE_INTEGER li;

				Query_performance_frequency(&li);
				freq=li.Quad_part;
			}

			next=roottimer;
			roottimer=this;
		}

		void timer::print_timers()
		{
			timer *t=roottimer;
			std::map<std::string,timer*> mp;

			while(t) {
				mp[t->ident]=t;
				t=t->next;
			}

			for(std::map<std::string,timer*>::iterator it=mp.begin();it!=mp.end();++it) {
				timer *t=it->second;
				unsigned int v1=(unsigned int)(t->value/freq);
				unsigned int v2=(unsigned int)(((t->value%freq)*1000000)/freq);
				RCPRINTF("timer %15s %5d.%06d %4d\n",it->first.c_str(),v1,v2,t->count);
			}
		}

		timeit::timeit(timer &tt) : t(tt)
		{
			tt.value-=timeval();
		}

		timeit::~timeit()
		{
			t.value+=timeval();
			t.count++;
		}
	}
}

#endif

#if defined _WIN32 || defined _WIN64
#include <Windows.h>
#endif

void debugprint(const char *txt)
{
#if defined _WIN32 || defined _WIN64
	OutputDebugStringA(txt);
#else
	printf("%s", txt);
#endif
}
