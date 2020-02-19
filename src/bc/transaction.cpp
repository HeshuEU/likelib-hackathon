#include "base/error.hpp"

#include "transaction.hpp"

namespace bc
{

Sign::Sign(base::RsaPublicKey sender_public_key, base::Bytes rsa_encrypted_hash)
    : _data{Data{std::move(sender_public_key), std::move(rsa_encrypted_hash)}}
{}


bool Sign::isNull() const noexcept
{
    return !_data.has_value();
}


const base::RsaPublicKey& Sign::getPublicKey() const
{
    if(isNull()) {
        RAISE_ERROR(base::LogicError, "attemping to get on null bc::Sign");
    }
    return _data->sender_public_key;
}



const base::Bytes& Sign::getRsaEncryptedHash() const
{
    if(isNull()) {
        RAISE_ERROR(base::LogicError, "attemping to get on null bc::Sign");
    }
    return _data->rsa_encrypted_hash;
}


void Sign::serialize(base::SerializationOArchive& oa) const
{
    if(!_data) {
        RAISE_ERROR(base::LogicError, "cannot serialize empty bc::Sign object");
    }
    oa.serialize(_data->sender_public_key);
    oa.serialize(_data->rsa_encrypted_hash);
}


Sign Sign::deserialize(base::SerializationIArchive& ia)
{
    auto sender_rsa_public_key = base::RsaPublicKey::deserialize(ia);
    auto rsa_encrypted_hash = ia.deserialize<base::Bytes>();
    return Sign{std::move(sender_rsa_public_key), std::move(rsa_encrypted_hash)};
}


Transaction::Transaction(bc::Address from, bc::Address to, bc::Balance amount, base::Time timestamp, bc::Sign sign)
    : _from{std::move(from)}, _to{std::move(to)}, _amount{amount}, _timestamp{timestamp}, _sign{std::move(sign)}
{
    if(_amount == 0) {
        RAISE_ERROR(base::LogicError, "Transaction cannot contain amount equal to 0");
    }
}


const bc::Address& Transaction::getFrom() const noexcept
{
    return _from;
}


const bc::Address& Transaction::getTo() const noexcept
{
    return _to;
}


const bc::Balance& Transaction::getAmount() const noexcept
{
    return _amount;
}


const base::Time& Transaction::getTimestamp() const noexcept
{
    return _timestamp;
}



bool Transaction::operator==(const Transaction& other) const
{
    return _amount == other._amount && _from == other._from && _to == other._to && _timestamp == other._timestamp;
}


bool Transaction::operator!=(const Transaction& other) const
{
    return !(*this == other);
}


void Transaction::sign(base::RsaPublicKey pub, const base::RsaPrivateKey& priv)
{
    auto hash = hashOfTxData();
    base::Bytes rsa_encrypted_hash = priv.encrypt(hash.getBytes());
    _sign = Sign{std::move(pub), rsa_encrypted_hash};
}


bool Transaction::checkSign() const
{
    LOG_DEBUG << "checking signature";
    if(_sign.isNull()) {
        LOG_DEBUG << "invalid signature";
        return false;
    }
    else {
        const auto& pub = _sign.getPublicKey();
        const auto& enc_hash = _sign.getRsaEncryptedHash();
        auto derived_addr = bc::Address::fromPublicKey(pub);
        if(_from != derived_addr) {
            LOG_DEBUG << "invalid signature";
            return false;
        }
        auto valid_hash = hashOfTxData();
        if(pub.decrypt(enc_hash) == valid_hash.getBytes()) {
            LOG_DEBUG << "signature validated! valid hash = " << valid_hash
                      << " decrypted hash = " << pub.decrypt(enc_hash);
            return true;
        }
        LOG_DEBUG << "invalid signature";
        return false;
    }
}


const Sign& Transaction::getSign() const noexcept
{
    return _sign;
}


base::Sha256 Transaction::hashOfTxData() const
{
    base::SerializationOArchive oa;
    oa.serialize(_from);
    oa.serialize(_to);
    oa.serialize(_amount);
    oa.serialize(_timestamp);
    return base::Sha256::compute(std::move(oa).getBytes());
}


Transaction Transaction::deserialize(base::SerializationIArchive& ia)
{
    auto from = bc::Address::deserialize(ia);
    auto to = bc::Address::deserialize(ia);
    bc::Balance balance;
    ia >> balance;
    ::base::Time timestamp;
    ia >> timestamp;
    return {std::move(from), std::move(to), balance, timestamp};
}


void Transaction::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_from);
    oa.serialize(_to);
    oa.serialize(_amount);
    oa.serialize(_timestamp);
}


std::ostream& operator<<(std::ostream& os, const Transaction& tx)
{
    return os << "from: " << tx.getFrom() << " to: " << tx.getTo() << " amount: " << tx.getAmount()
              << " timestamp: " << tx.getTimestamp() << "signed: " << (tx.checkSign() ? "true" : "false");
}


void TransactionBuilder::setFrom(bc::Address from)
{
    _from = std::move(from);
}


void TransactionBuilder::setTo(bc::Address to)
{
    _to = std::move(to);
}


void TransactionBuilder::setAmount(bc::Balance amount)
{
    _amount = amount;
}


void TransactionBuilder::setTimestamp(base::Time timestamp)
{
    _timestamp = timestamp;
}


Transaction TransactionBuilder::build() const&
{
    ASSERT(_from);
    ASSERT(_to);
    ASSERT(_amount);
    ASSERT(_timestamp);
    return {*_from, *_to, *_amount, *_timestamp};
}


Transaction TransactionBuilder::build() &&
{
    ASSERT(_from);
    ASSERT(_to);
    ASSERT(_amount);
    ASSERT(_timestamp);
    return {std::move(*_from), std::move(*_to), std::move(*_amount), std::move(*_timestamp)};
}


} // namespace bc
