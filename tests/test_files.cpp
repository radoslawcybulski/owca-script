#define DLLEXPORT
#define _CRT_SECURE_NO_WARNINGS

#include "owca\base.h"
#include "owca\tree_base.h"
#include <stdlib.h>
#include <stdio.h>

#define printf printf

#undef min
#undef max

#include "owca.h"
//#include "message.h"
//#include "exception.h"

using namespace owca;

static bool cmp(const char *s1, const char *s2)
{
	unsigned int i;
	for(i=0;s2[i]!=0;++i) if (s1[i]!=s2[i]) return false;
	return true;
}

enum testresult {
	TR_NONE,TR_VALUE,TR_ERROR,TR_EXCEPTION_CODE,TR_EXCEPTION_CREATE
};

struct tr_error_elem {
	unsigned int line;
	owca_message_type type;
	std::string type_name;
	bool used;

	tr_error_elem() : used(false) { }
};

static ExceptionCode find_exception_by_text(const std::string &s)
{
#define Q(a) if (s== #a) return ExceptionCode::a;
	Q(NONE)
	Q(CLASS_CREATION)
	Q(PARAM_ASSIGNED_TWICE)
	Q(PARAM_NOT_SET)
	Q(UNUSED_KEYWORD_PARAM)
	Q(KEYWORD_PARAM_NOT_STRING)
	Q(MISSING_KEY_PARAMETER)
	Q(MISSING_VALUE_PARAMETER)
	Q(NO_CONSTRUCTOR)
	Q(TOO_MANY_PARAMETERS)
	Q(NOT_ENOUGH_PARAMETERS)
	Q(INVALID_LIST_PARAM)
	Q(INVALID_MAP_PARAM)
	Q(INVALID_PARAM_TYPE)
	Q(INTEGER_OUT_OF_BOUNDS)
	Q(UNSUPPORTED_KEYWORD_PARAMETERS)
	Q(INVALID_VM)
	Q(DIVISION_BY_ZERO)
	Q(OVERFLOW)
	Q(INVALID_IDENT)
	Q(NOT_LVALUE)
	Q(NOT_RVALUE)
	Q(MISSING_MEMBER)
	Q(TOO_MANY_ITEMS_IN_ITER)
	Q(TOO_LITTLE_ITEMS_IN_ITER)
	Q(INVALID_RETURN_TYPE)
	Q(INVALID_OPERATOR_FUNCTION)
	Q(MISSING_RETURN_VALUE)
	Q(STACK_OVERFLOW)
	Q(CANT_INSERT)
	Q(KEY_NOT_FOUND)
	Q(NO_COROUTINE_TO_STOP)
	Q(CANT_STOP_FROM_WITHIN_USER_FUNCTION)
	Q(CANT_RESUME_FROM_COROUTINE)
	Q(CANT_RESUME_NORMAL_FUNCTION)
	Q(CANT_RESUME_FINISHED_COROUTINE)
	Q(CANT_STOP_COROUTINE_FROM_USER_FUNCTION)
	Q(CANT_CREATE_GENERATOR_FROM_USER_FUNCTION)
	Q(LIST_MODIFED_WHILE_BEING_SORTED)
	Q(MAP_MODIFED_WHILE_BEING_USED)
	Q(SET_MODIFED_WHILE_BEING_USED)
	Q(USER)
	RCASSERT(0);
	return (ExceptionCode)0;
#undef Q
}

static owca_message_type find_error_by_text(const std::string &s)
{
#define Q(a) if (s== #a) return a;
	Q(YERROR_SYNTAX_ERROR)
	Q(YERROR_NO_EXCEPT_CLAUSE)
	Q(YERROR_UNEXPECTED_INDENT)
	Q(YERROR_UNEXPECTED_RETURN_YIELD)
	Q(YERROR_UNEXPECTED_RETURN_VALUE)
	Q(YERROR_MISSING_VALUE_FOR_YIELD_RAISE)
	Q(YERROR_INCORRECT_INDENT)
	Q(YERROR_MIXED_CHARS_IN_INDENT)
	Q(YERROR_UNFINISHED_STRING_CONSTANT)
	Q(YERROR_WRONG_FORMAT_OF_NUMBER)
	Q(YERROR_UNEXPECTED_CHARACTER)
	Q(YERROR_UNEXPECTED_END_OF_FILE)
	Q(YERROR_NOT_LVALUE)
	Q(YERROR_NOT_RVALUE)
	Q(YERROR_ALREADY_DEFINED)
	Q(YERROR_UNDEFINED)
	Q(YERROR_INVALID_IDENT)
	Q(YERROR_UNMATCHED_BRACKET)
	Q(YERROR_UNEXPECTED_BREAK_CONTINUE_FINALLY_RESTART)
	Q(YERROR_KEYWORD_PARAM_USED_TWICE)
	Q(YERROR_VARIABLE_IS_LOCAL)
	Q(YERROR_MEANING_OF_VARIABLE_HAS_CHANGED)
	Q(YERROR_INTERNAL)
#undef Q
	RCASSERT(0);
	return (owca_message_type)0;
}

