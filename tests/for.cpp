#include "test.h"

using namespace OwcaScript;

class ForTest : public SimpleTest {

};

static int run_for(std::string code_text, OwcaValue add_val)
{
    OwcaVM vm;
    std::vector<std::string> tmp{ { "a" } };
    auto code = vm.compile("test.os", std::move(code_text), tmp);
    auto val = vm.execute(code, vm.create_map({ { "a", add_val } }));
    return (int)val.as_int(vm).internal_value();
}

TEST_F(ForTest, simple1)
{
    ASSERT_EQ(run_for(R"(
total = 0;
arr = [ 0, 1, 2 ];
for(i = arr) {
    if (i == 1) {
        if (a == 1) break;
        if (a == 2) continue;
    }
    total = total + 1;
}
return total;
	)", OwcaInt{ 0 }), 3);
}

TEST_F(ForTest, simple2)
{
    ASSERT_EQ(run_for(R"(
total = 0;
arr = [ 0, 1, 2 ];
for(i = arr) {
    if (i == 1) {
        if (a == 1) break;
        if (a == 2) continue;
    }
    total = total + 1;
}
return total;
	)", OwcaInt{ 1 }), 1);
}

TEST_F(ForTest, simple3)
{
    ASSERT_EQ(run_for(R"(
total = 0;
arr = [ 0, 1, 2 ];
for(i = arr) {
    if (i == 1) {
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

TEST_F(ForTest, loop_ident1)
{
    ASSERT_EQ(run_for(R"(
mode = 0;
arr = [ 0, 1, 2 ];
l1: for(i = arr) {
    l2: for(j = arr) {
        l3: for(k = arr) {
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
TEST_F(ForTest, loop_ident2)
{
    ASSERT_EQ(run_for(R"(
mode = 0;
arr = [ 0, 1, 2 ];
l1: for(i = arr) {
    l2: for(j = arr) {
        l3: for(k = arr) {
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
TEST_F(ForTest, loop_ident3)
{
    ASSERT_EQ(run_for(R"(
mode = 0;
arr = [ 0, 1, 2 ];
l1: for(i = arr) {
    l2: for(j = arr) {
        l3: for(k = arr) {
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
TEST_F(ForTest, loop_ident4)
{
    ASSERT_EQ(run_for(R"(
mode = 0;
arr = [ 0, 1, 2 ];
l1: for(i = arr) {
    l2: for(j = arr) {
        l3: for(k = arr) {
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
