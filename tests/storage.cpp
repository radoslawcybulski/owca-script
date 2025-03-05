#include "test.h"
#include "owca-script/code_buffer.h"

using namespace OwcaScript;

class StorageTest : public SimpleTest {

};

TEST_F(StorageTest, simple1)
{
	OwcaVM vm;
    OwcaVM vm2;
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
c = bound_value(b).v;
return c;
)");

    auto serialized = code.serialize_to_binary();
    auto loaded = vm2.load("test.os", serialized);
    ASSERT_TRUE(code.compare(loaded));
    auto ar = code.internal_value()->root();
    auto br = loaded.internal_value()->root();
    auto &a = code.internal_value()->peek_storage();
    auto &b = loaded.internal_value()->peek_storage();
    ASSERT_EQ(a.size(), b.size());

    auto val = vm.execute(code);
    auto val2 = vm2.execute(loaded);
    ASSERT_EQ((int)val.as_int(vm).internal_value(), (int)val2.as_int(vm2).internal_value());
}
