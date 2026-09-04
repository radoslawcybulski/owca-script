#include "test.h"

using namespace OwcaScript;

class TryTest : public SimpleTest {
public:
    static int run(int mode, unsigned int code_line = 0, std::string code_str = "")
    {
        if (code_str.empty()) {
            code_line = __LINE__ + 1;
            code_str = R"(
class A(Exception) {}
class B(Exception) {}
function r(a) {
    try {
        if (a == 1) throw A("q");
        if (a == 2) throw B("q");
    }
    catch(e: A) {
        return 1;
    }
    catch(e: B) {
        return 2;
    }
    return 3;
}
)";
        }
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

TEST_F(TryTest, simple1)
{
    auto val = run(1);
	ASSERT_EQ(val, 1);
}

TEST_F(TryTest, simple2)
{
    auto val = run(2);
	ASSERT_EQ(val, 2);
}

TEST_F(TryTest, simple3)
{
    auto val = run(3);
	ASSERT_EQ(val, 3);
}

TEST_F(TryTest, try_try)
{
    auto val = run(0, __LINE__, R"(
function r(a) {
    try {
        try {
            throw Exception("q");
        }
        catch(e) {
            if (e.inner_exception()) return $line;
            if (e.message() != "q") return $line;
            throw Exception("w");
        }
        return $line;
    }
    catch(e) {
        if (e.message() != "w") return $line;
        if (not e.inner_exception()) return $line;
        if (e.inner_exception().message() != "q") return $line;
    }
    return 0;
}
)");
	ASSERT_EQ(val, 1);
}

