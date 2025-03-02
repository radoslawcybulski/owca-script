#include "test.h"

using namespace OwcaScript;

class GeneratorTest : public SimpleTest {

};

static int run_gen(std::string code_text, OwcaValue add_val)
{
    OwcaVM vm;
    std::vector<std::string> tmp{ { "a" } };
    auto code = vm.compile("test.os", std::move(code_text), tmp);
    auto val = vm.execute(code, vm.create_map({ { "a", add_val } }));
    return (int)val.as_int(vm).internal_value();
}

static int run_try(int v)
{
    return run_gen(R"(
        class A(Exception) {}
        class B(Exception) {}
        
        function generator foo() {
            try {
                if (a == 1) throw A("q");
                if (a == 2) throw B("q");
            }
            catch(e: A) {
                yield 1;
            }
            catch(e: B) {
                yield 2;
            }
            yield 3;
        }
        for(q = foo()) {
            return q;
        }
        )", OwcaInt{ v });
}

TEST_F(GeneratorTest, try1)
{
    auto val = run_try(1);
	ASSERT_EQ(val, 1);
}

TEST_F(GeneratorTest, try2)
{
    auto val = run_try(2);
	ASSERT_EQ(val, 2);
}

TEST_F(GeneratorTest, try3)
{
    auto val = run_try(3);
	ASSERT_EQ(val, 3);
}

TEST_F(GeneratorTest, while_simple1)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    i = 0;
    total = 0;
    while(i < 3) {
        if (i == 1) {
            if (a == 1) break;
            if (a == 2) continue;
        }
        total = total + 1;
        i = i + 1;
    }
    yield total;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 3);
}

TEST_F(GeneratorTest, while_simple2)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    i = 0;
    total = 0;
    while(i < 3) {
        if (i == 1) {
            if (a == 1) break;
            if (a == 2) continue;
        }
        total = total + 1;
        i = i + 1;
    }
    yield total;
}
for(q = foo()) return q;
	)", OwcaInt{ 1 }), 1);
}

TEST_F(GeneratorTest, while_simple3)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    i = 0;
    total = 0;
    aa = a;
    while(i < 3) {
        i = i + 1;
        if (i == 2) {
            if (aa == 1) break;
            if (aa == 2) {
                aa = 0;
                continue;
            }
        }
        total = total + 1;
    }
    yield total;
}
for(q = foo()) return q;
	)", OwcaInt{ 2 }), 2);
}

TEST_F(GeneratorTest, while_loop_ident1)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    mode = 0;
    l1: while(true) {
        l2: while(true) {
            l3: while(true) {
                break l1;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    yield mode;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 0);
}
TEST_F(GeneratorTest, while_loop_ident2)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    mode = 0;
    l1: while(true) {
        l2: while(true) {
            l3: while(true) {
                break l2;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    yield mode;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 2);
}
TEST_F(GeneratorTest, while_loop_ident3)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    mode = 0;
    l1: while(true) {
        l2: while(true) {
            l3: while(true) {
                break l3;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    yield mode;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 3);
}
TEST_F(GeneratorTest, while_loop_ident4)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    mode = 0;
    l1: while(true) {
        l2: while(true) {
            l3: while(true) {
                break;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    yield mode;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 3);
}





TEST_F(GeneratorTest, for_simple1)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    total = 0;
    arr = [ 0, 1, 2 ];
    for(i = arr) {
        if (i == 1) {
            if (a == 1) break;
            if (a == 2) continue;
        }
        total = total + 1;
    }
    yield total;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 3);
}

TEST_F(GeneratorTest, for_simple2)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    total = 0;
    arr = [ 0, 1, 2 ];
    for(i = arr) {
        if (i == 1) {
            if (a == 1) break;
            if (a == 2) continue;
        }
        total = total + 1;
    }
    yield total;
}
for(q = foo()) return q;
	)", OwcaInt{ 1 }), 1);
}

TEST_F(GeneratorTest, for_simple3)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    total = 0;
    arr = [ 0, 1, 2 ];
    aa = a;
    for(i = arr) {
        if (i == 1) {
            if (aa == 1) break;
            if (aa == 2) {
                aa = 0;
                continue;
            }
        }
        total = total + 1;
    }
    yield total;
}
for(q = foo()) return q;
	)", OwcaInt{ 2 }), 2);
}

