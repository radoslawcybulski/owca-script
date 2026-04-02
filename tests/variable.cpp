#include "test.h"

using namespace OwcaScript;

class VariableTest : public SimpleTest {

};

namespace {
    struct Provider : public NativeCodeProvider {
        int &counter;

        Provider(int &counter) : counter(counter) {}

        struct NCI : public NativeClassInterface {
            int &counter;
            NCI(int &counter) : counter(counter) {}

            void initialize_storage(void* ptr, size_t s) override {
                *(std::uint64_t*)ptr = 1234;
                counter += 1;
            }
            void destroy_storage(void* ptr, size_t s) override {
                counter += 1000;
            }
            void gc_mark_members(const void* ptr, size_t s, OwcaVM, GenerationGC generation_gc) override {
                counter += 1000000;
            }
            size_t native_storage_size() override {
                return 8;
            }
        };
        std::shared_ptr<NativeClassInterface> native_class(std::string_view name, ClassToken) const override {
            if (name == "A")
                return std::make_shared<NCI>(counter);
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
)", std::make_shared<Provider>(counter));
    auto provider = Provider{ counter };

	auto val = vm.execute(code);
	ASSERT_EQ(val.as_object(vm).type(), "A");
    ASSERT_EQ(counter, 1);
    vm.run_gc();
    ASSERT_EQ(counter, 1001);
}
