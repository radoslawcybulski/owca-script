#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>
#include <optional>

static constexpr size_t iterations = 100000000ull;
namespace A {
    size_t foo1(const std::string &a) {
        std::string b = std::to_string((std::uintptr_t)a.c_str() * rand()) + "qwertyqwerty";
        auto v = a.size() + b.size();
        if ((v & 15) == 0) throw 1.0f;
        return v;
    }
    size_t foo2(size_t index) {
        std::string a = std::to_string(index * rand()) + "qwertyqwerty";
        return foo1(a);
    }
    void run_1() {
        size_t cnt = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0u; i < iterations; ++i) {
            try {
                cnt += foo2(i);
            }
            catch(float) {}
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << "ns, cnt: " << cnt << std::endl;
    }
}
namespace B {
    std::optional<size_t> foo1(const std::string &a) {
        std::string b = std::to_string((std::uintptr_t)a.c_str() * rand()) + "qwertyqwerty";
        auto v = a.size() + b.size();
        if ((v & 15) == 0) return std::nullopt;
        return v;
    }
    std::optional<size_t>  foo2(size_t index) {
        std::string a = std::to_string(index * rand()) + "qwertyqwerty";
        return foo1(a);
    }
    void run_1() {
        size_t cnt = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t i = 0u; i < iterations; ++i) {
            if (auto v = foo2(i)) {
                cnt += *v;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << "ns, cnt: " << cnt << std::endl;
    }
}
int main(int argc, char *argv[])
{
    srand(0);
    if (argv[1][0] == '1') {
        A::run_1();
    }
    else if (argv[1][0] == '2') {
        B::run_1();
    }
    else {
        std::cout << "wrong argument" << std::endl;
    }
    return 0;
}