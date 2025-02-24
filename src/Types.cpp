#include <Types.hpp>

namespace iotbc {
    std::string Address::toString() const
    {
        std::string str = "0x";
        str.reserve(42);

        for (unsigned char byte : bits) {
            str.push_back("0123456789abcdef"[byte >> 4]);
            str.push_back("0123456789abcdef"[byte & 0x0F]);
        }

        return str;
    }

    Address Address::fromString(const std::string &str)
    {
        if (str.size() != 42) {
            throw InvalidAddress("Invalid address length");
        }
        if (str.substr(0, 2) != "0x") {
            throw InvalidAddress("Invalid address prefix");
        }

        std::array<unsigned char, 20> bits;

        for (size_t i = 0; i < 20; i++) {
            if (!isxdigit(str[2 + i * 2]) || !isxdigit(str[3 + i * 2])) {
                throw InvalidAddress("Invalid character in address");
            }

            bits[i] = std::stoi(str.substr(2 + i * 2, 2), nullptr, 16);
        }

        return Address(bits);
    }

    Address Address::fromPublicKey(const PublicKey &public_key)
    {
        Address address;

        EVP_MD_CTX *ctx = EVP_MD_CTX_new();

        if (ctx == nullptr) {
            throw EvpError("Failed to create EVP_MD_CTX");
        }

        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to initialize digest");
        }

        if (EVP_DigestUpdate(ctx, public_key.data(), public_key.size()) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to update digest");
        }

        unsigned int len = 0;
        if (EVP_DigestFinal_ex(ctx, address.bits.data(), &len) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to finalize digest");
        }

        EVP_MD_CTX_free(ctx);

        return address;
    }

    Signer::Signer(const PrivateKey &private_key)
        : private_key(private_key)
    {
        secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);

        if (ctx == nullptr) {
            throw Secp256k1Error("Failed to create secp256k1 context");
        }

        secp256k1_pubkey s_pubkey;

        if (secp256k1_ec_pubkey_create(ctx, &s_pubkey, private_key.data()) != 1) {
            secp256k1_context_destroy(ctx);
            throw Secp256k1Error("Failed to create public key");
        }

        memcpy(public_key.data(), s_pubkey.data, 64);

        secp256k1_context_destroy(ctx);

        address = Address::fromPublicKey(public_key);
    }

    Hash Transaction::txHash() const
    {
        Hash h;

        EVP_MD_CTX *ctx = EVP_MD_CTX_new();

        if (ctx == nullptr) {
            throw EvpError("Failed to create EVP_MD_CTX");
        }

        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to initialize digest");
        }

        if (EVP_DigestUpdate(ctx, &nonce, sizeof(Nonce)) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to update digest");
        }

        if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to update digest");
        }

        if (EVP_DigestFinal_ex(ctx, h.data(), nullptr) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to finalize digest");
        }

        EVP_MD_CTX_free(ctx);

        return h;
    }

    void Transaction::sign(const PrivateKey &private_key)
    {
        secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

        if (ctx == nullptr) {
            throw Secp256k1Error("Failed to create secp256k1 context");
        }

        secp256k1_ecdsa_signature sig;

        Hash tx_hash = this->txHash();

        if (secp256k1_ecdsa_sign(ctx, &sig, tx_hash.data(), private_key.data(), nullptr, nullptr) != 1) {
            secp256k1_context_destroy(ctx);
            throw Secp256k1Error("Failed to sign transaction");
        }

        secp256k1_ecdsa_signature_serialize_compact(ctx, signature.data(), &sig);

        secp256k1_context_destroy(ctx);
    }

    void Transaction::verify() const
    {
        secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);

        if (ctx == nullptr) {
            throw Secp256k1Error("Failed to create secp256k1 context");
        }

        secp256k1_ecdsa_signature sig;

        if (secp256k1_ecdsa_signature_parse_compact(ctx, &sig, signature.data()) != 1) {
            secp256k1_context_destroy(ctx);
            throw Secp256k1Error("Failed to parse signature");
        }

        secp256k1_pubkey pubkey;
        memcpy(pubkey.data, from.data(), 64);

        if (secp256k1_ecdsa_verify(ctx, &sig, this->txHash().data(), &pubkey) != 1) {
            secp256k1_context_destroy(ctx);
            throw InvalidSignature("Invalid signature");
        }

        secp256k1_context_destroy(ctx);
    }

    std::vector<unsigned char> Transaction::serialize() const
    {
        std::vector<unsigned char> buf;

        buf.insert(buf.end(), from.begin(), from.end());

        for (size_t i = 0; i < sizeof(Nonce); i++) {
            buf.push_back((nonce >> (i * sizeof(Nonce))) & 0xFF);
        }

        for (size_t i = 0; i < sizeof(size_t); i++) {
            buf.push_back((data.size() >> (i * sizeof(size_t)) & 0xFF));
        }
        buf.insert(buf.end(), data.begin(), data.end());

        buf.insert(buf.end(), signature.begin(), signature.end());

        return buf;
    }

    Transaction Transaction::deserialize(const std::vector<unsigned char> &data)
    {
        std::size_t cur = 0;

        PublicKey from;
        for (size_t i = 0; i < from.size(); i++) {
            if (cur >= data.size()) {
                throw DeserializationError("Overflow");
            }
            from[i] = data[cur++];
        }

        Nonce nonce = 0;
        for (size_t i = 0; i < sizeof(Nonce); i++) {
            if (cur >= data.size()) {
                throw DeserializationError("Overflow");
            }
            nonce |= data[cur++] << (i * sizeof(Nonce));
        }

        size_t data_size = 0;
        for (size_t i = 0; i < sizeof(size_t); i++) {
            if (cur >= data.size()) {
                throw DeserializationError("Overflow");
            }
            data_size |= data[cur++] << (i * sizeof(size_t));
        }

        std::vector<unsigned char> tx_data;
        tx_data.reserve(data_size);
        for (size_t i = 0; i < data_size; i++) {
            if (cur >= data.size()) {
                throw DeserializationError("Overflow");
            }
            tx_data.push_back(data[cur++]);
        }

        std::array<unsigned char, 64> signature;
        for (size_t i = 0; i < signature.size(); i++) {
            if (cur >= data.size()) {
                throw DeserializationError("Overflow");
            }
            signature[i] = data[cur++];
        }

        Transaction tx(from, nonce, tx_data);
        tx.signature = signature;

        return tx;
    }
}