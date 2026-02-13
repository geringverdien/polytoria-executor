#include <ptoria/instance.h>

UnityString* Instance::Name() {
    return Unity::GetMethod<"get_Name">(StaticClass())->Invoke<UnityString*>(this);
}

UnityArray<Instance*>* Instance::Children() {
    return Unity::GetMethod<"GetChildren">(StaticClass())->Invoke<UnityArray<Instance*>*>(this);
}

UnityString* Instance::FullName() {
    return Unity::GetMethod<"get_FullName">(StaticClass())->Invoke<UnityString*>(this);
}