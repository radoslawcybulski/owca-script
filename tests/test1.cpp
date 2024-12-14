#define DLLEXPORT

#include "owca/base.h"
#include "owca.h"
#include "owca/exec_base.h"
#include "owca/debug_time_it.h"
#include <windows.h>
#undef min
#undef max

using namespace owca;

class Test {
public:
	enum {
		COMPILATION_ERROR,
		COMPILATION_ERROR2,
		COMPILATION_EXCEPTION_ERROR,
		EXCEPTION_ERROR,
		//VALUE_INT,
		//VALUE_REAL,
		VALUE_STRING,
		//VALUE_BOOL,
		//VALUE_NULL
	} restype;
	union {
		owca_int i;
		owca_real r;
		const char *s;
		bool b;
	} res_value;
	//std::string res_exception_name;
	exceptioncode res_exception_code;
	int res_exception_line;
	owca_message_type res_compilation_type,res_compilation_type2;
	int res_compilation_line,res_compilation_line2;
	std::string code;
	unsigned int cline;

	Test(unsigned int cline_, const std::string &code_, const char *s) : cline(cline_),code(code_),restype(VALUE_STRING) { res_value.s=s; }

	//Test(unsigned int cline_, const std::string &code_, exceptioncode exc, int exc_line) : cline(cline_),code(code_),restype(EXCEPTION_ERROR),
	//	res_exception_code(exc),res_exception_line(exc_line) { }
	//Test(exceptioncode exc, unsigned int cline_, const std::string &code_, int exc_line) : cline(cline_),code(code_),restype(COMPILATION_EXCEPTION_ERROR),
	//	res_exception_code(exc),res_exception_line(exc_line) { }
	//Test(unsigned int cline_, const std::string &code_, owca_message_type cmp, int cmp_line) : cline(cline_),code(code_),restype(COMPILATION_ERROR),
	//	res_compilation_type(cmp),res_compilation_line(cmp_line) { }
	//Test(unsigned int cline_, const std::string &code_, owca_message_type cmp, int cmp_line, owca_message_type cmp2, int cmp_line2) : cline(cline_),code(code_),restype(COMPILATION_ERROR2),
	//	res_compilation_type(cmp),res_compilation_line(cmp_line),res_compilation_type2(cmp2),res_compilation_line2(cmp_line2) { }
};
























void __test_failed(Test &t, unsigned int line)
{
	printf("Test failed in line %d, code line %d\n",line,t.cline);
	unsigned int index=0,linecount=1;
	//while(index<t.code.size()) {
	//	unsigned int j=(unsigned int)(t.code.find('\n',index));
	//	printf("%03d: %s\n",linecount,t.code.substr(index,j-index).c_str());
	//	index=j+1;
	//	++linecount;
	//}
	throw 1.0f;
}

owca_global execute(owca_vm &vm, const owca_global &exc, const std::string &member, int param=-1)
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

static void parse_exception(const owca_global &exc, Test &t)
{
	owca_global res=execute(*exc.vm(),exc,"code");
	owca_int code=res.int_get();
	RCASSERT(t.res_exception_code<0 || t.res_exception_code==code);

	owca_global line=execute(*exc.vm(),exc,"$read_1",0);

	res=execute(*exc.vm(),line,"$read_1",2);
	owca_int lineindex=res.int_get();

	RCASSERT(t.res_exception_line<0 || t.res_exception_line==lineindex);
}

#define T(a) do { if (!(a)) __test_failed(t,0); } while(0)


static void printfunction(const std::string &s)
{
	printf("%s",s.c_str());
}

static void test(Test &t)
{
	printf("------------------------ Test z linii %4d ------------------------\n",t.cline);

	{
	TIMER(tt1,"timer 1");
	TIMER(tt2,"timer 2");
	TIMER(tt3,"timer 3");
	TIMER(tt4,"timer 4");
	TIMER(tt5,"timer 5");
	TIMER(tt6,"timer 6");
	owca_vm vm;
	owca_message_list ml;
	bool success=false;
	owca_global res;
	owca_global gnspace=vm.create_namespace("main.ys");
	owca_namespace nspace=gnspace.namespace_get();
	owca_function_return_value result;

	vm.set_print_function(printfunction);

	{
		TIMEIT(tt1);
		result=vm.compile(res,nspace,ml,owca_source_file_Text(t.code.c_str()));
	}
	vm.run_gc();
	{
		switch(result.type()) {
		case owca_function_return_value::NO_RETURN_VALUE: break;
		case owca_function_return_value::RETURN_VALUE: RCASSERT(0);
		case owca_function_return_value::EXCEPTION:
			parse_exception(res,t);
			break;
		default:
			RCASSERT(0);
		}
		if (t.restype!=Test::COMPILATION_EXCEPTION_ERROR) {
			if (ml.has_errors()) {
				TIMEIT(tt3);
				for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
					printf("%5d:      %s\n",it->line(),it->text().c_str());
				}
				printf("\n");
				unsigned int cnt=0;
				T(t.restype==Test::COMPILATION_ERROR || t.restype==Test::COMPILATION_ERROR2);
				for(owca_message_list::T it=ml.begin();it!=ml.end();++it) if (it->is_error()) {
					++cnt;
					T(cnt<=2);
					T(cnt==1 || t.restype==Test::COMPILATION_ERROR2);
					T(it->type()==(cnt==1 ? t.res_compilation_type : t.res_compilation_type2));
					//T(std::string(it->location.filename())==std::string("main.y"));
					T((cnt==1 ? t.res_compilation_line : t.res_compilation_line2)<0 || it->line()==(cnt==1 ? t.res_compilation_line : t.res_compilation_line2));
				}
				T((cnt==1 && t.restype==Test::COMPILATION_ERROR) || (cnt==2 && t.restype==Test::COMPILATION_ERROR2));
			}
			else {
				TIMEIT(tt4);
				T(t.restype!=Test::COMPILATION_ERROR && t.restype!=Test::COMPILATION_ERROR2);
				for(owca_message_list::T it=ml.begin();it!=ml.end();++it) T(!it->is_error());

				{
					TIMEIT(tt5);
					result=nspace["main"].call(res);
					vm.run_gc();
				}

				switch(result.type()) {
				case owca_function_return_value::NO_RETURN_VALUE:
					RCASSERT(0);
					break;
				case owca_function_return_value::RETURN_VALUE:
					printf("value returned is %s\n",res.str().c_str());
					switch(t.restype) {
					case Test::VALUE_STRING: {
						T(res.string_is());
						owca_string ss(res.string_get());
						T(ss==std::string(t.res_value.s));
						break; }
					default:
						T(0);
					}
					break;
				case owca_function_return_value::EXCEPTION:
					vm.run_gc();
					T(t.restype==Test::EXCEPTION_ERROR);
					parse_exception(res,t);
					break;
				default:
					RCASSERT(0);
				}
			}
		}
	}
	nspace.clear();
	vm.run_gc(); }
}
#undef T

//static void test(unsigned int cline, const std::string &oper, const std::string &v1, const std::string &v2, exceptioncode exc, int line=2)
//{
//	Test t(cline,"function main():\n"
//				"	return "+v1+" "+oper+" "+v2+"\n",exc,line);
//	test(t);
//}
//
//static void test(unsigned int cline, const std::string &oper, const std::string &v1, const std::string &v2, owca_global result)
//{
//	if (result.string_is()) {
//		std::string ss=result.string_get().str();
//		result.null_set();
//		Test t(cline,	"function main():\n"
//						"	return "+v1+" "+oper+" "+v2+"\n",ss.c_str());
//		test(t);
//	}
//	else RCASSERT(0);
//}

