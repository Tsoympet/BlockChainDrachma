#include "../block/block.h"
#include "../crypto/tagged_hash.h"
#include "params.h"

Block CreateGenesisBlock(const consensus::Params& params) {
    Transaction coinbase;
    coinbase.vout.push_back({50 * 100000000ULL, params.genesisMessage});

    Block genesis;
    genesis.header.version = 1;
    genesis.header.prevBlockHash = uint256::Zero();
    genesis.header.time = params.nGenesisTime;
    genesis.header.bits = params.nGenesisBits;
    genesis.header.nonce = params.nGenesisNonce;
    genesis.transactions.push_back(coinbase);
    genesis.header.merkleRoot = ComputeMerkleRoot(genesis.transactions);

    return genesis;
}
