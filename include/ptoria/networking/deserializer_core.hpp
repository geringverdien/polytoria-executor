#include <polytoria/networking/network_stream.hpp>
#include <unity/u.hpp>

namespace mirror
{
    int GetStableHashCode(const std::string& str);
    uint32_t GetNetId(UG* gameObject);
    std::vector<UC*> GetNetworkBehaviours(UG* gameObject);
    UC* GetTargetIdentity(uint32_t netId);
    void DeserializePacket(NetworkStream& stream);
}