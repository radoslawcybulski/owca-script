#include "gtest/gtest.h"
#include "owca-script/owca-script.h"
#include "owca-script/allocation_base.h"
#include "owca-script/owca_vm.h"

bool operator == (OwcaScript::OwcaString, OwcaScript::OwcaString);

class SimpleTest : public testing::Test {
public:
	void SetUp();
	void TearDown() {
		ASSERT_EQ(OwcaScript::Internal::AllocationBase::get_currently_remaining_allocations(), 0);
	}

	template <typename ... ARGS> static auto compile(unsigned int first_line, OwcaScript::OwcaVM &vm, ARGS && ... args) {
		return vm.compile(std::forward<ARGS>(args)..., first_line);
	}

	template <typename ... ARGS> static auto compile_and_run_r(unsigned int first_line, std::string text, ARGS && ... args) {
		OwcaScript::OwcaVM vm;
		auto code = compile(first_line, vm, "test.os", text);
		auto val = vm.execute(code).member("r").call();
		return val.as_int();
	}
};