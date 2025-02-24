#include "test.h"

using namespace OwcaScript;

class VariableTest : public SimpleTest {

};

namespace {
    struct Provider : public OwcaVM::NativeCodeProvider {
        int &counter;

        Provider(int &counter) : counter(counter) {}

        struct NCI : public OwcaClass::NativeClassInterface {
            int &counter;
            NCI(int &counter) : counter(counter) {}

            void initialize_storage(void* ptr, size_t s) override {
                *(std::uint64_t*)ptr = 1234;
                ++counter;
            }
            void destroy_storage(void* ptr, size_t s) override {
                ++counter;
            }
            void gc_mark_members(void* ptr, size_t s, OwcaVM, GenerationGC generation_gc) override {
            }
            size_t native_storage_size() override {
                return 8;
            }
        };
        std::unique_ptr<OwcaClass::NativeClassInterface> native_class(std::string_view name, ClassToken) const override {
            if (name == "A")
                return std::make_unique<NCI>(counter);
            return nullptr;
        }
    };
}
TEST_F(VariableTest, simple1)
{
	OwcaVM vm;
    int counter = 0;
	auto code = vm.compile("test.os", R"(
class native A {
}
return A();
)", std::make_unique<Provider>(counter));
    auto provider = Provider{ counter };

	auto val = vm.execute(code);
	ASSERT_EQ(val.as_object(vm).type(), "A");
    ASSERT_EQ(counter, 1);
    vm.run_gc();
    ASSERT_EQ(counter, 2);
}

TEST_F(VariableTest, simple2)
{
	OwcaVM vm;
    int counter = 0;
	auto code = vm.compile("test.os", R"(
class native A {
}
return A();
)", std::make_unique<Provider>(counter));
    auto provider = Provider{ counter };

	auto val = vm.execute(code);
	ASSERT_EQ(val.as_object(vm).type(), "A");
    ASSERT_EQ(counter, 1);

    {
        auto var = OwcaVariable{ vm };
        var = val;
        vm.run_gc();
        ASSERT_EQ(counter, 1);
    }

    vm.run_gc();
    ASSERT_EQ(counter, 2);
}

TEST_F(VariableTest, simple3)
{
	OwcaVM vm;
    int counter = 0;
	auto code = vm.compile("test.os", R"(
class native A {
}
return A();
)", std::make_unique<Provider>(counter));
    auto provider = Provider{ counter };

	auto val = vm.execute(code);
	ASSERT_EQ(val.as_object(vm).type(), "A");
    ASSERT_EQ(counter, 1);

    auto var = OwcaVariable{ vm };
    var = val;
    vm.run_gc();
    ASSERT_EQ(counter, 1);

    var = OwcaValue{};
    vm.run_gc();
    ASSERT_EQ(counter, 2);
}
