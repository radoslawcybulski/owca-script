#include "test.h"


void SimpleTest::SetUp()
{
#if defined(DEBUG) || defined(_DEBUG)
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);
#endif
#endif
}

bool operator == (OwcaScript::OwcaString a, OwcaScript::OwcaString b) {
	return a.internal_value() == b.internal_value();
}