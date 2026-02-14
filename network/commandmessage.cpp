#include <network/commandmessage.h>
#include <mirror/networkclient.h>

void mirror::messages::CommandMessage::Deserialize(NetworkStream& stream)
{
    netId = stream.ReadVarUint();
    componentIndex = stream.ReadByte();
    functionHash = stream.read<uint16_t>();

    int payloadSize = stream.ReadVarUint() - 1;
    payload.resize(payloadSize);
    stream.ReadBytes(payload.data(), payloadSize);
}

UnityComponent* mirror::messages::CommandMessage::TargetIdentity()
{
    try {
        UnityComponent* target = NetworkClient::Spawned()->GetValueByKey(netId);
        nasec::Assert(target != nullptr, "Failed to get TargetIdentity from netId: " + std::to_string(netId));
        return target;
    }
    catch(const std::exception& e)
    {
        spdlog::error("Failed to get TargetIdentity from netId: {}: {}", netId, e.what());
        throw;
    }
}

UnityGameObject* mirror::messages::CommandMessage::Target()
{
    UnityGameObject* obj = TargetIdentity()->GetGameObject();
    nasec::Assert(obj != nullptr, "Failed to get GameObject from TargetIdentity");
    return obj;
}

void mirror::messages::CommandMessage::Print()
{
    spdlog::info("CommandMessage: netId={}({}), componentIndex={}, functionHash={:02x}", netId, Target()->GetName()->ToString(), componentIndex, functionHash);
}