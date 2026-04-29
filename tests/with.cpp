// #include "test.h"

// using namespace OwcaScript;

// class WithTest : public SimpleTest {

// };

// static auto run_with(std::string code_text, OwcaValue add_val)
// {
//     OwcaVM vm;
//     std::vector<std::string> tmp{ { "a" } };
//     auto code = vm.compile("test.os", std::move(code_text), tmp);
//     auto map_data = std::vector<std::pair<std::string, OwcaValue>>{ { { "a", add_val } } };
//     auto val = vm.execute(code, vm.create_map(map_data));
//     return std::string{ val.as_string(vm).text() };
// }
// TEST_F(WithTest, simple)
// {
//     auto val = run_with(R"(

// queue = [];
// class W {
//     function __init__(self, v) {
//         self.v = v + 4;
//         queue.push_back('A');
//         queue.push_back(self.v);
//     }
// }
// class Q {
//     function __init__(self, v) {
//         self.v = v;
//         queue.push_back('B');
//         queue.push_back(self.v);
//     }
//     function __enter__(self) {
//         queue.push_back('C');
//         queue.push_back(self.v);
//         return W(self.v);
//     }
//     function __exit__(self) {
//         queue.push_back('D');
//         queue.push_back(self.v);
//     }
// }

// with(q = Q(1)) {
//     queue.push_back('E');
//     queue.push_back(q.v);
//     with(w = Q(2)) {
//         queue.push_back('F');
//         queue.push_back(w.v);
//     }
// }

// s = '';
// for(q = queue) {
//     s = s + String(q);
// }
// return s;
// 	)", 0);
//     ASSERT_EQ(val, std::string_view{ "B1C1A5E5B2C2A6F6D2D1" });
// }

// TEST_F(WithTest, with_exc)
// {
//     auto val = run_with(R"(

// queue = [];
// class W {
//     function __init__(self, v) {
//         self.v = v + 4;
//         queue.push_back('A');
//         queue.push_back(self.v);
//     }
// }
// class Q {
//     function __init__(self, v) {
//         self.v = v;
//         queue.push_back('B');
//         queue.push_back(self.v);
//     }
//     function __enter__(self) {
//         queue.push_back('C');
//         queue.push_back(self.v);
//         return W(self.v);
//     }
//     function __exit__(self) {
//         queue.push_back('D');
//         queue.push_back(self.v);
//     }
// }

// try {
//     with(q = Q(1)) {
//         queue.push_back('E');
//         queue.push_back(q.v);
//         with(w = Q(2)) {
//             queue.push_back('F');
//             queue.push_back(w.v);
//             throw Exception("err");
//         }
//     }
//     return 'no exception';
// }
// catch(Exception) {}

// s = '';
// for(q = queue) {
//     s = s + String(q);
// }
// return s;
// 	)", 0);
//     ASSERT_EQ(val, std::string_view{ "B1C1A5E5B2C2A6F6D2D1" });
// }
