#include "test.h"


void SimpleTest::SetUp()
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);
#endif
}

bool operator == (OwcaScript::OwcaInt a, OwcaScript::OwcaInt b) {
	return a.internal_value() == b.internal_value();
}
bool operator == (OwcaScript::OwcaInt a, OwcaScript::OwcaFloat b) {
	return a.internal_value() == b.internal_value();
}
bool operator == (OwcaScript::OwcaFloat a, OwcaScript::OwcaInt b) {
	return a.internal_value() == b.internal_value();
}
bool operator == (OwcaScript::OwcaFloat a, OwcaScript::OwcaFloat b) {
	return a.internal_value() == b.internal_value();
}
bool operator == (OwcaScript::OwcaBool a, OwcaScript::OwcaBool b) {
	return a.internal_value() == b.internal_value();
}
bool operator == (OwcaScript::OwcaString a, OwcaScript::OwcaString b) {
	return a.internal_value() == b.internal_value();
}