#include "test.h"

using namespace OwcaScript;

class TupleTest : public SimpleTest {

};

TEST_F(TupleTest, simple1)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return ( 1, 2, 3, 4 )[1];
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_int(vm).internal_value(), 2);
}

TEST_F(TupleTest, simple2)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return ( 1, 2, 3, 4 )[3];
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_int(vm).internal_value(), 4);
}
