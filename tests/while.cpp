#include "test.h"

using namespace OwcaScript;

class WhileTest : public SimpleTest {

};

static int run_while(std::string code_text, OwcaValue add_val)
{
    OwcaVM vm;
    auto code = vm.compile("test.os", std::move(code_text));
    auto val = vm.execute(code);
    return (int)val.member("r").call(add_val).as_int(vm);
}
TEST_F(WhileTest, simple1)
{
    ASSERT_EQ(run_while(R"(
function r(a) {
    i = 0;
    total = 0;
    while(i < 3) {
        if (i == 1) {
            if (a == 1) break;
            if (a == 2) continue;
        }
        total = total + 1;
        i = i + 1;
    }
    return total;
}
	)", 0), 3);
}

TEST_F(WhileTest, simple2)
{
    ASSERT_EQ(run_while(R"(
function r(a) {
    i = 0;
    total = 0;
    while(i < 3) {
        if (i == 1) {
            if (a == 1) break;
            if (a == 2) continue;
        }
        total = total + 1;
        i = i + 1;
    }
    return total;
}
	)", 1), 1);
}

TEST_F(WhileTest, simple3)
{
    ASSERT_EQ(run_while(R"(
function r(a) {
    i = 0;
    total = 0;
    while(i < 3) {
        i = i + 1;
        if (i == 2) {
            if (a == 1) break;
            if (a == 2) {
                a = 0;
                continue;
            }
        }
        total = total + 1;
    }
    return total;
}
	)", 2), 2);
}

TEST_F(WhileTest, loop_ident1)
{
    ASSERT_EQ(run_while(R"(
function r(a) {
    mode = 0;
    l1: while(true) {
        l2: while(true) {
            l3: while(true) {
                break l1;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    return mode;
}
	)", 0), 0);
}
TEST_F(WhileTest, loop_ident2)
{
    ASSERT_EQ(run_while(R"(
function r(a) {
    mode = 0;
    l1: while(true) {
        l2: while(true) {
            l3: while(true) {
                break l2;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    return mode;
}
	)", 0), 2);
}
TEST_F(WhileTest, loop_ident3)
{
    ASSERT_EQ(run_while(R"(
function r(a) {
    mode = 0;
    l1: while(true) {
        l2: while(true) {
            l3: while(true) {
                break l3;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    return mode;
}
	)", 0), 3);
}
TEST_F(WhileTest, loop_ident4)
{
    ASSERT_EQ(run_while(R"(
function r(a) {
    mode = 0;
    l1: while(true) {
        l2: while(true) {
            l3: while(true) {
                break;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    return mode;
}
	)", 0), 3);
}
