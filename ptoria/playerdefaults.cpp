#include <ptoria/playerdefaults.h>

void PlayerDefaults::CmdLoadDefaults(Player* player)
{
    Unity::GetMethod<"CmdLoadDefaults">(StaticClass<PlayerDefaults>())->Invoke<void>(this, player);
}