static std::string ts(int v)
{
	return int_to_string((owca_int)v);
}

static std::string ts(long v)
{
	return int_to_string((owca_int)v);
}

static std::string ts(double v)
{
	return real_to_string((owca_real)v);
}

static std::string ts(long double v)
{
	return real_to_string((owca_real)v);
}

void test_operator_2(const std::string &optext, const std::string &opsym, const std::string &initcode)
{
	Test t(__LINE__,
		"class EXP:\n"
		"	function $init(v):\n"
		"		self.v=v\n"
		"	function $str():\n"
		"		return '<EXP '+$str(self.v)+'>'\n"
		"	function $eq(v):\n"
		"		if $type(v)!=EXP: return EXP==$type(v)\n"
		"		return self.v==v.v\n"
		"	function $noteq(v):\n"
		"		if $type(v)!=EXP: return EXP!=$type(v)\n"
		"		return self.v!=v.v\n"
		"invint=EXP("+int_to_string((owca_int)YEXCEPTION_INTEGEROUTOFBOUNDS)+")\n"
		"div=EXP("+int_to_string((owca_int)YEXCEPTION_DIVISIONBYZERO)+")\n"
		"class T:\n"
		"	function read prop(): return 0\n"
		"class G:\n"
		"	function $" +optext+ "(v): return 'ok'\n"
		"	function $r" +optext+ "(v): return 'rok'\n"
		"	function $s" +optext+ "(v): return 'sok'\n"
		"	function $str(): return 'str'\n"
		"	function $eq(v):\n"
		"		if $type(v)!=G: return G==$type(v)\n"
		"		return $id(self)==$id(v)\n"
		"	function $noteq(v):\n"
		"		if $type(v)!=G: return G!=$type(v)\n"
		"		return $id(self)!=$id(v)\n"
		"function gen():\n"
		"	yield 1\n"
		"t,g=T(),G()\n"
		"m1=(g,7,-8,0,6.5,-9.5,0.0,'abc','',true,false,gen(),T.prop,gen,t,null,2.0)\n"
		"m2=(g,7,-8,0,6.5,-9.5,0.0,'def','',true,false,gen(),T.prop,gen,t,null,2.0)\n"
		"mp={}\n"
		"function mpget(i1,i2): return mp.get((i1,i2),(null,null,null))\n"
		"ll=m1.size()\n"
		+initcode+
		"function ff(i1,i2):\n"
		"	r1,r2,r3=mp.get((i1,i2),(null,null,null))\n"
		"	v1,v2=m1[i1],m2[i2]\n"
//		"	$print(r1,r2,r3,join=' : ')\n"
//		"	$print(v1,v2,join=' : ')\n"
		"	try:\n"
		"		res=v1 " +opsym+ " v2\n"
		"	except as e1:\n"
		"		if $type(r1)==EXP and r1.v==e1.code(): res=r1\n"
		"		elif e1.code()!="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+":\n"
		"			$print(i1,' ',i2,' e1 ',e1.code(),' expected r1 ',r1)\n"
		"			return 2\n"
		"		else: res=null\n"
		"	if $type(r1) is $real and $type(res) is $real and -0.001<=r1-res<=0.001: res=r1\n"
		"	if res!=r1:\n"
		"		$print(i1,' ',i2,' value returned ',res,' expected r1 ',r1)\n"
		"		return 3\n"
		"	try:\n"
		"		res=v1.$" +optext+ "(v2)\n"
		"	except as e2:\n"
		"		if e2.code()=="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+" or e2.code()=="+int_to_string((owca_int)YEXCEPTION_MISSINGMEMBER)+": res=null\n"
		"		elif $type(r1)==EXP and r1.v==e2.code(): res=r1\n"
		"		else:\n"
		"			$print(i1,' ',i2,' e2 ',e2.code(),' expected r1 ',r1)\n"
		"			return 5\n"
		"	if res is null:\n"
		"		try:\n"
		"			res=v2.$r" +optext+ "(v1)\n"
		"		except as e2:\n"
		"			if e2.code()=="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+" or e2.code()=="+int_to_string((owca_int)YEXCEPTION_MISSINGMEMBER)+": res=null\n"
		"			elif $type(r1)==EXP and r1.v==e2.code(): res=r1\n"
		"			else:\n"
		"				$print(i1,' ',i2,' e2 ',e2.code(),' expected r1 ',r1)\n"
		"				return 4\n"
		//"	try:\n"
		//"		res=f(v)\n"
		//"	except as e2:\n"
		//"		if $type(r1)==EXP and r1.v==e2.code(): res=r1\n"
		//"		elif e2.code()!="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+": return 5\n"
		//"		else: res=null\n"
		"	if $type(r1) is $real and $type(res) is $real and -0.001<=r1-res<=0.001: res=r1\n"
		"	if res!=r1:\n"
		"		$print(i1,' ',i2,' value returned ',res,' expected r1 ',r1)\n"
		"		return 6\n"
		"	try:\n"
		"		res=v1\n"
		"		res" +opsym+ "=v2\n"
		"	except as e3:\n"
		"		if $type(r2)==EXP and r2.v==e3.code(): res=r2\n"
		"		elif e3.code()!="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+":\n"
		"			$print(i1,' ',i2,' e3 ',e3.code(),' expected r2 ',r2)\n"
		"			return 7\n"
		"		else: res=null\n"
		"	if $type(r2) is $real and $type(res) is $real and -0.001<=r2-res<=0.001: res=r2\n"
		"	if res!=r2:\n"
		"		$print(i1,' ',i2,' value returned ',res,' expected r2 ',r2)\n"
		"		return 8\n"
		"	try:\n"
		"		f=v1.$s" +optext+ "\n"
		"	except:\n"
		"		f=null\n"
		"	try:\n"
		"		res=f(v2)\n"
		"	except as e4:\n"
		"		if $type(r3)==EXP and r3.v==e4.code(): res=r3\n"
		"		if e4.code()!="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+":\n"
		"			$print(i1,' ',i2,' e4 ',e4.code(),' expected r3 ',r3)\n"
		"			return 10\n"
		"		else: res=null\n"
		"	if $type(r3) is $real and $type(res) is $real and -0.001<=r3-res<=0.001: res=r3\n"
		"	if res!=r3:\n"
		"		$print(i1,' ',i2,' value returned ',res,' expected r3 ',r3)\n"
		"		return 11\n"
		"	return 0\n"
		"function main():\n"
		"	class EEE($exception): pass\n"
		"	function f(k1,k2):\n"
		"		$print('k1 ',k1,' k2 ',k2)\n"
		"		val=ff(k1,k2)\n"
		"		if val:\n"
		"			$print(k1,' ',k2,' ',val)\n"
		"			raise EEE()\n"
		"	try:\n"
		"		#f(1,1)\n"
		"		for k1 = $range(ll):\n"
		"			for k2 = $range(ll):\n"
		"				f(k1,k2)\n"
		"	except EEE:\n"
		"		return 0\n"
		"	return 'ok'\n","ok");
		test(t);
}
#define T(a,b) test_operator_2(#a,opsym,b)

