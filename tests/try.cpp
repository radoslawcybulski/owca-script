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
        b = a == 1;
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
	ASSERT_EQ(val, 0);
}

TEST_F(TryTest, try_try_2)
{
    auto val = run(0, __LINE__, R"(
exceptions = [];
function get_message(e) {
    msg = '';
    while(e) {
        print(`e: {e}`);
        if (msg) msg += ";";
        msg += e.message();
        e = e.inner_exception();
    }
    print(`msg: {msg}`);
    return msg;
}
function r5() {
    throw Exception("r5");
}
function r4() {
    throw Exception("r4");
}
function r3() {
    try {
        r4();
    }
    catch(e) {
        exceptions.push_back(get_message(e));
        r5();
    }
}
function r2() {
    try {
        r3();
    }
    catch(e) {
        exceptions.push_back(get_message(e));
    }
    return 0;
}
function r(a) {
    r2();
    if (exceptions != [
        'r4',
        'r5;r4',
    ]) {
        index = 0;
        for(v = exceptions) {
            index += 1;
            print(`{index}: {v}`);
        }
        return 0;
    }
    return 1;
}
)");
	ASSERT_EQ(val, 1);
}

TEST_F(TryTest, try_try_3)
{
    auto val = run(0, __LINE__, R"(
exceptions = [];
function get_message(e) {
    msg = '';
    while(e) {
        if (msg) msg += ";";
        msg += e.message();
        e = e.inner_exception();
    }
    return msg;
}
function r5() {
    try {
        throw Exception("r5");
    }
    catch(e) {
        exceptions.push_back(get_message(e));
    }
}
function r4() {
    throw Exception("r4");
}
function r3() {
    try {
        r4();
    }
    catch(e) {
        exceptions.push_back(get_message(e));
        r5();
        throw;
    }
}
function r2() {
    try {
        r3();
    }
    catch(e) {
        exceptions.push_back(get_message(e));
    }
    return 0;
}
function r(a) {
    r2();
    if (exceptions != [
        'r4',
        'r5;r4',
        'r4',
    ]) {
        index = 0;
        for(v = exceptions) {
            index += 1;
            print(`{index}: {v}`);
        }
        return 0;
    }
    return 1;
}
)");
	ASSERT_EQ(val, 1);
}

TEST_F(TryTest, try_break)
{
    auto val = run(0, __LINE__, R"(
exceptions = [];
function get_message(e) {
    msg = '';
    while(e) {
        if (msg) msg += ";";
        msg += e.message();
        e = e.inner_exception();
    }
    return msg;
}

function r(a) {
    try {
        while(true) {
            try {
                throw Exception("q");
            }
            catch(e) {
                exceptions.push_back(get_message(e));
                break;
            }
        }
        throw Exception("w");
    }
    catch(e) {
        exceptions.push_back(get_message(e));
        return 0;
    }
    if (exceptions != [
        'q',
        'w',
    ]) {
        index = 0;
        for(v = exceptions) {
            index += 1;
            print(`{index}: {v}`);
        }
        return $line;
    }
   return 0;
}
)");
	ASSERT_EQ(val, 0);
}

