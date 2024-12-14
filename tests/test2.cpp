#define DLLEXPORT
#define _CRT_SECURE_NO_WARNINGS

#include "owca.h"
#include "owca/base.h"
#include "owca/exec_base.h"
#include <windows.h>
#undef min
#undef max

using namespace owca;

void test_object_init(owca_vm &vm, owca_vm &vm2, owca_namespace &nspace, owca_namespace &nspace2)
{
	owca_message_list ml;
	owca_global rv;
	owca_function_return_value r;

	r=vm.compile(rv,nspace,ml,owca_source_file_Text(
		"class A:\n"
		"	function $init(v):\n"
		"		self.aa=v\n"
		"	function $str(): return self.aa\n"
		"	function $add(v): return self.aa+$str(v)\n"
		"	function $radd(v): return $str(v)+self.aa\n"
		"	function $less(v): return $str(self)<$str(v)\n"
		"	function $rless(v): return $str(v)<$str(self)\n"
		"	function b(): return [1,2,3,4]\n"
		"class B:\n"
		"	function $less(v): raise $exception()\n"
		"	function $rless(v): raise $exception()\n"
		"	function $eq(v): raise $exception()\n"
		"	function $noteq(v): raise $exception()\n"
		"	function $add(v): raise $exception()\n"
		"	function $radd(v): raise $exception()\n"
		"function f():\n"
		"	pass\n"
		"$str.a=1\n"
		"string='abc'\n"
		"a=A('abc')\n"
		"b=B()\n"
		"ff=f.bind(f)\n"
		));
	RCASSERT(r==owca_function_return_value::RETURN_VALUE && rv.no_value_is());

	if (ml.has_errors() || ml.has_warnings()) {
		for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
			printf("%5d:      %s\n",it->line(),it->text().c_str());
		}
		RCASSERT(0);
	}
	r=vm2.compile(rv,nspace2,ml,owca_source_file_Text(
		"class A:\n"
		"	function $init(v): self.aa=v\n"
		"	function $str(): return self.aa\n"
		"	function $add(v): return self.aa+$str(v)\n"
		"	function $radd(v): return $str(v)+self.aa\n"
		"	function $less(v): return $str(self)<$str(v)\n"
		"	function $rless(v): return $str(v)<$str(self)\n"
		"	function b(): return [1,2,3,4]\n"
		"class B:\n"
		"	function $less(v): raise $exception()\n"
		"	function $rless(v): raise $exception()\n"
		"	function $req(v): raise $exception()\n"
		"	function $rnoteq(v): raise $exception()\n"
		"	function $add(v): raise $exception()\n"
		"	function $radd(v): raise $exception()\n"
		"string='def'\n"
		"function f():\n"
		"	pass\n"
		"$str.a=2\n"
		"a=A('def')\n"
		"b=B()\n"
		"ff=f.bind(f)\n"
		));
	RCASSERT(r==owca_function_return_value::RETURN_VALUE && rv.no_value_is());
	if (ml.has_errors() || ml.has_warnings()) {
		for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
			printf("%5d:      %s\n",it->line(),it->text().c_str());
		}
		RCASSERT(0);
	}
}

void test_object()
{
	{
		owca_vm vm;
	}
	{
		owca_vm vm;
		__owca__::exec_ref_counter_enable(false);
		{
			owca_vm vm2;
		}
		__owca__::exec_ref_counter_enable(true);
		//owca_global gnspace=vm.create_namespace(),gnspace2=vm2.create_namespace();
		//owca_namespace nspace=gnspace.namespace_get(),nspace2=gnspace2.namespace_get();
		//test_object_init(vm,vm2,nspace,nspace2);
	}
	{
		owca_vm vm;
		__owca__::exec_ref_counter_enable(false);
		{
			owca_vm vm2;
			owca_global gnspace=vm.create_namespace("main.ys"),gnspace2=vm2.create_namespace("main2.ys");
			owca_namespace nspace=gnspace.namespace_get(),nspace2=gnspace2.namespace_get();
			test_object_init(vm,vm2,nspace,nspace2);

			{
				owca_global l=vm.list();
				owca_global m=vm.map();
				owca_global l2=vm.list();
				owca_global t=vm.tuple();
				l.list_get().resize(4);
				vm.run_gc();
				l.list_get()[1].string_set("abcde\0f");
				RCASSERT(l.list_get()[1].string_get()=="abcde");
				vm.run_gc();
				l.list_get()[1].string_set("abcde\0f",7);
				RCASSERT(l.list_get()[1].string_get()==std::string("abcde\0f",7));
				vm.run_gc();
				l.list_get()[1].map_set(m.map_get());
				RCASSERT(l.list_get()[1].is(m));
				vm.run_gc();
				l.list_get()[1].list_set(l2.list_get());
				RCASSERT(l.list_get()[1].is(l2));
				vm.run_gc();
				l.list_get()[1].tuple_set(t.tuple_get());
				RCASSERT(l.list_get()[1].is(t));
				vm.run_gc();
			}

			owca_global A=nspace["A"],a,am,am2;
			RCASSERT(A.call(a,"dfgh")==owca_function_return_value::RETURN_VALUE);

			RCASSERT(a.get_member(am,"abc")==owca_function_return_value::RETURN_VALUE);
			//RCASSERT(am.get_member(am2,"code")==owca_function_return_value::RETURN_VALUE && am2.call(am2)==owca_function_return_value::RETURN_VALUE && am2.int_get()==ExceptionCode::MISSING_MEMBER);
			RCASSERT(a.set_member(am,"abc","qwer")==owca_function_return_value::RETURN_VALUE && am.string_get()=="qwer");
			RCASSERT(a.set_member(am,"abc",746)==owca_function_return_value::RETURN_VALUE && am.int_get()==746);
			RCASSERT(a.get_member(am,"abc")==owca_function_return_value::RETURN_VALUE && am.int_get()==746);
			RCASSERT(a.set_member(am,"abc",9999)==owca_function_return_value::RETURN_VALUE && am.int_get()==9999);
			RCASSERT(a.get_member(am,"abc")==owca_function_return_value::RETURN_VALUE && am.int_get()==9999);

			RCASSERT(a.get_member(am,"def")==owca_function_return_value::RETURN_VALUE);

			RCASSERT(a.set_member(am,"$abc",1)==owca_function_return_value::EXCEPTION);
			RCASSERT(am.get_member(am2,"code")==owca_function_return_value::RETURN_VALUE && am2.call(am2)==owca_function_return_value::RETURN_VALUE && am2.int_get()==(int)ExceptionCode::NOT_LVALUE);

			{
				owca_global g;
				RCASSERT(a.get_member(g,"$str")==owca_function_return_value::RETURN_VALUE);
				RCASSERT(g.call(g)==owca_function_return_value::RETURN_VALUE);

				RCASSERT(g.string_get()=="dfgh");
				RCASSERT(g.string_get()[0]=='d');
				RCASSERT(g.string_get()[1]=='f');
				RCASSERT(g.string_get()[-1]=='h');
				RCASSERT(g.string_get()[-2]=='g');
			}
			{
				owca_global g;
				RCASSERT(a.get_member(g, "b") == owca_function_return_value::RETURN_VALUE);
				RCASSERT(g.call(g) == owca_function_return_value::RETURN_VALUE);

				RCASSERT(g.get_member(am, "size") == owca_function_return_value::RETURN_VALUE);
				RCASSERT(am.call(am) == owca_function_return_value::RETURN_VALUE && am.int_get() == 4);

				{
					owca_list l = g.list_get();
					RCASSERT(l.size() == 4);
					RCASSERT(l[0].int_get() == 1);
					RCASSERT(l[1].int_get() == 2);
					RCASSERT(l[2].int_get() == 3);
					RCASSERT(l[3].int_get() == 4);
					l[0];
					try {
						l[4];
						RCASSERT(0);
					}
					catch (owca_exception &exc) {
					}
					try {
						l.get(4);
						RCASSERT(0);
					}
					catch (owca_exception &exc) {
					}
					try {
						l.set(4, "123");
						RCASSERT(0);
					}
					catch (owca_exception &exc) {
					}
				}

				{
					a = nspace["$tuple"];
					RCASSERT(a.call(a, g) == owca_function_return_value::RETURN_VALUE);
					RCASSERT(a.get_member(am, "size") == owca_function_return_value::RETURN_VALUE);
					RCASSERT(am.call(am) == owca_function_return_value::RETURN_VALUE && am.int_get() == 4);

					owca_tuple l = a.tuple_get();
					RCASSERT(l.size() == 4);
					RCASSERT(l[0].int_get() == 1);
					RCASSERT(l[1].int_get() == 2);
					RCASSERT(l[2].int_get() == 3);
					RCASSERT(l[3].int_get() == 4);
					l[0];
					try {
						l[4];
						RCASSERT(0);
					}
					catch (owca_exception &exc) {
					}
					try {
						l.get(4);
						RCASSERT(0);
					}
					catch (owca_exception &exc) {
					}
				}
			}
			
			am="abc";
			RCASSERT(am.get_member(am, "size") == owca_function_return_value::RETURN_VALUE);
			//RCASSERT(am.get_member(am2,"code")==owca_function_return_value::RETURN_VALUE && am2.call(am2)==owca_function_return_value::RETURN_VALUE && am2.int_get()==ExceptionCode::NOT_LVALUE);
		}
		__owca__::exec_ref_counter_enable(true);
	}
	{
		owca_vm vm;
		__owca__::exec_ref_counter_enable(false);
		{
			owca_vm vm2;
			owca_global gnspace=vm.create_namespace("main.ys"),gnspace2=vm2.create_namespace("main2.ys");
			owca_namespace nspace=gnspace.namespace_get(),nspace2=gnspace2.namespace_get();
			test_object_init(vm,vm2,nspace,nspace2);
			owca_global A=nspace["A"],A2=nspace2["A"];
			owca_global a,b,am,am2;
			
			RCASSERT(A.call(a,"abc")==owca_function_return_value::RETURN_VALUE);
			RCASSERT(A2.call(b,"def")==owca_function_return_value::RETURN_VALUE);
			owca_global c="ghi";
			owca_global r(vm);

			try {
				r.set_member(am,"abc",b);
				RCASSERT(0);
			} catch(owca_exception &exc) {
			}

			try {
				r.call(am,b);
				RCASSERT(0);
			} catch(owca_exception &exc) {
			}

			try {
				r.call(am,a,b);
				RCASSERT(0);
			} catch(owca_exception &exc) {
			}
			try {
				r.call(am,b,a);
				RCASSERT(0);
			} catch(owca_exception &exc) {
			}

			try {
				r.call(am,a,a,b);
				RCASSERT(0);
			} catch(owca_exception &exc) {
			}
			try {
				r.call(am,a,b,a);
				RCASSERT(0);
			} catch(owca_exception &exc) {
			}
			try {
				r.call(am,b,a,a);
				RCASSERT(0);
			} catch(owca_exception &exc) {
			}

			owca_call_parameters cp;
			cp.parameters.push_back(a);
			cp.parameters.push_back(b);
			try {
				r.call(am,cp);
				RCASSERT(0);
			} catch(owca_exception &exc) {
			}
		}
		__owca__::exec_ref_counter_enable(true);
	}

	__owca__::exec_ref_counter_enable(true);
	printf("object test done\n");
}



