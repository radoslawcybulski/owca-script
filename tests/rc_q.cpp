// rc_q.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "base.h"
#include "rcy.h"
#include <windows.h>
#undef min
#undef max

void test(void);

#ifdef _WIN32
#ifdef _DEBUG



namespace Y { namespace __Y__ {
	//static unsigned __int64 freq;
	//static std::map<std::pair<std::string,int>,std::pair<unsigned __int64, unsigned int> > mp;

	//static unsigned __int64 timeval() {
	//	LARGE_INTEGER li;

	//	Query_performance_counter(&li);
	//	return li.Quad_part;
	//}
	//timer::timer(const char *f_, int l_) : f(f_+17),l(l_)
	//{
	//	time=timeval();
	//}
	//timer::~timer()
	//{
	//	unsigned __int64 t=timeval();
	//	mp[std::pair<std::string,int>(f,l)].first+=t-time;
	//	++mp[std::pair<std::string,int>(f,l)].second;
	//}
	//static void init_timers() {
	//	LARGE_INTEGER li;

	//	Query_performance_frequency(&li);
	//	freq=li.Quad_part;
	//}
	//static void print_timers() {
	//	RCPRINTF("%20s %5s %10s %10s %8s\n","","","total","avg","cnt");
	//	for(std::map<std::pair<std::string,int>,std::pair<unsigned __int64, unsigned int> >::iterator it=mp.begin();it!=mp.end();++it) {
	//		unsigned __int64 val=it->second.first*1000/freq;
	//		unsigned int cnt=it->second.second;
	//		RCPRINTF("%20s %5d    %3d.%03d    %3d.%03d %5d\n",
	//			it->first.first.c_str(),it->first.second,
	//			(unsigned int)(val/1000),(unsigned int)(val%1000),
	//			(unsigned int)((val/cnt)/1000),(unsigned int)((val/cnt)%1000),
	//			cnt);
	//	}
	//}
} }
#endif
#endif
static void print_function(const std::string &txt)
{
	RCPRINTF("%s",txt.c_str());
}

int main(int argc, const char *argv[])
{
	_Crt_set_dbg_flag(_CRTDBG_ALLOC_MEM_DF);
	//_Crt_set_report_mode(_CRT_ERROR,_CRTDBG_MODE_DEBUG);
	//_Crt_set_report_mode(_CRT_ASSERT,_CRTDBG_MODE_DEBUG);
	//_Crt_set_report_mode(_CRT_WARN,_CRTDBG_MODE_DEBUG);

#ifdef RCDEBUG

	RCPRINTF("starting debug run...\n");
	RCPRINTF("sizeof(y_exec_variable) = %d\n",sizeof(Y::__Y__::y_exec_variable));
#endif
//#if defined(_WIN32) && defined(_DEBUG)
//	Y::__Y__::init_timers();
//#endif


	test();



//#if defined(_WIN32) && defined(_DEBUG)
//	Y::__Y__::print_timers();
//#endif
	RCPRINTF("done\n");
	getchar();

	return 0;
}







