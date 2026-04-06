#include "test.h"

using namespace OwcaScript;

class SetTest : public SimpleTest {

};

TEST_F(SetTest, size)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
a = { 1, 2, 3, 4 };
return a.size();
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 4);
}

static std::vector<double> to_sorted_vector(OwcaValue v, OwcaVM vm) {
    std::vector<double> res;
    for (auto item : v.as_set(vm)) {
        res.push_back(item.as_float(vm));
    }
    std::sort(res.begin(), res.end());
    return res;
}

TEST_F(SetTest, simple)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
a = { 1, 2, 3, 4 };
a.add(5);
a.remove(3);
return a;
)");
	auto val = vm.execute(code);
    auto data = to_sorted_vector(val, vm);
	ASSERT_EQ(data, std::vector<double>({ 1, 2, 4, 5 }));
}

TEST_F(SetTest, union)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
a = { 1, 2, 3, 4 };
b = { 3, 4, 5, 6 };
a = a.union_with(b);
return a;
)");
	auto val = vm.execute(code);
    auto data = to_sorted_vector(val, vm);
	ASSERT_EQ(data, std::vector<double>({ 1, 2, 3, 4, 5, 6 }));
}

TEST_F(SetTest, intersection)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
a = { 1, 2, 3, 4 };
b = { 3, 4, 5, 6 };
a = a.intersection_with(b);
return a;
)");
	auto val = vm.execute(code);
    auto data = to_sorted_vector(val, vm);
	ASSERT_EQ(data, std::vector<double>({ 3, 4 }));
}

TEST_F(SetTest, difference)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
a = { 1, 2, 3, 4 };
b = { 3, 4, 5, 6 };
a = a.difference_with(b);
return a;
)");
	auto val = vm.execute(code);
    auto data = to_sorted_vector(val, vm);
	ASSERT_EQ(data, std::vector<double>({ 1, 2 }));
}
