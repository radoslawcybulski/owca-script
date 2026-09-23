#include "test.h"

using namespace OwcaScript;

class VariableTest : public SimpleTest {
public:
    static int run(int mode, unsigned int code_line, std::string code_str)
    {
        OwcaVM vm;
        auto code = vm.compile("test.os", code_str, code_line);
        try {
            auto val = vm.execute(code);
            return (int)val.member("r").call(mode).as_int();
        }
        catch(OwcaException oe) {
            auto o = oe.frame(0);
            std::cout << "OwcaException: " << o.filename << ":" << o.line << ": " << oe.message() << std::endl;
            return -1;
        }
        catch(std::exception &e) {
            std::cout << "std::exception: " << e.what() << std::endl;
            return -2;
        }
        catch(...) {
            return -3;
        }
    }
};

namespace {
    struct Provider : public NativeCodeProvider {
        int &counter;

        Provider(int &counter) : counter(counter) {}

        struct NCI : public NativeClassInterfaceImplementation<std::uint64_t> {
            int &counter;
            NCI(int &counter) : counter(counter) {}

            void initialize_storage(void* ptr, size_t s) override {
                *(std::uint64_t*)ptr = 1234;
                counter += 1;
            }
            void destroy_storage(void* ptr, size_t s) override {
                counter += 1000;
            }
            void gc_mark_members(const void* ptr, size_t s, GenerationGC generation_gc) override {
                counter += 1000000;
            }
            size_t native_storage_size() override {
                return 8;
            }
        };
        std::shared_ptr<NativeClassInterface> native_class(std::string_view name) const override {
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
function r() {
    return A();
}
)");
    auto provider = Provider{ counter };

	auto val = vm.execute(code, std::make_shared<Provider>(counter));
	ASSERT_EQ(val.member("r").call().as_object().type(), "A");
    ASSERT_EQ(counter, 1);
    vm.run_gc();
    ASSERT_EQ(counter, 1001);
}

TEST_F(VariableTest, simple2)
{
    auto val = run(1, __LINE__ + 1, R"(
tmp = 1;
function f() {
    tmp = 2;
}
function r(a) {
    f();
    if (tmp == 1) return 1;
    return 0;
}
)");
	ASSERT_EQ(val, 1);
}

TEST_F(VariableTest, simple3)
{
    auto val = run(1, __LINE__ + 1, R"(
function r(a) {
    tmp = 1;
    function f1() {
        tmp = 2;
    }
    function f2() {
        return tmp;
    }
    f1();
    if (f2() == 1) return 1;
    return 0;
}
)");
	ASSERT_EQ(val, 1);
}

TEST_F(VariableTest, simple4)
{
    auto val = run(1, __LINE__ + 1, R"(
function r(a) {
    tmp = 1;
    function f1() {
        tmp = 2;
    }
    f1();
    function f2() {
        return tmp;
    }
    if (f2() == 1) return 1;
    return 0;
}
)");
	ASSERT_EQ(val, 1);
}

TEST_F(VariableTest, simple5)
{
    auto val = run(1, __LINE__ + 1, R"(
function r(a) {
    tmp = 1;
    function f1() {
        tmp = 2;
    }
    tmp = 2;
    function f2() {
        return tmp;
    }
    if (f2() == 2) return 1;
    return 0;
}
)");
	ASSERT_EQ(val, 1);
}

TEST_F(VariableTest, simple6)
{
    auto val = run(1, __LINE__ + 1, R"(
tmp = [ 1 ];
function f() {
    tmp[0] = 2;
}
function r(a) {
    f();
    if (tmp[0] == 2) return 1;
    return 0;
}
)");
	ASSERT_EQ(val, 1);
}

TEST_F(VariableTest, simple7)
{
    auto val = run(1, __LINE__ + 1, R"(
function r(a) {
    tmp = [ 1 ];
    function f1() {
        tmp[0] = 2;
    }
    function f2() {
        return tmp[0];
    }
    f1();
    if (f2() == 2) return 1;
    return 0;
}
)");
	ASSERT_EQ(val, 1);
}
