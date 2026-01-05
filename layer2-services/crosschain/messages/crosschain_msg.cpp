#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <array>
#include <openssl/evp.h>

namespace crosschain {

struct CrossChainMessage {
    uint32_t version{1};
    std::string source;
    std::string destination;
    std::vector<uint8_t> payload;

    std::vector<uint8_t> Serialize() const
    {
        std::vector<uint8_t> out;
        auto writeStr = [&out](const std::string& s){
            uint32_t len = static_cast<uint32_t>(s.size());
            out.insert(out.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + sizeof(len));
            out.insert(out.end(), s.begin(), s.end());
        };
        out.insert(out.end(), reinterpret_cast<const uint8_t*>(&version), reinterpret_cast<const uint8_t*>(&version) + sizeof(version));
        writeStr(source);
        writeStr(destination);
        uint32_t len = static_cast<uint32_t>(payload.size());
        out.insert(out.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + sizeof(len));
        out.insert(out.end(), payload.begin(), payload.end());
        return out;
    }

    std::array<uint8_t,32> Hash() const
    {
        auto data = Serialize();
        std::array<uint8_t,32> out{};
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) throw std::runtime_error("EVP_MD_CTX_new failed");
        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("EVP_DigestInit_ex failed");
        }
        if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("EVP_DigestUpdate failed");
        }
        unsigned int len = 32;
        if (EVP_DigestFinal_ex(ctx, out.data(), &len) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("EVP_DigestFinal_ex failed");
        }
        EVP_MD_CTX_free(ctx);
        return out;
    }
};

} // namespace crosschain
