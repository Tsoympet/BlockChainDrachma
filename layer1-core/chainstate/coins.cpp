#include "coins.h"

bool CoinsView::HaveUTXO(const OutPointKey& key) const {
    return mapUTXO.find(key) != mapUTXO.end();
}

const UTXO& CoinsView::GetUTXO(const OutPointKey& key) const {
    return mapUTXO.at(key);
}

void CoinsView::AddUTXO(const OutPointKey& key, const UTXO& utxo) {
    mapUTXO[key] = utxo;
}

void CoinsView::RemoveUTXO(const OutPointKey& key) {
    mapUTXO.erase(key);
}

Chainstate::Chainstate() {}

bool Chainstate::ApplyTransaction(const Transaction& tx,
                                  uint32_t height,
                                  BlockUndo& undo,
                                  std::string& error)
{
    // Spend inputs
    if (!tx.isCoinbase()) {
        for (const auto& in : tx.vin) {
            auto key = MakeOutPointKey(in.prevout.txid, in.prevout.index);

            if (!coins.HaveUTXO(key)) {
                error = "Missing UTXO";
                return false;
            }

            const UTXO& utxo = coins.GetUTXO(key);

            if (utxo.coinbase && height < utxo.height + COINBASE_MATURITY) {
                error = "Coinbase not matured";
                return false;
            }

            // Save undo
            undo.spent.push_back({ key, utxo });

            // Remove UTXO
            coins.RemoveUTXO(key);
        }
    }

    // Create outputs
    std::string txid = tx.getTxID();
    for (uint32_t i = 0; i < tx.vout.size(); ++i) {
        OutPointKey outKey = MakeOutPointKey(txid, i);
        coins.AddUTXO(outKey, UTXO(tx.vout[i], height, tx.isCoinbase()));
    }

    return true;
}

void Chainstate::ApplyBlock(const std::vector<Transaction>& txs,
                            uint32_t height,
                            BlockUndo& undo)
{
    for (const auto& tx : txs) {
        std::string error;
        if (!ApplyTransaction(tx, height, undo, error)) {
            throw std::runtime_error("Block apply failed: " + error);
        }
    }
}

void Chainstate::DisconnectBlock(const BlockUndo& undo)
{
    // Remove created UTXOs (reverse not needed if called before reorg rebuild)
    // Restore spent UTXOs
    for (auto it = undo.spent.rbegin(); it != undo.spent.rend(); ++it) {
        coins.AddUTXO(it->key, it->utxo);
    }
}

