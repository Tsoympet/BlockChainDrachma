#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>

class Serializer {
public:
    static void writeUint32(std::vector<uint8_t>& buf, uint32_t v) {
        for (int i = 0; i < 4; ++i)
            buf.push_back((v >> (8 * i)) & 0xFF);
    }

    static void writeUint64(std::vector<uint8_t>& buf, uint64_t v) {
        for (int i = 0; i < 8; ++i)
            buf.push_back((v >> (8 * i)) & 0xFF);
    }

    static uint32_t readUint32(const std::vector<uint8_t>& buf, size_t& off) {
        if (off + 4 > buf.size()) throw std::runtime_error("readUint32 OOB");
        uint32_t v = 0;
        for (int i = 0; i < 4; ++i)
            v |= uint32_t(buf[off++]) << (8 * i);
        return v;
    }

    static uint64_t readUint64(const std::vector<uint8_t>& buf, size_t& off) {
        if (off + 8 > buf.size()) throw std::runtime_error("readUint64 OOB");
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i)
            v |= uint64_t(buf[off++]) << (8 * i);
        return v;
    }

    static void writeVarBytes(std::vector<uint8_t>& buf, const std::vector<uint8_t>& data) {
        writeUint32(buf, static_cast<uint32_t>(data.size()));
        buf.insert(buf.end(), data.begin(), data.end());
    }

    static std::vector<uint8_t> readVarBytes(const std::vector<uint8_t>& buf, size_t& off) {
        uint32_t size = readUint32(buf, off);
        if (off + size > buf.size()) throw std::runtime_error("readVarBytes OOB");
        std::vector<uint8_t> out(buf.begin() + off, buf.begin() + off + size);
        off += size;
        return out;
    }
};
