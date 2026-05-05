#include "test.h"

using namespace OwcaScript;

class TryTest : public SimpleTest {
public:
    static int run(int mode)
    {
        OwcaVM vm;
        auto code = vm.compile("test.os", R"(
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
)");
        try {
            auto val = vm.execute(code);
            return (int)val.member("r").call(mode).as_int(vm);
        }
        catch(OwcaException oe) {
            return -1;
        }
        catch(std::exception &e) {
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
