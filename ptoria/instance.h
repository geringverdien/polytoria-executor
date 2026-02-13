#ifndef INSTANCE_H
#define INSTANCE_H

#include <ptoria/instancebase.h>
#include <ptoria/objectmixin.h>

struct Instance : public InstanceBase,
                  public ObjectMixin<Instance, "Instance", Unity::AssemblyCSharp> {};

#endif