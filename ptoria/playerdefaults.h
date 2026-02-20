#ifndef PLAYERDEFAULTS_H
#define PLAYERDEFAULTS_H

#include <ptoria/instance.h>
#include <ptoria/singleton.h>
#include <ptoria/player.h>

struct PlayerDefaults : public InstanceBase,
    public Object<PlayerDefaults, "PlayerDefaults", Unity::AssemblyCSharp>, 
    public Singleton<PlayerDefaults> 
{
    void CmdLoadDefaults(Player* player);
};

#endif
