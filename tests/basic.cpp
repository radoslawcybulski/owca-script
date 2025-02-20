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
	ASSERT_EQ(val.as_string(vm).text(), "qwerty");
}

TEST_F(SimpleTest, range)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return 'qwerty'[2:4];
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_string(vm).text(), "er");
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
		std::optional<Function> native_function(std::string_view full_name, FunctionToken token, std::span<const std::string_view> param_names) const override {
			if (full_name == "foo" && param_names.size() == 2 && param_names[0] == "a" && param_names[1] == "b") {
				return [](OwcaVM vm, std::span<OwcaValue> args) -> OwcaValue {
					assert(args.size() == 2);
					return OwcaInt{ args[0].convert_to_int(vm) + args[1].convert_to_int(vm) };
					};
			}
			return std::nullopt;
		}
	};
	auto code = vm.compile("test.os", R"(
function native foo(a, b);
return foo(1, 2);
)", std::make_unique<Provider>());
	auto val = vm.execute(code);
	ASSERT_EQ(val.as_int(vm).internal_value(), 3);
}
TEST_F(SimpleTest, external_vars)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
return q + b + c;
)", std::vector<std::string>{ "q", "b", "c" });
	auto val = vm.execute(code, vm.create_map({
		{ "q", OwcaInt{ 1 }},
		{ "b", OwcaInt{ 2 }},
		{ "c", OwcaInt{ 3 }},
		}));
	ASSERT_EQ(val.as_int(vm).internal_value(), 6);
}
TEST_F(SimpleTest, class_)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
class A {
	function __init__(self, a, b, c) {
		self.value = a + b + c;
	}
}
return A(q, b, c);
)", std::vector<std::string>{ "q", "b", "c" });
	auto val = vm.execute(code, vm.create_map({
		{ "q", OwcaInt{ 1 }},
		{ "b", OwcaInt{ 2 }},
		{ "c", OwcaInt{ 3 }},
		}));
	auto val2 = val.member(vm, "value");
	ASSERT_EQ(val2.as_int(vm).internal_value(), 6);
}

TEST_F(SimpleTest, native_class)
{
	OwcaVM vm;
	struct Provider : public OwcaVM::NativeCodeProvider {
		struct NCI : public OwcaClass::NativeClassInterface {
			void initialize_storage(void* ptr, size_t s) override {
				*(std::uint64_t*)ptr = 1234;
			}
			void destroy_storage(void* ptr, size_t s) override {
			}
			void gc_mark_members(void* ptr, size_t s, GenerationGC generation_gc) override {
			}
			size_t native_storage_size() override {
				return 8;
			}
		};
		std::unique_ptr<OwcaClass::NativeClassInterface> native_class(std::string_view name, ClassToken) const override {
			if (name == "A")
				return std::make_unique<NCI>();
			return nullptr;
		}
	};
	auto code = vm.compile("test.os", R"(
class native A {
	function __init__(self, a, b, c) {
		self.value = a + b + c;
	}
}
return A(q, b, c);
)", std::vector<std::string>{ "q", "b", "c" }, std::make_unique<Provider>());
	auto val = vm.execute(code, vm.create_map({
		{ "q", OwcaInt{ 1 }},
		{ "b", OwcaInt{ 2 }},
		{ "c", OwcaInt{ 3 }},
		}));
	auto val2 = val.member(vm, "value");
	ASSERT_EQ(val2.as_int(vm).internal_value(), 6);
}
