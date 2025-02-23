#include <gtest/gtest.h>

#include <Types.hpp>

#include <testers.hpp>

TEST(Transaction, SigningWorks)
{
    iotbc::Transaction tx(bob.public_key, 0, {0x00, 0x01, 0x02});

    tx.sign(bob);

    tx.verify();
}

TEST(Transaction, VerifyFailsIfNotSigned)
{
    iotbc::Transaction tx(bob.public_key, 0, {0x00, 0x01, 0x02});

    ASSERT_THROW(tx.verify(), iotbc::InvalidSignature);
}

TEST(Transaction, VerifyFailsIfSenderDoesNotMatch)
{
    iotbc::Transaction tx(bob.public_key, 0, {0x00, 0x01, 0x02});

    tx.sign(alice);

    ASSERT_THROW(tx.verify(), iotbc::InvalidSignature);
}
