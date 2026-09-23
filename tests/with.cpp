#include "test.h"

using namespace OwcaScript;

class WithTest : public SimpleTest {
public:
    static std::string run_with(unsigned int first_line, std::string code_text)
    {
        OwcaVM vm;
        auto code = vm.compile("test.os", std::move(code_text), first_line);
        auto val = vm.execute(code);
        return std::string{ val.member("r").call().as_string().text() };
    }
};

TEST_F(WithTest, simple)
{
    auto val = run_with(__LINE__, R"(
queue = [];
class W {
    function $init(self, v) {
        self.v = v + 4;
        queue.push_back('A');
        queue.push_back(self.v);
    }
}
class Q {
    function $init(self, v) {
        self.v = v;
        queue.push_back('B');
        queue.push_back(self.v);
    }
    function $enter(self) {
        queue.push_back('C');
        queue.push_back(self.v);
        return W(self.v);
    }
    function $exit(self) {
        queue.push_back('D');
        queue.push_back(self.v);
    }
}

function r() {
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
}
	)");
    ASSERT_EQ(val, std::string_view{ "B1C1A5E5B2C2A6F6D2D1" });
}

TEST_F(WithTest, with_exc)
{
    auto val = run_with(__LINE__, R"(

queue = [];
class W {
    function $init(self, v) {
        self.v = v + 4;
        queue.push_back('A');
        queue.push_back(self.v);
    }
}
class Q {
    function $init(self, v) {
        self.v = v;
        queue.push_back('B');
        queue.push_back(self.v);
    }
    function $enter(self) {
        queue.push_back('C');
        queue.push_back(self.v);
        return W(self.v);
    }
    function $exit(self) {
        queue.push_back('D');
        queue.push_back(self.v);
    }
}

function r() {
    try {
        with(q = Q(1)) {
            queue.push_back('E');
            queue.push_back(q.v);
            with(w = Q(2)) {
                queue.push_back('F');
                queue.push_back(w.v);
                throw Exception("err");
            }
        }
        return 'no exception';
    }
    catch(Exception) {}

    s = '';
    for(q = queue) {
        s = s + String(q);
    }
    return s;
}
	)");
    ASSERT_EQ(val, std::string_view{ "B1C1A5E5B2C2A6F6D2D1" });
}

TEST_F(WithTest, with_lambda)
{
    auto val = run_with(__LINE__, R"(
queue = [];

function r() {
    with(function () { queue.push_back('A'); }) {
        queue.push_back('B');
        with(function () { queue.push_back('C'); }) {
            queue.push_back('D');
        }
    }

    s = '';
    for(q = queue) {
        s = s + String(q);
    }
    return s;
}
	)");
    ASSERT_EQ(val, std::string_view{ "BDCA" });
}
