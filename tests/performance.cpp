#include "test.h"
#include <chrono>

using namespace OwcaScript;

class PerformanceTest : public SimpleTest {

};

TEST_F(PerformanceTest, DISABLED_simple_1)
{ // 5.29
	OwcaVM vm;
	auto code = compile(__LINE__, vm, "test.os", R"(
s = 0;
start = time();
i = 0;
while (i < 100000000) {
    s = (s * 11035 + 12345) & 0xffff;
    i = i + 1;
}
end = time();
print(`Time taken: {end - start} seconds`);
print(`Final result: {s} expected (46592) {s == 46592}`);
)");
	auto val = vm.execute(code);
}

TEST_F(PerformanceTest, DISABLED_simple_2)
{ // 24.89 // 100000000
	OwcaVM vm;
	auto code = compile(__LINE__, vm, "test.os", R"(
class A {
    function foo1(s) {
        return s + 1;
    }
    function foo2(self, s) {
        return self.foo1(s);
    }
    function foo3(self, s) {
        return self.foo2(s);
    }
    function foo4(self, s) {
        return self.foo3(s);
    }
    function foo5(self, s) {
        return self.foo4(s);
    }
}
a = A();
s = 0;
start = time();
i = 0;
while (i < 100000000) {
    s = a.foo5((s * 11035 + 12345) & 0xffff);
    i = i + 1;
}
end = time();
print(`Time taken: {end - start} seconds`);
print(`Final result: {s} expected (7168) {s == 7168}`);
)");
	auto val = vm.execute(code);
}

TEST_F(PerformanceTest, DISABLED_simple_3)
{ // 17.56 // 100000000
	OwcaVM vm;
	auto code = compile(__LINE__, vm, "test.os", R"(
class A {
    function foo1(self, s) {
        return s + 1;
    }
    function foo2(self, s) {
        return self.foo1(s);
    }
    function foo3(self, s) {
        return self.foo2(s);
    }
    function foo4(self, s) {
        return self.foo3(s);
    }
    function foo5(self, s) {
        return self.foo4(s);
    }
}
s = 0;
a = A();
start = time();
i = 0;
while (i < 100000) {
    s = a.foo5((s * 11035 + 12345) & 0xffff);
    i = i + 1;
}
end = time();
print(`Time taken: {end - start} seconds`);
print(`Final result: {s}`);
)");
	auto val = vm.execute(code);
}

