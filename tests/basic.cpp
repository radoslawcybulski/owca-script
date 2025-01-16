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
return 'qwerty'[2:4];
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_string(vm).internal_value(), "er");
}
TEST_F(SimpleTest, dict)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
v = { 'a': 1, 'b': 2 };
return v['a'] + v['b'];
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_int(vm).internal_value(), 3);
}
TEST_F(SimpleTest, native_func)
{
	OwcaVM vm;
	struct Provider : public OwcaVM::NativeCodeProvider {
		std::optional<Function> native_function(std::string_view name, std::span<const std::string> param_names) const override {
			if (name == "foo" && param_names.size() == 2 && param_names[0] == "a" && param_names[1] == "b") {
				return [](OwcaVM& vm, std::span<OwcaValue> args) -> OwcaValue {
					assert(args.size() == 2);
					return OwcaInt{ args[0].convert_to_int(vm) + args[1].convert_to_int(vm) };
					};
			}
			return std::nullopt;
		}
	};
	auto code = vm.compile("test.os", R"(
function foo(a, b);
return foo(1, 2);
)", Provider{});
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_int(vm).internal_value(), 3);
}
TEST_F(SimpleTest, external_vars)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return q + b + c;
)", std::vector<std::string>{ "q", "b", "c" });
	auto val = vm.execute(code, { {
		{ "q", OwcaInt{ 1 }},
		{ "b", OwcaInt{ 2 }},
		{ "c", OwcaInt{ 3 }},
		} });
	ASSERT_EQ(val.as_int(vm).internal_value(), 6);
}
