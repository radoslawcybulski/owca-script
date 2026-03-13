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
	struct Provider : public NativeCodeProvider {
		std::optional<Function> native_function(std::string_view full_name, std::optional<ClassToken> cls, FunctionToken token, std::span<const std::string_view> param_names) const override {
			if (full_name == "foo" && !cls && param_names.size() == 2 && param_names[0] == "a" && param_names[1] == "b") {
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
	auto map_data = std::vector<std::pair<std::string, OwcaValue>>{ { { "q", OwcaInt{ 1 } }, { "b", OwcaInt{ 2 } }, { "c", OwcaInt{ 3 } } } };
	auto val = vm.execute(code, vm.create_map(map_data));

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
	auto map_data = std::vector<std::pair<std::string, OwcaValue>>{ { { "q", OwcaInt{ 1 } }, { "b", OwcaInt{ 2 } }, { "c", OwcaInt{ 3 } } } };
	auto val = vm.execute(code, vm.create_map(map_data));
	auto val2 = val.member(vm, "value");
	ASSERT_EQ(val2.as_int(vm).internal_value(), 6);
}

TEST_F(SimpleTest, native_class)
{
	OwcaVM vm;
	struct Provider : public NativeCodeProvider {
		struct NCI : public NativeClassInterface {
			void initialize_storage(void* ptr, size_t s) override {
				*(std::uint64_t*)ptr = 1234;
			}
			void destroy_storage(void* ptr, size_t s) override {
			}
			void gc_mark_members(void* ptr, size_t s, OwcaVM, GenerationGC generation_gc) override {
			}
			size_t native_storage_size() override {
				return 8;
			}
		};
		std::unique_ptr<NativeClassInterface> native_class(std::string_view name, ClassToken) const override {
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
	auto map_data = std::vector<std::pair<std::string, OwcaValue>>{ { { "q", OwcaInt{ 1 } }, { "b", OwcaInt{ 2 } }, { "c", OwcaInt{ 3 } } } };
	auto val = vm.execute(code, vm.create_map(map_data));
	auto val2 = val.member(vm, "value");
	ASSERT_EQ(val2.as_int(vm).internal_value(), 6);
}

TEST_F(SimpleTest, native_class_with_funcs)
{
	OwcaVM vm;
	try {
		struct Provider : public NativeCodeProvider {
			struct NCI : public NativeClassInterface {
				void initialize_storage(void* ptr, size_t s) override {
					*(std::uint64_t*)ptr = 1234;
				}
				void destroy_storage(void* ptr, size_t s) override {
				}
				void gc_mark_members(void* ptr, size_t s, OwcaVM, GenerationGC generation_gc) override {
				}
				size_t native_storage_size() override {
					return sizeof(std::uint64_t);
				}
			};
			std::unique_ptr<NativeClassInterface> native_class(std::string_view name, ClassToken) const override {
				if (name == "A")
					return std::make_unique<NCI>();
				return nullptr;
			}
			std::optional<Function> native_function(std::string_view full_name, std::optional<ClassToken> cls, FunctionToken token, std::span<const std::string_view> param_names) const override {
				if (full_name == "A.set_value" && cls && param_names.size() == 2) {
					return [cls = *cls](OwcaVM vm, std::span<OwcaValue> args) -> OwcaValue {
						assert(args.size() == 2);
						auto self = args[0];
						auto v = args[1].convert_to_int(vm);
						self.as_object(vm).user_data<std::uint64_t>(cls) = v;
						return {};
					};
				}
				if (full_name == "A.get_value" && cls && param_names.size() == 1) {
					return [cls = *cls](OwcaVM vm, std::span<OwcaValue> args) -> OwcaValue {
						assert(args.size() == 1);
						auto self = args[0];
						auto v = self.as_object(vm).user_data<std::uint64_t>(cls);
						return OwcaInt{ static_cast<OwcaIntInternal>(v) };
					};
				}
				return std::nullopt;
			}
		};
		auto code = vm.compile("test.os", R"(
	class native A {
		function __init__(self, a, b, c) {
			self.set_value(a + b + c);
		}
		function native set_value(self, v);
		function native get_value(self);
	}
	return A(q, b, c);
	)", std::vector<std::string>{ "q", "b", "c" }, std::make_unique<Provider>());
		auto map_data = std::vector<std::pair<std::string, OwcaValue>>{ { { "q", OwcaInt{ 1 } }, { "b", OwcaInt{ 2 } }, { "c", OwcaInt{ 3 } } } };
		auto val = vm.execute(code, vm.create_map(map_data));
		
		code = vm.compile("test.os", "return a.get_value();", std::vector<std::string>{ "a" });
		map_data = std::vector<std::pair<std::string, OwcaValue>>{ { { "a", val } } };
		auto val2 = vm.execute(code, vm.create_map(map_data));
		ASSERT_EQ(val2.as_int(vm).internal_value(), 6);
	}
	catch(std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
		throw;
	}
	catch(OwcaException e) {
		std::cerr << "Exception: " << e.to_string() << "\n";
		throw;
	}
}

TEST_F(SimpleTest, native_class_with_vars)
{
	OwcaVM vm;
	try {
		struct Provider : public NativeCodeProvider {
			unsigned int &reads, &writes;
			Provider(unsigned int &reads, unsigned int &writes) : reads(reads), writes(writes) {}

			struct NCI : public NativeClassInterface {
				unsigned int &reads, &writes;
				NCI(unsigned int &reads, unsigned int &writes) : reads(reads), writes(writes) {}

				void initialize_storage(void* ptr, size_t s) override {
					*(std::uint64_t*)ptr = 1234;
				}
				void destroy_storage(void* ptr, size_t s) override {
				}
				void gc_mark_members(void* ptr, size_t s, OwcaVM, GenerationGC generation_gc) override {
				}
				size_t native_storage_size() override {
					return sizeof(std::uint64_t);
				}
				void get_member(OwcaVM vm, std::string_view name, std::span<char> native_storage, OwcaValue &val) override {
					if (name == "value") {
						++reads;
						auto v = *(std::uint64_t*)native_storage.data();
						val = OwcaInt{ static_cast<OwcaIntInternal>(v) };
						return;
					}
					NativeClassInterface::get_member(vm, name, native_storage, val);
				}
				void set_member(OwcaVM vm, std::string_view name, std::span<char> native_storage, const OwcaValue &val) override {
					if (name == "value") {
						++writes;
						*(std::uint64_t*)native_storage.data() = val.as_int(vm).internal_value();
						return;
					}
					NativeClassInterface::set_member(vm, name, native_storage, val);
				}
			};
			std::unique_ptr<NativeClassInterface> native_class(std::string_view name, ClassToken) const override {
				if (name == "A")
					return std::make_unique<NCI>(reads, writes);
				return nullptr;
			}
		};
		unsigned int reads = 0, writes = 0;
		auto code = vm.compile("test.os", R"(
	class native A {
		function __init__(self, a, b, c) {
			self.value = a + b + c;
		}
		var value;
	}
	return A(q, b, c);
	)", std::vector<std::string>{ "q", "b", "c" }, std::make_unique<Provider>(reads, writes));
		auto map_data = std::vector<std::pair<std::string, OwcaValue>>{ { { "q", OwcaInt{ 1 } }, { "b", OwcaInt{ 2 } }, { "c", OwcaInt{ 3 } } } };
		auto val = vm.execute(code, vm.create_map(map_data));
		ASSERT_EQ(writes, 1);
		ASSERT_EQ(reads, 0);
		
		code = vm.compile("test.os", "return a.value;", std::vector<std::string>{ "a" });
		map_data = std::vector<std::pair<std::string, OwcaValue>>{ { { "a", val } } };
		auto val2 = vm.execute(code, vm.create_map(map_data));
		ASSERT_EQ(val2.as_int(vm).internal_value(), 6);
		ASSERT_EQ(writes, 1);
		ASSERT_EQ(reads, 1);
	}
	catch(std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
		throw;
	}
	catch(OwcaException e) {
		std::cerr << "Exception: " << e.to_string() << "\n";
		throw;
	}
}
