#include <iostream>
#include <chrono>
#include <string>
#include <unordered_map>

void run(std::uint32_t &final_result) {
    struct Random {
        std::uint32_t state = 0;

        Random() = default;

        std::uint32_t next() {
            state = (state * 11035 + 12345) & 0xffff;
            return state;
        }
    };
    struct State {
        std::unordered_map<std::string, unsigned int> v;
    };
    std::unordered_map<unsigned int, State> states;
    Random random;

    auto update_state = [&](unsigned int name, bool is_bid, unsigned int val, int count) {
        auto &v = states[name];
        std::string key = count > 0 ? "+" : "-";
        key = key + std::to_string(val);
        auto old = v.v.find(key) != v.v.end() ? v.v[key] : 0;
        auto new_val = old + count;
        if (old > 0 && new_val <= 0) {
            v.v.erase(key);
        } else {
            v.v[key] = new_val;
            final_result = (final_result * 3) + (std::uint32_t)new_val;
        }
    };
    for(auto i = 0; i < 10000000; ++i) {
        auto v = random.next();
        auto s_index = v % 1000;
        auto is_bid = (random.next() % 2) == 0;
        auto val = random.next() % 20;
        auto count = (int)(random.next() % 10) - 5;
        update_state(s_index, is_bid, val, count);
    }
}
int main()
{
    std::uint32_t final_result     = 0;
    auto start = std::chrono::high_resolution_clock::now();
    run(final_result);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
    std::cout << "Final result: " << final_result << std::endl;

    return 0;
}