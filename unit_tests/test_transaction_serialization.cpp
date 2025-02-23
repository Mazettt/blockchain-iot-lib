#include <gtest/gtest.h>

#include <Types.hpp>

#include <testers.hpp>

TEST(Transaction, SerializationWorks)
{
    iotbc::Transaction tx(alice.public_key, 42, {0x00, 0x01, 0x02});

    auto serialized = tx.serialize();

    // Size is : public_key (64) + nonce (8) + data size (8) + data (3) + signature (64)
    ASSERT_EQ(serialized.size(), 147);

    // First is the public key
    ASSERT_EQ(memcmp(serialized.data(), alice.public_key.data(), 64), 0);

    // Then the nonce
    ASSERT_EQ(memcmp(serialized.data() + 64, &tx.nonce, 8), 0);

    // Then the data size
    size_t data_size = 0;
    for (size_t i = 0; i < sizeof(size_t); i++) {
        data_size |= serialized[64 + 8 + i] << (i * sizeof(size_t));
    }
    ASSERT_EQ(data_size, 3);

    // Finally the data
    ASSERT_EQ(memcmp(serialized.data() + 64 + 8 + sizeof(size_t), tx.data.data(), 3), 0);
}

TEST(Transaction, DeserializationWorks)
{
    iotbc::Transaction tx(alice.public_key, 42, {0x00, 0x01, 0x02});

    auto serialized = tx.serialize();

    auto deserialized = iotbc::Transaction::deserialize(serialized);

    ASSERT_EQ(deserialized.from, tx.from);
    ASSERT_EQ(deserialized.nonce, tx.nonce);
    ASSERT_EQ(deserialized.data, tx.data);
    ASSERT_EQ(deserialized.signature, tx.signature);
}

TEST(Transaction, DeserializationWorksWithEmptyData)
{
    iotbc::Transaction tx(alice.public_key, 42, {});

    auto serialized = tx.serialize();

    auto deserialized = iotbc::Transaction::deserialize(serialized);

    ASSERT_EQ(deserialized.from, tx.from);
    ASSERT_EQ(deserialized.nonce, tx.nonce);
    ASSERT_EQ(deserialized.data, tx.data);
    ASSERT_EQ(deserialized.signature, tx.signature);
}

TEST(Transaction, DeserializationFailsIfEmpty)
{
    ASSERT_THROW(iotbc::Transaction::deserialize({}), iotbc::DeserializationError);
}

TEST(Transaction, DeserializationFailsIfInvalidSize)
{
    ASSERT_THROW(iotbc::Transaction::deserialize({0, 0, 0}), iotbc::DeserializationError);
}

TEST(Transaction, DeserializationFailsIfDataOverflows)
{
    // Enough space for the public key, nonce, and data size, but not for the data
    std::vector<unsigned char> data(64 + 8 + sizeof(size_t), 0);

    size_t msg_size = 227;
    for (size_t i = 0; i < sizeof(size_t); i++) {
        data[64 + 8 + i] = (msg_size >> (i * sizeof(size_t)) & 0xff);
    }

    ASSERT_THROW(iotbc::Transaction::deserialize(data), iotbc::DeserializationError);
}
