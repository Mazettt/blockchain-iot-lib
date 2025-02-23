#include <gtest/gtest.h>

#include <Block.hpp>
#include <testers.hpp>

TEST(Block, MineEmptyBlock)
{
    iotbc::Block block(iotbc::NULL_HASH);

    block.mine(0);

    ASSERT_EQ(block.nonce, 0);

    ASSERT_EQ(block.merkleRoot, iotbc::EMPTY_STRING_HASH);

    ASSERT_NE(block.blockHash(), iotbc::NULL_HASH);
}

TEST(Block, MineBlockWithTx)
{
    iotbc::Block block(iotbc::NULL_HASH);

    iotbc::Transaction tx(alice.public_key, 0, {0x00, 0x01, 0x02});
    tx.sign(alice);

    block.addTransaction(tx);

    block.mine(0);

    ASSERT_EQ(block.nonce, 0);

    ASSERT_NE(block.merkleRoot, iotbc::NULL_HASH);
    ASSERT_NE(block.merkleRoot, iotbc::EMPTY_STRING_HASH);

    ASSERT_NE(block.blockHash(), iotbc::NULL_HASH);
}

TEST(Block, MineBlockWithSomeDifficulty)
{
    iotbc::Block block(iotbc::NULL_HASH);

    iotbc::Transaction tx(alice, 0, {0x00, 0x01, 0x02});
    tx.sign(alice);

    block.addTransaction(tx);

    block.mine(2);

    // It is very unlikely that the nonce will be 0 with some difficulty
    ASSERT_NE(block.nonce, 0);

    ASSERT_NE(block.merkleRoot, iotbc::NULL_HASH);
    ASSERT_NE(block.merkleRoot, iotbc::EMPTY_STRING_HASH);

    ASSERT_NE(block.blockHash(), iotbc::NULL_HASH);
}

TEST(Block, VerifyTransactionsAllValid)
{
    iotbc::Block block(iotbc::NULL_HASH);

    iotbc::Transaction tx(alice, 0, {0x00, 0x01, 0x02});
    tx.sign(alice);

    iotbc::Transaction tx2(bob, 1, {0x00, 0x01, 0x02});
    tx2.sign(bob);

    block.addTransaction(tx);

    block.verifyTransactions();
}

TEST(Block, VerifyTransactionsOneInvalidGotInserted)
{
    iotbc::Block block(iotbc::NULL_HASH);

    iotbc::Transaction tx(alice, 0, {0x00, 0x01, 0x02});
    tx.sign(alice);

    block.addTransaction(tx);

    iotbc::Transaction tx2(bob, 1, {0x00, 0x01, 0x02});
    tx2.sign(alice);

    // Forcefully add an invalid transaction in the struct
    block.transactions.push_back(tx2);

    ASSERT_THROW(block.verifyTransactions(), iotbc::InvalidSignature);
}

TEST(Block, TryToAddOneInvalidTx)
{
    iotbc::Block block(iotbc::NULL_HASH);

    iotbc::Transaction tx(bob, 1, {0x00, 0x01, 0x02});
    tx.sign(alice);

    ASSERT_THROW(block.addTransaction(tx), iotbc::InvalidSignature);
}