static void test_operator_add()
{
	// exec_code.cpp
#define optext "add"
#define opsym_ +
#define opsym "+"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	T(add,
		"for i = $range(ll): mp[(i,0)]=('rok','rok',null)\n"
		"for i = $range(ll): mp[(0,i)]=('ok',g,'sok')\n"
		"mp[(7,7)]=('abcdef','abcdef',null)\n"
		"mp[(7,8)]=('abc','abc',null)\n"
		"mp[(8,7)]=('def','def',null)\n"
		"mp[(8,8)]=('','',null)\n"
		O(1, 1,7,   7)
		O(1, 2,7,  -8)
		O(1, 3,7,   0)
		O(1, 4,7, 6.5)
		O(1, 5,7,-9.5)
		O(1, 6,7, 0.0)
		O(1,16,7, 2.0)
		O(2, 1,-8,   7)
		O(2, 2,-8,  -8)
		O(2, 3,-8,   0)
		O(2, 4,-8, 6.5)
		O(2, 5,-8,-9.5)
		O(2, 6,-8, 0.0)
		O(2,16,-8, 2.0)
		O(3, 1,0,   7)
		O(3, 2,0,  -8)
		O(3, 3,0,   0)
		O(3, 4,0, 6.5)
		O(3, 5,0,-9.5)
		O(3, 6,0, 0.0)
		O(3,16,0, 2.0)
		O(4, 1,6.5,   7)
		O(4, 2,6.5,  -8)
		O(4, 3,6.5,   0)
		O(4, 4,6.5, 6.5)
		O(4, 5,6.5,-9.5)
		O(4, 6,6.5, 0.0)
		O(4,16,6.5, 2.0)
		O(5, 1,-9.5,   7)
		O(5, 2,-9.5,  -8)
		O(5, 3,-9.5,   0)
		O(5, 4,-9.5, 6.5)
		O(5, 5,-9.5,-9.5)
		O(5, 6,-9.5, 0.0)
		O(5,16,-9.5, 2.0)
		O(6, 1,0.0,   7)
		O(6, 2,0.0,  -8)
		O(6, 3,0.0,   0)
		O(6, 4,0.0, 6.5)
		O(6, 5,0.0,-9.5)
		O(6, 6,0.0, 0.0)
		O(6,16,0.0, 2.0)
		O(16, 1,2.0,   7)
		O(16, 2,2.0,  -8)
		O(16, 3,2.0,   0)
		O(16, 4,2.0, 6.5)
		O(16, 5,2.0,-9.5)
		O(16, 6,2.0, 0.0)
		O(16,16,2.0, 2.0));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_sub()
{
#define optext "sub"
#define opsym_ -
#define opsym "-"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	T(sub,
		"for i = $range(ll): mp[(i,0)]=('rok','rok',null)\n"
		"for i = $range(ll): mp[(0,i)]=('ok',g,'sok')\n"
		O(1, 1,7,   7)
		O(1, 2,7,  -8)
		O(1, 3,7,   0)
		O(1, 4,7, 6.5)
		O(1, 5,7,-9.5)
		O(1, 6,7, 0.0)
		O(1,16,7, 2.0)
		O(2, 1,-8,   7)
		O(2, 2,-8,  -8)
		O(2, 3,-8,   0)
		O(2, 4,-8, 6.5)
		O(2, 5,-8,-9.5)
		O(2, 6,-8, 0.0)
		O(2,16,-8, 2.0)
		O(3, 1,0,   7)
		O(3, 2,0,  -8)
		O(3, 3,0,   0)
		O(3, 4,0, 6.5)
		O(3, 5,0,-9.5)
		O(3, 6,0, 0.0)
		O(3,16,0, 2.0)
		O(4, 1,6.5,   7)
		O(4, 2,6.5,  -8)
		O(4, 3,6.5,   0)
		O(4, 4,6.5, 6.5)
		O(4, 5,6.5,-9.5)
		O(4, 6,6.5, 0.0)
		O(4,16,6.5, 2.0)
		O(5, 1,-9.5,   7)
		O(5, 2,-9.5,  -8)
		O(5, 3,-9.5,   0)
		O(5, 4,-9.5, 6.5)
		O(5, 5,-9.5,-9.5)
		O(5, 6,-9.5, 0.0)
		O(5,16,-9.5, 2.0)
		O(6, 1,0.0,   7)
		O(6, 2,0.0,  -8)
		O(6, 3,0.0,   0)
		O(6, 4,0.0, 6.5)
		O(6, 5,0.0,-9.5)
		O(6, 6,0.0, 0.0)
		O(6,16,0.0, 2.0)
		O(16, 1,2.0,   7)
		O(16, 2,2.0,  -8)
		O(16, 3,2.0,   0)
		O(16, 4,2.0, 6.5)
		O(16, 5,2.0,-9.5)
		O(16, 6,2.0, 0.0)
		O(16,16,2.0, 2.0));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_mul()
{
	// exec_code.cpp
#define optext "mul"
#define opsym_ *
#define opsym "*"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	T(mul,
		"for i = $range(ll): mp[(i,0)]=('rok','rok',null)\n"
		"for i = $range(ll): mp[(0,i)]=('ok',g,'sok')\n"
		"mp[(7,1)]= ('abcabcabcabcabcabcabc','abcabcabcabcabcabcabc',null)\n"
		"mp[(7,2)]= (invint,invint,null)\n"
		"mp[(7,3)]= ('','',null)\n"
		"mp[(7,6)]= ('','',null)\n"
		"mp[(7,16)]=('abcabc','abcabc',null)\n"
		"mp[(8,1)]= ('','',null)\n"
		"mp[(8,2)]= (invint,invint,null)\n"
		"mp[(8,3)]= ('','',null)\n"
		"mp[(8,6)]= ('','',null)\n"
		"mp[(8,16)]=('','',null)\n"
		"mp[(1,7)]= ('defdefdefdefdefdefdef','defdefdefdefdefdefdef',null)\n"
		"mp[(1,8)]= ('','',null)\n"
		"mp[(2,7)]= (invint,invint,null)\n"
		"mp[(2,8)]= (invint,invint,null)\n"
		"mp[(3,7)]= ('','',null)\n"
		"mp[(3,8)]= ('','',null)\n"
		"mp[(6,7)]= ('','',null)\n"
		"mp[(6,8)]= ('','',null)\n"
		"mp[(16,7)]=('defdef','defdef',null)\n"
		"mp[(16,8)]=('','',null)\n"
		O(1, 1,7,   7)
		O(1, 2,7,  -8)
		O(1, 3,7,   0)
		O(1, 4,7, 6.5)
		O(1, 5,7,-9.5)
		O(1, 6,7, 0.0)
		O(1,16,7, 2.0)
		O(2, 1,-8,   7)
		O(2, 2,-8,  -8)
		O(2, 3,-8,   0)
		O(2, 4,-8, 6.5)
		O(2, 5,-8,-9.5)
		O(2, 6,-8, 0.0)
		O(2,16,-8, 2.0)
		O(3, 1,0,   7)
		O(3, 2,0,  -8)
		O(3, 3,0,   0)
		O(3, 4,0, 6.5)
		O(3, 5,0,-9.5)
		O(3, 6,0, 0.0)
		O(3,16,0, 2.0)
		O(4, 1,6.5,   7)
		O(4, 2,6.5,  -8)
		O(4, 3,6.5,   0)
		O(4, 4,6.5, 6.5)
		O(4, 5,6.5,-9.5)
		O(4, 6,6.5, 0.0)
		O(4,16,6.5, 2.0)
		O(5, 1,-9.5,   7)
		O(5, 2,-9.5,  -8)
		O(5, 3,-9.5,   0)
		O(5, 4,-9.5, 6.5)
		O(5, 5,-9.5,-9.5)
		O(5, 6,-9.5, 0.0)
		O(5,16,-9.5, 2.0)
		O(6, 1,0.0,   7)
		O(6, 2,0.0,  -8)
		O(6, 3,0.0,   0)
		O(6, 4,0.0, 6.5)
		O(6, 5,0.0,-9.5)
		O(6, 6,0.0, 0.0)
		O(6,16,0.0, 2.0)
		O(16, 1,2.0,   7)
		O(16, 2,2.0,  -8)
		O(16, 3,2.0,   0)
		O(16, 4,2.0, 6.5)
		O(16, 5,2.0,-9.5)
		O(16, 6,2.0, 0.0)
		O(16,16,2.0, 2.0));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_div()
{
	// exec_code.cpp
#define optext "div"
#define opsym_ /
#define opsym "/"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	// "m1=(g,7,-8,0,6.5,-9.5,0.0,'abc','',true,false,gen(),T.prop,gen,t,null)\n"
	T(div,
		"for i = $range(7):\n"
		"	mp[(i,3)]=(div,div,null)\n"
		"	mp[(i,6)]=(div,div,null)\n"
		"mp[(16,3)]=(div,div,null)\n"
		"mp[(16,6)]=(div,div,null)\n"
		"for i = $range(ll): mp[(i,0)]=('rok','rok',null)\n"
		"for i = $range(ll): mp[(0,i)]=('ok',g,'sok')\n"
		O(1, 1,7,   7)
		O(1, 2,7,  -8.0)
		O(1, 4,7, 6.5)
		O(1, 5,7,-9.5)
		O(1,16,7, 2.0)
		O(2, 1,-8,   7.0)
		O(2, 2,-8,  -8)
		O(2, 4,-8, 6.5)
		O(2, 5,-8,-9.5)
		O(2,16,-8, 2.0)
		O(3, 1,0,   7)
		O(3, 2,0,  -8)
		O(3, 4,0, 6.5)
		O(3, 5,0,-9.5)
		O(3,16,0, 2.0)
		O(4, 1,6.5,   7)
		O(4, 2,6.5,  -8)
		O(4, 4,6.5, 6.5)
		O(4, 5,6.5,-9.5)
		O(4,16,6.5, 2.0)
		O(5, 1,-9.5,   7)
		O(5, 2,-9.5,  -8)
		O(5, 4,-9.5, 6.5)
		O(5, 5,-9.5,-9.5)
		O(5,16,-9.5, 2.0)
		O(6, 1,0.0,   7)
		O(6, 2,0.0,  -8)
		O(6, 4,0.0, 6.5)
		O(6, 5,0.0,-9.5)
		O(6,16,0.0, 2.0)
		O(16, 1,2.0,   7)
		O(16, 2,2.0,  -8)
		O(16, 4,2.0, 6.5)
		O(16, 5,2.0,-9.5)
		O(16,16,2.0, 2.0));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_mod()
{
	// exec_code.cpp
#define optext "mod"
#define opsym_ %
#define opsym "%"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	T(mod,
		"mp[(1,3)]=(div,div,null)\n"
		"mp[(2,3)]=(div,div,null)\n"
		"mp[(3,3)]=(div,div,null)\n"
		"mp[(6,3)]=(div,div,null)\n"
		"mp[(16,3)]=(div,div,null)\n"
		"mp[(1,6)]=(div,div,null)\n"
		"mp[(2,6)]=(div,div,null)\n"
		"mp[(3,6)]=(div,div,null)\n"
		"mp[(6,6)]=(div,div,null)\n"
		"mp[(16,6)]=(div,div,null)\n"
		"for i = $range(ll): mp[(i,0)]=('rok','rok',null)\n"
		"for i = $range(ll): mp[(0,i)]=('ok',g,'sok')\n"
		O(1, 1,7,   7)
		O(1, 2,7,  -8)
		O(1,16,7,   2)
		O(2, 1,-8,   7)
		O(2, 2,-8,  -8)
		O(2,16,-8,   2)
		O(3, 1,0,   7)
		O(3, 2,0,  -8)
		O(3,16,0,   2)
		O(6, 1,0,   7)
		O(6, 2,0,  -8)
		O(6,16,0,   2)
		O(16, 1,2,   7)
		O(16, 2,2,  -8)
		O(16,16,2,   2));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_lshift()
{
	// exec_code.cpp
#define optext "lshift"
#define opsym_ <<
#define opsym "<<"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	T(lshift,
		"mp[(1,2)]=(invint,invint,null)\n"
		"mp[(2,2)]=(invint,invint,null)\n"
		"mp[(3,2)]=(invint,invint,null)\n"
		"mp[(6,2)]=(invint,invint,null)\n"
		"mp[(16,2)]=(invint,invint,null)\n"
		"for i = $range(ll): mp[(i,0)]=('rok','rok',null)\n"
		"for i = $range(ll): mp[(0,i)]=('ok',g,'sok')\n"
		O(1, 1,7,   7)
		O(1, 3,7,   0)
		O(1, 6,7,   0)
		O(1,16,7,   2)
		O(2, 1,-8,   7)
		O(2, 3,-8,   0)
		O(2, 6,-8,   0)
		O(2,16,-8,   2)
		O(3, 1,0,   7)
		O(3, 3,0,   0)
		O(3, 6,0,   0)
		O(3,16,0,   2)
		O(6, 1,0,   7)
		O(6, 3,0,   0)
		O(6, 6,0,   0)
		O(6,16,0,   2)
		O(16, 1,2,   7)
		O(16, 3,2,   0)
		O(16, 6,2,   0)
		O(16,16,2,   2));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_rshift()
{
	// exec_code.cpp
#define optext "rshift"
#define opsym_ >>
#define opsym ">>"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	T(rshift,
		"mp[(1,2)]=(invint,invint,null)\n"
		"mp[(2,2)]=(invint,invint,null)\n"
		"mp[(3,2)]=(invint,invint,null)\n"
		"mp[(6,2)]=(invint,invint,null)\n"
		"mp[(16,2)]=(invint,invint,null)\n"
		"for i = $range(ll): mp[(i,0)]=('rok','rok',null)\n"
		"for i = $range(ll): mp[(0,i)]=('ok',g,'sok')\n"
		O(1, 1,7,   7)
		O(1, 3,7,   0)
		O(1, 6,7,   0)
		O(1,16,7,   2)
		O(2, 1,-8,   7)
		O(2, 3,-8,   0)
		O(2, 6,-8,   0)
		O(2,16,-8,   2)
		O(3, 1,0,   7)
		O(3, 3,0,   0)
		O(3, 6,0,   0)
		O(3,16,0,   2)
		O(6, 1,0,   7)
		O(6, 3,0,   0)
		O(6, 6,0,   0)
		O(6,16,0,   2)
		O(16, 1,2,   7)
		O(16, 3,2,   0)
		O(16, 6,2,   0)
		O(16,16,2,   2));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_binand()
{
	// exec_code.cpp
#define optext "binand"
#define opsym_ &
#define opsym "&"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	T(binand,
		"for i = $range(ll): mp[(i,0)]=('rok','rok',null)\n"
		"for i = $range(ll): mp[(0,i)]=('ok',g,'sok')\n"
		O(1, 1,7,   7)
		O(1, 2,7,  -8)
		O(1, 3,7,   0)
		O(1, 6,7,   0)
		O(1,16,7,   2)
		O(2, 1,-8,   7)
		O(2, 2,-8,  -8)
		O(2, 3,-8,   0)
		O(2, 6,-8,   0)
		O(2,16,-8,   2)
		O(3, 1,0,   7)
		O(3, 2,0,  -8)
		O(3, 3,0,   0)
		O(3, 6,0,   0)
		O(3,16,0,   2)
		O(6, 1,0,   7)
		O(6, 2,0,  -8)
		O(6, 3,0,   0)
		O(6, 6,0,   0)
		O(6,16,0,   2)
		O(16, 1,2,   7)
		O(16, 2,2,  -8)
		O(16, 3,2,   0)
		O(16, 6,2,   0)
		O(16,16,2,   2));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_binor()
{
	// exec_code.cpp
#define optext "binor"
#define opsym_ |
#define opsym "|"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	T(binor,
		"for i = $range(ll): mp[(i,0)]=('rok','rok',null)\n"
		"for i = $range(ll): mp[(0,i)]=('ok',g,'sok')\n"
		O(1, 1,7,   7)
		O(1, 2,7,  -8)
		O(1, 3,7,   0)
		O(1, 6,7,   0)
		O(1,16,7,   2)
		O(2, 1,-8,   7)
		O(2, 2,-8,  -8)
		O(2, 3,-8,   0)
		O(2, 6,-8,   0)
		O(2,16,-8,   2)
		O(3, 1,0,   7)
		O(3, 2,0,  -8)
		O(3, 3,0,   0)
		O(3, 6,0,   0)
		O(3,16,0,   2)
		O(6, 1,0,   7)
		O(6, 2,0,  -8)
		O(6, 3,0,   0)
		O(6, 6,0,   0)
		O(6,16,0,   2)
		O(16, 1,2,   7)
		O(16, 2,2,  -8)
		O(16, 3,2,   0)
		O(16, 6,2,   0)
		O(16,16,2,   2));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_binxor()
{
#define optext "binxor"
#define opsym_ ^
#define opsym "^"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	T(binxor,
		"for i = $range(ll): mp[(i,0)]=('rok','rok',null)\n"
		"for i = $range(ll): mp[(0,i)]=('ok',g,'sok')\n"
		O(1, 1,7,   7)
		O(1, 2,7,  -8)
		O(1, 3,7,   0)
		O(1, 6,7,   0)
		O(1,16,7,   2)
		O(2, 1,-8,   7)
		O(2, 2,-8,  -8)
		O(2, 3,-8,   0)
		O(2, 6,-8,   0)
		O(2,16,-8,   2)
		O(3, 1,0,   7)
		O(3, 2,0,  -8)
		O(3, 3,0,   0)
		O(3, 6,0,   0)
		O(3,16,0,   2)
		O(6, 1,0,   7)
		O(6, 2,0,  -8)
		O(6, 3,0,   0)
		O(6, 6,0,   0)
		O(6,16,0,   2)
		O(16, 1,2,   7)
		O(16, 2,2,  -8)
		O(16, 3,2,   0)
		O(16, 6,2,   0)
		O(16,16,2,   2));
#undef optext
#undef opsym_
#undef opsym
#undef O
}