static tr_error_elem parse_error_decl(const char *buf)
{
	unsigned int err_line;
	char err_text[64];
	char cc=0;

	int cnt=sscanf(buf+9,"%d %63s %c",&err_line,err_text,&cc);
	RCASSERT(cnt==2);
	tr_error_elem t;
	t.line=err_line;
	t.type=find_error_by_text(err_text);
	t.type_name=err_text;
	return t;
}

static unsigned int parse_exception(const owca_global &exc, ExceptionCode type, const std::string &name, unsigned int lines[10], unsigned int lines_count)
{
	ExceptionCode code=(ExceptionCode)exc.get_member("code").call().int_get();
	auto linecount = exc.get_member("size").call().int_get();

	if (code!=type) {
		return __LINE__;
	}
	if (linecount!=lines_count) {
		return __LINE__;
	}

	std::vector<unsigned int> exclines(lines_count);

	for(unsigned int i=0;i<lines_count;++i) {
		auto e = exc.get_member("$read_1").call(i);
		RCASSERT(e.get_member("$read_1").call(1).string_get()=="main.ys");
		exclines[i]=(unsigned int)e.get_member("$read_1").call(2).int_get();
	}
	for(unsigned int i=0;i<lines_count;++i) {
		if (exclines[i]!=lines[i]) {
			return __LINE__;
		}
	}

	return 0;
}

static void test_files_printfunction(const std::string &s)
{
	printf("%s",s.c_str());
}

static std::list<std::string> lines;

#ifdef _WIN32
#include <windows.h>

LONG WINAPI global_exception_filter(EXCEPTION_POINTERS *)
{
    ExitProcess(0xdedd000D);
}
#endif

