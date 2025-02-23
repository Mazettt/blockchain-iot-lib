#include <gtest/gtest.h>

#include <Block.hpp>
#include <Blockchain.hpp>
#include <testers.hpp>


TEST(Blockchain, DoesntFailIfFolderIsEmpty)
{
    iotbc::Blockchain chain;

    chain.loadExistingBlocks("/tmp/non_existent_folder");
}
