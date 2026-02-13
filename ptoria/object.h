#ifndef OBJECT
#define OBJECT

#include <unity/unity.h>
#include <type_traits>

template <nasec::meta::String Name, nasec::meta::String Assembly, nasec::meta::String Namespace = "*", nasec::meta::String Parent = "*">
struct Object {
    using is_ptoria_object = void;

    static auto StaticClass() -> UnityClass* {
        return Unity::GetClass<Name, Assembly, Namespace, Parent>();
    }
};

namespace nasec {
    namespace meta {
        template <typename T, typename = void>
        struct is_object : std::false_type {};

        template <typename T>
        struct is_object<T, std::void_t<typename T::is_ptoria_object>> : std::true_type {};
    }
}

#endif /* OBJECT */
