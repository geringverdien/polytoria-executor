#include <ptoria/basescript.h>

void BaseScript::SetRunning(bool value) {
    Unity::SetFieldValue<bool, "running">(StaticClass(), this, value);
}

bool BaseScript::Running() {
    return Unity::GetFieldValue<bool, "running">(StaticClass(), this);
}

void BaseScript::SetSource(UnityString* value) {
    Unity::SetFieldValue<UnityString*, "source">(StaticClass(), this, value);
}

UnityString* BaseScript::Source() {
    return Unity::GetFieldValue<UnityString*, "source">(StaticClass(), this);
}

bool BaseScript::RequestedRun() {
    return Unity::GetFieldValue<bool, "requestedRun">(StaticClass(), this);
}

void BaseScript::SetRequestedRun(bool value) {
    Unity::SetFieldValue<bool, "requestedRun">(StaticClass(), this, value);
}