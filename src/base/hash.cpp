#include "hash.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

#include <openssl/evp.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>

#include <stdint.h>
#include <string>

namespace base
{

Sha256::Sha256(const Bytes& data)
  : _bytes(data)
{
    if (_bytes.size() != LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Sha256::Sha256(Bytes&& data)
  : _bytes(data)
{
    if (_bytes.size() != LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Sha256::Sha256(const FixedBytes<Sha256::LENGTH>& data)
  : _bytes(data)
{}


Sha256::Sha256(FixedBytes<Sha256::LENGTH>&& data)
  : _bytes(data)
{}


std::string Sha256::toHex() const
{
    return base::toHex<FixedBytes<LENGTH>>(_bytes);
}


const base::FixedBytes<Sha256::LENGTH>& Sha256::getBytes() const noexcept
{
    return _bytes;
}


Sha256 Sha256::null()
{
    return Sha256(base::Bytes(LENGTH));
}


Sha256 Sha256::fromHex(const std::string& hex_view)
{
    return Sha256(base::fromHex<Bytes>(hex_view));
}


bool Sha256::operator==(const Sha256& another) const
{
    return getBytes() == another.getBytes();
}


bool Sha256::operator!=(const Sha256& another) const
{
    return getBytes() != another.getBytes();
}


bool Sha256::operator<(const Sha256& another) const
{
    return getBytes() < another.getBytes();
}


Sha256 Sha256::compute(const base::Bytes& data)
{
    base::FixedBytes<LENGTH> ret;
    SHA256(data.getData(), data.size(), ret.getData());
    return Sha256(ret);
}


void Sha256::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
}


Sha256 Sha256::deserialize(SerializationIArchive& ia)
{
    return Sha256(ia.deserialize<FixedBytes<LENGTH>>());
}


std::ostream& operator<<(std::ostream& os, const Sha256& sha)
{
    return os << toHex<FixedBytes<Sha256::LENGTH>>(sha.getBytes());
}

} // namespace base


std::size_t std::hash<base::Sha256>::operator()(const base::Sha256& k) const
{
    return std::hash<base::FixedBytes<base::Sha256::LENGTH>>{}(k.getBytes());
}


namespace base
{

Sha1::Sha1(const Bytes& data)
  : _bytes(data)
{
    if (_bytes.size() != LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Sha1::Sha1(Bytes&& data)
  : _bytes(data)
{
    if (_bytes.size() != LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Sha1::Sha1(const FixedBytes<Sha1::LENGTH>& data)
  : _bytes(data)
{}


Sha1::Sha1(FixedBytes<Sha1::LENGTH>&& data)
  : _bytes(data)
{}


std::string Sha1::toHex() const
{
    return base::toHex<FixedBytes<LENGTH>>(_bytes);
}


const base::FixedBytes<Sha1::LENGTH>& Sha1::getBytes() const noexcept
{
    return _bytes;
}


Sha1 Sha1::fromHex(const std::string_view& hex_view)
{
    return Sha1(base::fromHex<Bytes>(hex_view));
}


bool Sha1::operator==(const Sha1& another) const
{
    return getBytes() == another.getBytes();
}


bool Sha1::operator!=(const Sha1& another) const
{
    return getBytes() != another.getBytes();
}


Sha1 Sha1::compute(const base::Bytes& data)
{
    base::FixedBytes<LENGTH> ret;
    SHA1(data.getData(), data.size(), reinterpret_cast<unsigned char*>(ret.getData()));
    return Sha1(ret);
}


void Sha1::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
}


Sha1 Sha1::deserialize(SerializationIArchive& ia)
{
    return Sha1(ia.deserialize<base::FixedBytes<LENGTH>>());
}


std::ostream& operator<<(std::ostream& os, const Sha1& sha)
{
    return os << base::toHex<FixedBytes<Sha1::LENGTH>>(sha.getBytes());
}

} // namespace base


std::size_t std::hash<base::Sha1>::operator()(const base::Sha1& k) const
{
    return std::hash<base::FixedBytes<base::Sha1::LENGTH>>{}(k.getBytes());
}


namespace base
{

Ripemd160::Ripemd160(const Bytes& data)
  : _bytes(data)
{
    if (_bytes.size() != LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size for Ripemd160");
    }
}


Ripemd160::Ripemd160(Bytes&& data)
  : _bytes(data)
{
    if (_bytes.size() != LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size for Ripemd160");
    }
}


Ripemd160::Ripemd160(const FixedBytes<Ripemd160::LENGTH>& data)
  : _bytes(data)
{}


Ripemd160::Ripemd160(FixedBytes<Ripemd160::LENGTH>&& data)
  : _bytes(data)
{}


std::string Ripemd160::toHex() const
{
    return base::toHex<FixedBytes<Ripemd160::LENGTH>>(_bytes);
}


const base::FixedBytes<Ripemd160::LENGTH>& Ripemd160::getBytes() const noexcept
{
    return _bytes;
}


Ripemd160 Ripemd160::fromHex(const std::string& hex_view)
{
    return Ripemd160(base::fromHex<Bytes>(hex_view));
}


bool Ripemd160::operator==(const Ripemd160& another) const
{
    return getBytes() == another.getBytes();
}


bool Ripemd160::operator!=(const Ripemd160& another) const
{
    return getBytes() != another.getBytes();
}


Ripemd160 Ripemd160::compute(const base::Bytes& data)
{
    base::FixedBytes<LENGTH> ret;
    RIPEMD160_CTX context;
    if (1 != RIPEMD160_Init(&context)) {
        RAISE_ERROR(CryptoError, "failed to initialize context for Ripemd160");
    }

    if (1 != RIPEMD160_Update(&context, data.getData(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to hash data in Ripemd160");
    }

    if (1 != RIPEMD160_Final(ret.getData(), &context)) {
        RAISE_ERROR(CryptoError, "failed to hash data in Ripemd160");
    }
    return Ripemd160(ret);
}


void Ripemd160::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
}


Ripemd160 Ripemd160::deserialize(SerializationIArchive& ia)
{
    return Ripemd160(ia.deserialize<base::FixedBytes<LENGTH>>());
}


std::ostream& operator<<(std::ostream& os, const Ripemd160& ripemd)
{
    return os << ripemd.toHex();
}

} // namespace base


std::size_t std::hash<base::Ripemd160>::operator()(const base::Ripemd160& k) const
{
    return std::hash<base::FixedBytes<base::Ripemd160::LENGTH>>{}(k.getBytes());
}


namespace base
{
Sha3::Sha3(const Bytes& data)
  : _type(getSha3Type(data.size()))
  , _bytes(data)
{}


Sha3::Sha3(Bytes&& data)
  : _type(getSha3Type(data.size()))
  , _bytes(data)
{}


std::string Sha3::toHex() const
{
    return base::toHex<Bytes>(_bytes);
}


const base::Bytes& Sha3::getBytes() const noexcept
{
    return _bytes;
}


Sha3::Sha3Type Sha3::getType() const noexcept
{
    return _type;
}


std::size_t Sha3::size() const noexcept
{
    return _bytes.size();
}


Sha3 Sha3::fromHex(const std::string_view& hex_view)
{
    auto bytes = base::fromHex<Bytes>(hex_view);
    return Sha3(bytes);
}


bool Sha3::operator==(const Sha3& another) const
{
    return getBytes() == another.getBytes();
}


bool Sha3::operator!=(const Sha3& another) const
{
    return getBytes() != another.getBytes();
}


Sha3 Sha3::compute(const base::Bytes& data, Sha3::Sha3Type type)
{
    base::Bytes ret(static_cast<std::size_t>(type));
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(), EVP_MD_CTX_free);

    switch (type) {
        case (Sha3::Sha3Type::Sha3Type224):
            if (1 != EVP_DigestInit_ex(context.get(), EVP_sha3_224(), NULL)) {
                RAISE_ERROR(CryptoError, "failed to initialize context");
            }
            break;
        case (Sha3::Sha3Type::Sha3Type256):
            if (1 != EVP_DigestInit_ex(context.get(), EVP_sha3_256(), NULL)) {
                RAISE_ERROR(CryptoError, "failed to initialize context");
            }
            break;
        case (Sha3::Sha3Type::Sha3Type384):
            if (1 != EVP_DigestInit_ex(context.get(), EVP_sha3_384(), NULL)) {
                RAISE_ERROR(CryptoError, "failed to initialize context");
            }
            break;
        case (Sha3::Sha3Type::Sha3Type512):
            if (1 != EVP_DigestInit_ex(context.get(), EVP_sha3_512(), NULL)) {
                RAISE_ERROR(CryptoError, "failed to initialize context");
            }
            break;
        default:
            RAISE_ERROR(InvalidArgument, "Sha3 type is not valid");
    }

    if (1 != EVP_DigestUpdate(context.get(), data.getData(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to hash data");
    }

    std::unique_ptr<unsigned int> hash_length = std::make_unique<unsigned int>();
    if (1 != EVP_DigestFinal_ex(context.get(), ret.getData(), hash_length.get())) {
        RAISE_ERROR(CryptoError, "failed to hash data");
    }

    ASSERT(ret.size() == *hash_length);
    return Sha3(ret);
}


Sha3::Sha3Type Sha3::getSha3Type(const std::size_t size)
{
    switch (size) {
        case (static_cast<std::size_t>(Sha3::Sha3Type::Sha3Type224)):
            return Sha3::Sha3Type::Sha3Type224;
            break;
        case (static_cast<std::size_t>(Sha3::Sha3Type::Sha3Type256)):
            return Sha3::Sha3Type::Sha3Type256;
            break;
        case (static_cast<std::size_t>(Sha3::Sha3Type::Sha3Type384)):
            return Sha3::Sha3Type::Sha3Type384;
            break;
        case (static_cast<std::size_t>(Sha3::Sha3Type::Sha3Type512)):
            return Sha3::Sha3Type::Sha3Type512;
            break;
        default:
            RAISE_ERROR(InvalidArgument, "bytes size for Sha3 is not valid");
    }
}


void Sha3::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
}


Sha3 Sha3::deserialize(SerializationIArchive& ia)
{
    Bytes data = ia.deserialize<base::Bytes>();
    return Sha3(data);
}


std::ostream& operator<<(std::ostream& os, const Sha3& sha)
{
    return os << toHex<Bytes>(sha.getBytes());
}

} // namespace base


std::size_t std::hash<base::Sha3>::operator()(const base::Sha3& k) const
{
    return std::hash<base::Bytes>{}(k.getBytes());
}


namespace
{

#define BLOCK_SIZE ((1600 - 256 * 2) / 8)

#define I64(x) x##LL
#define ROTL64(qword, n) ((qword) << (n) ^ ((qword) >> (64 - (n))))
#define le2me_64(x) (x)
#define IS_ALIGNED_64(p) (0 == (7 & ((const char*)(p) - (const char*)0)))
#define me64_to_le_str(to, from, length) memcpy((to), (from), (length))

#define sha3_max_permutation_size 25
#define sha3_max_rate_in_qwords 24

#define TYPE_ROUND_INFO 0
#define TYPE_PI_TRANSFORM 24
#define TYPE_RHO_TRANSFORM 48

// clang-format off
const uint8_t constants[]  = {

        1, 26, 94, 112, 31, 33, 121, 85, 14, 12, 53, 38, 63, 79, 93, 83, 82, 72, 22, 102, 121, 88, 33, 116,

//const uint8_t pi_transform[] PROGMEM = {
        1, 6, 9, 22, 14, 20, 2, 12, 13, 19, 23, 15, 4, 24, 21, 8, 16, 5, 3, 18, 17, 11, 7, 10,

//const uint8_t rhoTransforms[] PROGMEM = {
        1, 62, 28, 27, 36, 44, 6, 55, 20, 3, 10, 43, 25, 39, 41, 45, 15, 21, 8, 18, 2, 61, 56, 14,
};
// clang-format on


uint8_t getConstant(uint8_t type, uint8_t index)
{
    return constants[type + index];
    // return pgm_read_byte(&constants[type + index]);
}


static uint64_t get_round_constant(uint8_t round)
{
    uint64_t result = 0;

    uint8_t roundInfo = getConstant(TYPE_ROUND_INFO, round);
    if (roundInfo & (1 << 6)) {
        result |= ((uint64_t)1 << 63);
    }
    if (roundInfo & (1 << 5)) {
        result |= ((uint64_t)1 << 31);
    }
    if (roundInfo & (1 << 4)) {
        result |= ((uint64_t)1 << 15);
    }
    if (roundInfo & (1 << 3)) {
        result |= ((uint64_t)1 << 7);
    }
    if (roundInfo & (1 << 2)) {
        result |= ((uint64_t)1 << 3);
    }
    if (roundInfo & (1 << 1)) {
        result |= ((uint64_t)1 << 1);
    }
    if (roundInfo & (1 << 0)) {
        result |= ((uint64_t)1 << 0);
    }

    return result;
}


typedef struct SHA3_CTX
{
    uint64_t hash[sha3_max_permutation_size];
    uint64_t message[sha3_max_rate_in_qwords];
    uint16_t rest;
} SHA3_CTX;


extern "C"
{
    void keccak_init(SHA3_CTX* ctx) { memset(ctx, 0, sizeof(SHA3_CTX)); }
    static void keccak_theta(uint64_t* A)
    {
        uint64_t C[5], D[5];

        for (uint8_t i = 0; i < 5; i++) {
            C[i] = A[i];
            for (uint8_t j = 5; j < 25; j += 5) {
                C[i] ^= A[i + j];
            }
        }

        for (uint8_t i = 0; i < 5; i++) {
            D[i] = ROTL64(C[(i + 1) % 5], 1) ^ C[(i + 4) % 5];
        }

        for (uint8_t i = 0; i < 5; i++) {
            for (uint8_t j = 0; j < 25; j += 5) {
                A[i + j] ^= D[i];
            }
        }
    }

    static void keccak_pi(uint64_t* A)
    {
        uint64_t A1 = A[1];
        for (uint8_t i = 1; i < 24; i++) {
            A[getConstant(TYPE_PI_TRANSFORM, i - 1)] = A[getConstant(TYPE_PI_TRANSFORM, i)];
        }
        A[10] = A1;
    }

    /*
    ketch uses 30084 bytes (93%) of program storage space. Maximum is 32256 bytes.
    Global variables use 743 bytes (36%) of dynamic memory, leaving 1305 bytes for local variables. Maximum is 2048
    bytes.
    */
    /* Keccak chi() transformation */
    static void keccak_chi(uint64_t* A)
    {
        for (uint8_t i = 0; i < 25; i += 5) {
            uint64_t A0 = A[0 + i], A1 = A[1 + i];
            A[0 + i] ^= ~A1 & A[2 + i];
            A[1 + i] ^= ~A[2 + i] & A[3 + i];
            A[2 + i] ^= ~A[3 + i] & A[4 + i];
            A[3 + i] ^= ~A[4 + i] & A0;
            A[4 + i] ^= ~A0 & A1;
        }
    }

    static void sha3_permutation(uint64_t* state)
    {
        for (uint8_t round = 0; round < 24; round++) {
            keccak_theta(state);

            for (uint8_t i = 1; i < 25; i++) {
                state[i] = ROTL64(state[i], getConstant(TYPE_RHO_TRANSFORM, i - 1));
            }

            keccak_pi(state);
            keccak_chi(state);

            *state ^= get_round_constant(round);
        }
    }

    /**
     * The core transformation. Process the specified block of data.
     *
     * @param hash the algorithm state
     * @param block the message block to process
     * @param block_size the size of the processed block in bytes
     */
    static void sha3_process_block(uint64_t hash[25], const uint64_t* block)
    {
        for (uint8_t i = 0; i < 17; i++) {
            hash[i] ^= le2me_64(block[i]);
        }

        /* make a permutation of the hash */
        sha3_permutation(hash);
    }

    //#define SHA3_FINALIZED 0x80000000
    //#define SHA3_FINALIZED 0x8000

    /**
     * Calculate message hash.
     * Can be called repeatedly with chunks of the message to be hashed.
     *
     * @param ctx the algorithm context containing current hashing state
     * @param msg message chunk
     * @param size length of the message chunk
     */
    void keccak_update(SHA3_CTX* ctx, const unsigned char* msg, uint16_t size)
    {
        uint16_t idx = (uint16_t)ctx->rest;

        ctx->rest = (unsigned)((ctx->rest + size) % BLOCK_SIZE);

        if (idx) {
            uint16_t left = BLOCK_SIZE - idx;
            memcpy((char*)ctx->message + idx, msg, (size < left ? size : left));
            if (size < left)
                return;

            sha3_process_block(ctx->hash, ctx->message);
            msg += left;
            size -= left;
        }

        while (size >= BLOCK_SIZE) {
            uint64_t* aligned_message_block;
            if (IS_ALIGNED_64(msg)) {
                aligned_message_block = (uint64_t*)(void*)msg;
            }
            else {
                memcpy(ctx->message, msg, BLOCK_SIZE);
                aligned_message_block = ctx->message;
            }

            sha3_process_block(ctx->hash, aligned_message_block);
            msg += BLOCK_SIZE;
            size -= BLOCK_SIZE;
        }

        if (size) {
            memcpy(ctx->message, msg, size);
        }
    }
    /**
     * Store calculated hash into the given array.
     *
     * @param ctx the algorithm context containing current hashing state
     * @param result calculated hash in binary form
     */
    void keccak_final(SHA3_CTX* ctx, unsigned char* result)
    {
        uint16_t digest_length = 100 - BLOCK_SIZE / 2;

        memset((char*)ctx->message + ctx->rest, 0, BLOCK_SIZE - ctx->rest);
        ((char*)ctx->message)[ctx->rest] |= 0x01;
        ((char*)ctx->message)[BLOCK_SIZE - 1] |= 0x80;

        sha3_process_block(ctx->hash, ctx->message);

        if (result) {
            me64_to_le_str(result, ctx->hash, digest_length);
        }
    }
}

}


namespace base
{

Keccak256::Keccak256(const Bytes& data)
  : _bytes(data)
{
    if (_bytes.size() != LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Keccak256::Keccak256(Bytes&& data)
  : _bytes(data)
{
    if (_bytes.size() != LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Keccak256::Keccak256(const FixedBytes<Keccak256::LENGTH>& data)
  : _bytes(data)
{}


Keccak256::Keccak256(FixedBytes<Keccak256::LENGTH>&& data)
  : _bytes(data)
{}


std::string Keccak256::toHex() const
{
    return base::toHex<FixedBytes<LENGTH>>(_bytes);
}


const base::FixedBytes<Keccak256::LENGTH>& Keccak256::getBytes() const noexcept
{
    return _bytes;
}


Keccak256 Keccak256::fromHex(const std::string_view& hex_view)
{
    return Keccak256(base::fromHex<Bytes>(hex_view));
}


bool Keccak256::operator==(const Keccak256& another) const
{
    return getBytes() == another.getBytes();
}


bool Keccak256::operator!=(const Keccak256& another) const
{
    return getBytes() != another.getBytes();
}


Keccak256 Keccak256::compute(const base::Bytes& data)
{
    base::FixedBytes<LENGTH> hash;
    SHA3_CTX ctx;
    keccak_init(&ctx);
    keccak_update(&ctx, data.getData(), data.size());
    keccak_final(&ctx, hash.getData());
    return hash;
}


void Keccak256::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
}


Keccak256 Keccak256::deserialize(SerializationIArchive& ia)
{
    return Keccak256(ia.deserialize<base::FixedBytes<LENGTH>>());
}


std::ostream& operator<<(std::ostream& os, const Keccak256& sha)
{
    return os << base::toHex<FixedBytes<Keccak256::LENGTH>>(sha.getBytes());
}

} // namespace base


std::size_t std::hash<base::Keccak256>::operator()(const base::Keccak256& k) const
{
    return std::hash<base::FixedBytes<base::Keccak256::LENGTH>>{}(k.getBytes());
}