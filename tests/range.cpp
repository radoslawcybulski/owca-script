#include "test.h"

using namespace OwcaScript;

class RangeTest : public SimpleTest {

};

TEST_F(RangeTest, simple_1)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
sum = 0;
for(i = 1:10:2) {
    sum = sum + i * i;
}
return sum;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 165);
}

TEST_F(RangeTest, simple_2)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
sum = 0;
for(i = 1:10:-2) {
    sum = sum + i * i;
}
return sum;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 0);
}

TEST_F(RangeTest, simple_3)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
sum = 0;
for(i = 1:-10:2) {
    sum = sum + i * i;
}
return sum;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 0);
}

TEST_F(RangeTest, simple_4)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
sum = 0;
for(i = -1:10:2) {
    sum = sum + i * i;
}
return sum;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 166);
}

TEST_F(RangeTest, simple_5)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
sum = 0;
for(i = -1:-10:-2) {
    sum = sum + i * i;
}
return sum;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 165);
}

TEST_F(RangeTest, size_1)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return (1:10:2).size();
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 5);
}

TEST_F(RangeTest, size_2)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return (1:10:-2).size();
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 0);
}

TEST_F(RangeTest, array_1)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
sum = 0;
tmp = [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
for(i = tmp[0:9]) {
    sum = sum + i * i;
}
return sum;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 285);
}

TEST_F(RangeTest, array_2)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
sum = 0;
tmp = [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
for(i = tmp[0:9:2]) {
    sum = sum + i * i;
}
return sum;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 165);
}

TEST_F(RangeTest, array_3)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
sum = 0;
tmp = [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
for(i = tmp[:9]) {
    sum = sum + i * i;
}
return sum;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 285);
}

TEST_F(RangeTest, array_4)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
sum = 0;
tmp = [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
for(i = tmp[2:]) {
    sum = sum + i * i;
}
return sum;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 280);
}

TEST_F(RangeTest, array_5)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
sum = 0;
tmp = [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
for(i = tmp[2::2]) {
    sum = sum + i * i;
}
return sum;
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_float(vm), 164);
}

