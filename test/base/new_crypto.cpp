#include <boost/test/unit_test.hpp>

#include "base/asymmetric_crypto.hpp"
#include "base/symmetric_crypto.hpp"
#include "base/error.hpp"

#include <cstring>

namespace base
{

Bytes encryptMessage(const Bytes& message, const rsa::PrivateKey& key)
{
    base::aes::Key symmetric_key(base::aes::KeyType::Aes256BitKey);
    auto encrypted_message = symmetric_key.encrypt(message);
    auto serialized_symmetric_key = symmetric_key.toBytes();
    auto encrypted_serialized_symmetric_key = key.encrypt(serialized_symmetric_key);

    Bytes encrypted_serialized_key_size(sizeof(std::uint_least32_t));
    std::uint_least32_t key_size = encrypted_serialized_symmetric_key.size();
    std::memcpy(encrypted_serialized_key_size.toArray(), &key_size, encrypted_serialized_key_size.size());

    return encrypted_serialized_key_size.append(encrypted_serialized_symmetric_key).append(encrypted_message);
}

Bytes decryptMessage(const Bytes& message, const rsa::PublicKey& key)
{
    Bytes encrypted_serialized_key_size = message.takePart(0, sizeof(std::uint_least32_t));
    std::uint_least32_t key_size = 0;
    std::memcpy(&key_size, encrypted_serialized_key_size.toArray(), encrypted_serialized_key_size.size());

    auto encrypted_serialized_symmetric_key =
        message.takePart(sizeof(std::uint_least32_t), key_size + sizeof(std::uint_least32_t));
    auto encrypted_message = message.takePart(key_size + sizeof(std::uint_least32_t), message.size());

    auto serialized_symmetric_key = key.decrypt(encrypted_serialized_symmetric_key);
    base::aes::Key symmetric_key(serialized_symmetric_key);

    return symmetric_key.decrypt(encrypted_message);
}

} // namespace base

BOOST_AUTO_TEST_CASE(long_message_encrypt_decrypt_check)
{
    auto rsa_keys = base::rsa::generateKeys(2048);
    base::Bytes target_message{"fkldnvibbeucbsdfvcui"
                               "sadbfvuicaheiyfbvcbouvneg yb"
                               "gyncyegrvyuf gvrbuncreanfrcifnemdulhvyin35"
                               "gcmfa,oxmervhnutgy754tcnmx,"
                               "z5msnygtc5v7mtygs4nd2nftonvcmufgac ljvwbt"
                               "pygqin3cxg096mcgf5nsm2089ycpmfhv07n4m5v;kjfb jucgvjaf"
                               "jvbdubfdhbcufinag bcnrbxgmicnyvgb6nytgc53m8noy47mg5n4"
                               "rt83yfiudhg34btn3cby5cn8oxmqc3cnvbc34tn83toалротгшминтамт  мрауимамвыгми"
                               "оракумтпсьуквинтемьсч47ст7е27нч"};

    auto encrypted_message = base::encryptMessage(target_message, rsa_keys.second);
    auto decrypted_message = base::decryptMessage(encrypted_message, rsa_keys.first);

    BOOST_CHECK(decrypted_message.toString() == target_message.toString());
}