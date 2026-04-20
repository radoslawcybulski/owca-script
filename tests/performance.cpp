#include "test.h"

using namespace OwcaScript;

class PerformanceTest : public SimpleTest {

};

TEST_F(PerformanceTest, DISABLED_simple_1)
{
    // GTEST_SKIP();
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
s = 0;
start = time();
i = 0;
while (i < 10000000) {
    s = (s * 11035 + 12345) & 0xffff;
    i = i + 1;
}
end = time();
print(`Time taken: {end - start} seconds`);
print(`Final result: {s}`);
)");
	auto val = vm.execute(code);
}

TEST_F(PerformanceTest, DISABLED_simple_2)
{
    // GTEST_SKIP();
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function foo1(s) {
    return s + 1;
}
function foo2(s) {
    return foo1(s);
}
function foo3(s) {
    return foo2(s);
}
function foo4(s) {
    return foo3(s);
}
function foo5(s) {
    return foo4(s);
}
s = 0;
start = time();
i = 0;
while (i < 100) {
    s = foo5((s * 11035 + 12345) & 0xffff);
    i = i + 1;
}
end = time();
print(`Time taken: {end - start} seconds`);
print(`Final result: {s}`);
)");
	auto val = vm.execute(code);
}

TEST_F(PerformanceTest, DISABLED_book_building)
{
    // GTEST_SKIP();
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
states = {};
final_result = [ 0 ];

function update_state(name, is_bid, val, count) {
    v = states.set_default(name, {});
    key = val * 2;
    if (is_bid) key = key + 1;
    old = v.get_or_default(key, 0);
    new = old + count;
    if (old > 0 and new <= 0) {
        v.pop(key);
    }
    else {
        v[key] = new;
        old = final_result[0];
        final_result[0] = (old * 3 + new) & 0xffffffff;
    }
}

function run() {
    class Random {
        function __init__(self) {
            self.state = 0;
        }
        function next(self) {
            self.state = (self.state * 11035 + 12345) & 0xffff;
            return self.state;
        }
    }
    random = Random();

    for(i = 0:10000000) {
        v = random.next();
        s_index = v % 1000;
        is_bid = (random.next() % 2) == 0;
        val = random.next() % 20;
        count = (random.next() % 10) - 5;
        update_state(s_index, is_bid, val, count);
    }
}
start = time();
run();
end = time();
print("Time taken: " + String(end - start) + " seconds");
print("Final result: " + String(final_result[0]));
)");
	auto val = vm.execute(code);
}