void test_namespace()
{
	owca_vm vm;
	owca_global gnspace=vm.create_namespace("main.ys"),res;
	owca_namespace nspace=gnspace.namespace_get();
	owca_message_list ml;
	
	RCASSERT(vm.compile(res,nspace,ml,owca_source_file_Text(
		"throw=false\n"
		"class T1:\n"
		"	function $init(v): self.v=v\n"
		"	function $hash(): return 0\n"
		"	function $eq(v):\n"
		"		if throw: raise $exception()\n"
		"		return self.v==v.v\n"
		"	function read fread(): raise $exception()\n"
		"class T2:\n"
		"	function $str(): raise $exception()\n"
		"function t1():\n"
		"	m={T1('a'):T1('b'),T1('c'):T1('d')}\n"
		"	throw:=true\n"
		"	$print('abc',**m)\n"
		"	return 'ok'\n"
		"function t2(a):\n"
		"	$print('abc',join=a)\n"
		"function t3():\n"
		"	$print('abc',T2(),'def',join='ghi')\n"
		"function t4():\n"
		"	$print(*['abc','def'],join='ghi')\n"
		"	try:\n"
		"		$print(*['abc',T2(),'def'],join='ghi')\n"
		"		return 1\n"
		"	except $exception as e:\n"
		"		if $type(e) is not $exception: return 2\n"
		"	try:\n"
		"		$print('abc',T2(),'def',join='ghi')\n"
		"		return 3\n"
		"	except $exception as e:\n"
		"		if $type(e) is not $exception: return 4\n"
		"	return 'ok'\n"
		"function t5():\n"
		"	$range('a','b','c','d')\n"
		"function t6():\n"
		"	if $list($range(4))!=[0,1,2,3]: return 1\n"
		"	if $list($range(-4))!=[0,-1,-2,-3]: return 2\n"
		"	if $list($range(0))!=[]: return 3\n"
		"	if $list($range(2,5))!=[2,3,4]: return 4\n"
		"	if $list($range(5,2))!=[5,4,3]: return 5\n"
		"	if $list($range(5,5))!=[]: return 6\n"
		"	if $list($range(2,10,2))!=[2,4,6,8]: return 7\n"
		"	if $list($range(10,2,-2))!=[10,8,6,4]: return 8\n"
		"	return 'ok'\n"
		"function t7():\n"
		"	loop : for q = (\n"
		"		('a'),\n"
		"		(5,'a'),\n"
		"		('a',5),\n"
		"		(5,10,'a'),\n"
		"		(5,'a',10),\n"
		"		('a',5,10),\n"
		"		(2,10,0),\n"
		"		(2,10,-1),\n"
		"		(2,-10,1),\n"
		"		):\n"
		"		if $type(q) is not $tuple: q=$tuple([q])\n"
		"		for x = $range(q.size()+1):\n"
		"			$print(q,loop,x)\n"
		"			try:\n"
		"				if x==0: $range(*q)\n"
		"				elif x==1: $range(q[0],*q[1:])\n"
		"				elif x==2: $range(q[0],q[1],*q[2:])\n"
		"				elif x==3: $range(q[0],q[1],q[2])\n"
		"				else: return loop+0.125*x\n"
		"			except:\n"
		"				pass\n"
		"	return 'ok'\n"
		"function t8():\n"
		"	t=T1('abc')\n"
		"	if $memberget(t,'v')!='abc': return 1\n"
		"	if $memberget(t,'abc','def')!='def': return 2\n"
		"	try:\n"
		"		return $memberget(t,'abc')\n"
		"	except:\n"
		"		pass\n"
		"	try:\n"
		"		return $memberget(t,'fread')\n"
		"	except:\n"
		"		pass\n"
		"	return 'ok'\n"
		"function t9():\n"
		"	t=T1('def')\n"
		"	if $memberpop(t,'v')!='def': return 1\n"
		"	try:\n"
		"		return $memberpop(t,'v')\n"
		"	except:\n"
		"		pass\n"
		"	if $memberpop(t,'v','abc')!='abc': return 2\n"
		"	try:\n"
		"		return $memberpop('abc','v')\n"
		"	except:\n"
		"		pass\n"
		"	if $memberpop('abc','v','abc')!='abc': return 3\n"
		"	$memberset(t,'a','ok')\n"
		"	return $memberpop(t,'a')\n"
		"function t10(a):\n"
		"	class T:\n"
		"		function $str(): return a\n"
		"	return $str(T())\n"
		"function t11(a,b,c,d):\n"
		"	yield '1'+a+b+c+d\n"
		"	yield '2'+a+b+c+d\n"
		"	yield '3'+a+b+c+d\n"
		"function t12(a,b,*p,**m):\n"
		"	yield '1'+a+b+p[0]+p[1]+m['c']+m['2']\n"
		"	yield '2'+a+b+p[0]+p[1]+m['c']+m['2']\n"
		"	yield '3'+a+b+p[0]+p[1]+m['c']+m['2']\n"
		))==owca_function_return_value::RETURN_VALUE);
	if (ml.has_errors() || ml.has_warnings()) {
		for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
			printf("%5d:      %s\n",it->line(),it->text().c_str());
		}
		RCASSERT(0);
	}
	owca_global r,am,am2;

	am=nspace["t1"];
	RCASSERT(am.call(am2)==owca_function_return_value::EXCEPTION);
	RCASSERT(am2.get_member(am2,"code")==owca_function_return_value::RETURN_VALUE && am2.call(am2)==owca_function_return_value::RETURN_VALUE && am2.int_get()==(int)ExceptionCode::KEYWORD_PARAM_NOT_STRING);

	am=nspace["t2"];
	RCASSERT(am.call(am2,"abc")==owca_function_return_value::RETURN_VALUE);

	am=nspace["t3"];
	RCASSERT(am.call(am2)==owca_function_return_value::EXCEPTION);
	RCASSERT(am2.get_member(am2,"code")==owca_function_return_value::RETURN_VALUE && am2.call(am2)==owca_function_return_value::RETURN_VALUE && am2.int_get()==(int)ExceptionCode::USER);

	am=nspace["t4"];
	RCASSERT(am.call(r)==owca_function_return_value::RETURN_VALUE);
	RCASSERT(r.string_get()=="ok");

	am=nspace["t5"];
	RCASSERT(am.call(am2)==owca_function_return_value::EXCEPTION);
	RCASSERT(am2.get_member(am2,"code")==owca_function_return_value::RETURN_VALUE && am2.call(am2)==owca_function_return_value::RETURN_VALUE && am2.int_get()==(int)ExceptionCode::TOO_MANY_PARAMETERS);

	am=nspace["t6"];
	RCASSERT(am.call(r)==owca_function_return_value::RETURN_VALUE);
	RCASSERT(r.string_get()=="ok");

	am=nspace["t7"];
	RCASSERT(am.call(r)==owca_function_return_value::RETURN_VALUE);
	RCASSERT(r.string_get()=="ok");

	am=nspace["t8"];
	RCASSERT(am.call(r)==owca_function_return_value::RETURN_VALUE);
	RCASSERT(r.string_get()=="ok");

	am=nspace["t9"];
	RCASSERT(am.call(r)==owca_function_return_value::RETURN_VALUE);
	RCASSERT(r.string_get()=="ok");

	am=nspace["t10"];
	RCASSERT(am.call(r,"abc")==owca_function_return_value::RETURN_VALUE);
	RCASSERT(r.string_get()=="abc");

	{
		owca_call_parameters cp;
		cp.parameters.push_back("a");
		cp.parameters.push_back("b");
		cp.parameters.push_back("c");
		cp.parameters.push_back("d");
		am=nspace["t11"];
		RCASSERT(am.call(r,cp)==owca_function_return_value::RETURN_VALUE);
		RCASSERT(r.call(am2)==owca_function_return_value::RETURN_VALUE);
		RCASSERT(am2.string_get()=="1abcd");
		RCASSERT(r.call(am2)==owca_function_return_value::RETURN_VALUE);
		RCASSERT(am2.string_get()=="2abcd");
		RCASSERT(r.call(am2)==owca_function_return_value::RETURN_VALUE);
		RCASSERT(am2.string_get()=="3abcd");
		RCASSERT(r.call(am2)==owca_function_return_value::RETURN_VALUE);
	}

	{
		{
			owca_global lst=vm.list();
			lst.list_get().push_back("c");
			lst.list_get().push_back("d");
			owca_global mp=vm.map();
			owca_global wr1,am;
			RCASSERT(mp.get_member(wr1,"$write_1")==owca_function_return_value::RETURN_VALUE);
			RCASSERT(wr1.call(am,"c","e")==owca_function_return_value::RETURN_VALUE);
			RCASSERT(wr1.call(am,"2","f")==owca_function_return_value::RETURN_VALUE);

			owca_call_parameters yp;
			yp.parameters.push_back("a");
			yp.parameters.push_back("b");
			yp.list_parameter=lst;
			yp.map_parameter=mp;
			am=nspace["t12"];
			RCASSERT(am.call(r,yp)==owca_function_return_value::RETURN_VALUE);
		}
		RCASSERT(r.call(am2)==owca_function_return_value::RETURN_VALUE);
		RCASSERT(am2.string_get()=="1abcdef");
		RCASSERT(r.call(am2)==owca_function_return_value::RETURN_VALUE);
		RCASSERT(am2.string_get()=="2abcdef");
		RCASSERT(r.call(am2)==owca_function_return_value::RETURN_VALUE);
		RCASSERT(am2.string_get()=="3abcdef");
		RCASSERT(r.call(am2)==owca_function_return_value::RETURN_VALUE);
	}
}

static owca_function_return_value func_param_0(owca_global &retval, owca_namespace &nspace)
{
	retval="qwerty";
	return owca_function_return_value::RETURN_VALUE;
}

static owca_function_return_value func_param_1(owca_global &retval, owca_namespace &nspace, const owca_global &p1)
{
	retval=p1;
	return owca_function_return_value::RETURN_VALUE;
}

static owca_function_return_value func_param_2(owca_global &retval, owca_namespace &nspace, const owca_global &p1, const owca_global &p2)
{
	owca_function_return_value r;
	owca_global z;

	r=p1.get_member(z,"$add");
	if (r==owca_function_return_value::EXCEPTION) { retval=z; return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);

	r=z.call(retval,p2);
	if (r==owca_function_return_value::EXCEPTION) { return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);
	
	return owca_function_return_value::RETURN_VALUE;
}

static owca_function_return_value func_param_3(owca_global &retval, owca_namespace &nspace, const owca_global &p1, const owca_global &p2, const owca_global &p3)
{
	owca_function_return_value r;
	owca_global z;

	r=p1.get_member(z,"$add");
	if (r==owca_function_return_value::EXCEPTION) { retval=z; return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);

	r=z.call(z,p2);
	if (r==owca_function_return_value::EXCEPTION) { retval=z; return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);
	
	r=z.get_member(z,"$add");
	if (r==owca_function_return_value::EXCEPTION) { retval=z; return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);

	r=z.call(retval,p3);
	if (r==owca_function_return_value::EXCEPTION) { return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);

	return owca_function_return_value::RETURN_VALUE;
}

