#pragma once

#include <Types.hpp>

namespace iotbc {
    /// @brief Interface for a layer in the blockchain
    /// @note A layer is a set of functions executed for every block in the blockchain
    /// that can be used for custom functionnality
    /// They are run every time a block is added to the blockchain
    class ILayer {
        public:
            virtual ~ILayer() = default;

            /// @brief Function called for every block in the blockchain
            /// @param block The block to process
            virtual void processBlock(const Block &block) = 0;
    };
}
