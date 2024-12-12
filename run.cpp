#define _CRT_SECURE_NO_WARNINGS

#include "owca.h"
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <string>
//#include <Windows.h>

using namespace owca;

static void printfunction(const std::string &s)
{
	printf("%s",s.c_str());
}

static owca_global execute(owca_vm &vm, const owca_global &exc, const std::string &member, int param=-1)
{
	owca_global z,res;
	owca_function_return_value r=exc.get_member(z,member);
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);
	if (param>=0) {
		r=z.call(res,param);
	}
	else {
		r=z.call(res);
	}
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);
	return res;
}

static void print_exception(owca_global &exc)
{
	owca_global res=execute(*exc.vm(),exc,"code");
	exceptioncode code=(exceptioncode)res.int_get();
	res=execute(*exc.vm(),exc,"size");
	unsigned int linecount=(unsigned int)res.int_get();

	for(unsigned int i=0;i<linecount;++i) {
		owca_global e=execute(*exc.vm(),exc,"$read_1",i);

		res=execute(*exc.vm(),e,"$read_1",0);
		std::string func=res.string_get().str();
		res=execute(*exc.vm(),e,"$read_1",1);
		std::string file=res.string_get().str();
		res=execute(*exc.vm(),e,"$read_1",2);
		unsigned int line=(unsigned int)res.int_get();
		if (file.empty()) {
			printf("builtin %s",func.c_str());
		}
		else if (func.empty()) {
			printf("%s:%d:lambda function",file.c_str(),line);
		}
		else {
			printf("%s:%d:%s",file.c_str(),line,func.c_str());
		}
	}
}

int main(int argc, const char *argv[])
{
	const char *filename=argv[1];
	std::list<std::string> text;
	unsigned int totalsize=0;

	FILE *fl=fopen(filename,"rb");
	if (fl==NULL) return __LINE__;

	for(;;) {
		char buf[2048];
		buf[0]=0;
		if (!fgets(buf,2048,fl)) break;
		text.push_back(buf);
		totalsize+=text.back().size();
	}
	fclose(fl);
	std::string totaltext;

	totaltext.reserve(totalsize+1);
	while(!text.empty()) {
		totaltext+=text.front();
		text.pop_front();
	}

#if 0
	{
		unsigned int index=0;
		for(std::list<std::string>::iterator it=lines.begin();it!=lines.end();++it) {
			char buf[2048];
			sprintf(buf,"%3d: %s\n",++index,it->c_str());
			Output_debug_string_a(buf);
		}
	}
#endif

	try {
		owca::owca_vm vm;
		owca::owca_message_list ml;
		owca::owca_global res;
		owca::owca_global gnspace=vm.create_namespace("run");
		owca::owca_namespace nspace=gnspace.namespace_get();

		vm.set_print_function(printfunction);
		owca::owca_source_file_Text input(totaltext.c_str());
		owca_function_return_value r=vm.compile(res,nspace,ml,input);

		while(r.type() == owca_function_return_value::DEBUG_BREAK) {
			// $debugbreak function was called. it pauses the execution
			// and allows to inspect VM, check values and so on
			// ignore it for now
			r = vm.resume_execution();
		}

		vm.run_gc();
		switch(r.type()) {
		case owca_function_return_value::RETURN_VALUE:
			RCASSERT(0);
			return __LINE__;
		case owca_function_return_value::NO_RETURN_VALUE:
			break;
		case owca_function_return_value::EXCEPTION:
			print_exception(res);
			vm.run_gc();
			return __LINE__;
		default:
			RCASSERT(0);
		}

		if (ml.has_errors()) {
			for(owca::owca_message_list::T it=ml.begin();it!=ml.end();++it) {
				printf("%5d:      %s\n",it->line(),it->text().c_str());
			}
			printf("\n");
			return __LINE__;
		}
	}
	catch(owca_exception e) {
		printf("%s exception\n",e.message().c_str());
	}
	return 0;
}



