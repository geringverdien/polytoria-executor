#ifndef INSTANCE
#define INSTANCE

#include <ptoria/object.h>
#include <unity/unity.h>

/**
 * @brief Base Instance class corresponding to Polytoria's "Instance"
 */
struct Instance : public Object<Instance, "Instance", Unity::AssemblyCSharp> {
    
    UnityString* Name();
    UnityArray<Instance*>* Children();
    UnityString* FullName();
};

// /**
//  * @brief Helper for creating derived Instance types
//  */
// template <nasec::meta::String ClassName, nasec::meta::String Assembly = Unity::AssemblyCSharp, nasec::meta::String Namespace = "*", nasec::meta::String Parent = "*">
// struct DerivedInstance : public Instance {
//     static auto StaticClass() -> UnityClass* {
//         return Unity::GetClass<ClassName, Assembly, Namespace, Parent>();
//     }
// };


#endif /* INSTANCE */