void test_operator_2_cmp(const std::string &optext, const std::string &opsym, const std::string &initcode)
{
	Test t(__LINE__,
		"class EXP:\n"
		"	function $init(v):\n"
		"		self.v=v\n"
		"	function $str():\n"
		"		return '<EXP '+$str(self.v)+'>'\n"
		"	function $eq(v):\n"
		"		if $type(v)!=EXP: return EXP==$type(v)\n"
		"		return self.v==v.v\n"
		"	function $noteq(v):\n"
		"		if $type(v)!=EXP: return EXP!=$type(v)\n"
		"		return self.v!=v.v\n"
		"invint=EXP("+int_to_string((owca_int)YEXCEPTION_INTEGEROUTOFBOUNDS)+")\n"
		"div=EXP("+int_to_string((owca_int)YEXCEPTION_DIVISIONBYZERO)+")\n"
		"class T:\n"
		"	function read prop(): return 0\n"
		"class G:\n"
		"	function $str(): return 'str'\n"
		"	function $eq(v):\n"
		"		return self is v\n"
		"	function $noteq(v):\n"
		"		return not self is v\n"
		"	function $req(v):\n"
		"		return self is v\n"
		"	function $rnoteq(v):\n"
		"		return not self is v\n"
		"function gen():\n"
		"	yield 1\n"
		"t,g=T(),G()\n"
//		"m1=(g,7,-8,0,6.5,-9.5,0.0,2.0,'abc','',true,false,gen(),T.prop,gen,t,null)\n"
		"m1=(g,-9.5,-8,0,0.0,2.0,6.5,7,'','abc',false,true,gen(),T.prop,gen,t,null)\n"
		"m2=(g,-9.5,-8,0,0.0,2.0,6.5,7,'','def',false,true,gen(),T.prop,gen,t,null)\n"
		"m1str=('g','-9.5','-8','0','0.0','2.0','6.5','7',\"''\",\"'abc'\",'true','false','gen()','T.prop','gen','t','null')\n"
		"m2str=('g','-9.5','-8','0','0.0','2.0','6.5','7',\"''\",\"'def'\",'true','false','gen()','T.prop','gen','t','null')\n"
//		"m2=(g,7,-8,0,6.5,-9.5,0.0,2.0,'def','',true,false,gen(),T.prop,gen,t,null)\n"
		"mp={}\n"
		"ll=m1.size()\n"
		+initcode+
		"function mpget(i1,i2): return mp.get((i1,i2),defaultvalue)\n"
		"function ff(i1,i2):\n"
		"	r1,r2,r3=mp.get((i1,i2),defaultvalue)\n"
		"	#$print('k1 ',m1str[i1],' k2 ',m2str[i2],' expected r1 ',r1,' expected r2 ',r2,' expected r3 ',r3)\n"
		"	v1,v2=m1[i1],m2[i2]\n"
		"	try:\n"
		"		#$trap()\n"
		"		res=v1 " +opsym+ " v2\n"
		"	except as e1:\n"
		"		if $type(r1)==EXP and r1.v==e1.code(): res=r1\n"
		"		elif e1.code()!="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+":\n"
		"			$print(m1str[i1],' ',m2str[i2],' e1 ',e1.code(),' expected r1 ',r1)\n"
		"			return 2\n"
		"		else: res=null\n"
		"	if $type(r1) is $real and $type(res) is $real and -0.001<=r1-res<=0.001: res=r1\n"
		"	if res!=r1:\n"
		"		$print(m1str[i1],' ',m2str[i2],' value returned ',res,' expected r1 ',r1)\n"
		"		return 3\n"
		"	try:\n"
		"		res=v1.$" +optext+ "(v2)\n"
		"	except as e2:\n"
		"		if e2.code()=="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+" or e2.code()=="+int_to_string((owca_int)YEXCEPTION_MISSINGMEMBER)+": res=null\n"
		"		elif $type(r2)==EXP and r2.v==e2.code(): res=r2\n"
		"		else:\n"
		"			$print(m1str[i1],' ',m2str[i2],' e2 ',e2.code(),' expected r2 ',r2)\n"
		"			return 4\n"
		"	if $type(r2) is $real and $type(res) is $real and -0.001<=r2-res<=0.001: res=r2\n"
		"	if res!=r2:\n"
		"		$print(m1str[i1],' ',m2str[i2],' value returned ',res,' expected r2 ',r2)\n"
		"		return 5\n"
		"	try:\n"
		"		res=v2.$r" +optext+ "(v1)\n"
		"	except as e2:\n"
		"		if e2.code()=="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+" or e2.code()=="+int_to_string((owca_int)YEXCEPTION_MISSINGMEMBER)+": res=null\n"
		"		elif $type(r3)==EXP and r3.v==e2.code(): res=r3\n"
		"		else:\n"
		"			$print(m1str[i1],' ',m2str[i2],' e2 ',e2.code(),' expected r3 ',r3)\n"
		"			return 6\n"
		"	if $type(r3) is $real and $type(res) is $real and -0.001<=r3-res<=0.001: res=r3\n"
		"	if res!=r3:\n"
		"		$print(m1str[i1],' ',m2str[i2],' value returned ',res,' expected r3 ',r3)\n"
		"		return 7\n"
		"	return 0\n"
		"function main():\n"
		"	class EEE($exception): pass\n"
		"	function f(k1,k2):\n"
		"		$print('k1 ',k1,' k2 ',k2)\n"
		"		val=ff(k1,k2)\n"
		"		if val:\n"
		"			$print(k1,' ',k2,' ',val)\n"
		"			raise EEE()\n"
		"	try:\n"
		"		f(12,12)\n"
		"		#$trap()\n"
		"		for k1 = $range(ll):\n"
		"			for k2 = $range(ll):\n"
		"				f(k1,k2)\n"
		"	except EEE:\n"
		"		return 0\n"
		"	return 'ok'\n","ok");
		test(t);
}
#undef T
#define T(a,b) test_operator_2_cmp(#a,opsym,b)

