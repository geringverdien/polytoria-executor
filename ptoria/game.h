#ifndef GAME_H
#define GAME_H

#include <ptoria/instancebase.h>
#include <ptoria/objectmixin.h>
#include <ptoria/singleton.h>

struct Game : public InstanceBase,
              public ObjectMixin<Game, "Game", Unity::AssemblyCSharp>,
              public Singleton<Game> {
    UnityString* GameName();
    int GameID();
};

struct GameIO : public InstanceBase,
                public ObjectMixin<GameIO, "GameIO", Unity::AssemblyCSharp>,
                public Singleton<GameIO> {
    void SaveToFile(const char* path);
};

#endif