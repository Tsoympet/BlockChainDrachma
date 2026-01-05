#include <gtest/gtest.h>
#include <filesystem>
#include <openssl/evp.h>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>
#include "../../layer2-services/crosschain/bridge/bridge_manager.h"
#include "../../layer2-services/crosschain/relayer/relayer.h"
#include "../../layer2-services/net/p2p.h"

using namespace crosschain;

namespace {
std::array<uint8_t, 32> Hash(const std::vector<uint8_t>& data)
{
    std::array<uint8_t, 32> h{};
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
    if (EVP_DigestFinal_ex(ctx, h.data(), &len) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("EVP_DigestFinal_ex failed");
    }
    EVP_MD_CTX_free(ctx);
    return h;
}
}

TEST(BridgeFlow, InitiateClaimRefund)
{
    auto tmp = std::filesystem::temp_directory_path() / "bridge_flow";
    BridgeManager mgr(tmp.string());

    ChainConfig cfg{};
    mgr.RegisterChain("bitcoin", cfg);

    std::vector<uint8_t> secret = {1, 2, 3, 4};
    auto secretHash = Hash(secret);
    std::array<uint8_t, 32> priv{};
    priv[0] = 1;
    std::vector<uint8_t> sig;
    auto lock = mgr.InitiateOutboundLock("bitcoin", "tx1", "addr", 50, secretHash, 100, priv, sig);
    EXPECT_FALSE(sig.empty());

    std::vector<uint8_t> claimSig;
    EXPECT_TRUE(mgr.Claim(lock.id, secret, 10, claimSig));
    EXPECT_FALSE(claimSig.empty());

    EXPECT_FALSE(mgr.Refund(lock.id, 50));
    EXPECT_TRUE(mgr.Refund(lock.id, 150));
}

TEST(BridgeFlow, DetectsInboundLock)
{
    auto tmp = std::filesystem::temp_directory_path() / "bridge_flow_inbound";
    BridgeManager mgr(tmp.string());
    ChainConfig cfg{};
    mgr.RegisterChain("litecoin", cfg);

    HeaderProof proof{};
    proof.height = 1;
    proof.header.fill(0);

    BridgeLock observed{};
    observed.chain = "litecoin";
    observed.txid = "lock";
    observed.destination = "drachma";
    observed.amount = 100;
    observed.timeoutHeight = 50;

    EXPECT_TRUE(mgr.DetectInboundLock("litecoin", {proof}, observed));
    auto pending = mgr.PendingFor("drachma");
    EXPECT_FALSE(pending.empty());
}

TEST(RelayerFlow, HandlesEmptyEndpoint)
{
    auto tmp = std::filesystem::temp_directory_path() / "relayer_tmp";
    BridgeManager mgr(tmp.string());
    boost::asio::io_context io;
    net::P2PNode p2p(io, 0);

    ChainConfig cfg{};
    cfg.rpcEndpoint = "";
    crosschain::Relayer rel(mgr, p2p, io);
    rel.AddWatchedChain("empty", cfg);
    rel.Start();
    rel.Stop();
    EXPECT_EQ(rel.Metrics().detected.load(), 0u);
}

TEST(RelayerFlow, InvalidProofResponseIsIgnored)
{
    auto tmp = std::filesystem::temp_directory_path() / "relayer_invalid";
    BridgeManager mgr(tmp.string());
    boost::asio::io_context io;
    net::P2PNode p2p(io, 0);

    // Minimal HTTP server returning 404 to trigger proof rejection.
    boost::asio::io_context serverIo;
    boost::asio::ip::tcp::acceptor acc(serverIo, {boost::asio::ip::tcp::v4(), 0});
    const auto port = acc.local_endpoint().port();
    std::thread server([&]() {
        boost::asio::ip::tcp::socket sock(serverIo);
        acc.accept(sock);
        const std::string resp = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        boost::asio::write(sock, boost::asio::buffer(resp));
    });

    crosschain::Relayer rel(mgr, p2p, io);
    ChainConfig cfg{};
    cfg.rpcEndpoint = "http://127.0.0.1:" + std::to_string(port) + "/proof";
    rel.AddWatchedChain("invalid", cfg);
    rel.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    rel.Stop();

    EXPECT_EQ(rel.Metrics().detected.load(), 0u);
    server.join();
}
