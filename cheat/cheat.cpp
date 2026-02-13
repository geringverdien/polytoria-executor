#include <ptoria/instance.h>
#include <nasec/assert.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <core/core.h>
#include <ptoria/game.h>

int main_thread()
{
    OpenConsole();

    
    Unity::Init();
    Unity::ThreadAttach();

    UnityAssembly* assembly = Unity::GetAssembly<Unity::AssemblyCSharp>();

    
    spdlog::info("Assembly Name: {}", assembly->name);
    UnityClass* instanceClass = Instance::StaticClass();
    nasec::Assert(instanceClass != nullptr, "Failed to get Instance class");

    UnityClass* gameClass = Game::StaticClass();
    nasec::Assert(gameClass != nullptr, "Failed to get Game class");

    spdlog::info("Instance Class Name: {}", instanceClass->name);
    spdlog::info("Game Class Name: {}", gameClass->name);

    Game* gameInstance = Game::GetSingleton();
    nasec::Assert(gameInstance != nullptr, "Failed to get Game singleton instance");

    spdlog::info("Game Instance Address: 0x{:016X}", reinterpret_cast<uintptr_t>(gameInstance));
    spdlog::info("Game Instance Type: 0x{:016X}", reinterpret_cast<uintptr_t>(gameInstance->StaticClass()->GetType()));
    spdlog::info("Downcasted Instance Class Name: {}", ((Instance*)gameInstance)->StaticClass()->name);

    spdlog::info("Game Instance Name: {}", gameInstance->Name()->ToString());
    spdlog::info("Game Instance Full Name: {}", gameInstance->FullName()->ToString());
    for (auto& child: gameInstance->Children()->ToVector()) {
        spdlog::info("Child Instance Name: {}", child->Name()->ToString());
    }
    return 0;
}