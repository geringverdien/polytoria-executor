#ifndef OBJECTMIXIN_H
#define OBJECTMIXIN_H

#include <ptoria/object.h>

template <typename Derived,
          nasec::meta::String ClassName,
          nasec::meta::String Assembly = Unity::AssemblyCSharp,
          nasec::meta::String Namespace = "*",
          nasec::meta::String Parent = "*">
struct ObjectMixin : public Object<Derived, ClassName, Assembly, Namespace, Parent> {
    // This class exists solely to give Derived a unique Object base.
};

#endif