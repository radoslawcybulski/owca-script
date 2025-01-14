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

TEST_F(SimpleTest, string)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", "return 'qwe' + 'rty';");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_string(vm).internal_value(), "qwerty");
}

TEST_F(SimpleTest, range)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return 'qwerty'[2..4];
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_string(vm).internal_value(), "er");
}
