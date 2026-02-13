#ifndef OBJECT
#define OBJECT

#include <unity/unity.h>
#include <type_traits>

template <typename Derived,
          nasec::meta::String ClassName,
          nasec::meta::String Assembly = Unity::AssemblyCSharp,
          nasec::meta::String Namespace = "*",
          nasec::meta::String Parent = "*">
struct Object
{
    using is_ptoria_object = void; // Marker type for SFINAE
};

// Extract params from Object (SFINAE helper)
template <typename>
struct ObjectParams;

template <typename Derived, nasec::meta::String ClassName, nasec::meta::String Assembly, nasec::meta::String Namespace, nasec::meta::String Parent>
struct ObjectParams<Object<Derived, ClassName, Assembly, Namespace, Parent>> {
    static constexpr auto _class = ClassName;
    static constexpr auto _assembly = Assembly;
    static constexpr auto _namespace = Namespace;
    static constexpr auto _parent = Parent;
};

namespace detail {
    // Matches the exact Object<Derived,...> (based on the SFINAE marker) if present
    template <typename Derived,
                nasec::meta::String ClassName,
                nasec::meta::String Assembly,
                nasec::meta::String Namespace,
                nasec::meta::String Parent>
    static ObjectParams<Object<Derived, ClassName, Assembly, Namespace, Parent>> Test(Object<Derived, ClassName, Assembly, Namespace, Parent>*);
    static void Test(...);

    template <typename T>
    struct GetObjectParams {
        using type = decltype(Test(std::declval<T*>()));
        static_assert(!std::is_same_v<type, void>, "Type does not derive from Object");
    };
}

template <typename T>
using ObjectParamsT = typename detail::GetObjectParams<T>::type;

/**
 * @brief Retrieve the static UnityClass* for a given Object-derived type T using its compile-time metadata
 * 
 * @tparam T 
 * @return UnityClass* 
 */
template <typename T>
auto StaticClass() -> UnityClass* {
    using Params = ObjectParamsT<T>;
    return Unity::GetClass<Params::_class, Params::_assembly, Params::_namespace, Params::_parent>();
}

namespace nasec
{
    namespace meta
    {
        template <typename T, typename = void>
        struct is_object : std::false_type
        {
        };

        template <typename T>
        struct is_object<T, std::void_t<typename T::is_ptoria_object>> : std::true_type
        {
        };
    }
}

#endif /* OBJECT */
