#include "test.h"

using namespace OwcaScript;

class MapTest : public SimpleTest {

};

TEST_F(MapTest, size)
{
	OwcaVM vm;
	auto code = compile(__LINE__, vm, "test.os", R"(
function r() {
a = {
		1: 2,
		3: 4,
		5: 6
	};
	return a.size();
}
)");
	auto val = vm.execute(code).member("r").call();
	ASSERT_EQ(val.as_float(vm), 3);
}

TEST_F(MapTest, simple)
{
	OwcaVM vm;
	auto code = compile(__LINE__, vm, "test.os", R"(
function r() {
	a = {
		1: 2,
		3: 4,
		5: 6
	};
	a[7] = 9;
	tmp = a[3];
	sum = 0;
	for(kv = a.items()) {
			k = kv[0];
			v = kv[1];
			sum = sum + tmp * (k + v);
	}
	return sum;
}
)");
	auto val = vm.execute(code).member("r").call();
	ASSERT_EQ(val.as_float(vm), 4 * (3 + 4) + 4 * (1 + 2) + 4 * (5 + 6) + 4 * (7 + 9));
}

TEST_F(MapTest, with_default)
{
	OwcaVM vm;
	auto code = compile(__LINE__, vm, "test.os", R"(
function r() {
	a = {
		1: 2,
		3: 4,
		5: 6
	};
	if (a.get_or_default(7, 9) != 9) return 1;
	if (a.get_or_default(5, 9) != 6) return 2;
	if (a.set_default(7, 8) != 8) return 3;
	if (a[7] != 8) return 4;
	if (a.set_default(1, 10) != 2) return 5;
	if (a[1] != 2) return 6;
	return 0;
}
)");
	auto val = vm.execute(code).member("r").call();
	ASSERT_EQ(val.as_float(vm), 0);
}

TEST_F(MapTest, iters)
{
	OwcaVM vm;
	auto code = compile(__LINE__, vm, "test.os", R"(
function r() {
	a = {
		1: 2,
		3: 4,
		5: 6
	};
	if (Set(a.keys()) != Set([ 1, 3, 5 ])) return 1;
	if (Set(a.values()) != Set([ 2, 4, 6 ])) return 2;
	if (Set(a.items()) != Set([ (1, 2), (3, 4), (5, 6) ])) return 3;
	return 0;
}
)");
	auto val = vm.execute(code).member("r").call();
	ASSERT_EQ(val.as_float(vm), 0);
}

TEST_F(MapTest, iters_mod)
{
	OwcaVM vm;
	auto code = compile(__LINE__, vm, "test.os", R"(
function r() {
	a = {
		1: 2,
		3: 4,
		5: 6
	};

	sum = 0;
	try {
		counter: for(kv = a.items()) {
				sum = sum + kv[0] + kv[1];
				if (counter == 2) {
					a[7] = 9;
				}
		}
		return -1;
	}
	catch(Exception) {
	}
	return sum;
}
)");
	auto val = vm.execute(code).member("r").call();
	ASSERT_EQ(val.as_float(vm), 1 + 2 + 3 + 4 + 5 + 6);
}

TEST_F(MapTest, pop)
{
	OwcaVM vm;
	auto code = compile(__LINE__, vm, "test.os", R"(
function r() {
	a = {
		1: 2,
		3: 4,
		5: 6
	};
	a.pop(3);
	if (a.pop_or_default(7, 1) != 1) return 1;
	if (a.pop_or_default(3, 1) != 1) return 2;
	try {
		a.pop(3);
		return 3;
	}
	catch(Exception) {
	}
	sum = 0;
	for(kv = a.items()) {
		k = kv[0];
		v = kv[1];
		sum = sum + k * v;
	}
	return sum;
}
)");
	auto val = vm.execute(code).member("r").call();
	ASSERT_EQ(val.as_float(vm), 1 * 2 + 5 * 6);
}

