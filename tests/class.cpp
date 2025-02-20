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
	return A.name();
	)");
		auto val = vm.execute(code);
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
class A {
}
return A.full_name();
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_string(vm).text(), "A");
}

