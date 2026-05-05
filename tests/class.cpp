#include "test.h"

using namespace OwcaScript;

class ClassTest : public SimpleTest {

};

TEST_F(ClassTest, name)
{
	try {
		OwcaVM vm;
		auto code = vm.compile("test.os", R"(
	class A {
	}
	function r() { return A.name(); }
	)");
		auto val = vm.execute(code).member("r").call();
		ASSERT_EQ(val.as_string(vm).text(), "A");
	}
	catch(OwcaVM::CompilationFailed &e) {
		FAIL() << e.what();
	}
}

TEST_F(ClassTest, full_name)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return A.full_name(); }
class A {
}
)");
	auto val = vm.execute(code).member("r").call();;
	ASSERT_EQ(val.as_string(vm).text(), "A");
}

