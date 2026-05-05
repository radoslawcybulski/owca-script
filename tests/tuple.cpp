#include "test.h"

using namespace OwcaScript;

class TupleTest : public SimpleTest {

};

TEST_F(TupleTest, simple1)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() {
	return ( 1, 2, 3, 4 )[1];
}
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_float(vm), 2);
}

TEST_F(TupleTest, simple2)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() {
	return ( 1, 2, 3, 4 )[3];
}
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_float(vm), 4);
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
function r() {
	return Tuple(foo()) == ( 1, 2, 3, 4 );
}
)");
	auto val = vm.execute(code);
	ASSERT_TRUE(val.member("r").call().as_bool(vm));
}

TEST_F(TupleTest, owca_iter)
{
	OwcaVM vm;
	auto t = vm.create_tuple({1, 2, 3, 4});
	std::vector<double> values;
	for(auto q : t) {
		values.push_back(q.as_float(vm));
	}
	ASSERT_EQ(values, std::vector<double>({1, 2, 3, 4}));
}

