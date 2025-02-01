#include "test.h"

using namespace OwcaScript;

class StringTest : public SimpleTest {

};

TEST_F(StringTest, size)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return 'qwerty'.size();
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_int(vm).internal_value(), 6);
}
