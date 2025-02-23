#include <gtest/gtest.h>

#include <Block.hpp>
#include <Blockchain.hpp>
#include <testers.hpp>

TEST(Blockchain, ValidBlockchain)
{
    iotbc::Blockchain chain;

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

    chain.verifyExistingChain();
}

TEST(Blockchain, InvalidBlockchainHashMismatch)
{
    iotbc::Blockchain chain;

    iotbc::Block block1(iotbc::NULL_HASH);

    iotbc::Transaction tx(alice.public_key, 0, {0x00, 0x01, 0x02});
    tx.sign(alice);

    block1.addTransaction(tx);

    block1.mine(0);

    chain.addBlock(block1);

    iotbc::Block block2(iotbc::NULL_HASH); // Uses NULL_HASH as prevHash, again

    iotbc::Transaction tx2(alice.public_key, 1, {0x01, 0x02, 0x03});
    tx2.sign(alice);

    block2.addTransaction(tx2);

    block2.mine(0);

    ASSERT_THROW(chain.addBlock(block2), iotbc::InvalidBlock);
}

TEST(Blockchain, InvalidBlockchainMalformedTx)
{
    iotbc::Blockchain chain;

    iotbc::Block block1(iotbc::NULL_HASH);

    iotbc::Transaction tx(alice.public_key, 0, {0x00, 0x01, 0x02});
    tx.sign(alice);

    block1.addTransaction(tx);

    block1.mine(0);

    chain.addBlock(block1);

    iotbc::Block block2(iotbc::NULL_HASH); // Uses NULL_HASH as prevHash, again

    iotbc::Transaction tx2(bob.public_key, 1, {0x01, 0x02, 0x03});
    tx2.sign(alice); // Signs with the wrong key

    // Forcefully add a malformed transaction
    block2.transactions.push_back(tx2);

    block2.mine(0);

    ASSERT_THROW(chain.addBlock(block2), iotbc::InvalidBlock);
}