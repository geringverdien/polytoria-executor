#include <mirror/localconnectiontoserver.h>
#include <hooking/hookmanager.h>
#include <network/stream.h>
#include <network/deserializer.h>

void LocalConnectionToServer::SendHook(LocalConnectionToServer *self, ArraySegment *segment, int channelId)
{
    std::printf("Sent packet on channel %d\n", channelId);

    NetworkStream stream = NetworkStream((const char*)segment->array->GetData() + segment->offset, (const char*)segment->array->GetData() + segment->offset + segment->count);
    
    stream.PrintPayload();

    try
    {
        mirror::DeserializePacket(stream);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error while deserializing packet: " << e.what() << '\n';
    }
    
    

    HookManager::Call(SendHook, self, segment, channelId);
}