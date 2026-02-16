// Little network stream class to do handle proper deserializing of Packets
#ifndef NETWORK_STREAM_HPP
#define NETWORK_STREAM_HPP

#include <vector>
#include <iostream>


class NetworkStream{
private:
    std::vector<uint8_t> data;
    size_t readPosition;
    size_t writePosition;
public:
    NetworkStream(const char* start, const char* end, bool copyData = false);
    NetworkStream(const std::vector<uint8_t>& data);
    NetworkStream(); // Default constructor for writing

    template<typename T>
    T read(){
        T result;
        if(readPosition + sizeof(T) > data.size())
            throw std::runtime_error("Not enough data");

        std::memcpy(&result, data.data() + readPosition, sizeof(T));
        readPosition += sizeof(T);
        return result;
    }

    template<typename T>
    void read(T& val)
    {
        if(readPosition + sizeof(T) > data.size())
            throw std::runtime_error("Not enough data");

        val = *(T*)(data.data() + readPosition);
        readPosition += sizeof(T);
    }

    uint8_t ReadByte();
    void ReadBytes(uint8_t* buffer, size_t size);

    [[nodiscard]] size_t GetSize() const { return data.size(); }

    [[nodiscard]] const char* GetData() { return (const char*)data.data(); };

    [[nodiscard]] const size_t& GetReadPosition() const { return readPosition; }
    void SetReadPosition(size_t position) { readPosition = position; }

    [[nodiscard]] const size_t& GetWritePosition() const { return writePosition; }
    void SetWritePosition(size_t position) { writePosition = position; }


    template <typename T>
    void write(T val)
    {
        uint8_t* bytes = (uint8_t*)&val;
        data.insert(data.begin() + writePosition, bytes, bytes + sizeof(T));
        writePosition += sizeof(T);
    }

    void WriteByte(uint8_t byte);
    void WriteBytes(const uint8_t* buffer, size_t size);

    void WriteVarUint(uint64_t value);
    uint64_t ReadVarUint();

    void PrintPayload();
};

#endif /* NETWORK_STREAM_HPP */