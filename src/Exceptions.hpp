#pragma once

#include <stdexcept>

#define _CUSTOM_EXCEPTION(name, base) \
    class name : public base { \
    public: \
        name(const std::string &msg) : base(#name ": " + msg) {} \
    }

namespace iotbc {
    _CUSTOM_EXCEPTION(DeserializationError, std::runtime_error);
    _CUSTOM_EXCEPTION(InvalidBlockchainSave, std::runtime_error);
    _CUSTOM_EXCEPTION(InvalidBlock, std::runtime_error);
    _CUSTOM_EXCEPTION(InvalidTransaction, std::runtime_error);
    _CUSTOM_EXCEPTION(InvalidSignature, std::runtime_error);
    _CUSTOM_EXCEPTION(InvalidAddress, std::runtime_error);
    _CUSTOM_EXCEPTION(Secp256k1Error, std::runtime_error);
    _CUSTOM_EXCEPTION(EvpError, std::runtime_error);
    _CUSTOM_EXCEPTION(IoError, std::runtime_error);
}
