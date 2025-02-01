#include "test.h"

using namespace OwcaScript;

class FunctionTest : public SimpleTest {

};

TEST_F(FunctionTest, bound_value)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
class A {
	function __init__(self, v) {
		self.v = v;
	}
	
	function foo(self) {
	}
}

a = A(4);
b = a.foo;
c = b.bound_value().v;
return c;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_int(vm).internal_value(), 4);
}

TEST_F(FunctionTest, bind)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
class A {
	function __init__(self, v) {
		self.v = v;
	}
	
	function foo(self) {
		return self.v;
	}
}

a = A(4);
b = A(5);
c = a.foo;
d = c.bind(b);
return d();
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_int(vm).internal_value(), 5);
}
