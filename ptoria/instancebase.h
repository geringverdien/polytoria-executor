#ifndef INSTANCEBASE_H
#define INSTANCEBASE_H

#include <unity/unity.h>

struct InstanceBase {
    UnityString* Name();
    UnityArray<InstanceBase*>* Children();
    UnityString* FullName();
};

#endif