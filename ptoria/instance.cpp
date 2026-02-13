#include <ptoria/instance.h>

UnityString* Instance::Name() {
    return Unity::GetMethod<"get_Name">(StaticClass<Instance>())->Invoke<UnityString*>(this);
}

UnityArray<Instance*>* Instance::Children() {
    return Unity::GetMethod<"GetChildren">(StaticClass<Instance>())->Invoke<UnityArray<Instance*>*>(this);
}

UnityString* Instance::FullName() {
    return Unity::GetMethod<"get_FullName">(StaticClass<Instance>())->Invoke<UnityString*>(this);
}