// basescript.h
#ifndef BASESCRIPT_H
#define BASESCRIPT_H

#include <ptoria/instancebase.h>
#include <ptoria/objectmixin.h>

struct BaseScript : public InstanceBase,
                    public ObjectMixin<BaseScript, "BaseScript", Unity::AssemblyCSharp> {
    void SetRunning(bool value);
    bool Running();
    void SetSource(UnityString* value);
    UnityString* Source();
    bool RequestedRun();
    void SetRequestedRun(bool value);
};

#endif