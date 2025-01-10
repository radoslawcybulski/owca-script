#include "test.h"

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