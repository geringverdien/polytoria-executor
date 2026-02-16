#ifndef SINGLETON
#define SINGLETON

#include <ptoria/instance.h>

namespace ptoria
{

    template <typename T>
    struct Singleton
    {
        static auto GetSingleton() -> T*
        {
            static_assert(nasec::meta::IsObject<T>, "T must satisfy IsObject concept");
            
            static T* instance;
            if (!instance) instance = ::GetField<T, "singleton">()->GetStaticValue<T*>(&instance);
            nasec::Assert(instance, "Failed to get singleton instance of class: %s", T::Class::StaticClass()->name.c_str());
            return instance;
        }
    };
};

#endif /* SINGLETON */
