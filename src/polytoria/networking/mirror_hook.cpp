#include <polytoria/networking/mirror_hook.hpp>
#include <polytoria/networking/network_stream.hpp>
#include <polytoria/networking/deserializer_core.hpp>
#include <unity/u.hpp>

using namespace mirror;

struct ArraySegment {
    UArray<uint8_t>* array;
    int offset;
    int count;
};

typedef void (*Send_t)(UO* self, ArraySegment* segment, int channelId);

void dtSend(UO* self, ArraySegment* segment, int channelId)
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
    
    

    HookManager::Call(dtSend, self, segment, channelId);
}

void mirror::InstallHooks()
{
    // we hook Mirror::LocalConnectionToServer::Send for outgoing packets

    static auto method = U::Get("Mirror.dll")->Get("LocalConnectionToServer", "Mirror")->Get<UM>("Send");
    
    if(!method) {
        std::printf("Failed to find method to hook\n");
        return;
    }

    std::printf("Hooking Mirror::LocalConnectionToServer::Send at address %p\n", method->function);
    HookManager::Install((Send_t)method->function, dtSend);
}

void mirror::SendPacket(const char* data, size_t size)
{
    UO* connection = nullptr; // NetworkConnectionToServer
    U::Get(ASSEMBLY_MIRROR)->Get("NetworkClient")->Get<UF>("<connection>k__BackingField")->GetStaticValue(&connection);

    auto send = U::Get(ASSEMBLY_MIRROR)->Get("LocalConnectionToServer", "Mirror")->Get<UM>("Send", {"System.ArraySegment<System.Byte>", "System.Int32"});
    UArray<uint8_t>* array = UArray<uint8_t>::New(U::Get("mscorlib.dll")->Get("Byte", "System"), size);
    std::memcpy((void*)array->GetData(), data, size);

	void* segment = calloc(1, 0xC);
    *(uintptr_t*)segment = (uintptr_t)array;
    *(uintptr_t*)((uintptr_t)segment + 0xC) = size;

    //U::Get(ASSEMBLY_MIRROR)->Get("NetworkConnectionToServer", "Mirror")->Get<UM>("SendToTransport")->Invoke<void>(nullptr, segment, 0);
    send->Invoke<void>(connection, segment, 0);
}