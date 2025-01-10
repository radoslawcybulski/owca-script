#include "test.h"

using namespace OwcaScript;

TEST_F(SimpleTest, empty)
{
	OwcaVM vm;
}

TEST_F(SimpleTest, simple)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", "return 14;");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_int(vm).internal_value(), 14);
}
