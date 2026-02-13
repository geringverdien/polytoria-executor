#include <ptoria/basescript.h>

void BaseScript::SetRunning(bool value) {
    Unity::SetFieldValue<bool, "running">(StaticClass<BaseScript>(), this, value);
}

bool BaseScript::Running() {
    return Unity::GetFieldValue<bool, "running">(StaticClass<BaseScript>(), this);
}

void BaseScript::SetSource(UnityString* value) {
    Unity::SetFieldValue<UnityString*, "source">(StaticClass<BaseScript>(), this, value);
}

UnityString* BaseScript::Source() {
    return Unity::GetFieldValue<UnityString*, "source">(StaticClass<BaseScript>(), this);
}

bool BaseScript::RequestedRun() {
    return Unity::GetFieldValue<bool, "requestedRun">(StaticClass<BaseScript>(), this);
}

void BaseScript::SetRequestedRun(bool value) {
    Unity::SetFieldValue<bool, "requestedRun">(StaticClass<BaseScript>(), this, value);
}