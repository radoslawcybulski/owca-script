#include "test.h"

using namespace OwcaScript;

class StringTest : public SimpleTest {

};

TEST_F(StringTest, f_string_simple_1)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return `qwe{1}rty`; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qwe1rty");
}

TEST_F(StringTest, f_string_simple_2)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return `{1}rty`; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "1rty");
}

TEST_F(StringTest, f_string_simple_3)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return `qwe{1}`; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qwe1");
}

TEST_F(StringTest, f_string_simple_4)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return `qwe{1}rty{2}Z`; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qwe1rty2Z");
}

TEST_F(StringTest, size)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return 'qwerty'.size(); }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_float(vm), 6);
}

TEST_F(StringTest, add_add_add)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return ('qwe' + 'rty') + '123' + '456'; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qwerty123456");
}

TEST_F(StringTest, add_add_mul)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return ('qwe' + 'rty' + '123') * 2; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qwerty123qwerty123");
}

TEST_F(StringTest, add_add_sub_1)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return ('qwe' + 'rty' + '123') [4:6]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "ty");
}

TEST_F(StringTest, add_add_sub_2)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return ('qwe' + 'rty' + '123') [4:8]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "ty12");
}

TEST_F(StringTest, add_add_sub_3)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return ('qwe' + 'rty' + '123') [4:9]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "ty123");
}

TEST_F(StringTest, add_add_sub_4)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return ('qwe' + 'rty' + '123') [3:6]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "rty");
}

TEST_F(StringTest, add_mul_add)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' + 'rty') * 2) + '123'; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qwertyqwerty123");
}

TEST_F(StringTest, add_mul_mul)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' + 'rty') * 2 ) * 2; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qwertyqwertyqwertyqwerty");
}

TEST_F(StringTest, add_mul_sub_1)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' + 'rty') * 2)[2:6]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "erty");
}

TEST_F(StringTest, add_mul_sub_2)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' + 'rty') * 2)[2:5]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "ert");
}

TEST_F(StringTest, add_mul_sub_3)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' + 'rty') * 2)[2:7]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "ertyq");
}

TEST_F(StringTest, add_mul_sub_4)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' + 'rty') * 2)[3:6]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "rty");
}

TEST_F(StringTest, add_sub_add)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' + 'rty')[2:4]) + '123'; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "er123");
}

TEST_F(StringTest, add_sub_mul)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return 4 * (('qwe' + 'rty')[2:4]); }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "erererer");
}

TEST_F(StringTest, add_sub_sub)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' + 'rty')[2:5])[1:2]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "r");
}

TEST_F(StringTest, mul_add_add)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' * 3) + '123') + '456'; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qweqweqwe123456");
}

TEST_F(StringTest, mul_add_mul)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' * 3) + '123') * 2; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qweqweqwe123qweqweqwe123");
}

TEST_F(StringTest, mul_add_sub)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' * 3) + '123')[7:11]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "we12");
}

TEST_F(StringTest, mul_mul_add)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' * 3) * 2) + '123'; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qweqweqweqweqweqwe123");
}

TEST_F(StringTest, mul_mul_mul)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' * 3) * 2) * 1; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "qweqweqweqweqweqwe");
}

TEST_F(StringTest, mul_mul_sub)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r() { return (('qwe' * 3) * 2)[8:14]; }
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.member("r").call().as_string(vm).text(), "eqweqw");
}

