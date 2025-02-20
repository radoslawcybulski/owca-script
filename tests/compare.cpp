#include "test.h"

using namespace OwcaScript;

class CompareTest : public SimpleTest {
public:
    void order_tuple(unsigned int expected, std::optional<int> p3 = std::nullopt, std::optional<int> p4 = std::nullopt) {
        order_array(expected, p3, p4, true);
    }
    void order_array(unsigned int expected, std::optional<int> p3 = std::nullopt, std::optional<int> p4 = std::nullopt, bool is_tuple = false) {
        OwcaVM vm;
        std::vector<OwcaValue> pp;
        pp.push_back(OwcaInt{ 1 });
        pp.push_back(OwcaInt{ 2 });
        if (p3) {
            pp.push_back(OwcaInt{ *p3 });
            if (p4) {
                pp.push_back(OwcaInt{ *p4 });
            }
        }
        auto code = vm.compile("test.os", std::format(R"(
a = {} 1, 2, 3 {};
result = 0;
if (a == b) result = result | 1;
if (a != b) result = result | 2;
if (a <= b) result = result | 4;
if (a >= b) result = result | 8;
if (a <  b) result = result | 16;
if (a >  b) result = result | 32;
return result;
    )", is_tuple ? "(" : "[", is_tuple ? ")" : "]"), std::vector<std::string>{ "b" });
        auto val = vm.execute(code, vm.create_map({ { "b", is_tuple ? vm.create_tuple(std::move(pp)) : vm.create_array(std::move(pp)) } }));
        ASSERT_EQ(val.as_int(vm).internal_value(), expected);
    }
    void run_basic_test(int mode) {
        OwcaVM vm;

        auto code = vm.compile("test.os", R"(
    function cmp(a, b, c, d) {
        if (a == b) {
            if (not c) return d + 1;
        }
        if (a != b) {
            if (c) return d + 2;
        }
        return 0;
    }

    class Q {}
    q = Q();
    q2 = Q();

    objects = {
        0: nul,
        1: (1:2),
        2: true,
        3: 1,
        4: 1.0,
        5: "1",
        6: cmp,
        7: { 1: 2 },
        8: String,
        9: q,
        10: (1, 2),
        11: [ 1, 2 ],
        12: { 1, 2 },
    };
    function create() {
        return objects[mode];
    }
    d = cmp(create(), nul, mode == 0, 10);
    if (d) return d;

    d = cmp(create(), nul, mode == 0, 15);
    if (d) return d;

    d = cmp(create(), (1:2), mode == 1, 20);
    if (d) return d;

    d = cmp(create(), (2:3), false, 25);
    if (d) return d;

    d = cmp(create(), true, mode == 2, 30);
    if (d) return d;

    d = cmp(create(), false, false, 35);
    if (d) return d;

    d = cmp(create(), 1, mode == 3 or mode == 4, 40);
    if (d) return d;

    d = cmp(create(), 2, false, 45);
    if (d) return d;

    d = cmp(create(), 1.0, mode == 4 or mode == 3, 50);
    if (d) return d;

    d = cmp(create(), 1.5, false, 55);
    if (d) return d;

    d = cmp(create(), "1", mode == 5, 60);
    if (d) return d;

    d = cmp(create(), "2", false, 65);
    if (d) return d;

    d = cmp(create(), cmp, mode == 6, 70);
    if (d) return d;

    d = cmp(create(), create, false, 75);
    if (d) return d;

    d = cmp(create(), { 1 : 2 }, mode == 7, 80);
    if (d) return d;

    d = cmp(create(), { 1: 2, 3: 4 }, false, 85);
    if (d) return d;

    d = cmp(create(), String, mode == 8, 90);
    if (d) return d;

    d = cmp(create(), Int, false, 95);
    if (d) return d;

    d = cmp(create(), q, mode == 9, 100);
    if (d) return d;

    d = cmp(create(), q2, false, 105);
    if (d) return d;

    d = cmp(create(), (1, 2), mode == 10, 110);
    if (d) return d;

    d = cmp(create(), (1, ), false, 115);
    if (d) return d;

    d = cmp(create(), [ 1, 2 ], mode == 11, 120);
    if (d) return d;

    d = cmp(create(), [ 1, 3 ], false, 125);
    if (d) return d;

    d = cmp(create(), { 1, 2 }, mode == 12, 120);
    if (d) return d;

    d = cmp(create(), { 1, 3 }, false, 125);
    if (d) return d;

    return 0;
    )", std::vector<std::string>{ "mode" });
        auto val = vm.execute(code, vm.create_map({ { "mode", OwcaInt(mode) } }));
        ASSERT_EQ(val.as_int(vm).internal_value(), 0);
    }
};

TEST_F(CompareTest, nul)
{
    run_basic_test(0);
}

TEST_F(CompareTest, range)
{
    run_basic_test(1);
}

TEST_F(CompareTest, bool)
{
    run_basic_test(2);
}

TEST_F(CompareTest, int)
{
    run_basic_test(3);
}

TEST_F(CompareTest, float)
{
    run_basic_test(4);
}

TEST_F(CompareTest, string)
{
    run_basic_test(5);
}

TEST_F(CompareTest, function)
{
    run_basic_test(6);
}

TEST_F(CompareTest, map)
{
    run_basic_test(7);
}

TEST_F(CompareTest, class)
{
    run_basic_test(8);
}

TEST_F(CompareTest, object)
{
    run_basic_test(9);
}

TEST_F(CompareTest, tuple)
{
    run_basic_test(10);
}

TEST_F(CompareTest, array)
{
    run_basic_test(11);
}

TEST_F(CompareTest, set)
{
    run_basic_test(12);
}

// if (a == b) result = result | 1;
// if (a != b) result = result | 2;
// if (a <= b) result = result | 4;
// if (a >= b) result = result | 8;
// if (a <  b) result = result | 16;
// if (a >  b) result = result | 32;

static constexpr unsigned int Eq = 1, NotEq = 2, LessEq = 4, MoreEq = 8, Less = 16, More = 32, All = Eq | NotEq | LessEq | MoreEq | Less | More;

TEST_F(CompareTest, array_order_eq)
{
    order_array(Eq | LessEq | MoreEq, 3);
}

TEST_F(CompareTest, array_order_longer)
{
    order_array(NotEq | More | MoreEq);
}

TEST_F(CompareTest, array_order_shorter)
{
    order_array(NotEq | Less | LessEq, 3, 4);
}

TEST_F(CompareTest, array_order_bigger)
{
    order_array(NotEq | More | MoreEq, 2);
}

TEST_F(CompareTest, array_order_smaller)
{
    order_array(NotEq | Less | LessEq, 4);
}

TEST_F(CompareTest, tuple_order_eq)
{
    order_tuple(Eq | LessEq | MoreEq, 3);
}

TEST_F(CompareTest, tuple_order_longer)
{
    order_tuple(NotEq | More | MoreEq);
}

TEST_F(CompareTest, tuple_order_shorter)
{
    order_tuple(NotEq | Less | LessEq, 3, 4);
}

TEST_F(CompareTest, tuple_order_bigger)
{
    order_tuple(NotEq | More | MoreEq, 2);
}

TEST_F(CompareTest, tuple_order_smaller)
{
    order_tuple(NotEq | Less | LessEq, 4);
}