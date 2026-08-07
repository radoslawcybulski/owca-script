#include "owca-script/identifier_index.h"
#include "test.h"
#include <unordered_map>

using namespace OwcaScript;

class UserPtrTest : public SimpleTest {

};

struct MyUserPtr : public OwcaScript::PtrObjectInterface {
    static constexpr const char *type_name = "MyUserPtr";
    std::unordered_map<OwcaScript::IdentifierIndex, OwcaValue> values;
    bool read, write, obj_acquired = false;

    MyUserPtr(bool read = true, bool write = true) : PtrObjectInterface(0), read(read), write(write) {}

    std::string_view type() const override { return "MyUserPtr"; }
    std::string to_string() const override { return "MyUserPtr(" + std::to_string((std::uintptr_t)this) + ")"; }

    void acquired() override {
        obj_acquired = true;
    }
    void released() override {
        obj_acquired = false;
    }
    bool get_member(IdentifierIndex ii, OwcaValue &val) override {
        if (!read) return false;
        auto it = values.find(ii);
        assert(it != values.end());
        val = it->second;
        return true;
    }
    bool set_member(IdentifierIndex ii, OwcaValue val) override {
        if (!write) return false;
        values[ii] = val;
        return true;
    }

    void gc_mark_members_impl(GenerationGC generation_gc) const override {
        for(auto &p : values) {
            gc_mark_value(generation_gc, p.second);
        }
    }
};

TEST_F(UserPtrTest, simple)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
function r(a) {
	a.x = 1;
    a.y = 2;
    a.z = 3;
    return a.x + a.y + a.z;
}
)");
	auto val = vm.execute(code);
    auto ptr = MyUserPtr{};
    ASSERT_FALSE(ptr.obj_acquired);
	ASSERT_EQ(val.member("r").call(&ptr).as_float(), 6);
    ASSERT_TRUE(ptr.obj_acquired);
    ASSERT_EQ(ptr.values.size(), 3);
    ASSERT_EQ(ptr.values[vm.get_identifier_index("x")].as_float(), 1);
    ASSERT_EQ(ptr.values[vm.get_identifier_index("y")].as_float(), 2);
    ASSERT_EQ(ptr.values[vm.get_identifier_index("z")].as_float(), 3);
    vm.run_gc();
    ASSERT_FALSE(ptr.obj_acquired);
}
