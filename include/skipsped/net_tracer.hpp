#ifndef NET_TRACER
#define NET_TRACER
#include <HookManager.hpp>

#include <polytoria/polytoria.hpp>

struct NetTracer {

    static bool isReady;
    
    static void UserCode_InvokeCmd__NetMessage__NetworkConnectionToClientHooked(polytoria::NetMessage* message, polytoria::NetworkConnectionToClient* conn) {
        std::cout << "[NetTracer] Cmd Invoked: " << std::endl;

    }

    static void UserCode_InvokeClientRpc__NetMessage(polytoria::NetMessage* message) {
        std::cout << "[NetTracer] ClientRpc Invoked: " << std::endl;
    }
    
    static void InstallHooks()
    {
        HookManager::Install(polytoria::NetworkEvent::UserCode_InvokeCmd__NetMessage__NetworkConnectionToClientPtr()->Cast<void, polytoria::NetMessage*, polytoria::NetworkConnectionToClient*>(), UserCode_InvokeCmd__NetMessage__NetworkConnectionToClientHooked);
        HookManager::Install(polytoria::NetworkEvent::UserCode_InvokeClientRpc__NetMessagePtr()->Cast<void, polytoria::NetMessage*>(), UserCode_InvokeClientRpc__NetMessage);
    }
};




#endif /* NET_TRACER */
