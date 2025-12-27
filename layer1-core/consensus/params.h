#pragma once
#include <cstdint>
#include <string>

namespace consensus {

struct Params {
    uint32_t nSubsidyHalvingInterval;
    uint32_t nPowTargetSpacing;
    uint32_t nPowTargetTimespan;
    uint32_t nDifficultyAdjustmentInterval;
    uint64_t nMaxMoneyOut;

    bool fPowAllowMinDifficultyBlocks;

    uint32_t nGenesisTime;
    uint32_t nGenesisBits;
    uint32_t nGenesisNonce;
    std::string genesisMessage;
};

const Params& Main();
const Params& Testnet();

} // namespace consensus