static owca_function_return_value func_param_4(owca_global &retval, owca_namespace &nspace, const owca_global &p1, const owca_global &p2, const owca_global &p3, const owca_global &p4)
{
	owca_function_return_value r;
	owca_global z;

	r=p1.get_member(z,"$add");
	if (r==owca_function_return_value::EXCEPTION) { retval=z; return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);

	r=z.call(z,p2);
	if (r==owca_function_return_value::EXCEPTION) { retval=z; return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);
	
	r=z.get_member(z,"$add");
	if (r==owca_function_return_value::EXCEPTION) { retval=z; return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);

	r=z.call(z,p3);
	if (r==owca_function_return_value::EXCEPTION) { retval=z; return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);
	
	r=z.get_member(z,"$add");
	if (r==owca_function_return_value::EXCEPTION) { retval=z; return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);

	r=z.call(retval,p4);
	if (r==owca_function_return_value::EXCEPTION) { return owca_function_return_value::EXCEPTION; }
	RCASSERT(r==owca_function_return_value::RETURN_VALUE);

	return owca_function_return_value::RETURN_VALUE;
}

void test_object_2()
{
	owca_vm vm;
	owca_global gnspace=vm.create_namespace("main.ys"),ret;
	owca_namespace nspace=gnspace.namespace_get();
	owca_message_list ml;

	RCASSERT(vm.compile(ret,nspace,ml,owca_source_file_Text(
		"class E:\n"
		"	function $req(v): return $str(v)+'f'\n"
		"	function $eq(v): return $str(v)+'e'\n"
		"	function $rless(v): return $str(v)+'d'\n"
		"	function $less(v): return $str(v)+'c'\n"
		"	function $read_1(i1): raise $exception()\n"
		"	function $read_2(i1,i2): raise $exception()\n"
		"	function $write_1(i1,v): raise $exception()\n"
		"	function $write_2(i1,i2,v): raise $exception()\n"
		"	function $add(v): raise $exception()\n"
		"	function read r(): raise $exception()\n"
		"	function read r(): raise $exception()\n"
		"	function write w(v): raise $exception()\n"
		"	function write w(v): raise $exception()\n"
		"	function write rw(v): raise $exception()\n"
		"	function read rw(): raise $exception()\n"
		"	function f2(a,b):\n"
		"		c=a,b\n"
		"		raise $exception()\n"
		"	function f3(a,b,c):\n"
		"		c=a,b,c\n"
		"		raise $exception()\n"
		"	function f4(a,b,c,d):\n"
		"		c=a,b,c,d\n"
		"		raise $exception()\n"
		"	function f5(a,b,c,d,e):\n"
		"		c=a,b,c,d,e\n"
		"		raise $exception()\n"
		"	function f6(a,b,c,d,e,f):\n"
		"		c=a,b,c,d,e,f\n"
		"		raise $exception()\n"
		"	function f7(a,b,c,d,e,f,g):\n"
		"		c=a,b,c,d,e,f,g\n"
		"		raise $exception()\n"
		"	function f8(a,b,c,d,e,f,g,h):\n"
		"		c=a,b,c,d,e,f,g,h\n"
		"		raise $exception()\n"
		"	function f4_(a,b,c,d):\n"
		"		return a+b+c+d\n"
		"	function f5_(a,b,c,d,e):\n"
		"		return a+b+c+d+e\n"
		"	function f6_(a,b,c,d,e,f):\n"
		"		return a+b+c+d+e+f\n"
		"	function f7_(a,b,c,d,e,f,g):\n"
		"		return a+b+c+d+e+f+g\n"
		"	function f8_(a,b,c,d,e,f,g,h):\n"
		"		return a+b+c+d+e+f+g+h\n"
		"class F:\n"
		"	function $req(v): return true\n"
		"	function $eq(v): return true\n"
		"	function $rless(v): return true\n"
		"	function $less(v): return true\n"
		"	function $read_1(i1): return $str(i1)+'a'\n"
		"	function $read_2(i1,i2): return $str(i1)+'a'+$str(i2)\n"
		"	function $write_1(i1,v): return $str(i1)+'b'+$str(v)\n"
		"	function $write_2(i1,i2,v): return $str(i1)+'b'+$str(i2)+$str(v)\n"
		"	function $add(v): return $str(v)+'+'\n"
		"	function f2(a,b):\n"
		"		c=a,b\n"
		"	function f3(a,b,c):\n"
		"		c=a,b,c\n"
		"	function f4(a,b,c,d):\n"
		"		c=a,b,c,d\n"
		"class T:\n"
		"	function $eq(v): return false\n"
		"	function $noteq(v): return false\n"
		"	function $init(v): self.v=v\n"
		"	function $str(): return $str(self.v)\n"
		"	function f(): pass\n"
		"	function $hash(): return $hash(self.v)\n"
		"function ff1(): pass\n"
		"ff2=ff1.bind(ff1)\n"
		"$str.a=1\n"
		"e=E()\n"
		"f=F()\n"
		"s1='abc'\n"
		"s2='def'\n"
		"s3='ghi'\n"
		"s1s2='abcdef'\n"
		"t1=T('abc')\n"
		"t2=T('def')\n"
		"t3=T('ghi')\n"
		"function test1(q):\n"
		"	if q: return 1\n"
		"	return 0\n"
		"function test2():\n"
		"	class T:\n"
		"		function $bool(): pass\n"
		"	if T(): return 1\n"
		"	return 0\n"
		"function test3(a):\n"
		"	if a[0]!='a': return 1\n"
		"	if a[-3]!='a': return 2\n"
		"	try:\n"
		"		return a['abc']\n"
		"	except:\n"
		"		return 'ok'\n"
		"function test4(a):\n"
		"	if a[0:2]!='ab': return 1\n"
		"	if a[-3:-1]!='ab': return 2\n"
		"	try:\n"
		"		return a['abc':'def']\n"
		"	except:\n"
		"		return 'ok'\n"
		"function test5(a1,a2,b1,b2):\n"
		"	if a1>=b1: return 1\n"
		"	if a2>=b1: return 2\n"
		"	if a1>=b2: return 3\n"
		"	if a2>=b2: return 4\n"
		"	if a1==b1: return 5\n"
		"	if a2==b1: return 6\n"
		"	if a1==b2: return 7\n"
		"	if a2==b2: return 8\n"
		"	if a1==test5: return 9\n"
		"	if a2==test5: return 10\n"
		"	if test5==a1: return 11\n"
		"	if test5==a2: return 12\n"
		"	if test5!=test5: return 13\n"
		"	if test5.bind(test5)==test5: return 14\n"
		"	if test5==test5.bind(test5): return 15\n"
		"	if test5.bind(test5)!=test5.bind(test5): return 16\n"
		"	if test4==test5: return 13\n"
		"	if test4.bind(test5)==test5: return 14\n"
		"	if test4==test5.bind(test5): return 15\n"
		"	if test4.bind(test5)==test5.bind(test5): return 16\n"
		"	if null: return 17\n"
		"	return 'ok'\n"
		"function test_add(a,b):\n"
		"	return a+b\n"
		"function test_mul(a,b):\n"
		"	return a*b\n"
		"function test_call_1(*params):\n"
		"	fnc=params[0]\n"
		"	if params.size()==1: return fnc()\n"
		"	if params.size()==2: return fnc(params[1])\n"
		"	if params.size()==3: return fnc(params[1],params[2])\n"
		"	if params.size()==4: return fnc(params[1],params[2],params[3])\n"
		"	if params.size()==5: return fnc(params[1],params[2],params[3],params[4])\n"
		"	raise $exception()\n"
		"function test_call_2(*params):\n"
		"	fnc=params[0]\n"
		"	return fnc(*params[1:])\n"
		"	raise $exception()\n"
		"function test_call_3(*params):\n"
		"	fnc=params[0]\n"
		"	if params.size()==1: return fnc(*[])\n"
		"	if params.size()==2: return fnc(params[1],*[])\n"
		"	if params.size()==3: return fnc(params[1],params[2],*[])\n"
		"	if params.size()==4: return fnc(params[1],params[2],params[3],*[])\n"
		"	if params.size()==5: return fnc(params[1],params[2],params[3],params[4],*[])\n"
		"	raise $exception()\n"
		"function test_call_4(a,b,d,c):\n"
		"	return a(b,**{d:c})\n"
		"test1_b=test1.bind(test1)\n"
		"test2_b=test2.bind(test1)\n"
		"test1_c=test1.bind(test1_b)\n"
		"test1_e=test1.bind(e)\n"
		"test1_d=test1.bind(test1_e)\n"
		))==owca_function_return_value::RETURN_VALUE);
	if (ml.has_errors() || ml.has_warnings()) {
		for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
			printf("%5d:      %s\n",it->line(),it->text().c_str());
		}
		RCASSERT(0);
	}
	owca_global e=nspace["e"],f=nspace["f"],s1=nspace["s1"],s2=nspace["s2"],s3=nspace["s3"],s1s2=nspace["s1s2"];
	owca_global t1=nspace["t1"],t2=nspace["t2"],t3=nspace["t3"],a,b,am;
	ExceptionCode exccode;

	nspace["func_param_0"].set(func_param_0);
	nspace["func_param_1"].set(func_param_1,"p1");
	nspace["func_param_1_"].set(func_param_1,"p1","abc");
	nspace["func_param_2"].set(func_param_2,"p1","p2");
	nspace["func_param_2_"].set(func_param_2,"p1","p2","abc","def");
	nspace["func_param_3"].set(func_param_3,"p1","p2","p3");
	nspace["func_param_3_"].set(func_param_3,"p1","p2","p3","abc","def","ghi");

	{
		owca_call_parameters cp;
		cp.parameters.push_back(nspace["func_param_1"]);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(t1);
		cp.parameters.push_back(s2);
		RCASSERT(nspace["test_call_4"].call(a,cp)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::KEYWORD_PARAM_NOT_STRING);
	}
	{
		owca_call_parameters cp;
		cp.parameters.push_back(nspace["func_param_1"]);
		cp.parameters.push_back(s1);
		cp.parameters.push_back("p1");
		cp.parameters.push_back(s2);
		RCASSERT(nspace["test_call_4"].call(a,cp)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::PARAM_ASSIGNED_TWICE);
	}

	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_0"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="qwerty");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_1"],s1)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abc");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_2"],s1,s2)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_3"],s1,s2,s1)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefabc");

	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_0"],s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)(exccode=(ExceptionCode)am.int_get()))==ExceptionCode::TOO_MANY_PARAMETERS);
	
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_1"])==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);

	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_1"],s1,s2)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);

	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_2"])==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);

	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_2"],s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);

	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_2"],s1,s2,s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);

	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_3"])==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);

	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_3"],s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);

	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_3"],s1,s2)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);

	{
		owca_call_parameters cp;
		cp.parameters.push_back(nspace["func_param_3"]);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		RCASSERT(nspace["test_call_3"].call(a,cp)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);

	}

	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_3_"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_3_"],"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_3_"],"abc","def")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_3_"],"abc","def","ghi")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_2_"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_2_"],"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_2_"],"abc","def")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_1_"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="abc");
	RCASSERT(nspace["test_call_3"].call(a,nspace["func_param_1_"],"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abc");

	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_0"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="qwerty");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_1"],s1)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abc");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_2"],s1,s2)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_3"],s1,s2,s1)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefabc");


	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_0"],s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_1"])==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_1"],s1,s2)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_2"])==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_2"],s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);

	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_2"],s1,s2,s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_3"])==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_3"],s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_3"],s1,s2)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);
	{
		owca_call_parameters cp;
		cp.parameters.push_back(nspace["func_param_3"]);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		RCASSERT(nspace["test_call_2"].call(a,cp)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);
	}


	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_3_"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_3_"],"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_3_"],"abc","def")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_3_"],"abc","def","ghi")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_2_"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_2_"],"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_2_"],"abc","def")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_1_"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="abc");
	RCASSERT(nspace["test_call_2"].call(a,nspace["func_param_1_"],"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abc");

	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_0"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="qwerty");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_1"],s1)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abc");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_2"],s1,s2)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_3"],s1,s2,s1)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefabc");


	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_0"],s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_1"])==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_1"],s1,s2)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_2"])==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_2"],s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);

	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_2"],s1,s2,s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_3"])==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_3"],s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_3"],s1,s2)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_ENOUGH_PARAMETERS);

	{
		owca_call_parameters cp;
		cp.parameters.push_back(nspace["func_param_3"]);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		RCASSERT(nspace["test_call_1"].call(a,cp)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::TOO_MANY_PARAMETERS);
	}

	//RCASSERT(nspace["test_call_1"](nspace["func_param_3_"]).string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_3_"],"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_3_"],"abc","def")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_3_"],"abc","def","ghi")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdefghi");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_2_"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_2_"],"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_2_"],"abc","def")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_1_"])==owca_function_return_value::RETURN_VALUE && a.string_get()=="abc");
	RCASSERT(nspace["test_call_1"].call(a,nspace["func_param_1_"],"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abc");

	RCASSERT(nspace["test_mul"].call(a,2,"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcabc");
	RCASSERT(nspace["test_mul"].call(a,2,s1)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcabc");
	RCASSERT(nspace["test_mul"].call(a,2.0,"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcabc");
	RCASSERT(nspace["test_mul"].call(a,2.0,s1)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcabc");
	RCASSERT(nspace["test_mul"].call(a,"abc",2.0)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcabc");

	RCASSERT(nspace["test_mul"].call(a,2.5,s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

	RCASSERT(nspace["test_mul"].call(a,2.5,"abc")==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

	RCASSERT(nspace["test_mul"].call(a,s1,2.5)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

	RCASSERT(nspace["test_mul"].call(a,"abc",2.5)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

	RCASSERT(nspace["test_mul"].call(a,"abc",-2)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INTEGER_OUT_OF_BOUNDS);
	RCASSERT(nspace["test_mul"].call(a,"abc",-2.0)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INTEGER_OUT_OF_BOUNDS);
	RCASSERT(nspace["test_mul"].call(a,"abc",f)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

	RCASSERT(nspace["test_mul"].call(a,s1,-2)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INTEGER_OUT_OF_BOUNDS);
	RCASSERT(nspace["test_mul"].call(a,s1,-2.0)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INTEGER_OUT_OF_BOUNDS);
	RCASSERT(nspace["test_mul"].call(a,s1,f)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

	RCASSERT(nspace["test5"].call(a,"abc",s1,"def",s2)==owca_function_return_value::RETURN_VALUE && a.string_get()=="ok");
	RCASSERT(nspace["test_add"].call(a,"abc","def")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcdef");

	a=nspace["test1"];
	RCASSERT(a.function_obj().null_is());
	a=a.function_bind(nspace["test1"]);
	RCASSERT(a.function_obj().is(nspace["test1"]));
	b=a.function_bind(a);
	RCASSERT(b.function_obj().is(a));
	b.null_set();
	a=b;


	try {
		owca_global().call(a);
		RCASSERT(0);
	} catch(owca_exception &exc) {
	}

	b=owca_global(vm);
	RCASSERT(b.call(a)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);
	RCASSERT(b.call(a,s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);
	RCASSERT(b.call(a,s1,s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);
	RCASSERT(b.call(a,s1,s1,s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);
	RCASSERT(b.call(a,s1,s1,s1,s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);
	{
		owca_call_parameters cp;
		cp.parameters.push_back(s1);
		RCASSERT(b.call(a,cp)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);
	}

	RCASSERT(e.get_member(a,"f2")==owca_function_return_value::RETURN_VALUE);
	RCASSERT(a.call(a,s1,s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::USER);

	RCASSERT(e.get_member(a,"f3")==owca_function_return_value::RETURN_VALUE);
	RCASSERT(a.call(a,s1,s1,s1)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::USER);

	RCASSERT(e.get_member(a,"f4")==owca_function_return_value::RETURN_VALUE);
	RCASSERT(a.call(a,s1,s1,s2,s2)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::USER);

	RCASSERT(e.get_member(a,"f4_")==owca_function_return_value::RETURN_VALUE);
	RCASSERT(a.call(a,s1,s1,s2,s2)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcabcdefdef");

	{
		owca_call_parameters cp;
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s1);
		RCASSERT(e.get_member(a,"f5")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(a.call(a,cp)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::USER);
		RCASSERT(e.get_member(a,"f5_")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(a.call(a,cp)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcabcdefdefabc");
	}

	{
		owca_call_parameters cp;
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s1);
		RCASSERT(e.get_member(a,"f6")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(a.call(a,cp)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::USER);
		RCASSERT(e.get_member(a,"f6_")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(a.call(a,cp)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcabcdefdefabcabc");
	}

	{
		owca_call_parameters cp;
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		RCASSERT(e.get_member(a,"f7")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(a.call(a,cp)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::USER);
		RCASSERT(e.get_member(a,"f7_")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(a.call(a,cp)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcabcdefdefabcabcdef");
	}

	{
		owca_call_parameters cp;
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s1);
		cp.parameters.push_back(s2);
		cp.parameters.push_back(s2);
		RCASSERT(e.get_member(a,"f8")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(a.call(a,cp)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::USER);
		RCASSERT(e.get_member(a,"f8_")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(a.call(a,cp)==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcabcdefdefabcabcdefdef");
	}

	try {
		owca_global g;
		g.function_bind(nspace["test1"]);
		RCASSERT(0);
	} catch(owca_exception &e) {
	}
	try {
		owca_global g;
		g.function_obj();
		RCASSERT(0);
	} catch(owca_exception &e) {
	}

	s1.type();
	e.type();
	f.type();

	a="abc";
	RCASSERT(a.string_get()[0]=='a');
	RCASSERT(a.string_get()[-3]=='a');
	try {
		a.string_get()[-5];
		RCASSERT(0);
	} catch(owca_exception &e) {
	}
	RCASSERT(a.string_get().get(0,3)=="abc");
	RCASSERT(a.string_get().get(-3,-1)=="ab");
	RCASSERT(a.string_get().get(-5,-1)=="ab");
	RCASSERT(a.string_get().get(-3,10)=="abc");
	RCASSERT(a.string_get().get(2,1)=="");
	RCASSERT(a.string_get().get(6,8)=="");

	RCASSERT(nspace["test4"].call(a,"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="ok");
	RCASSERT(nspace["test3"].call(a,"abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="ok");

	a=s1;
	RCASSERT(a.string_get()[0]=='a');
	RCASSERT(a.string_get()[-3]=='a');
	try {
		a.string_get()[-5];
		RCASSERT(0);
	} catch(owca_exception &e) {
	}
	RCASSERT(a.string_get().get(0,3)=="abc");
	RCASSERT(a.string_get().get(-3,-1)=="ab");
	RCASSERT(a.string_get().get(-5,-1)=="ab");
	RCASSERT(a.string_get().get(-3,10)=="abc");
	RCASSERT(a.string_get().get(2,1)=="");
	RCASSERT(a.string_get().get(6,8)=="");

	try {
		e.string_get().get(1,2);
		RCASSERT(0);
	} catch(owca_exception &e) {
	}

	RCASSERT(t1.get_member(a,"v")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abc");
	RCASSERT(t1.get_member(a,"__qwe__")==owca_function_return_value::RETURN_VALUE);
	RCASSERT(owca_global(vm).get_member(a,"__qwe__")==owca_function_return_value::RETURN_VALUE);
	RCASSERT(owca_global(vm).set_member(a,"__qwe__","qwe")==owca_function_return_value::RETURN_VALUE);
	try {
		owca_global().get_member(a,"__qwe__");
		RCASSERT(0);
	} catch(owca_exception &e) {
	}
	try {
		owca_global().set_member(a,"__qwe__","qwe");
		RCASSERT(0);
	} catch(owca_exception &e) {
	}

	RCASSERT(e.get_member(a,"r")==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::USER);

	RCASSERT(e.set_member(a,"w","rty")==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::USER);


	a=nspace["$int"];
	RCASSERT(a.get_member(a,"$new")==owca_function_return_value::RETURN_VALUE);
	RCASSERT(a.call(b,"-123")==owca_function_return_value::RETURN_VALUE && b.int_get()==-123);
	RCASSERT(a.call(b,"- 123")==owca_function_return_value::EXCEPTION);
	RCASSERT(b.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

	a=nspace["$real"];
	RCASSERT(a.get_member(a,"$new")==owca_function_return_value::RETURN_VALUE);
	RCASSERT(a.call(b,"-123.0")==owca_function_return_value::RETURN_VALUE && b.real_get()==-123.0);
	RCASSERT(a.call(b,"- 123.0")==owca_function_return_value::EXCEPTION);
	RCASSERT(b.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

	a=nspace["$str"];
	RCASSERT(a.get_member(a,"$new")==owca_function_return_value::RETURN_VALUE);
	RCASSERT(a.call(b,"abcdef")==owca_function_return_value::RETURN_VALUE && b.string_get()=="abcdef");

	RCASSERT(nspace["test1"].call(a,b)==owca_function_return_value::RETURN_VALUE && a.int_get()==1);
	RCASSERT(nspace["test2"].call(a)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);
}

static owca_function_return_value func_param_strings(owca_global &retval, owca_namespace &nspace, owca_int mode, const owca_global &s1, const owca_global &s2)
{
	switch(mode) {
	case 31: retval=s1.string_get().get(s2.int_get(),s2.int_get()+1); break;
	case 32: retval=s1.string_get().get(s2.int_get(),s2.int_get()<-4 ? s2.int_get()+4 : s1.string_get().character_count()); break;
	case 33: 
		try {
			retval=(owca_int)s1.string_get()[s2.int_get()]; 
		} catch(owca_exception &exc) {
			retval="_exception_";
		}
		break;
	case 41: retval=s1.string_get().hash()==s2.int_get(); break;
	case 42: retval=s1.string_get().str(); break;
	case 43: retval=s1.string_get().character_count()==s2.int_get(); break;
	case 44: retval=s1.string_get(); break;
	default:
		RCASSERT(0);
	}
	return owca_function_return_value::RETURN_VALUE;
}
//
static void test_2_printfunction(const std::string &s)
{
	printf("%s",s.c_str());
}

void test_object_3()
{
	owca_message_list ml;
	owca_vm vm;
	owca_global gnspace=vm.create_namespace("main.ys"),res;
	owca_namespace nspace=gnspace.namespace_get();
	
	nspace["fnc"].set(func_param_strings,"mode","p1","p2");
	RCASSERT(vm.compile(res,nspace,ml,owca_source_file_Text(
		"class exp_:\n"
		"	function $str(): return 'exception'\n"
		"exp=exp_()\n"
		"function main():\n"
		"	for mode,s1,s2,retval = (\n"
		"		(31,'abc',2,'c'),\n"
		"		(31,'abc',-2,'b'),\n"
		"		(31,'abc',6,''),\n"
		"		(31,'abc',-6,''),\n"
		"		(32,'abcdef',2,'cdef'),\n"
		"		(32,'abcdef',-2,'ef'),\n"
		"		(33,'abcdef',2,99),\n"
		"		(33,'abcdef',-2,101),\n"
		"		(33,'abc',6,'_exception_'),\n"
		"		(33,'abc',-6,'_exception_'),\n"
		"		(41,'abcdef',$hash('abcdef'),true),\n"
		"		(41,'abc',$hash('abc'),true),\n"
		"		(41,'abc',$hash('def'),false),\n"
		"		(42,'abc',null,'abc'),\n"
		"		(43,'abc',3,true),\n"
		"		(43,'abcdef',6,true),\n"
		"		(43,'abcdef',3,false),\n"
		"		(43,'',0,true),\n"
		"		(44,'abcdef',null,'abcdef'),\n"
		"		):\n"
		"		try:\n"
		"			rv=fnc(mode,s1,s2)\n"
		"			if rv!=retval:\n"
		"				$print(mode,s1,s2,'expected',retval)\n"
		"				$print('got',rv)\n"
		"				return false\n"
		"		except:\n"
		"			if retval is not exp:\n"
		"				$print(mode,s1,s2,'expected',retval)\n"
		"				$print('exception')\n"
		"				return false\n"
		"	return true\n"
		))==owca_function_return_value::RETURN_VALUE);
	if (ml.has_errors() || ml.has_warnings()) {
		for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
			printf("%5d:      %s\n",it->line(),it->text().c_str());
		}
		RCASSERT(0);
	}
	vm.set_print_function(test_2_printfunction);
	RCASSERT(nspace["main"].call(res)==owca_function_return_value::RETURN_VALUE && res.bool_get()==true);
}




static owca_function_return_value func_param_other(owca_global &retval, owca_namespace &nspace, const owca_parameters &parameters)
{
	owca_global params[5];

	if (!parameters.get_arguments(retval,NULL,params,5,5) || !parameters.get_keyword_arguments(retval,NULL,NULL,NULL,0)) return owca_function_return_value::EXCEPTION;
	int mode=(int)params[0].int_get();
	owca_global &p1=params[1];
	owca_global &p2=params[2];
	owca_global &p3=params[3];
	owca_global &p4=params[4];

	try {
		switch(mode) {
		case   1: retval=p1.tuple_get()[p2.int_get()]; break;
		case   2: retval=p1.tuple_get().get(p2.int_get()); break;
		case   3: retval=p1.tuple_get().get(p2.int_get(),p3.int_get()); break;
		case   4: retval=p1.tuple_get().size(); break;
		case   5: retval=p1.tuple_get(); break;
		case   6: retval=p1.tuple_get().clone(); break;

		case 101: retval=p1.list_get()[p2.int_get()]; break;
		case 102: retval=p1.list_get().size(); break;
		case 103: retval=p1.list_get().get(p2.int_get()); break;
		case 104: retval=p1.list_get().get(p2.int_get(),p3.int_get()); break;
		case 105:
			p1.list_get()[p2.int_get()]=p3;
			retval=p1;
			break;
		case 106: p1.list_get().set(p2.int_get(),p3); retval=p1; break;
		case 108: p1.list_get().resize((unsigned int)p2.int_get()); retval=p1; break;
		case 109: retval=p1.list_get(); break;
		case 110: retval=p1.list_get().clone(); break;

		case 114: p1.list_get().clear(); retval=p1; break;

		case 131: p1.list_get().push_front(p2); retval=p1; break;
		case 132: p1.list_get().push_back(p2); retval=p1; break;
		case 133: p1.list_get().insert(p2.int_get(),p3); retval=p1; break;
		case 134: retval=p1.list_get().pop_front(); break;
		case 135: retval=p1.list_get().pop_back(); break;
		case 136: retval=p1.list_get().pop(p2.int_get()); break;

		case 203: retval=p1.map_get(); break;
		case 204: retval=p1.map_get().clone(); break;
		case 205: retval=p1.map_get().size(); break;
		case 206: {
			owca_global g=nspace.vm().map();
			owca_global insert;
			RCASSERT(g.get_member(insert,"$write_1")==owca_function_return_value::RETURN_VALUE);
			owca_map_iterator mi;
			owca_map mp=p1.map_get();
			owca_global key,value;

			while(mp.next(mi,key,value)) {
				owca_global z;
				RCASSERT(insert.call(z,key,value)==owca_function_return_value::RETURN_VALUE);
			}
			retval=g; 
			break; }
		case 207:
			p1.map_get().clear();
			retval=p1;
			break;
		default:
			RCASSERT(0);
		}
	} catch(owca_exception &exc) {
		retval="_exception_";
	}

	return owca_function_return_value::RETURN_VALUE;
}

//
void test_object_4()
{
	owca_message_list ml;
	owca_vm vm;
	owca_global gnspace=vm.create_namespace("main.ys"),ret;
	owca_namespace nspace=gnspace.namespace_get();

	nspace["fnc"].set(func_param_other);
	RCASSERT(vm.compile(ret,nspace,ml,owca_source_file_Text(



		"class exp_:\n"
		"	function $str(): return 'exp'\n"
		"	pass\n"
		"exp=exp_()\n"
		"function main():\n"
		"	input = (\n"
		"		(1,('a','b','c','d'),2,'c'),\n"
		"		(1,('a','b','c','d'),-2,'c'),\n"
		"		(1,('a','b','c','d'),6,'_exception_'),\n"
		"		(1,('a','b','c','d'),-6,'_exception_'),\n"
		"		(2,('a','b','c','d'),2,'c'),\n"
		"		(2,('a','b','c','d'),-2,'c'),\n"
		"		(2,('a','b','c','d'),6,'_exception_'),\n"
		"		(2,('a','b','c','d'),-6,'_exception_'),\n"
		"		(3,('a','b','c','d'),0,2,('a','b')),\n"
		"		(3,('a','b','c','d'),2,6,('c','d')),\n"
		"		(3,('a','b','c','d'),-4,-2,('a','b')),\n"
		"		(3,('a','b','c','d'),-5,-2,('a','b')),\n"
		"		(4,('a','b','c','d'),4),\n"
		"		(4,(),0),\n"
		"		(5,('a','b','c','d'),('a','b','c','d')),\n"
		"		(6,('a','b','c','d'),('a','b','c','d')),\n"
		"		(101,['a','b','c','d'],2,'c'),\n"
		"		(101,['a','b','c','d'],-2,'c'),\n"
		"		(101,['a','b','c','d'],6,'_exception_'),\n"
		"		(101,['a','b','c','d'],-6,'_exception_'),\n"
		"		(102,['a','b','c','d'],4),\n"
		"		(103,['a','b','c','d'],2,'c'),\n"
		"		(103,['a','b','c','d'],-2,'c'),\n"
		"		(103,['a','b','c','d'],6,'_exception_'),\n"
		"		(103,['a','b','c','d'],-6,'_exception_'),\n"
		"		(104,['a','b','c','d'],0,2,['a','b']),\n"
		"		(104,['a','b','c','d'],-3,-2,['b']),\n"
		"		(104,['a','b','c','d'],2,6,['c','d']),\n"
		"		(104,['a','b','c','d'],-8,-3,['a']),\n"
		"		(105,['a','b','c','d'],2,'q',['a','b','q','d']),\n"
		"		(105,['a','b','c','d'],-2,'q',['a','b','q','d']),\n"
		"		(105,['a','b','c','d'],6,'q','_exception_'),\n"
		"		(105,['a','b','c','d'],-6,'q','_exception_'),\n"
		"		(106,['a','b','c','d'],2,'q',['a','b','q','d']),\n"
		"		(106,['a','b','c','d'],-2,'q',['a','b','q','d']),\n"
		"		(106,['a','b','c','d'],6,'q','_exception_'),\n"
		"		(106,['a','b','c','d'],-6,'q','_exception_'),\n"
		"		(108,['a','b','c','d'],2,['a','b']),\n"
		"		(108,['a','b','c','d'],6,['a','b','c','d',null,null]),\n"
		"		(109,['a','b','c','d'],['a','b','c','d']),\n"
		"		(110,['a','b','c','d'],['a','b','c','d']),\n"
		"		(114,['a','b'],[]),\n"
		"		(131,['a','b','c','d'],'q',['q','a','b','c','d']),\n"
		"		(132,['a','b','c','d'],'q',['a','b','c','d','q']),\n"
		"		(133,['a','b','c','d'],3,'q',['a','b','c','q','d']),\n"
		"		(133,['a','b','c','d'],-3,'q',['a','q','b','c','d']),\n"
		"		(133,['a','b','c','d'],8,'q','_exception_'),\n"
		"		(133,['a','b','c','d'],-8,'q','_exception_'),\n"
		"		(134,['a','b','c','d'],'a'),\n"
		"		(134,[],'_exception_'),\n"
		"		(135,['a','b','c','d'],'d'),\n"
		"		(135,[],'_exception_'),\n"
		"		(136,['a','b','c','d'],2,'c'),\n"
		"		(136,['a','b','c','d'],-2,'c'),\n"
		"		(136,['a','b','c','d'],6,'_exception_'),\n"
		"		(136,['a','b','c','d'],-6,'_exception_'),\n"
		"		(203,{'a':'A','b':'B','c':'C'},{'a':'A','b':'B','c':'C'}),\n"
		"		(204,{'a':'A','b':'B','c':'C'},{'a':'A','b':'B','c':'C'}),\n"
		"		(205,{'a':'A','b':'B','c':'C'},3),\n"
		"		(206,{'a':'A','b':'B','c':'C'},{'a':'A','b':'B','c':'C'}),\n"
		"		(207,{'a':'A','b':'B','c':'C'},{}),\n"
		"		)\n"
		"	function f(index):\n"
		"		q=input[index]\n"
		"		mode=q[0]\n"
		//"		$print('f',index,q)\n"
		"		retval=q[-1]\n"
		"		params=q[1:-1]\n"
		"		params+=$tuple(['']*(4-params.size()))\n"
		"		try:\n"
		"			rv=fnc(mode,*params)\n"
		"			if rv!=retval:\n"
		"				$print('f',index,q)\n"
		"				$print('value',rv)\n"
		"				return false\n"
		"		except as e:\n"
		"			if retval is not exp:\n"
		"				$print('f',index,q)\n"
		"				$print('exception')\n"
		"				return false\n"
		"	for q = $range(input.size()):\n"
		"		f(q)\n"
		"	return true\n"
		))==owca_function_return_value::RETURN_VALUE);
	if (ml.has_errors() || ml.has_warnings()) {
		for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
			printf("%5d:      %s\n",it->line(),it->text().c_str());
		}
		RCASSERT(0);
	}
	vm.set_print_function(test_2_printfunction);
	RCASSERT(nspace["main"].call(ret)==owca_function_return_value::RETURN_VALUE && ret.bool_get()==true);
}

void test_object_5()
{
	owca_global g1,g2;
	owca_message_list ml;
	owca_vm vm;
	owca_global gnspace=vm.create_namespace("main.ys"),res;
	owca_namespace nspace=gnspace.namespace_get();

	RCASSERT(vm.compile(res,nspace,ml,owca_source_file_Text(
		"function f():\n"
		"	yield 'a'\n"
		"class T: pass\n"

		"a1=f()\n"
		"b1=$property()\n"
		"c1=f\n"
		"d1=f.bind(f)\n"
		"t1=T()\n"
		"a2=f()\n"
		"b2=$property()\n"
		"c2=f\n"
		"d2=f.bind(f)\n"
		"t2=T()\n"
		))==owca_function_return_value::RETURN_VALUE);
	if (ml.has_errors() || ml.has_warnings()) {
		for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
			printf("%5d:      %s\n",it->line(),it->text().c_str());
		}
		RCASSERT(0);
	}
	vm.set_print_function(test_2_printfunction);

	g1.bool_set(true);
	RCASSERT(g1.is(true) && !g1.is(false));
	g1.int_set(4);
	RCASSERT(g1.is(4) && !g1.is(5));
	g1.real_set(4.5);
	RCASSERT(g1.is(4.500) && !g1.is(5.5));
	g1=nspace["a1"]; g2=nspace["a2"];
	RCASSERT(g1.is(g1) & !g1.is(g2)); RCASSERT(g2.is(g2) & !g2.is(g1)); 
	g1=nspace["b1"]; g2=nspace["b2"];
	RCASSERT(g1.is(g1) & !g1.is(g2)); RCASSERT(g2.is(g2) & !g2.is(g1)); 
	g1=nspace["c1"]; g2=nspace["d2"];
	RCASSERT(g1.is(g1) & !g1.is(g2)); RCASSERT(g2.is(g2) & !g2.is(g1)); 
	g1=nspace["d1"]; g2=nspace["c2"];
	RCASSERT(g1.is(g1) & !g1.is(g2)); RCASSERT(g2.is(g2) & !g2.is(g1)); 

	g1=nspace["t1"];
	try {
		g1.set_member(res,"",14);
		RCASSERT(0);
	} catch(owca_exception &exc) {
	}
	try {
		g1.set_member(res," ",14);
		RCASSERT(0);
	} catch(owca_exception &exc) {
	}
	try {
		g1.set_member(res,"9a",14);
		RCASSERT(0);
	} catch(owca_exception &exc) {
	}
}

void test_object_6()
{
	owca_message_list ml;
	owca_vm vm;
	owca_global gnspace=vm.create_namespace("main.ys"),res;
	owca_namespace nspace=gnspace.namespace_get();

	RCASSERT(vm.compile(res,nspace,ml,owca_source_file_Text(
		"class T:\n"
		"	function read  f1(): return 'qwe'\n"
		"	function read  f2(): return\n"
		"	function write f1(v): return 'qwe'\n"
		"	function write f2(v): return\n"
		"class K1:\n"
		"	function $init(v): self.v=v\n"
		"	function $str(): return $str(self.v)\n"
		"function k1():\n"
		"	return self\n"
		"function k2():\n"
		"	return $str(self)\n"
		"t=T()\n"
		"\n"
		))==owca_function_return_value::RETURN_VALUE);
	if (ml.has_errors() || ml.has_warnings()) {
		for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
			printf("%5d:      %s\n",it->line(),it->text().c_str());
		}
		RCASSERT(0);
	}
	vm.set_print_function(test_2_printfunction);

	{
		owca_global g(4);
		RCASSERT(g.type().null_is());
		owca_global g1(vm),g2(vm);
		g1=4;
		g2=8;
		RCASSERT(g1.type().is(g2.type()));
		g2=8.0;
		RCASSERT(!g1.type().is(g2.type()));
		g1=nspace["T"];
		RCASSERT(g1.type().is(g2.type().type()));
		RCASSERT(!g1.type().is(g2.type()));
	}

	{
		owca_global g1=nspace["t"],a,am;
		RCASSERT(g1.get_member(a,"f1")==owca_function_return_value::RETURN_VALUE && a.string_get()=="qwe");
		RCASSERT(g1.get_member(a,"f2")==owca_function_return_value::RETURN_VALUE && a.null_is());

		owca_global g(vm);
		RCASSERT(g.set_member(a,"qwe","rty")==owca_function_return_value::RETURN_VALUE);
		//RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::NOT_LVALUE);

		RCASSERT(g1.set_member(a,"f1","asd")==owca_function_return_value::RETURN_VALUE && a.string_get()=="qwe");
		RCASSERT(g1.set_member(a,"f2","asd")==owca_function_return_value::RETURN_VALUE && a.null_is());
	}

	{
		std::string ident="Z";
		owca_global g1=nspace["t"],a,am;
		RCASSERT(g1.get_member(a,ident)==owca_function_return_value::RETURN_VALUE);
		//RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::MISSING_MEMBER);
	}

	{
		owca_global g=nspace["k1"],z,a,g2;
		z=g.function_bind("qwe");
		z=z.function_bind("qwe");
		RCASSERT(z.call(a)==owca_function_return_value::RETURN_VALUE && a.string_get()=="qwe");
		RCASSERT(z.function_obj().string_get()=="qwe");
		g=nspace["k2"];
		RCASSERT(nspace["K1"].call(g2,"asd")==owca_function_return_value::RETURN_VALUE);
		z=g.function_bind(g2);
		z=z.function_bind(g2);
		RCASSERT(z.call(a)==owca_function_return_value::RETURN_VALUE && a.string_get()=="asd");
	}
}

void test_object_7()
{
	owca_message_list ml;
	owca_vm vm;
	owca_global gnspace=vm.create_namespace("main.ys"),res,a,am;
	owca_namespace nspace=gnspace.namespace_get();
	ExceptionCode exccode;

	RCASSERT(vm.compile(res,nspace,ml,owca_source_file_Text(
		"class T:\n"
		"	function read f(): return 'qwe'\n"
		"	function write f(v): return v+'qwe'\n"
		"function k1():\n"
		"	$print('qwe',**{k1:'asd'})\n"
		"function k2():\n"
		"	return $print('q','w',*['r','t'])\n"
		"function k3():\n"
		"	return $print('q','w',*['r','t'],join='_')\n"
		"function k4():\n"
		"	z=''\n"
		"	for x = $range(2,10,3):\n"
		"		z+=$str(x)\n"
		"	return z\n"
		"function k5():\n"
		"	for x = $range(2,step=3):\n"
		"		z+=$str(x)\n"
		"	return z\n"
		"t=T()\n"
		"\n"
		))==owca_function_return_value::RETURN_VALUE);
	if (ml.has_errors() || ml.has_warnings()) {
		for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
			printf("%5d:      %s\n",it->line(),it->text().c_str());
		}
		RCASSERT(0);
	}
	vm.set_print_function(test_2_printfunction);

	RCASSERT(nspace["k1"].call(a)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::KEYWORD_PARAM_NOT_STRING);

	RCASSERT(nspace["k2"].call(a)==owca_function_return_value::RETURN_VALUE && a.string_get()=="q w r t");
	RCASSERT(nspace["k3"].call(a)==owca_function_return_value::RETURN_VALUE && a.string_get()=="q_w_r_t");
	RCASSERT(nspace["k4"].call(a)==owca_function_return_value::RETURN_VALUE && a.string_get()=="258");
	RCASSERT(nspace["$memberget"].call(a,nspace["t"],"f")==owca_function_return_value::RETURN_VALUE && a.string_get()=="qwe");
	RCASSERT(nspace["$memberset"].call(a,nspace["t"],"f","abc")==owca_function_return_value::RETURN_VALUE && a.string_get()=="abcqwe");
	
	RCASSERT(nspace["$memberset"].call(a,4,"abc","qwe")==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::MISSING_MEMBER);

	RCASSERT(nspace["k5"].call(a)==owca_function_return_value::EXCEPTION);
	RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);
}

struct test_user_fncs_str {
	//static owca_function_return_value func_param_other(owca_global &retval, owca_namespace &nspace, const owca_parameters &parameters)
	static owca_function_return_value fnc_1(owca_global &retval, const owca_global &ll, owca_namespace &nspace)
	{
		retval=ll;
		return owca_function_return_value::RETURN_VALUE;
	}
	static owca_function_return_value fnc_2(owca_global &retval, const owca_global &ll, owca_namespace &nspace)
	{
		retval=nspace.vm().construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,"");
		return owca_function_return_value::EXCEPTION;
	}
	static owca_function_return_value fnc_3(owca_global &retval, const owca_global &ll, owca_namespace &nspace, const owca_global &p1)
	{
		retval=p1;
		return owca_function_return_value::RETURN_VALUE;
	}
	static owca_function_return_value fnc_4(owca_global &retval, const owca_global &ll, owca_namespace &nspace, const owca_global &p1)
	{
		owca_global z=nspace["$exception"];
		RCASSERT(z.call(retval,"",(owca_int)ExceptionCode::INVALID_PARAM_TYPE)==owca_function_return_value::RETURN_VALUE);
		return owca_function_return_value::EXCEPTION;
	}
	static owca_function_return_value fnc_5(owca_global &retval, const owca_global &ll, owca_namespace &nspace, const owca_global &p1)
	{
		owca_global z=p1.function_bind(ll);
		return z.call(retval);
	}
	static owca_function_return_value fnc_6(owca_global &retval, const owca_global &ll, owca_namespace &nspace, const owca_global &p1)
	{
		retval=p1;
		return owca_function_return_value::RETURN_VALUE;
	}
	static owca_function_return_value fnc_7(owca_global &retval, const owca_global &ll, owca_namespace &nspace, const owca_string &p1)
	{
		retval=p1;
		return owca_function_return_value::RETURN_VALUE;
	}
	static owca_function_return_value fnc_8(owca_global &retval, const owca_global &ll, owca_namespace &nspace, const owca_string &p1, const owca_string &p2)
	{
		owca_global z;
		RCASSERT(owca_global(p1).get_member(z,"$add")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(z.call(retval,p2)==owca_function_return_value::RETURN_VALUE);
		return owca_function_return_value::RETURN_VALUE;
	}
	static owca_function_return_value fnc_9(owca_global &retval, const owca_global &ll, owca_namespace &nspace, const owca_string &p1, const owca_string &p2, const owca_string &p3)
	{
		owca_global z;
		RCASSERT(owca_global(p1).get_member(z,"$add")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(z.call(z,p2)==owca_function_return_value::RETURN_VALUE);
		RCASSERT(z.get_member(z,"$add")==owca_function_return_value::RETURN_VALUE);
		RCASSERT(z.call(retval,p3)==owca_function_return_value::RETURN_VALUE);
		return owca_function_return_value::RETURN_VALUE;
	}
};

void test_user_fncs()
{
	owca_vm vm;
	{
		owca_message_list ml;
		owca_global gnspace=vm.create_namespace("main.ys"),res,a,am;
		owca_namespace nspace=gnspace.namespace_get();
		ExceptionCode exccode;

		nspace["fnc_1"].set(test_user_fncs_str::fnc_1);
		nspace["fnc_2"].set(test_user_fncs_str::fnc_2);
		nspace["fnc_3"].set(test_user_fncs_str::fnc_3,"p1");
		nspace["fnc_4"].set(test_user_fncs_str::fnc_4,"p1");
		nspace["fnc_5"].set(test_user_fncs_str::fnc_5,"p1");
		nspace["fnc_6"].set(test_user_fncs_str::fnc_6,"p1","ok");
		nspace["fnc_7"].set(test_user_fncs_str::fnc_7,"p1","ok");
		nspace["fnc_8"].set(test_user_fncs_str::fnc_8,"p1","p2");
		nspace["fnc_9"].set(test_user_fncs_str::fnc_9,"p1","p2","p3");

		static const char *tcode=
			"function test_1():\n"
			"	z=fnc_1.bind('ok')\n"
			"	return z()\n"
			"function test_2():\n"
			"	z=fnc_2.bind('ok')\n"
			"	return z()\n"
			"function test_3():\n"
			"	z=fnc_3.bind('ok')\n"
			"	return z('ok')\n"
			"function test_4():\n"
			"	z=fnc_4.bind('ok')\n"
			"	return z('ok')\n"
			"function test_5(q):\n"
			"	z=fnc_5.bind('ok')\n"
			"	return z(q)\n"
			"function test_6(q=null):\n"
			"	if not q is null: return fnc_6(q)\n"
			"	return fnc_6()\n"
			"function test_7(q=null):\n"
			"	if not q is null: return fnc_7(q)\n"
			"	return fnc_7()\n"
			"function test_8(a,b):\n"
			"	return fnc_8(a,b)\n"
			"function test_9(a,b,c):\n"
			"	return fnc_9(a,b,c)\n"
			;

		RCASSERT(vm.compile(res,nspace,ml,owca_source_file_Text(tcode))==owca_function_return_value::RETURN_VALUE);

		if (ml.has_errors()) {
			for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
				printf("%5d:      %s\n",it->line(),it->text().c_str());
			}
			RCASSERT(0);
			return;
		}

		RCASSERT(nspace["test_1"].call(res)==owca_function_return_value::RETURN_VALUE && res.string_get()=="ok");

		RCASSERT(nspace["test_2"].call(res)==owca_function_return_value::EXCEPTION);
		RCASSERT(res.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

		RCASSERT(nspace["test_3"].call(res)==owca_function_return_value::RETURN_VALUE && res.string_get()=="ok");

		RCASSERT(nspace["test_4"].call(res)==owca_function_return_value::EXCEPTION);
		RCASSERT(res.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

		RCASSERT(nspace["test_5"].call(res,nspace["test_3"])==owca_function_return_value::RETURN_VALUE && res.string_get()=="ok");

		RCASSERT(nspace["test_5"].call(res,nspace["test_4"])==owca_function_return_value::EXCEPTION);
		RCASSERT(res.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::INVALID_PARAM_TYPE);

		RCASSERT(nspace["test_6"].call(res)==owca_function_return_value::RETURN_VALUE && res.string_get()=="ok");
		RCASSERT(nspace["test_6"].call(res,"abc")==owca_function_return_value::RETURN_VALUE && res.string_get()=="abc");

		RCASSERT(nspace["test_7"].call(res)==owca_function_return_value::RETURN_VALUE && res.string_get()=="ok");

		RCASSERT(nspace["test_8"].call(res,"o","k")==owca_function_return_value::RETURN_VALUE && res.string_get()=="ok");

		RCASSERT(nspace["test_9"].call(res,"o","k","a")==owca_function_return_value::RETURN_VALUE && res.string_get()=="oka");
	}
}

struct matrix_struct : public owca_object_base<matrix_struct> {
	owca_int width,height;
	owca_real *data;

	matrix_struct(owca_vm &vm, unsigned int oversize)
	{
		data=NULL;
		width=height=0;
	}
	~matrix_struct() {
		delete [] data;
	}

	//void destroy(owca_vm &vm)
	//{
	//	delete [] data;
	//}

	void marker(const gc_iteration &gc) const
	{
	}

	double &get(unsigned int w, unsigned int h) {
		RCASSERT(w<width && h<height);
		return data[w+h*width];
	}
};

static owca_function_return_value matrix_init(owca_global &retval, matrix_struct *ms, owca_namespace &nspace, const owca_parameters &params)
{
	unsigned int width,height;

	if (params.count()==0) {
		nspace.vm().construct_builtin_exception(ExceptionCode::NOT_ENOUGH_PARAMETERS,OWCA_ERROR_FORMAT("not enough parameters"));
		return owca_function_return_value::EXCEPTION;
	}

	if (params.count()==2 && (params.get(0).int_is() || params.get(0).real_is()) && (params.get(1).int_is() || params.get(1).real_is())) {
		owca_int ww,hh;

		if (!params.convert_parameters(retval,ww,hh)) {
			return owca_function_return_value::EXCEPTION;
		}

		if (ww<=0) {
			nspace.vm().construct_builtin_exception(ExceptionCode::INTEGER_OUT_OF_BOUNDS,OWCA_ERROR_FORMAT("column count is negative or zero"));
			return owca_function_return_value::EXCEPTION;
		}
		if (hh<=0) {
			nspace.vm().construct_builtin_exception(ExceptionCode::INTEGER_OUT_OF_BOUNDS,OWCA_ERROR_FORMAT("row count is negative or zero"));
			return owca_function_return_value::EXCEPTION;
		}

		delete [] ms->data;
		ms->width=(unsigned int)ww;
		ms->height=(unsigned int)hh;
		ms->data=new owca_real[(size_t)(ms->width*ms->height)];
		for(unsigned int i=0;i<ms->width*ms->height;++i) ms->data[i]=0;
	}
	else {
		height=(unsigned int)params.count();
		for(unsigned int i=0;i<height;++i) {
			unsigned int w;
			owca_global g=params.get(i);

			if (g.list_is()) w=g.list_get().size();
			else if (g.tuple_is()) w=g.tuple_get().size();
			else {
				nspace.vm().construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT("parameter is not a list or tuple of real values"));
				return owca_function_return_value::EXCEPTION;
			}
			if (i==0) {
				if (w==0) {
					nspace.vm().construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT("empty list / tuple used"));
					return owca_function_return_value::EXCEPTION;
				}
				width=w;
			}
			else if (w!=width) {
				nspace.vm().construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT("list / tuples of different width used"));
				return owca_function_return_value::EXCEPTION;
			}
		}

		owca_real *data=new owca_real[width*height];

		for(unsigned int h=0;h<params.count();++h) {
			owca_global g=params.get(h);

			if (g.list_is()) {
				owca_list l=g.list_get();

				for(unsigned int w=0;w<width;++w) {
					owca_real v;
					if (!l[w].get(v)) {
						nspace.vm().construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT2("parameter at row %1, column %2 is not a real value",int_to_string(h+1),int_to_string(w+1)));
						delete data;
						return owca_function_return_value::EXCEPTION;						
					}
					data[h*width+w]=v;
				}
			}
			else {
				RCASSERT(g.tuple_is());

				owca_tuple t=g.tuple_get();
				for(unsigned int w=0;w<width;++w) {
					owca_real v;
					if (!t[w].get(v)) {
						nspace.vm().construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT2("parameter at row %1, column %2 is not a real value",int_to_string(h+1),int_to_string(w+1)));
						delete data;
						return owca_function_return_value::EXCEPTION;						
					}
					data[h*width+w]=v;
				}
			}
		}
		delete [] ms->data;
		ms->data=data;
		ms->width=width;
		ms->height=height;
	}
	retval.null_set(true);
	return owca_function_return_value::RETURN_VALUE;
}

static owca_function_return_value matrix_width_read(owca_global &ret, matrix_struct *ms, owca_namespace &nspace)
{
	ret=ms->width;
	return owca_function_return_value::RETURN_VALUE;
}

static owca_function_return_value matrix_height_read(owca_global &ret, matrix_struct *ms, owca_namespace &nspace)
{
	ret=ms->height;
	return owca_function_return_value::RETURN_VALUE;
}

static owca_function_return_value matrix_add(owca_global &ret, matrix_struct *left, owca_namespace &nspace, matrix_struct *right)
{
	if (left->width!=right->width || left->height!=right->height) {
		nspace.vm().construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT("matrixes dimensions are different!"));
		return owca_function_return_value::EXCEPTION;
	}

	RCASSERT(nspace.vm().executing_function().function_member_of().call(ret,left->height,left->width)==owca_function_return_value::RETURN_VALUE);
	matrix_struct *r;
	RCASSERT(ret.get(r));

	for(unsigned int i=0;i<left->width*left->height;++i) {
		r->data[i]=left->data[i]+right->data[i];
	}

	return owca_function_return_value::RETURN_VALUE;
}

static owca_function_return_value matrix_str(owca_global &ret, matrix_struct *self, owca_namespace &nspace)
{
	owca_string_buffer sb;
	char buf[64];

	for(unsigned int h=0;h<self->height;++h) {
		sb << (h==0 ? "[ " : "  ");

		for(unsigned int w=0;w<self->width;++w) {
			int len=sprintf(buf,"%f",self->data[w+h*self->width]);
			if (len<8) sb << ("        "+len);
			sb << buf;
			if (w<self->width-1) sb << "  ";
		}
		if (h==self->height-1) sb << " ]";
		sb << '\n';
	}
	ret=sb.get(nspace.vm());

	return owca_function_return_value::RETURN_VALUE;
}

class proxy_call : public owca_user_function_base_object {
	unsigned int index;
public:
	proxy_call() : index(0) { 
		catch_exceptions(false);
	}
	owca_function_return_value run(owca_global &retval, bool is_exception) {
		switch(index) {
		case 0:
			index = 1;
			return parameters.parameter(0).prepare_call(retval);
		case 1:
			break;
		}
		retval.null_set(true);
		return owca_function_return_value::RETURN_VALUE;
	}
};

class matrix_gen : public owca_user_function_base_object {
	matrix_struct *ms;
	owca_int w,h;
public:
	matrix_gen() { 
		catch_exceptions(false);
	}
	owca_function_return_value initialize(owca_global &retval) {
		if (!self.get(ms)) {
			retval=vm().construct_builtin_exception(ExceptionCode::INVALID_PARAM_TYPE,OWCA_ERROR_FORMAT("self is not of type matrix"));
			return owca_function_return_value::EXCEPTION;
		}
		w=h=0;
		return owca_function_return_value::CREATE_GENERATOR;
	}
	owca_function_return_value run(owca_global &retval, bool is_exception) {
		RCASSERT(!is_exception);
		if (h >= ms->height) {
			retval.null_set(true);
			return owca_function_return_value::RETURN_VALUE;
		}
		retval=ms->data[w+h*ms->width];
		++w;
		if (w>=ms->width) {
			++h;
			w=0;
		}
		return owca_function_return_value::RETURN_VALUE;
	}
};

owca_function_return_value invalid_operator_function_fast_self(owca_global &ret, const owca_global &self, owca_namespace &nspace, const owca_parameters &params) { ret=0; return owca_function_return_value::RETURN_VALUE; }
owca_function_return_value invalid_operator_function_fast     (owca_global &ret, owca_namespace &nspace, const owca_parameters &params) { ret=0; return owca_function_return_value::RETURN_VALUE; }
owca_function_return_value invalid_operator_function_fast_0   (owca_global &ret, owca_namespace &nspace) { ret=0; return owca_function_return_value::RETURN_VALUE; }
owca_function_return_value invalid_operator_function_fast_1   (owca_global &ret, owca_namespace &nspace, const owca_global &p1) { ret=0; return owca_function_return_value::RETURN_VALUE; }
owca_function_return_value invalid_operator_function_fast_2   (owca_global &ret, owca_namespace &nspace, const owca_global &p1, const owca_global &p2) { ret=0; return owca_function_return_value::RETURN_VALUE; }
owca_function_return_value invalid_operator_function_fast_3   (owca_global &ret, owca_namespace &nspace, const owca_global &p1, const owca_global &p2, const owca_global &p3) { ret=0; return owca_function_return_value::RETURN_VALUE; }

void test_class()
{
	owca_vm vm;
	ExceptionCode exccode;

	{
		owca_message_list ml;
		owca_global gnspace=vm.create_namespace("main.ys"),res;
		owca_namespace nspace=gnspace.namespace_get();
		nspace["proxy_call"].set<proxy_call>();

		static const char *tcode=
			"function f():\n"
			"	pass\n"
			"function test():\n"
			"	proxy_call(f)\n"
			"	return 'ok'\n"
			;

		RCASSERT(vm.compile(res,nspace,ml,owca_source_file_Text(tcode))==owca_function_return_value::RETURN_VALUE);
		vm.set_print_function(test_2_printfunction);

		if (ml.has_errors()) {
			for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
				printf("%5d:      %s\n",it->line(),it->text().c_str());
			}
			RCASSERT(0);
			return;
		}
		RCASSERT(nspace["test"].call(res)==owca_function_return_value::RETURN_VALUE);
		printf("return value: %s\n",res.string_get().str().c_str());
		RCASSERT(res.string_get()=="ok");			
	}
	{
		owca_global gnspace=vm.create_namespace("main.ys"),a,am;
		owca_namespace nspace=gnspace.namespace_get();
		owca_class cls(nspace,"matrix");
		cls["$add"].set(invalid_operator_function_fast);
		RCASSERT(cls.construct(a)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::CLASS_CREATION);
	}
	{
		owca_global gnspace=vm.create_namespace("main.ys"),a,am;
		owca_namespace nspace=gnspace.namespace_get();
		owca_class cls(nspace,"matrix");
		cls["$add"].set(invalid_operator_function_fast_self);
		RCASSERT(cls.construct(a)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::CLASS_CREATION);
	}
	{
		owca_global gnspace=vm.create_namespace("main.ys"),a,am;
		owca_namespace nspace=gnspace.namespace_get();
		owca_class cls(nspace,"matrix");
		cls["$add"].set(invalid_operator_function_fast_0);
		RCASSERT(cls.construct(a)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::CLASS_CREATION);
	}
	{
		owca_global gnspace=vm.create_namespace("main.ys"),a,am;
		owca_namespace nspace=gnspace.namespace_get();
		owca_class cls(nspace,"matrix");
		cls["$add"].set(invalid_operator_function_fast_1,"q");
		RCASSERT(cls.construct(a)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::CLASS_CREATION);
	}
	{
		owca_global gnspace=vm.create_namespace("main.ys"),a,am;
		owca_namespace nspace=gnspace.namespace_get();
		owca_class cls(nspace,"matrix");
		cls["$add"].set(invalid_operator_function_fast_2,"q","w");
		RCASSERT(cls.construct(a)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::CLASS_CREATION);
	}
	{
		owca_global gnspace=vm.create_namespace("main.ys"),a,am;
		owca_namespace nspace=gnspace.namespace_get();
		owca_class cls(nspace,"matrix");
		cls["$add"].set(invalid_operator_function_fast_3,"q","w","e");
		RCASSERT(cls.construct(a)==owca_function_return_value::EXCEPTION);
		RCASSERT(a.get_member(am,"code")==owca_function_return_value::RETURN_VALUE && am.call(am)==owca_function_return_value::RETURN_VALUE && (exccode=(ExceptionCode)am.int_get())==ExceptionCode::CLASS_CREATION);
	}
	{
		owca_message_list ml;
		owca_global gnspace=vm.create_namespace("main.ys"),res;
		owca_namespace nspace=gnspace.namespace_get();
		owca_class cls(nspace,"matrix");

		cls.set_struct<matrix_struct>();
		cls["$init"].set(matrix_init);
		cls["width"].set_property(matrix_width_read);
		cls["height"].set_property(matrix_height_read);
		cls["$str"].set(matrix_str);
		cls["$add"].set(matrix_add,"other");
		cls["print"].set<matrix_gen>();

		RCASSERT(cls.construct(res)==owca_function_return_value::RETURN_VALUE);
		nspace["matrix"]=res;

		static const char *tcode=
			"function test():\n"
			"	try:\n"
			"		matrix(-1,2)\n"
			"		return 1\n"
			"	except:\n"
			"		pass\n"
			"	try:\n"
			"		matrix(1,-2)\n"
			"		return 2\n"
			"	except:\n"
			"		pass\n"
			"	try:\n"
			"		matrix(-1,-2)\n"
			"		return 3\n"
			"	except:\n"
			"		pass\n"
			"	m1=matrix(\n"
			"		(1,2,3,4),\n"
			"		(5,6,7,8),\n"
			"		)\n"
			"	m2=matrix(\n"
			"		(-1,-2,-3,-4),\n"
			"		(-5,-6,-7,-8),\n"
			"		)\n"
			"	$print(m1.width,m1.height)\n"
			"	$print(m2.width,m2.height)\n"
			"	$print('m1')\n"
			"	$print(m1)\n"
			"	$print('m2')\n"
			"	$print(m2)\n"
			"	$print('m1+m2')\n"
			"	$print(m1+m2)\n"
			"	lp: for a = m1.print():\n"
			"		$print('m1',lp,a)\n"
			"	lp: for a = m2.print():\n"
			"		$print('m2',lp,a)\n"
			"	lp: for a = (m1+m2).print():\n"
			"		$print('m1+m2',lp,a)\n"
			"	return 'ok'\n"
			"a=4"
			;

		RCASSERT(vm.compile(res,nspace,ml,owca_source_file_Text(tcode))==owca_function_return_value::RETURN_VALUE);
		vm.set_print_function(test_2_printfunction);

		if (ml.has_errors()) {
			for(owca_message_list::T it=ml.begin();it!=ml.end();++it) {
				printf("%5d:      %s\n",it->line(),it->text().c_str());
			}
			RCASSERT(0);
			return;
		}
		RCASSERT(nspace["test"].call(res)==owca_function_return_value::RETURN_VALUE);
		printf("return value: %s\n",res.string_get().str().c_str());
		RCASSERT(res.string_get()=="ok");			
	}
	return;
}

namespace error_1 {
	static bool test_val = false;
	static owca_global fnc;
	static owca_function_return_value f2(owca_global &ret, owca_namespace &nspace, const owca_string &p1)
	{
		test_val = (p1 == "qa");
		ret = p1.str() + "b";
		return owca_function_return_value::RETURN_VALUE;
	}
	static owca_function_return_value f3(owca_global &ret, owca_namespace &nspace, const owca_global &p1)
	{
		RCASSERT(p1.function_is());
		fnc = p1;
		ret.null_set(true);
		return owca_function_return_value::RETURN_VALUE;
	}

	static void test()
	{
		owca_vm vm;
		ExceptionCode exccode;

		{
			owca_message_list ml;
			owca_global gnspace = vm.create_namespace("main.ys"), res;
			owca_namespace nspace = gnspace.namespace_get();
			nspace["f2"].set(f2,"a");
			nspace["f3"].set(f3,"a");

			static const char *tcode =
				"function f1(key):\n"
				"	return f2(key + 'a')\n"
				"f3(f1)\n"
				;
			static const char *name_array[] = {
				"f2", "f3"
			};
			std::vector<unsigned char> tmp = vm.compile(ml, owca_source_file_Text(tcode), name_array, name_array + 2);
			if (tmp.empty()) {
				RCASSERT(ml.has_errors());
				for (owca_message_list::T it = ml.begin(); it != ml.end(); ++it) {
					printf("%5d:      %s\n", it->line(), it->text().c_str());
				}
				RCASSERT(0);
				return;
			}
			vm.run_gc();
			vm.set_print_function(test_2_printfunction);

			{
				owca_global gnspace2 = nspace.copy("main2.ys");
				owca_namespace nspace2 = gnspace2.namespace_get();

				auto res_type = nspace2.apply_code(res, tmp);
				RCASSERT(res_type == owca_function_return_value::RETURN_VALUE);
				vm.run_gc();
			}

			auto t = fnc.call(res, "q");
			RCASSERT(t == owca_function_return_value::RETURN_VALUE);
			RCASSERT(res.string_is() && res.string_get() == "qab");
			vm.run_gc();
		}
		RCASSERT(test_val);
		return;
	}
}

namespace error_2 {
	static const char *source1[] = {
        "a = c('abc')",
        "a.b('abc')",
        "a.b('def')",
    };
	static const char *source2[] = {
        "function b(txt):",
        "   $print('0' + $str(txt) + '0')",
    };
    static const char *name_array[] = {
		"c",
	};
	static owca_function_return_value c(owca_global &ret, owca_namespace &nspace, const owca_string &p1)
	{
        RCASSERT(p1 == "abc");

        nspace.vm().run_gc();
        owca_global namespace_object = nspace.copy("qq2");
	    owca_namespace ns = namespace_object.namespace_get();

        owca_message_list errors;
        std::vector<unsigned char> code = ns.vm().compile(errors, owca_source_file_Text_array(source2,sizeof(source2) / sizeof(source2[0])), name_array, name_array + sizeof(name_array) / sizeof(name_array[0]));
        RCASSERT(!code.empty());

	    owca_global result;
	    auto res = ns.apply_code(result, code);
        RCASSERT(res == owca_function_return_value::RETURN_VALUE);

        ret = namespace_object;
        nspace.vm().run_gc();
		return owca_function_return_value::RETURN_VALUE;
	}

	static void test()
	{
		owca_vm vm;
		ExceptionCode exccode;

		vm.set_print_function(test_2_printfunction);
		{
			owca_message_list ml;
			owca_global gnspace = vm.create_namespace("main.ys"), res;
			owca_namespace nspace = gnspace.namespace_get();
			nspace["c"].set(c,"src");

			std::vector<unsigned char> tmp = vm.compile(ml, owca_source_file_Text_array(source1,sizeof(source1) / sizeof(source1[0])), name_array, name_array + sizeof(name_array) / sizeof(name_array[0]));
			if (tmp.empty()) {
				RCASSERT(ml.has_errors());
				for (owca_message_list::T it = ml.begin(); it != ml.end(); ++it) {
					printf("%5d:      %s\n", it->line(), it->text().c_str());
				}
				RCASSERT(0);
				return;
			}
            auto tmp2 = nspace.apply_code(res,tmp);
            RCASSERT(tmp2 == owca_function_return_value::RETURN_VALUE);
			vm.run_gc();
		}
		return;
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

	//test_object();
	//test_object_2();
	//test_object_3();
	//test_object_4();
	//test_object_5();
	//test_object_6();
	//test_object_7();
	//test_namespace();
	//test_user_fncs();
	//test_class();

	//error_1::test();
	error_2::test();

	return 0;
}
