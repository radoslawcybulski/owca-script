#include "test.h"

using namespace OwcaScript;

class PerformanceTest : public SimpleTest {

};

TEST_F(PerformanceTest, DISABLED_book_building)
{
    // GTEST_SKIP();
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
states = {};
function update_state(name, is_bid, val, count) {
    v = states.set_default(name, {});
    if (count > 0)
        key = '+';
    else
        key = '-';
    key = key + String(val);
    old = v.get_or_default(key, 0);
    new = old + count;
    if (old > 0 and new <= 0) {
        v.pop(key);
    }
    else {
        v[key] = new;
    }
}

function run() {
    class Random {
        function __init__(self) {
            self.state = 0;
        }
        function next(self) {
            self.state = (self.state * 1103515245 + 12345) % 2147483648;
            return self.state;
        }
    }
    random = Random();

    for(i = 0:100000) {
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
)");
	auto val = vm.execute(code);
}
