#include "test.h"

using namespace OwcaScript;

class ArrayTest : public SimpleTest {

};

TEST_F(ArrayTest, simple1)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return [ 1, 2, 3, 4 ][1];
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 2);
}

TEST_F(ArrayTest, simple2)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return [ 1, 2, 3, 4 ][3];
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 4);
}

TEST_F(ArrayTest, simple3)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
v = [ 1, 2, 3, 4 ][2:3];
return v == [ 3 ];
)");
	auto val = vm.execute(code);
	ASSERT_TRUE(val.as_bool(vm));
}

TEST_F(ArrayTest, simple4)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
v = [ 1, 2, 3, 4 ][2:6];
return v == [ 3, 4 ];
)");
	auto val = vm.execute(code);
	ASSERT_TRUE(val.as_bool(vm));
}

TEST_F(ArrayTest, simple5)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
v = [ 1, 2, 3, 4 ][-10:2];
return v == [ 1, 2 ];
)");
	auto val = vm.execute(code);
	ASSERT_TRUE(val.as_bool(vm));
}

TEST_F(ArrayTest, simple6)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
v = [ 1, 2, 3, 4 ][-2:-1];
return v == [ 3 ];
)");
	auto val = vm.execute(code);
	ASSERT_TRUE(val.as_bool(vm));
}

TEST_F(ArrayTest, simple7)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
v = [ 1, 2, 3, 4 ][-2:];
return v == [ 3, 4 ];
)");
	auto val = vm.execute(code);
	ASSERT_TRUE(val.as_bool(vm));
}

TEST_F(ArrayTest, simple8)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
v = [ 1, 2, 3, 4 ][:-2];
return v == [ 1, 2 ];
)");
	auto val = vm.execute(code);
	ASSERT_TRUE(val.as_bool(vm));
}

TEST_F(ArrayTest, simple9)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
v = [ 1, 2, 3, 4 ][:];
return v == [ 1, 2, 3, 4 ];
)");
	auto val = vm.execute(code);
	ASSERT_TRUE(val.as_bool(vm));
}

TEST_F(ArrayTest, update)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
v = [ 1, 2, 3, 4 ];
v[2] = 5;
return v[2];
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 5);
}

TEST_F(ArrayTest, from_iter)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function generator foo() {
	yield 1;
	yield 2;
	yield 3;
	yield 4;
}
return Array(foo()) == [ 1, 2, 3, 4 ];
)");
	auto val = vm.execute(code);
	ASSERT_TRUE(val.as_bool(vm));
}

TEST_F(ArrayTest, owca_iter)
{
	OwcaVM vm;
	auto t = vm.create_array({1, 2, 3, 4});
	std::vector<double> values;
	for(auto q : t) {
		values.push_back(q.as_float(vm));
	}
	ASSERT_EQ(values, std::vector<double>({1, 2, 3, 4}));
}
