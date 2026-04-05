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
	ASSERT_EQ(val.as_float(vm), 2);
}

TEST_F(TupleTest, simple2)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return ( 1, 2, 3, 4 )[3];
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 4);
}

TEST_F(TupleTest, from_iter)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function generator foo() {
	yield 1;
	yield 2;
	yield 3;
	yield 4;
}
return Tuple(foo()) == ( 1, 2, 3, 4 );
)");
	auto val = vm.execute(code);
	ASSERT_TRUE(val.as_bool(vm));
}

