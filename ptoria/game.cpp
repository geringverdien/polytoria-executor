#include <ptoria/game.h>

UnityString* Game::GameName() {
    return Unity::GetStaticFieldValue<UnityString*, "GameName">(StaticClass());
}

int Game::GameID() {
    return Unity::GetStaticFieldValue<int, "gameID">(StaticClass());
}

void GameIO::SaveToFile(const char* path) {
    Unity::GetMethod<"SaveToFile">(StaticClass())->Invoke<void, void*, UnityString*>(this, UnityString::New(path));
}