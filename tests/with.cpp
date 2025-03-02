#include "test.h"

using namespace OwcaScript;

class WithTest : public SimpleTest {

};

static auto run_with(std::string code_text, OwcaValue add_val)
{
    OwcaVM vm;
    std::vector<std::string> tmp{ { "a" } };
    auto code = vm.compile("test.os", std::move(code_text), tmp);
    auto val = vm.execute(code, vm.create_map({ { "a", add_val } }));
    return std::string{ val.as_string(vm).text() };
}
TEST_F(WithTest, simple1)
{
    ASSERT_EQ(run_with(R"(

queue = [];
class W {
    function __init__(self, v) {
        self.v = v + 4;
        queue.push_back('A');
        queue.push_back(self.v);
    }
}
class Q {
    function __init__(self, v) {
        self.v = v;
        queue.push_back('B');
        queue.push_back(self.v);
    }
    function __enter__(self) {
        queue.push_back('C');
        queue.push_back(self.v);
        return W(self.v);
    }
    function __exit__(self) {
        queue.push_back('D');
        queue.push_back(self.v);
    }
}

with(q = Q(1)) {
    queue.push_back('E');
    queue.push_back(q.v);
    with(w = Q(2)) {
        queue.push_back('F');
        queue.push_back(w.v);
    }
}

s = '';
for(q = queue) {
    s = s + String(q);
}
return s;
	)", OwcaInt{ 0 }), std::string_view{ "B1C1A5E5B2C2A6F6D2D1" });
}
