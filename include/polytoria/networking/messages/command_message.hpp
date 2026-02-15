#include <polytoria/networking/messages/network_message.hpp>
#include <polytoria/networking/deserializer_core.hpp>
#include <unity/u.hpp>

namespace mirror::messages
{
    class CommandMessage : public NetworkMessage
    {
    public:
        CommandMessage() {
            msgId = MsgId::CommandMessageId;
        };

        uint32_t netId;
        uint8_t componentIndex;
        uint16_t functionHash;
        std::vector<uint8_t> payload;

        void Deserialize(NetworkStream& stream) override
        {
            netId = stream.ReadVarUint();
            componentIndex = stream.ReadByte();
            functionHash = stream.read<uint16_t>();

            int payloadSize = stream.ReadVarUint() - 1;
            payload.resize(payloadSize);
            stream.ReadBytes(payload.data(), payloadSize);
        }

        NetworkStream Serialize(UG* target, const std::string& methodName, const std::vector<uint8_t>& payload)
        {

            netId = mirror::GetNetId(target);
            std::printf("Valid netid? %d\n", netId);

            for(auto component : mirror::GetNetworkBehaviours(target))
            {        
                auto klass = U::GetClass(component->Il2CppClass.klass);

                std::printf("Found component: %s\n", klass->name.c_str());

                for (const auto& method : klass->methods)
                {
                    //std::printf("Found method: %s\n", method->name.c_str());
                    if(method->name == methodName)
                    {
                        std::string fullName = method->return_type->name + " " + klass->namespaze + "." + klass->name + "::" + method->name + "()";
                        std::printf("Found method: %s\n", fullName.c_str());
                        functionHash = mirror::GetStableHashCode(fullName);
                        std::printf("Method hash: %02x\n", functionHash);

                        componentIndex = U::Get(ASSEMBLY_MIRROR)->Get("NetworkBehaviour")->GetValue<uint8_t>(component, "<ComponentIndex>k__BackingField");
                        break;
                    }
                }

                if(functionHash) break;
            }

            NetworkStream stream;

            stream.write<uint16_t>(msgId);
            stream.WriteVarUint(netId);
            stream.WriteByte(componentIndex);
            stream.write<uint16_t>(functionHash);
            stream.WriteVarUint(payload.size() + 1);
            stream.WriteBytes(payload.data(), payload.size());

            return stream;
        }

        UC* GetTargetIdentity()
        {
            return mirror::GetTargetIdentity(netId);
        }

        UG* GetTarget()
        {
            return GetTargetIdentity()->GetGameObject();
        }

        void Print() override
        {
            std::printf("CommandMessage: netId=%d(%s), componentIndex=%d, functionHash=%02x\n", netId, GetTarget()->GetName()->ToString().c_str(), componentIndex, functionHash);
        }
    };
}