TEST_F(GeneratorTest, for_loop_ident1)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    mode = 0;
    arr = [ 0, 1, 2 ];
    l1: for(i = arr) {
        l2: for(j = arr) {
            l3: for(k = arr) {
                break l1;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    yield mode;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 0);
}
TEST_F(GeneratorTest, for_loop_ident2)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    mode = 0;
    arr = [ 0, 1, 2 ];
    l1: for(i = arr) {
        l2: for(j = arr) {
            l3: for(k = arr) {
                break l2;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    yield mode;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 2);
}
TEST_F(GeneratorTest, for_loop_ident3)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    mode = 0;
    arr = [ 0, 1, 2 ];
    l1: for(i = arr) {
        l2: for(j = arr) {
            l3: for(k = arr) {
                break l3;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    yield mode;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 3);
}
TEST_F(GeneratorTest, for_loop_ident4)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    mode = 0;
    arr = [ 0, 1, 2 ];
    l1: for(i = arr) {
        l2: for(j = arr) {
            l3: for(k = arr) {
                break;
            }
            mode = mode | 1;
            break;
        }
        mode = mode | 2;
        break;
    }
    yield mode;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 3);
}





TEST_F(GeneratorTest, if_simple1)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a) {
        yield 1;
    }
    else {
        yield 2;
    }
}
for(q = foo()) return q;
	)", OwcaBool{ false }), 2);
}

TEST_F(GeneratorTest, if_simple2)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a) {
        yield 1;
    }
    else {
        yield 2;
    }
}
for(q = foo()) return q;
	)", OwcaInt{ 1 }), 1);
}

TEST_F(GeneratorTest, if_simple3)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a) {
        yield 1;
    }
    else {
        yield 2;
    }
}
for(q = foo()) return q;
	)", OwcaInt{ true }), 1);
}

TEST_F(GeneratorTest, no_else1)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a) {
        yield 1;
    }
    yield 2;
}
for(q = foo()) return q;
	)", OwcaInt{ 1 }), 1);
}

TEST_F(GeneratorTest, no_else2)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a) {
        yield 1;
    }
    yield 2;
}
for(q = foo()) return q;
	)", OwcaInt{ 2 }), 1);
}

TEST_F(GeneratorTest, no_else3)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a) {
        yield 1;
    }
    yield 2;
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 2);
}

TEST_F(GeneratorTest, elif1)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a == 1) {
        yield 1;
    }
    elif (a == 2) {
        yield 2;
    }
    elif (a == 3) {
        yield 3;
    }
    else {
        yield 4;
    }
}
for(q = foo()) return q;
	)", OwcaInt{ 0 }), 4);
}

TEST_F(GeneratorTest, elif2)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a == 1) {
        yield 1;
    }
    elif (a == 2) {
        yield 2;
    }
    elif (a == 3) {
        yield 3;
    }
    else {
        yield 4;
    }
}
for(q = foo()) return q;
	)", OwcaInt{ 1 }), 1);
}

TEST_F(GeneratorTest, elif3)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a == 1) {
        yield 1;
    }
    elif (a == 2) {
        yield 2;
    }
    elif (a == 3) {
        yield 3;
    }
    else {
        yield 4;
    }
}
for(q = foo()) return q;
	)", OwcaInt{ 2 }), 2);
}

TEST_F(GeneratorTest, elif4)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a == 1) {
        yield 1;
    }
    elif (a == 2) {
        yield 2;
    }
    elif (a == 3) {
        yield 3;
    }
    else {
        yield 4;
    }
}
for(q = foo()) return q;
	)", OwcaInt{ 3 }), 3);
}

TEST_F(GeneratorTest, elif5)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    if (a == 1) {
        yield 1;
    }
    elif (a == 2) {
        yield 2;
    }
    elif (a == 3) {
        yield 3;
    }
    else {
        yield 4;
    }
}
for(q = foo()) return q;
	)", OwcaInt{ 4 }), 4);
}

TEST_F(GeneratorTest, simple)
{
    ASSERT_EQ(run_gen(R"(
function generator foo() {
    yield 1;
    yield 2;
    yield 3;
}

b = 0;
for(a = foo()) {
    b = b + a;
}
return b;
)", OwcaInt{ 0 }), 6);
}
