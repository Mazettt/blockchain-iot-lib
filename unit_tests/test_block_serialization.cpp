#include <gtest/gtest.h>

#include <Block.hpp>
#include <testers.hpp>

TEST(Block, SerializeEmptyBlock)
{
    iotbc::Block block(iotbc::NULL_HASH);

    block.mine(0);

    auto serialized = block.serialize();

    // prevHash (32) + txCount (8) + no txs (0) + merkleRoot (32) + nonce (8) + blockHash (32)
    ASSERT_EQ(serialized.size(), 32 + 8 + 32 + 8 + 32);

    auto deserialized = iotbc::Block::deserialize(serialized);

    ASSERT_EQ(block.prevHash, deserialized.prevHash);
    ASSERT_EQ(block.transactions.size(), deserialized.transactions.size());
    ASSERT_EQ(block.merkleRoot, deserialized.merkleRoot);
    ASSERT_EQ(block.nonce, deserialized.nonce);
}

TEST(Block, SerializeBlockWithFewTxs)
{
    iotbc::Block block(iotbc::NULL_HASH);

    iotbc::Transaction tx(alice, 0, {0x00, 0x01, 0x02});
    tx.sign(alice);

    iotbc::Transaction tx2(bob, 1, {0x00, 0x01, 0x02});
    tx2.sign(bob);

    block.mine(0);

    auto serialized = block.serialize();

    auto deserialized = iotbc::Block::deserialize(serialized);

    ASSERT_EQ(block.prevHash, deserialized.prevHash);
    ASSERT_EQ(block.transactions.size(), deserialized.transactions.size());

    for (size_t i = 0; i < block.transactions.size(); i++) {
        ASSERT_EQ(block.transactions[i].from, deserialized.transactions[i].from);
        ASSERT_EQ(block.transactions[i].nonce, deserialized.transactions[i].nonce);
        ASSERT_EQ(block.transactions[i].data, deserialized.transactions[i].data);
        ASSERT_EQ(block.transactions[i].signature, deserialized.transactions[i].signature);
    }

    ASSERT_EQ(block.merkleRoot, deserialized.merkleRoot);
    ASSERT_EQ(block.nonce, deserialized.nonce);
}