static void test_operator_cmp_eq()
{
#define optext "eq"
#define opsym_ ==
#define opsym "=="
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	// "m1=(g,-9.5,-8,0,0.0,2.0,6.5,7,'','abc',true,false,gen(),T.prop,gen,t,null)\n"
	T(eq,
		"defaultvalue=(false,null,null)\n"
		"for i = $range(ll):\n"
		"	mp[(i,i)]=(true,true,null)\n"
		"for i = $range(1,ll): mp[(i,0)]=(i==0,null,i==0)\n"
		"for i = $range(1,ll): mp[(0,i)]=(i==0,i==0,null)\n"
		"mp[(0,0)]=(true,true,true)\n"
		"mp[(12,12)]=(false,false,null)\n"
		"mp[(15,15)]=(true,null,null)\n"
		"for i = $range(1,8):\n"
		"	for j = $range(1,8):\n"
		"		z=i==j or (i==3 and j==4) or (i==4 and j==3)\n"
		"		mp[(i,j)]=(z,z,null)\n"
		"for i = $range(8,10):\n"
		"	for j = $range(8,10):\n"
		"		z=i==8 and j==8\n"
		"		mp[(i,j)]=(z,z,null)\n"
		"for i = $range(10,12):\n"
		"	for j = $range(10,12):\n"
		"		z=i==j\n"
		"		mp[(i,j)]=(z,z,null)\n"
		);
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_cmp_noteq()
{
#define optext "noteq"
#define opsym_ !=
#define opsym "!="
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	// "m1=(g,7,-8,0,6.5,-9.5,0.0,2.0,'abc','',true,false,gen(),T.prop,gen,t,null)\n"
	T(noteq,
		"defaultvalue=(true,null,null)\n"
		"for i = $range(ll):\n"
		"	mp[(i,i)]=(false,false,null)\n"
		"for i = $range(1,ll): mp[(i,0)]=(i!=0,null,i!=0)\n"
		"for i = $range(1,ll): mp[(0,i)]=(i!=0,i!=0,null)\n"
		"mp[(0,0)]=(false,false,false)\n"
		"mp[(12,12)]=(true,true,null)\n"
		"mp[(15,15)]=(false,null,null)\n"
		"for i = $range(1,8):\n"
		"	for j = $range(1,8):\n"
		"		z=not (i==j or (i==3 and j==4) or (i==4 and j==3))\n"
		"		mp[(i,j)]=(z,z,null)\n"
		"for i = $range(8,10):\n"
		"	for j = $range(8,10):\n"
		"		z=not (i==8 and j==8)\n"
		"		mp[(i,j)]=(z,z,null)\n"
		"for i = $range(10,12):\n"
		"	for j = $range(10,12):\n"
		"		z=not (i==j)\n"
		"		mp[(i,j)]=(z,z,null)\n"
		);
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_cmp_less()
{
#define optext "less"
#define opsym_ <
#define opsym "<"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	// "m1=(g,-9.5,-8,0,0.0,2.0,6.5,7,'','abc',false,true,gen(),T.prop,gen,t,null)\n"
	T(less,
		"defaultvalue=(null,null,null)\n"
//		"for i = $range(1,ll): mp[(i,0)]=(i!=0,null,i!=0)\n"
//		"for i = $range(1,ll): mp[(0,i)]=(i!=0,i!=0,null)\n"
		"for i = $range(1,8):\n"
		"	for j = $range(1,8):\n"
		"		v=(i==3 and j==4) or (i==4 and j==3) ? 0 : i-j\n"
		"		z=v<0\n"
		"		mp[(i,j)]=(z,z,null)\n"
		"for i = $range(8,10):\n"
		"	for j = $range(8,10):\n"
		"		z=i-j\n"
		"		if i==j==9: z=-1\n"
		"		z=z<0\n"
		"		mp[(i,j)]=(z,z,null)\n"
		);
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_cmp_more()
{
#define optext "more"
#define opsym_ >
#define opsym ">"
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	// "m1=(g,7,-8,0,6.5,-9.5,0.0,2.0,'abc','',true,false,gen(),T.prop,gen,t,null)\n"
	T(more,
		"defaultvalue=(null,null,null)\n"
//		"for i = $range(1,ll): mp[(i,0)]=(i!=0,null,i!=0)\n"
//		"for i = $range(1,ll): mp[(0,i)]=(i!=0,i!=0,null)\n"
		"for i = $range(1,8):\n"
		"	for j = $range(1,8):\n"
		"		v=(i==3 and j==4) or (i==4 and j==3) ? 0 : i-j\n"
		"		z=v>0\n"
		"		#$print('gen',i,j,v,z)\n"
		"		mp[(i,j)]=(z,z,null)\n"
		"for i = $range(8,10):\n"
		"	for j = $range(8,10):\n"
		"		z=i-j\n"
		"		if i==j==9: z=-1\n"
		"		z=z>0\n"
		"		mp[(i,j)]=(z,z,null)\n"
		);
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_cmp_lesseq()
{
#define optext "lesseq"
#define opsym_ <=
#define opsym "<="
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	// "m1=(g,7,-8,0,6.5,-9.5,0.0,2.0,'abc','',true,false,gen(),T.prop,gen,t,null)\n"
	T(lesseq,
		"defaultvalue=(null,null,null)\n"
//		"for i = $range(1,ll): mp[(i,0)]=(i!=0,null,i!=0)\n"
//		"for i = $range(1,ll): mp[(0,i)]=(i!=0,i!=0,null)\n"
		"for i = $range(1,8):\n"
		"	for j = $range(1,8):\n"
		"		v=(i==3 and j==4) or (i==4 and j==3) ? 0 : i-j\n"
		"		z=v<=0\n"
		"		mp[(i,j)]=(z,z,null)\n"
		"for i = $range(8,10):\n"
		"	for j = $range(8,10):\n"
		"		z=i-j\n"
		"		if i==j==9: z=-1\n"
		"		z=z<=0\n"
		"		mp[(i,j)]=(z,z,null)\n"
		);
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_cmp_moreeq()
{
#define optext "moreeq"
#define opsym_ >=
#define opsym ">="
#define O(m1,m2,m3,m4) "mp[(" #m1 "," #m2 ")]=("+ts((m3) opsym_ (m4))+","+ts((m3) opsym_ (m4))+",null)\n"
	// "m1=(g,7,-8,0,6.5,-9.5,0.0,2.0,'abc','',true,false,gen(),T.prop,gen,t,null)\n"
	T(moreeq,
		"defaultvalue=(null,null,null)\n"
//		"for i = $range(1,ll): mp[(i,0)]=(i!=0,null,i!=0)\n"
//		"for i = $range(1,ll): mp[(0,i)]=(i!=0,i!=0,null)\n"
		"for i = $range(1,8):\n"
		"	for j = $range(1,8):\n"
		"		v=(i==3 and j==4) or (i==4 and j==3) ? 0 : i-j\n"
		"		z=v>=0\n"
		"		mp[(i,j)]=(z,z,null)\n"
		"for i = $range(8,10):\n"
		"	for j = $range(8,10):\n"
		"		z=i-j\n"
		"		if i==j==9: z=-1\n"
		"		z=z>=0\n"
		"		mp[(i,j)]=(z,z,null)\n"
		);
#undef optext
#undef opsym_
#undef opsym
#undef O
}








#undef T
#define T(a,b) test_operator_1(#a,opsym,b)

void test_operator_1(const std::string &optext, const std::string &opsym, const std::string &initcode)
{
	Test t(__LINE__,
		"class EXP:\n"
		"	function $init(v):\n"
		"		self.v=v\n"
		"	function $str():\n"
		"		return '<EXP '+$str(self.v)+'>'\n"
		"	function $eq(v):\n"
		"		if $type(v)!=EXP: return EXP==$type(v)\n"
		"		return self.v==v.v\n"
		"	function $noteq(v):\n"
		"		if $type(v)!=EXP: return EXP!=$type(v)\n"
		"		return self.v!=v.v\n"
		"invint=EXP("+int_to_string((owca_int)YEXCEPTION_INTEGEROUTOFBOUNDS)+")\n"
		"div=EXP("+int_to_string((owca_int)YEXCEPTION_DIVISIONBYZERO)+")\n"
		"class T:\n"
		"	function read prop(): return 0\n"
		"class G:\n"
		"	function $" +optext+ "(): return 'ok'\n"
		"	function $eq(v):\n"
		"		if $type(v)!=G: return G==$type(v)\n"
		"		return $id(self)==$id(v)\n"
		"	function $noteq(v):\n"
		"		if $type(v)!=G: return G!=$type(v)\n"
		"		return $id(self)!=$id(v)\n"
		"function gen():\n"
		"	yield 1\n"
		"t,g=T(),G()\n"
		"m1=(g,7,-8,0,6.5,-9.5,0.0,'abc','',true,false,gen(),T.prop,gen,t,null,2.0)\n"
		"mp={}\n"
		"mp[0]='ok'\n"
		"ll=m1.size()\n"
		"function mpget(i1): return mp.get(i1,null)\n"
		+initcode+
		"function ff(i1):\n"
		"	r1=mp.get(i1,null)\n"
		"	v1=m1[i1]\n"
		"	try:\n"
		"		res="+opsym+" v1\n"
		"	except as e1:\n"
		"		if $type(r1)==EXP and r1.v==e1.code(): res=r1\n"
		"		elif e1.code()!="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+": return 2\n"
		"		else: res=null\n"
		"	if res!=r1:\n"
		"		$print(i1,' value returned ',res,' expected r1 ',r1)\n"
		"		return 3\n"
		"	try:\n"
		"		res=v1.$" +optext+ "()\n"
		"	except as e2:\n"
		"		if e2.code()=="+int_to_string((owca_int)YEXCEPTION_INVALIDPARAMTYPE)+" or e2.code()=="+int_to_string((owca_int)YEXCEPTION_MISSINGMEMBER)+": res=null\n"
		"		elif $type(r1)==EXP and r1.v==e2.code(): res=r1\n"
		"		else:\n"
		"			$print(i1,' e2 ',e2.code(),' expected r1 ',r1)\n"
		"			return 4\n"
		"	if res!=r1:\n"
		"		$print(i1,' value returned ',res,' expected r1 ',r1)\n"
		"		return 5\n"
		"	return 0\n"
		"function main():\n"
		"	class EEE($exception): pass\n"
		"	function f(k1):\n"
		"		$print('k1 ',k1)\n"
		"		val=ff(k1)\n"
		"		if val:\n"
		"			$print(k1,' ',val)\n"
		"			raise EEE()\n"
		"	try:\n"
		"		pass\n"
		"		for k1 = $range(ll):\n"
		"			f(k1)\n"
		"	except EEE:\n"
		"		return 0\n"
		"	return 'ok'\n","ok");
		test(t);
}

static void test_operator_sign()
{
	// exec_code.cpp
#define optext "sign"
#define opsym_ -
#define opsym "-"
#define O(m1,m2) "mp[" #m1 "]="+ts(opsym_ (m2))+"\n"
	T(sign,
		O(1,   7)
		O(2,  -8)
		O(3,   0)
		O(4, 6.5)
		O(5,-9.5)
		O(6, 0.0)
		O(16,2.0));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

static void test_operator_binnot()
{
	// exec_code.cpp
#define optext "binnot"
#define opsym_ ~
#define opsym "~"
#define O(m1,m2) "mp[" #m1 "]="+ts(opsym_ (m2))+"\n"
	T(binnot,
		O(1,   7)
		O(2,  -8)
		O(3,   0)
		O(6,   0)
		O(16,  2));
#undef optext
#undef opsym_
#undef opsym
#undef O
}

//#include "rc_y.h"
//
//static void hm_erase(owca_vm &vm, hashmap &hm, owca_int key)
//{
//	exec_variable k;
//	k.set_int(key);
//	hm.erase(vm,k);
//}
//
//static void hm_insert(owca_vm &vm, hashmap &hm, owca_int key, owca_int value)
//{
//	exec_variable k,v;
//	k.set_int(key);
//	v.set_int(value);
//	//unsigned int sz=hm.size();
//	hm.insert(vm,k,v);
//	//RCASSERT(hm.size()==sz+1);
//}
//
//static bool hm_get(owca_vm &vm, hashmap &hm, owca_int &ret, owca_int key)
//{
//	exec_variable k,v;
//	k.set_int(key);
//	if (hm.get(v,vm,k)) {
//		RCASSERT(v.mode()==VAR_INT);
//		ret=v.get_int();
//		return true;
//	}
//	return false;
//}
//
//static owca_int hm_getdef(owca_vm &vm, hashmap &hm, owca_int key, owca_int defval)
//{
//	exec_variable k,v,r;
//	k.set_int(key);
//	v.set_int(defval);
//	hm.get(r,vm,k,v);
//	if (!vm.exception_thrown()) {
//		RCASSERT(r.mode()==VAR_INT);
//		return r.get_int();
//	}
//	return 0;
//}
//
//#define E(k) hm_erase(vm,hm,k)
//#define I(k,v) hm_insert(vm,hm,k,v)
//#define G(ret,k) hm_get(vm,hm,ret,k)
//#define D(k,def) hm_getdef(vm,hm,k,def)
//
//static void test_hashmap()
//{
//	owca_vm vm;
//	hashmap hm;
//	owca_int k;
//
//	I(1,1);
//	RCASSERT(hm.size()==1);
//	RCASSERT(G(k,1) && k==1);
//	I(2,2);
//	RCASSERT(hm.size()==2);
//	I(3,3);
//	RCASSERT(hm.size()==3);
//	I(4,4);
//	RCASSERT(hm.size()==4);
//	I(5,5);
//	RCASSERT(hm.size()==5);
//	RCASSERT(hm.tablesize==hashmap::minsize);
//	I(6,6);
//	RCASSERT(hm.size()==6);
//	RCASSERT(hm.tablesize==2*hashmap::minsize);
//	I(7,7);
//	RCASSERT(hm.size()==7);
//	RCASSERT(hm.tablesize==2*hashmap::minsize);
//
//	for(unsigned int i=1;i<8;++i) {
//		RCASSERT(G(k,i) && k==i);
//	}
//
//	for(unsigned int i=1;i<8;++i) {
//		RCASSERT(hm.size()==7);
//		RCASSERT(hm.tablesize==2*hashmap::minsize);
//		I(i,2*i);
//		RCASSERT(hm.size()==7);
//		RCASSERT(hm.tablesize==2*hashmap::minsize);
//	}
//
//	for(unsigned int i=1;i<8;++i) {
//		RCASSERT(G(k,i) && k==i*2);
//	}
//
//	E(16);
//	RCASSERT(hm.size()==7);
//	RCASSERT(hm.tablesize==2*hashmap::minsize);
//	E(4);
//	RCASSERT(hm.size()==6);
//	RCASSERT(hm.tablesize==2*hashmap::minsize);
//	E(3);
//	RCASSERT(hm.size()==5);
//	RCASSERT(hm.tablesize==2*hashmap::minsize);
//	E(2);
//	RCASSERT(hm.size()==4);
//	RCASSERT(hm.tablesize==2*hashmap::minsize);
//	E(12);
//	RCASSERT(hm.size()==4);
//	RCASSERT(hm.tablesize==2*hashmap::minsize);
//	I(3,6);
//	RCASSERT(hm.size()==5);
//	RCASSERT(hm.tablesize==hashmap::minsize);
//
//	for(unsigned int i=1;i<8;++i) if (i!=4 && i!=2) {
//		RCASSERT(G(k,i) && k==i*2);
//	}
//	else {
//		RCASSERT(!G(k,i));
//	}
//
//	for(unsigned int i=2;i<8;++i) E(i);
//
//	RCASSERT(hm.size()==1 && hm.tablesize==hashmap::minsize);
//
//	RCASSERT(D(2,2)==2);
//	RCASSERT(hm.size()==2 && hm.tablesize==hashmap::minsize);
//	RCASSERT(D(2,2)==2);
//	RCASSERT(hm.size()==2 && hm.tablesize==hashmap::minsize);
//	RCASSERT(D(2,3)==2);
//	RCASSERT(hm.size()==2 && hm.tablesize==hashmap::minsize);
//	RCASSERT(D(1,2)==2);
//	RCASSERT(hm.size()==2 && hm.tablesize==hashmap::minsize);
//	RCASSERT(D(3,3)==3);
//	RCASSERT(hm.size()==3 && hm.tablesize==hashmap::minsize);
//	RCASSERT(D(4,4)==4);
//	RCASSERT(hm.size()==4 && hm.tablesize==hashmap::minsize);
//	RCASSERT(D(5,5)==5);
//	RCASSERT(hm.size()==5 && hm.tablesize==hashmap::minsize);
//	RCASSERT(D(6,6)==6);
//	RCASSERT(hm.size()==6 && hm.tablesize==2*hashmap::minsize);
//	RCASSERT(D(7,7)==7);
//	RCASSERT(hm.size()==7 && hm.tablesize==2*hashmap::minsize);
//
//	hm.clear();
//	RCASSERT(hm.size()==0 && hm.tablesize==0);
//}
//
//#undef E
//#undef I
//#undef G
//#undef D

void test_object_init(owca_vm &vm, owca_vm &vm2, owca_namespace &nspace, owca_namespace &nspace2);
void test_object();
void test_namespace();
void test_object_2();
void test_object_3();
void test_object_4();
void test_object_5();
void test_object_6();
void test_object_7();
void test_time();
void test_user_fncs();
void test_class();
void test_measure();
void test_from_files();

#ifdef RCDEBUG_LINE_NUMBERS
namespace Y { namespace __Y__ {
	void _rc_linemarker_init();
	void _rc_linemarker_print();
} }
#endif
















namespace Z {
	const char *code[]={
		"function main():",
		"	return 'hello world'",
		NULL};


	owca_global execute(owca_vm &vm, const owca_global &exc, const std::string &member, int param=-1)
	{
		owca_global z,res;
		owca_function_return_value r=exc.get_member(z,member);
		
		if (r==owca_function_return_value::RETURN_VALUE) {
			if (param>=0) {
				r=z.call(res,param);
			}
			else {
				r=z.call(res);
			}

			if (r==owca_function_return_value::RETURN_VALUE) return res;
		}
		return owca_global();
	}

		
	void print_exception(owca_global &exc)
	{
		owca_global res=execute(*exc.vm(),exc,"code");
		exceptioncode code=(exceptioncode)
			(res.int_is() ? res.int_get() : exceptioncode::YEXCEPTION_NONE);

		printf("exception: %s\n",exceptioncode_text(code));

		// size of stack bound with exception object
		res=execute(*exc.vm(),exc,"size");
		owca_int lines_count=res.int_is() ? res.int_get() : 0;

		for(unsigned int i=0;i<lines_count;++i) {
			owca_global e=execute(*exc.vm(),exc,"$read_1",i);
			owca_tuple t=e.tuple_get();
			
			std::string function_name=t.get(0).str();
			std::string filename_name=t.get(1).str();
			std::string function_line=t.get(2).str();

			printf("%s: %s: %s\n",filename_name.c_str(),function_line.c_str(),
					function_name.c_str());
		}
	}

	int main()
	{
		owca_vm vm;	
		owca_global result,namespace_object;
		owca_namespace nspace;
		owca_message_list errorlist;
		
		namespace_object=vm.create_namespace("foo.y");
		nspace=namespace_object.namespace_get();

		owca_function_return_value r=vm.compile(result,nspace,errorlist,owca_source_file_Text_array(code));
		
		if (errorlist.has_errors()) {
			printf("compilation failed.\n");
			for(owca_message_list::T it=errorlist.begin();it!=errorlist.end();++it) {
				printf("%3d: %s\n",it->line(),it->text().c_str());
			}
		}
		else {
			switch(r.type()) {
			// this should never happen, when compiling
			case owca_function_return_value::RETURN_VALUE: 
				break;
				
			// main block executed without problems
			case owca_function_return_value::NO_RETURN_VALUE: 			
				break;
				
			// main block raised an exception
			// result contains an exception object
			case owca_function_return_value::EXCEPTION: 
				printf("an exception is raised.\n");
				print_exception(result);
				break;
			}
		
			if (r.type()==owca_function_return_value::NO_RETURN_VALUE) {
				owca_global mainfnc=nspace["main"];
				r=mainfnc.call(result);
			
				switch(r.type()) {
				// function returned value (either by return or by yield)
				case owca_function_return_value::RETURN_VALUE:
					printf("function returned '%s'.\n",result.str().c_str());
					break;
					
				// function returned without value (either by return, 
				// or by falling out of the scope)
				case owca_function_return_value::NO_RETURN_VALUE: 			
					printf("function returned without value.\n");
					break;
					
				// function raised an exception
				// result contains an exception object
				case owca_function_return_value::EXCEPTION: 
					printf("an exception is raised.\n");
					print_exception(result);
					break;
				}
			
			}
		}
		return 0;
	}
}

#ifdef _WIN32
#include <windows.h>

LONG WINAPI global_exception_filter(EXCEPTION_POINTERS *)
{
    ExitProcess(0xdedd000D);
}
#endif

int main()
{
#ifdef _WIN32
	SetUnhandledExceptionFilter(global_exception_filter);
#endif

	Z::main();

	test_operator_add();
	test_operator_sub();
	test_operator_mul();
	test_operator_div();

	test_operator_mod();
	test_operator_lshift();
	test_operator_rshift();

	test_operator_binand();
	test_operator_binor();
	test_operator_binxor();

	test_operator_cmp_eq();
	test_operator_cmp_noteq();
	test_operator_cmp_less();
	test_operator_cmp_more();
	test_operator_cmp_lesseq();
	test_operator_cmp_moreeq();

	test_operator_sign();
	test_operator_binnot();

	return 0;
}
