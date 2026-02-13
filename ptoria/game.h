#ifndef GAME
#define GAME

#include <ptoria/instance.h>
#include <ptoria/singleton.h>

struct Game : public Instance, public Object<"Game", Unity::AssemblyCSharp>, public Singleton<Game> {

    UnityString *GameName();

    int GameID();
};

struct GameIO : public Instance, public Object<"GameIO", Unity::AssemblyCSharp>, public Singleton<GameIO> {

    void SaveToFile(const char* path);
};

#endif /* GAME */
