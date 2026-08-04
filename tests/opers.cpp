#include "owca-script/identifier_index.h"
#include "test.h"

class OpersTest : public SimpleTest {

};

using namespace OwcaScript;

TEST_F(OpersTest, add) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { return 1 + 2; }"), 3); }
TEST_F(OpersTest, sub) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { return 1 - 2; }"), -1); }
TEST_F(OpersTest, mul) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { return 2 * 3; }"), 6); }
TEST_F(OpersTest, div) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { return 5 / 2; }"), 2); }
TEST_F(OpersTest, mod) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { return 7 % 2; }"), 1); }
TEST_F(OpersTest, bin_or) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { return 11 | 34; }"), 11 | 34); }
TEST_F(OpersTest, bin_and) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { return 11 & 34; }"), 11 & 34); }
TEST_F(OpersTest, bin_xor) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { return 11 ^ 34; }"), 11 ^ 34); }
TEST_F(OpersTest, bin_lshift) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { return 11 << 3; }"), 11 << 3); }
TEST_F(OpersTest, bin_rshift) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { return 41 >> 3; }"), 41 >> 3); }

TEST_F(OpersTest, self_add) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { a = 1; a += 2; return a; }"), 3); }
TEST_F(OpersTest, self_sub) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { a = 1; a -= 2; return a; }"), -1); }
TEST_F(OpersTest, self_mul) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { a = 2; a *= 3; return a; }"), 6); }
TEST_F(OpersTest, self_div) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { a = 5; a /= 2; return a; }"), 2); }
TEST_F(OpersTest, self_mod) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { a = 7; a %= 2; return a; }"), 1); }
TEST_F(OpersTest, self_bin_or) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { a = 11; a |= 34; return a; }"), 11 | 34); }
TEST_F(OpersTest, self_bin_and) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { a = 11; a &= 34; return a; }"), 11 & 34); }
TEST_F(OpersTest, self_bin_xor) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { a = 11; a ^= 34; return a; }"), 11 ^ 34); }
TEST_F(OpersTest, self_bin_lshift) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { a = 11; a <<= 3; return a; }"), 11 << 3); }
TEST_F(OpersTest, self_bin_rshift) { ASSERT_EQ(compile_and_run_r(__LINE__, "function r() { a = 41; a >>= 3; return a; }"), 41 >> 3); }
