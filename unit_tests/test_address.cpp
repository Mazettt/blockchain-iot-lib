#include <gtest/gtest.h>

#include <Types.hpp>

TEST(Address, fromString)
{
    iotbc::Address addr = iotbc::Address::fromString("0x00ff000000000000000000000000000000000001");
    std::array<unsigned char, 20> bits = {0};
    bits[1] = 255;
    bits[19] = 1;
    iotbc::Address expected(bits);

    ASSERT_EQ(addr, expected);
}

TEST(Address, fromStringInvalidLength)
{
    ASSERT_THROW(iotbc::Address::fromString("0x00"), iotbc::InvalidAddress);
}

TEST(Address, fromStringInvalidPrefix)
{
    ASSERT_THROW(iotbc::Address::fromString("coucou"), iotbc::InvalidAddress);
}

TEST(Address, fromStringInvalidCharacter)
{
    ASSERT_THROW(iotbc::Address::fromString("0xz000000000000000000000000000000000000000"), iotbc::InvalidAddress);
}

TEST(Address, toString)
{
    iotbc::Address addr;
    addr.bits[0] = 1;
    addr.bits[1] = 2;
    addr.bits[2] = 255;
    addr.bits[19] = 1;
    std::string str = addr.toString();

    ASSERT_EQ(str, "0x0102ff0000000000000000000000000000000001");
}