#include <ptoria/networking/network_stream.hpp> 

NetworkStream::NetworkStream(const char* start, const char* end, bool copyData)
    : readPosition(0)
{
    if (copyData)
    {
        data.assign(start, end);
    }
    else
    {
        data = std::vector<uint8_t>(start, end);
    }
}

NetworkStream::NetworkStream(const std::vector<uint8_t>& data)
    : data(data), readPosition(0)
{
}

NetworkStream::NetworkStream()
    : readPosition(0)
{
    data = std::vector<uint8_t>(0);
}

uint8_t NetworkStream::ReadByte()
{
    if(readPosition >= data.size())
        throw std::runtime_error("Not enough data");

    return data[readPosition++];
}

void NetworkStream::ReadBytes(uint8_t* buffer, size_t size)
{
    if(readPosition + size > data.size())
        throw std::runtime_error("Not enough data");

    std::memcpy(buffer, data.data() + readPosition, size);
    readPosition += size;
}

void NetworkStream::WriteBytes(const uint8_t* buffer, size_t size)
{
    data.insert(data.begin() + writePosition, buffer, buffer + size);
    writePosition += size;
}

void NetworkStream::WriteByte(uint8_t byte)
{
    data.insert(data.begin() + writePosition, byte);
    writePosition++;
}

void NetworkStream::PrintPayload()
{
    for (size_t i = 0; i < GetSize(); i++)
    {
        std::printf("%02x ", data[i]);
    }
    std::printf("\n");
}


// stripped from mirror source
uint64_t NetworkStream::ReadVarUint()
{
    uint8_t a0 = ReadByte();
    if (a0 < 241)
    {
        return a0;
    }

    uint8_t a1 = ReadByte();
    if (a0 <= 248)
    {
        return 240 + ((a0 - (uint64_t)241) << 8) + a1;
    }

    uint8_t a2 = ReadByte();
    if (a0 == 249)
    {
        return 2288 + ((uint64_t)a1 << 8) + a2;
    }

    uint8_t a3 = ReadByte();
    if (a0 == 250)
    {
        return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16);
    }

    uint8_t a4 = ReadByte();
    if (a0 == 251)
    {
        return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16) + (((uint64_t)a4) << 24);
    }

    uint8_t a5 = ReadByte();
    if (a0 == 252)
    {
        return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16) + (((uint64_t)a4) << 24) + (((uint64_t)a5) << 32);
    }

    uint8_t a6 = ReadByte();
    if (a0 == 253)
    {
        return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16) + (((uint64_t)a4) << 24) + (((uint64_t)a5) << 32) + (((uint64_t)a6) << 40);
    }

    uint8_t a7 = ReadByte();
    if (a0 == 254)
    {
        return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16) + (((uint64_t)a4) << 24) + (((uint64_t)a5) << 32) + (((uint64_t)a6) << 40) + (((uint64_t)a7) << 48);
    }

    uint8_t a8 = ReadByte();
    if (a0 == 255)
    {
        return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16) + (((uint64_t)a4) << 24) + (((uint64_t)a5) << 32) + (((uint64_t)a6) << 40) + (((uint64_t)a7) << 48)  + (((uint64_t)a8) << 56);
    }


    throw std::runtime_error("Invalid VarUint encoding");
}

void NetworkStream::WriteVarUint(uint64_t value)
{
    if (value <= 240)
    {
        uint8_t a = (uint8_t)value;
        write(a);
        return;
    }
    if (value <= 2287)
    {
        uint8_t a = (uint8_t)(((value - 240) >> 8) + 241);
        uint8_t b = (uint8_t)((value - 240) & 0xFF);
        write<uint16_t>((uint16_t)(b << 8 | a));
        return;
    }
    if (value <= 67823)
    {
        uint8_t a = (uint8_t)249;
        uint8_t b = (uint8_t)((value - 2288) >> 8);
        uint8_t c = (uint8_t)((value - 2288) & 0xFF);
        write(a);
        write<uint16_t>((uint16_t)(c << 8 | b));
        return;
    }
    if (value <= 16777215)
    {
        uint8_t a = (uint8_t)250;
        uint32_t b = (uint32_t)(value << 8);
        write<uint32_t>(b | a);
        return;
    }
    if (value <= 4294967295)
    {
        uint8_t a = (uint8_t)251;
        uint32_t b = (uint32_t)value;
        write(a);
        write<uint32_t>(b);
        return;
    }
    if (value <= 1099511627775)
    {
        uint8_t a = (uint8_t)252;
        uint8_t b = (uint8_t)(value & 0xFF);
        uint32_t c = (uint32_t)(value >> 8);
        write<uint16_t>((uint16_t)(b << 8 | a));
        write<uint32_t>(c);
        return;
    }
    if (value <= 281474976710655)
    {
        uint8_t a = (uint8_t)253;
        uint8_t b = (uint8_t)(value & 0xFF);
        uint8_t c = (uint8_t)((value >> 8) & 0xFF);
        uint32_t d = (uint32_t)(value >> 16);
        write(a);
        write<uint16_t>((uint16_t)(c << 8 | b));
        write<uint32_t>(d);
        return;
    }
    if (value <= 72057594037927935)
    {
        uint8_t a = (uint8_t)254;
        uint64_t b = (uint64_t)(value << 8);
        write<uint64_t>(b | a);
        return;
    }

    write<uint8_t>(255);
    write<uint64_t>(value);
}