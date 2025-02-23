#pragma once

#include <Types.hpp>

namespace iotbc {
    std::string hashToString(const Hash &hash)
    {
        std::string str;
        str.reserve(hash.size() * 2);

        for (const auto &byte : hash) {
            str += "0123456789abcdef"[byte >> 4];
            str += "0123456789abcdef"[byte & 0x0F];
        }

        return str;
    }
}
