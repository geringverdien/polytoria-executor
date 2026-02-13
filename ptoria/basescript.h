#ifndef BASESCRIPT
#define BASESCRIPT

#include <ptoria/instance.h>

struct BaseScript : public Instance, public Object<BaseScript, "BaseScript", Unity::AssemblyCSharp> {
    void SetRunning(bool value);
    bool Running();

    void SetSource(UnityString* value);
    UnityString* Source();

    bool RequestedRun();
    void SetRequestedRun(bool value);
};

#endif /* BASESCRIPT */