static unsigned int test_file(const char *filename)
{
	static char test_file_text[1000000];
	char buf[2048];
	char *ptr;
	std::string tr_value;
	std::vector<tr_error_elem> tr_error;
	ExceptionCode tr_exception_type;
	std::string tr_exception_name;
	unsigned int tr_exception_lines[10],tr_exception_lines_count;
	testresult tr=TR_NONE;

	printf("testing file %s\n",filename);
#if defined(_WIN32) && defined(RCDEBUGSTUDIO)
	Output_debug_string_a("testing file ");
	Output_debug_string_a(filename);
	Output_debug_string_a("\n");
#endif

	FILE *fl=fopen(filename,"rb");
	if (fl==NULL) return __LINE__;
	
	bool code=true;
	ptr=test_file_text;
	for(;;) {
		buf[0]=0;
		if (!fgets(buf,2048,fl)) break;
		{
			char *ptr=buf;
			while(*ptr) ++ptr;
			while(ptr>buf && (*(ptr-1)==' ' || *(ptr-1)=='\r' || *(ptr-1)=='\n')) --ptr;
			*ptr=0;
			lines.push_back(buf);
		}
		if (buf[0]=='+' && buf[1]=='+' && buf[2]=='+') {
			*ptr=0;
			code=false;
			if (cmp(buf,"+++value ")) {
				RCASSERT(tr==TR_NONE);
				const char *c=buf+9;
				while(*c>0 && *c<=' ') ++c;
				RCASSERT(*c=='\'');
				const char *begin=c+1;
				++c;
				while(*c!=0 && *c!='\'') ++c;
				RCASSERT(*c=='\'');
				tr_value=std::string(begin,(unsigned int)(c-begin));
				tr=TR_VALUE;
			}
			else if (cmp(buf,"+++error ")) {
				RCASSERT(tr==TR_NONE || tr==TR_ERROR);
				tr=TR_ERROR;
				tr_error.push_back(parse_error_decl(buf));
			}
			else if (cmp(buf,"+++exception ")) {
				RCASSERT(tr==TR_NONE);
				tr=TR_EXCEPTION_CODE;
				char exc_text[64];
				unsigned int lines[10];
				char cc=0;

				int cnt=sscanf(buf+13,"%63s %d %d %d %d %d %d %d %d %d %d %c",exc_text,lines+0,lines+1,lines+2,lines+3,lines+4,lines+5,lines+6,lines+7,lines+8,lines+9,&cc);
				RCASSERT(cnt>=2 && cnt<=11);

				tr_exception_type=find_exception_by_text(exc_text);
				tr_exception_name=exc_text;
				for(int i=0;i<cnt-1;++i) tr_exception_lines[i]=lines[i];
				tr_exception_lines_count=cnt-1;
			}
			else if (cmp(buf,"+++compilation_exception ")) {
				// compilation_exception 11 INVALID_PARAM_TYPE 2
				// 22
				tr=TR_EXCEPTION_CREATE;
				char exc_text[64];
				unsigned int lines[10];
				char cc=0;

				int cnt=sscanf(buf+25,"%63s %d %d %d %d %d %d %d %d %d %d %c",exc_text,lines+0,lines+1,lines+2,lines+3,lines+4,lines+5,lines+6,lines+7,lines+8,lines+9,&cc);
				RCASSERT(cnt>=2 && cnt<=11);

				tr_exception_type=find_exception_by_text(exc_text);
				tr_exception_name=exc_text;
				for(int i=0;i<cnt-1;++i) tr_exception_lines[i]=lines[i];
				tr_exception_lines_count=cnt-1;
			}
			else {
				RCASSERT(0);
			}
		}
		else {
			unsigned int i;

			RCASSERT(code);
			for(i=0;buf[i]!=0;++i) ptr[i]=buf[i];
			ptr+=i;
			*ptr='\n';
			++ptr;
		}
	}
	RCASSERT(tr!=TR_NONE);

	fclose(fl);

#if defined(_WIN32) && defined(RCDEBUGSTUDIO)
	{
		unsigned int index=0;
		for(std::list<std::string>::iterator it=lines.begin();it!=lines.end();++it) {
			char buf[2048];
			sprintf(buf,"%3d: %s\n",++index,it->c_str());
			Output_debug_string_a(buf);
		}
	}
#endif

	if (tr!=TR_NONE) {
		owca::owca_vm vm;
		owca::owca_message_list ml;
		bool success=false;
		owca::owca_global gnspace=vm.create_namespace("main.ys");
		owca::owca_namespace nspace=gnspace.namespace_get();

		vm.set_print_function(test_files_printfunction);
		
		try {
			vm.compile(nspace, ml, owca::owca_source_file_Text(test_file_text));
			vm.run_gc();
		}
		catch (owca_exception& oe) {
			if (!oe.has_exception_object()) return __LINE__;
			if (tr!=TR_EXCEPTION_CODE) return __LINE__;
			return parse_exception(oe.exception_object(), tr_exception_type, tr_exception_name, tr_exception_lines, tr_exception_lines_count);
		}
		catch (std::exception& e) {
			printf("got exception: %s\n", e.what());
			return __LINE__;
		}

		if (ml.has_errors()) {
			if (tr!=TR_ERROR) {
				return __LINE__;
			}

			for(owca::owca_message_list::T it=ml.begin();it!=ml.end();++it) if (it->is_error()) {
				unsigned int i;
				for(i=0;i<tr_error.size();++i) if (!tr_error[i].used && tr_error[i].line==it->line() && tr_error[i].type==it->type()) {
					tr_error[i].used=true;
					break;
				}
				if (i>=tr_error.size()) {
					goto print_errors;
				}
			}
			for(unsigned int i=0;i<tr_error.size();++i) if (!tr_error[i].used) {
print_errors:
				for(owca::owca_message_list::T it=ml.begin();it!=ml.end();++it) {
					printf("%5d:      %s\n",it->line(),it->text().c_str());
				}
				printf("\n");
				return __LINE__;
			}
		}
		else {
			if (tr==TR_ERROR || tr==TR_EXCEPTION_CREATE) {
				return __LINE__;
			}
			for(owca::owca_message_list::T it=ml.begin();it!=ml.end();++it) {
				RCASSERT(!it->is_error());
			}
			try {
				owca::owca_global f=nspace["main"];
				RCASSERT(f.function_is());
				auto res = f.call();
				vm.run_gc();
				f = owca::owca_global();
				vm.run_gc();

				if (!res.string_is()) return __LINE__;
				if (tr != TR_VALUE) return __LINE__;
				if (res.string_get() != tr_value) return __LINE__;
			}
			catch (owca_exception& oe) {
				if (!oe.has_exception_object()) return __LINE__;
				if (tr!=TR_EXCEPTION_CODE) return __LINE__;
				unsigned int v=parse_exception(oe.exception_object(), tr_exception_type, tr_exception_name, tr_exception_lines, tr_exception_lines_count);
				if (v!=0) {
					return v;
				}
			}
			catch(std::exception &e) {
				printf("got exception: %s\n", e.what());
				return __LINE__;
			}
		}

		vm.run_gc();
		nspace.clear();
		vm.run_gc();
	}

	return 0;
}

int main(int argc, const char **argv)
{
#ifdef _WIN32
	SetUnhandledExceptionFilter(global_exception_filter);
#endif
//#ifdef RCDEBUG_DEBUG_STRINGS
//	printf("flag RCDEBUG_DEBUG_STRINGS used\n");
//#endif 
//#ifdef RCDEBUG_MEMORY_BLOCKS
//	printf("flag RCDEBUG_MEMORY_BLOCKS used\n");
//#endif 
//#ifdef RCDEBUG_GC_TEST
//	printf("flag RCDEBUG_GC_TEST used\n");
//#endif 
//#ifdef RCDEBUG_GC
//	printf("flag RCDEBUG_GC used\n");
//#endif 
//#ifdef RCDEBUG_ASSERT
//	printf("flag RCDEBUG_ASSERT used\n");
//#endif
#ifndef RCDEBUG_GC
	RCASSERT(0);
#endif 
	if (argc==2) {
		const char *file=argv[1];
		unsigned int line=test_file(file);

#ifdef RCDEBUG_MEMORY_BLOCKS
		owca::__owca__::tree_base::check_blocks();
		owca::__owca__::exec_base_id::check_blocks();
#endif

#if defined(_WIN32) && defined(RCDEBUGSTUDIO)
		if (line!=0) {
			line=line;
		}
#endif
		return line;
	}
	return __LINE__;
}
