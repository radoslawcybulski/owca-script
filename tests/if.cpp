#include "test.h"

using namespace OwcaScript;

class IfTest : public SimpleTest {

};

static int run_if(std::string code_text, OwcaValue add_val)
{
    OwcaVM vm;
    std::vector<std::string> tmp{ { "a" } };
    auto code = vm.compile("test.os", std::move(code_text), tmp);
    auto val = vm.execute(code, std::unordered_map<std::string, OwcaValue>{ { "a", add_val } });
    return (int)val.as_int(vm).internal_value();
}
TEST_F(IfTest, simple1)
{
    ASSERT_EQ(run_if(R"(
if (a) {
    return 1;
}
else {
    return 2;
}
	)", OwcaBool{ false }), 2);
}

TEST_F(IfTest, simple2)
{
    ASSERT_EQ(run_if(R"(
if (a) {
    return 1;
}
else {
    return 2;
}
	)", OwcaInt{ 1 }), 1);
}

TEST_F(IfTest, simple3)
{
    ASSERT_EQ(run_if(R"(
if (a) {
    return 1;
}
else {
    return 2;
}
	)", OwcaInt{ true }), 1);
}

TEST_F(IfTest, no_else1)
{
    ASSERT_EQ(run_if(R"(
if (a) {
    return 1;
}
return 2;
	)", OwcaInt{ 1 }), 1);
}

TEST_F(IfTest, no_else2)
{
    ASSERT_EQ(run_if(R"(
if (a) {
    return 1;
}
return 2;
	)", OwcaInt{ 2 }), 1);
}

TEST_F(IfTest, no_else3)
{
    ASSERT_EQ(run_if(R"(
if (a) {
    return 1;
}
return 2;
	)", OwcaInt{ 0 }), 2);
}

TEST_F(IfTest, elif1)
{
    ASSERT_EQ(run_if(R"(
if (a == 1) {
    return 1;
}
elif (a == 2) {
    return 2;
}
elif (a == 3) {
    return 3;
}
else {
    return 4;
}
	)", OwcaInt{ 0 }), 4);
}

TEST_F(IfTest, elif2)
{
    ASSERT_EQ(run_if(R"(
if (a == 1) {
    return 1;
}
elif (a == 2) {
    return 2;
}
elif (a == 3) {
    return 3;
}
else {
    return 4;
}
	)", OwcaInt{ 1 }), 1);
}

TEST_F(IfTest, elif3)
{
    ASSERT_EQ(run_if(R"(
if (a == 1) {
    return 1;
}
elif (a == 2) {
    return 2;
}
elif (a == 3) {
    return 3;
}
else {
    return 4;
}
	)", OwcaInt{ 2 }), 2);
}

TEST_F(IfTest, elif4)
{
    ASSERT_EQ(run_if(R"(
if (a == 1) {
    return 1;
}
elif (a == 2) {
    return 2;
}
elif (a == 3) {
    return 3;
}
else {
    return 4;
}
	)", OwcaInt{ 3 }), 3);
}

TEST_F(IfTest, elif5)
{
    ASSERT_EQ(run_if(R"(
if (a == 1) {
    return 1;
}
elif (a == 2) {
    return 2;
}
elif (a == 3) {
    return 3;
}
else {
    return 4;
}
	)", OwcaInt{ 4 }), 4);
}



