#include "gtest/gtest.h"
#include "owca-script/owca-script.h"
#include "owca-script/allocation_base.h"

bool operator == (OwcaScript::OwcaInt, OwcaScript::OwcaInt);
bool operator == (OwcaScript::OwcaInt, OwcaScript::OwcaFloat);
bool operator == (OwcaScript::OwcaFloat, OwcaScript::OwcaInt);
bool operator == (OwcaScript::OwcaFloat, OwcaScript::OwcaFloat);
bool operator == (OwcaScript::OwcaBool, OwcaScript::OwcaBool);
bool operator == (OwcaScript::OwcaString, OwcaScript::OwcaString);

class SimpleTest : public testing::Test {
public:
	void SetUp();
	void TearDown() {
		ASSERT_EQ(OwcaScript::Internal::AllocationBase::get_currently_remaining_allocations(), 0);
	}
};