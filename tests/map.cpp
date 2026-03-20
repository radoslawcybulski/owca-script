#include "test.h"

using namespace OwcaScript;

class MapTest : public SimpleTest {

};

TEST_F(MapTest, size)
{
	OwcaVM vm;
	auto code = vm.compile("test.os", R"(
a = {
	1: 2,
	3: 4,
	5: 6
};
return a.size();
)");
	auto val = vm.execute(code);
	ASSERT_EQ(val.convert_to_float(vm), 3);
}
