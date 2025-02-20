#include "test.h"

using namespace OwcaScript;

class WhileTest : public SimpleTest {

};

static int run_while(std::string code_text, OwcaValue add_val)
{
    OwcaVM vm;
    std::vector<std::string> tmp{ { "a" } };
    auto code = vm.compile("test.os", std::move(code_text), tmp);
    auto val = vm.execute(code, vm.create_map({ { "a", add_val } }));
    return (int)val.as_int(vm).internal_value();
}
TEST_F(WhileTest, simple1)
{
    ASSERT_EQ(run_while(R"(
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
	)", OwcaInt{ 0 }), 3);
}

TEST_F(WhileTest, simple2)
{
    ASSERT_EQ(run_while(R"(
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
	)", OwcaInt{ 1 }), 1);
}

TEST_F(WhileTest, simple3)
{
    ASSERT_EQ(run_while(R"(
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
	)", OwcaInt{ 2 }), 2);
}

TEST_F(WhileTest, loop_ident1)
{
    ASSERT_EQ(run_while(R"(
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
	)", OwcaInt{ 0 }), 0);
}
TEST_F(WhileTest, loop_ident2)
{
    ASSERT_EQ(run_while(R"(
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
	)", OwcaInt{ 0 }), 2);
}
TEST_F(WhileTest, loop_ident3)
{
    ASSERT_EQ(run_while(R"(
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
	)", OwcaInt{ 0 }), 3);
}
TEST_F(WhileTest, loop_ident4)
{
    ASSERT_EQ(run_while(R"(
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
	)", OwcaInt{ 0 }), 3);
}