TEST_F(PerformanceTest, DISABLED_simple_4)
{ // 17.56 // 100000000
	OwcaVM vm;
	auto code = compile(__LINE__, vm, "test.os", R"(
function foo1(s) {
    return s;
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
while (i < 100000) {
    foo5(0);
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
	auto code = compile(__LINE__, vm, "test.os", R"(
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

class PerfRunner {
public:
    virtual std::tuple<unsigned int, std::string, size_t, size_t, size_t> code() = 0;

    void run() {
        OwcaVM vm;
        const auto [ first_line, script_text_code, iterations, min_time, max_time ] = this->code();
        auto code = vm.compile("test.os", std::move(script_text_code), first_line);
        auto val = vm.execute(code);
        auto f = val.member("r");

        std::vector<std::uint32_t> times;
        times.reserve(iterations);

        size_t total_value = 0;
        for(auto i = 0u; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            f.call();
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back((std::uint32_t)static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()));
            total_value += times.back();
        }
        std::sort(times.begin(), times.end());
        std::cout << "Min time: " << times.front() << " ns" << std::endl;
        std::cout << "P10 time: " << times[times.size() / 10] << " ns" << std::endl;
        std::cout << "P50 time: " << times[times.size() / 2] << " ns" << std::endl;
        std::cout << "P90 time: " << times[(times.size() * 9) / 10] << " ns" << std::endl;
        std::cout << "Max time: " << times.back() << " ns" << std::endl;

        std::vector<std::uint32_t> histogram;
        histogram.resize((max_time - min_time) + 1, 0);

        for(auto j = times.size() / 10; j < times.size(); ++j) {
            auto t = times[j];
            if (t < min_time) {
                t = min_time;
            }
            if (t > max_time) {
                t = max_time;
            }
            ++histogram[t - min_time];
        }

        size_t total = 0;
        for(auto &h : histogram) {
            total += h;
        }
        size_t median = 0;
        for(auto i = 0u; i < histogram.size(); ++i) {
            auto h = histogram[i];
            if (median + h >= total / 2) {
                std::cout << "Median time: " << (i + min_time) << " ns" << std::endl;
                break;
            }
            median += h;
        }

        const size_t columns = 200u;
        const size_t rows = 20u;

        size_t max_count = 0;
        for(auto &h : histogram) {
            if (h > max_count) {
                max_count = h;
            }
        }
        max_count = max_count * (2 * rows + 1) / (2 * rows);

        std::vector<unsigned int> heights;
        heights.resize(columns, 0);
        for(auto i = 0u; i < histogram.size(); ++i) {
            auto v = histogram[i] * rows / max_count;
            auto x = i * columns / histogram.size();
            if (heights[x] < v) {
                heights[x] = v;
            }
        }
        for(auto r = 0u; r < rows; ++r) {
            printf("|");
            for(auto c = 0u; c < columns; ++c) {
                if (heights[c] >= rows - r) {
                    printf("*");
                }
                else {
                    printf(" ");
                }
            }
            printf("|\n");
        }

        size_t step = 0;
        if (max_time - min_time > 2000) {
            step = 1000;
        }
        else if (max_time - min_time > 200) {
            step = 100;
        }
        else {
            step = 10;
        }
        size_t previous_label = min_time / step;
        printf("|");
        for(auto c = 0u; c < columns; ++c) {
            auto t = c * (max_time - min_time) / columns + min_time;
            if (t / step > previous_label) {
                printf("+");
                previous_label = t / step;
            }
            else {
                printf(" ");
            }
        }
        printf("|\n");

        printf("|");
        previous_label = min_time / step;
        size_t skip_until_column = 0;
        for(auto c = 0u; c < columns; ++c) {
            if (c < skip_until_column) {
                continue;
            }
            auto t = c * (max_time - min_time) / columns + min_time;
            if (t / step > previous_label) {
                auto txt = std::to_string(t);
                printf("%s  ", txt.c_str());
                skip_until_column = c + txt.size() + 2;
                previous_label = t / step;
            }
            else {
                printf(" ");
            }
        }
        printf("|\n");
    }
};

class PerfRunner1 : public PerfRunner {
    unsigned int scale;
public:
    PerfRunner1(unsigned int scale = 10) : scale(scale) {}

    std::tuple<unsigned int, std::string, size_t, size_t, size_t> code() override {
        return { __LINE__, std::string("iterations = ") + std::to_string(scale) + ";" + R"(
    class A {
        function foo1(s) {
            return s + 1;
        }
        function foo2(self, s) {
            return self.foo1(s);
        }
        function foo3(self, s) {
            return self.foo2(s);
        }
        function foo4(self, s) {
            return self.foo3(s);
        }
        function foo5(self, s) {
            return self.foo4(s);
        }
    }
    a = A();
    function r() {
        s = 0;
        i = 0;
        while (i < iterations) {
            s = a.foo5((s * 11035 + 12345) & 0xffff);
            i = i + 1;
        }
    })", std::max(1u, 1024 * 1024 * 10 / scale), 15000u, 30000u };
    }
};

class PerfRunner2 : public PerfRunner {
    unsigned int scale;
public:
    PerfRunner2(unsigned int scale = 10) : scale(scale) {}

    std::tuple<unsigned int, std::string, size_t, size_t, size_t> code() override {
        return { __LINE__, std::string("iterations = ") + std::to_string(scale) + ";\n" + R"(
    function r() {
        s = 0;
        i = 0;
        while (i < iterations) {
            s = (s * 11035 + 12345) & 0xffff;
            i = i + 1;
        }
    })", std::max(1u, 1024 * 1024 * 10 / scale), 2500u, 7500u };
    }
};

TEST_F(PerformanceTest, DISABLED_measure_1)
{
    PerfRunner1 p(100);
    p.run();
}

TEST_F(PerformanceTest, DISABLED_measure_2)
{
    PerfRunner2 p(100);
    p.run();
}

