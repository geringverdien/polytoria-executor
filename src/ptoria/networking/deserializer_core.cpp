#include <ptoria/networking/deserializer_core.hpp>
#include <ptoria/networking/messages/network_message.hpp>
#include <ptoria/networking/messages/command_message.hpp>

using namespace mirror;

std::unique_ptr<messages::NetworkMessage> CreateMessageFromId(messages::MsgId msgId)
{
    switch (msgId)
    {
        case messages::MsgId::CommandMessageId:
            return std::make_unique<messages::CommandMessage>();
        default:
            throw std::runtime_error("Unknown message ID");
    }
}

void mirror::DeserializePacket(NetworkStream& stream)
{
    messages::MsgId msgId = stream.read<messages::MsgId>();
    std::unique_ptr<messages::NetworkMessage> message = CreateMessageFromId(msgId);
    message->Deserialize(stream);
    message->Print();
}

UC* mirror::GetTargetIdentity(uint32_t netId)
{
    UDictionary<uint32_t, UC*>* spawnedDict = nullptr;
    U::Get(ASSEMBLY_MIRROR)->Get("NetworkClient")->Get<UF>("spawned")->GetStaticValue(&spawnedDict);

    return spawnedDict->GetValueByKey(netId);
}

uint32_t mirror::GetNetId(UG* gameObject)
{
    const auto assembly = U::Get(ASSEMBLY_MIRROR);
    const auto componentKlass = assembly->Get("NetworkIdentity", "Mirror");

   

    return componentKlass->GetValue<uint32_t>(gameObject->GetComponent<UC*>(componentKlass), "<netId>k__BackingField");
}
std::vector<UC*> mirror::GetNetworkBehaviours(UG* gameObject)
{
    std::vector<UC*> behaviors;
    const auto assembly = U::Get("UnityEngine.CoreModule.dll");
    const auto componentClass = assembly->Get("Component", "UnityEngine");

    for(auto component : gameObject->GetComponents<UC*>(componentClass))
    {
        auto klass = U::GetClass(component->Il2CppClass.klass);

        auto currKlass = klass;
        while (currKlass)
        {
            if (currKlass->name == "NetworkBehaviour") 
            {
                behaviors.push_back(component);
                break;
            }
            currKlass = U::GetClass(currKlass->parent);
        }
    }

    return behaviors;
}

int mirror::GetStableHashCode(const std::string& str)
{
    uint32_t hash = 0x811C9DC5; // FNV-1a 32-bit offset basis
    uint32_t prime = 0x01000193; // FNV-1a 32-bit prime

    for (char c : str)
    {
        hash = hash ^ c;
        hash *= prime;
    }
    return hash;
}