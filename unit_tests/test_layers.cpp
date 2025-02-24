#include <gtest/gtest.h>

#include <Block.hpp>
#include <Blockchain.hpp>
#include <testers.hpp>

// A basic test layer that just counts the overall number of transactions
class TransactionCountLayer : public iotbc::ILayer {
    public:
        size_t transactionCount = 0;

        void processBlock(const iotbc::Block &block) override {
            transactionCount += block.transactions.size();
        }
};

TEST(LayersTest, TestTransactionCount)
{
    iotbc::Blockchain chain;

    std::shared_ptr<TransactionCountLayer> layer = std::make_shared<TransactionCountLayer>();
    chain.addLayer(layer);

    iotbc::Block block1(iotbc::NULL_HASH);

    iotbc::Transaction tx(alice.public_key, 0, {0x00, 0x01, 0x02});
    tx.sign(alice);

    block1.addTransaction(tx);

    block1.mine(0);

    chain.addBlock(block1);

    iotbc::Block block2(block1.blockHash());

    iotbc::Transaction tx2(alice.public_key, 1, {0x01, 0x02, 0x03});
    tx2.sign(alice);

    block2.addTransaction(tx2);

    block2.mine(0);

    chain.addBlock(block2);

    chain.verifyExistingChain();

    ASSERT_EQ(layer->transactionCount, 2);
}