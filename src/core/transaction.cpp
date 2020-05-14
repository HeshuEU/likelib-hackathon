#include "transaction.hpp"

#include <base/error.hpp>


namespace lk
{


ContractData::ContractData(base::Bytes message, base::PropertyTree abi)
  : _message{ std::move(message) }
  , _abi{ std::move(abi) }
{}


void ContractData::setMessage(base::Bytes message)
{
    _message = std::move(message);
}


void ContractData::setAbi(base::PropertyTree abi)
{
    _abi = std::move(abi);
}


const base::Bytes& ContractData::getMessage() const noexcept
{
    return _message;
}


const base::PropertyTree& ContractData::getAbi() const noexcept
{
    return _abi;
}


void ContractData::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_message);
    oa.serialize(base::Bytes(_abi.toString()));
}


ContractData ContractData::deserialize(base::SerializationIArchive& ia)
{
    auto message = ia.deserialize<base::Bytes>();
    auto abi = base::parseJson(ia.deserialize<base::Bytes>().toString());
    return { std::move(message), std::move(abi) };
}

//
// Sign::Sign(base::RsaPublicKey sender_public_key, base::Bytes rsa_encrypted_hash)
//  : _data{ Data{ std::move(sender_public_key), std::move(rsa_encrypted_hash) } }
//{}
//
//
// bool Sign::isNull() const noexcept
//{
//    return !_data.has_value();
//}
//
//
// const base::RsaPublicKey& Sign::getPublicKey() const
//{
//    if (isNull()) {
//        RAISE_ERROR(base::LogicError, "attemping to get on null lk::Sign");
//    }
//    return _data->sender_public_key;
//}
//
//
// const base::Bytes& Sign::getRsaEncryptedHash() const
//{
//    if (isNull()) {
//        RAISE_ERROR(base::LogicError, "attemping to get on null lk::Sign");
//    }
//    return _data->rsa_encrypted_hash;
//}
//
//
// void Sign::serialize(base::SerializationOArchive& oa) const
//{
//    if (!_data) {
//        oa.serialize(base::Byte{ false });
//    }
//    else {
//        oa.serialize(base::Byte{ true });
//        oa.serialize(_data->sender_public_key);
//        oa.serialize(_data->rsa_encrypted_hash);
//    }
//}
//
//
// Sign Sign::deserialize(base::SerializationIArchive& ia)
//{
//    auto flag = ia.deserialize<base::Byte>();
//    if (flag) {
//        auto sender_rsa_public_key = base::RsaPublicKey::deserialize(ia);
//        auto rsa_encrypted_hash = ia.deserialize<base::Bytes>();
//        return Sign{ std::move(sender_rsa_public_key), std::move(rsa_encrypted_hash) };
//    }
//    else {
//        return Sign{};
//    }
//}
//
//
// Sign Sign::fromBase64(const std::string& base64_signature)
//{
//    const auto& signature_bytes = base::base64Decode(base64_signature);
//    base::SerializationIArchive ia(signature_bytes);
//    return deserialize(ia);
//}
//
//
// std::string Sign::toBase64() const
//{
//    base::SerializationOArchive oa;
//    serialize(oa);
//    return base::base64Encode(oa.getBytes());
//}


Transaction::Transaction(lk::Address from,
                         lk::Address to,
                         lk::Balance amount,
                         std::uint64_t fee,
                         base::Time timestamp,
                         base::Bytes data,
                         lk::Sign sign)
  : _from{ std::move(from) }
  , _to{ std::move(to) }
  , _amount{ amount }
  , _fee{ fee }
  , _timestamp{ timestamp }
  , _data{ std::move(data) }
  , _sign{ std::move(sign) }
{
    if ((_amount == 0) && (_fee == 0)) {
        RAISE_ERROR(base::LogicError, "Transaction cannot contain amount equal to 0");
    }
}


const lk::Address& Transaction::getFrom() const noexcept
{
    return _from;
}


const lk::Address& Transaction::getTo() const noexcept
{
    return _to;
}


const lk::Balance& Transaction::getAmount() const noexcept
{
    return _amount;
}


const base::Time& Transaction::getTimestamp() const noexcept
{
    return _timestamp;
}


const std::uint64_t& Transaction::getFee() const noexcept
{
    return _fee;
}


const base::Bytes& Transaction::getData() const noexcept
{
    return _data;
}


void Transaction::sign(const base::Secp256PrivateKey& key)
{
    auto hash = hashOfTransaction();
    _sign = key.sign(hash.getBytes());
}


bool Transaction::checkSign() const
{
    if (_sign.toBytes().isEmpty()) {
        return false;
    }
    else {
        auto valid_hash = hashOfTransaction();
        try {
            auto pub = base::Secp256PrivateKey::decodeSignatureToPublicKey(_sign, valid_hash.getBytes());
            auto derived_addr = lk::Address(pub);
            return _from == derived_addr;
        }
        catch (const base::CryptoError& ex) {
            return false;
        }
    }
}


const lk::Sign& Transaction::getSign() const noexcept
{
    return _sign;
}


bool Transaction::operator==(const Transaction& other) const
{
    return _amount == other._amount && _from == other._from && _to == other._to && _timestamp == other._timestamp &&
           _fee == other._fee && _data == other._data;
}


bool Transaction::operator!=(const Transaction& other) const
{
    return !(*this == other);
}


base::Sha256 Transaction::hashOfTransaction() const
{
    base::SerializationOArchive oa;
    serializeHeader(oa);
    // see http_specification.md
    auto from_address_str = base::base58Encode(_from.getBytes());
    auto to_address_str = base::base58Encode(_to.getBytes());
    auto amount_str = _amount.toString();
    auto fee_str = std::to_string(_fee);
    auto timestamp_str = std::to_string(_timestamp.getSecondsSinceEpoch());
    auto data_str = _data.toString();

    auto contactanated_data = from_address_str + to_address_str + amount_str + fee_str + timestamp_str + data_str;

    return base::Sha256::compute(base::Bytes(contactanated_data));
}


void Transaction::serializeHeader(base::SerializationOArchive& oa) const
{
    oa.serialize(_from);
    oa.serialize(_to);
    oa.serialize(_amount);
    oa.serialize(_fee);
    oa.serialize(_timestamp);
    oa.serialize(_data);
}


Transaction Transaction::deserialize(base::SerializationIArchive& ia)
{
    auto from = ia.deserialize<lk::Address>();
    auto to = ia.deserialize<lk::Address>();
    auto amount = ia.deserialize<lk::Balance>();
    auto fee = ia.deserialize<std::uint64_t>();
    auto timestamp = ia.deserialize<base::Time>();
    auto data = ia.deserialize<base::Bytes>();
    auto sign = ia.deserialize<lk::Sign>();
    return { std::move(from), std::move(to), amount, fee, timestamp, std::move(data), std::move(sign) };
}


void Transaction::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_from);
    oa.serialize(_to);
    oa.serialize(_amount);
    oa.serialize(_fee);
    oa.serialize(_timestamp);
    oa.serialize(_data);
    oa.serialize(_sign);
}


std::ostream& operator<<(std::ostream& os, const Transaction& tx)
{
    return os << "from: " << tx.getFrom() << " to: " << tx.getTo() << " amount: " << tx.getAmount()
              << " fee: " << tx.getFee() << " timestamp: " << tx.getTimestamp();
}


void TransactionBuilder::setFrom(lk::Address from)
{
    _from = std::move(from);
}


void TransactionBuilder::setTo(lk::Address to)
{
    _to = std::move(to);
}


void TransactionBuilder::setAmount(lk::Balance amount)
{
    _amount = amount;
}


void TransactionBuilder::setTimestamp(base::Time timestamp)
{
    _timestamp = timestamp;
}


void TransactionBuilder::setFee(std::uint64_t fee)
{
    _fee = fee;
}


void TransactionBuilder::setSign(lk::Sign sign)
{
    _sign = std::move(sign);
}


void TransactionBuilder::setData(base::Bytes data)
{
    _data = std::move(data);
}


Transaction TransactionBuilder::build() const&
{
    ASSERT(_from);
    ASSERT(_to);
    ASSERT(_amount);
    ASSERT(_fee);
    ASSERT(_timestamp);
    ASSERT(_data);

    if (_sign) {
        return { *_from, *_to, *_amount, *_fee, *_timestamp, *_data, *_sign };
    }
    else {
        return { *_from, *_to, *_amount, *_fee, *_timestamp, *_data };
    }
}


Transaction TransactionBuilder::build() &&
{
    ASSERT(_from);
    ASSERT(_to);
    ASSERT(_amount);
    ASSERT(_fee);
    ASSERT(_timestamp);
    ASSERT(_data);

    if (_sign) {
        return { std::move(*_from),      std::move(*_to),   std::move(*_amount), std::move(*_fee),
                 std::move(*_timestamp), std::move(*_data), std::move(*_sign) };
    }
    else {
        return { std::move(*_from), std::move(*_to),        std::move(*_amount),
                 std::move(*_fee),  std::move(*_timestamp), std::move(*_data) };
    }
}


const Transaction& invalidTransaction()
{
    static const Transaction invalid_tx{ lk::Address::null(), lk::Address::null(), 0, 1, base::Time(), {} };
    return invalid_tx;
}


TransactionStatus::TransactionStatus(StatusCode status,
                                     ActionType type,
                                     std::uint64_t fee_left,
                                     const std::string& message) noexcept
  : _status{ status }
  , _action(type)
  , _message{ message }
  , _fee_left{ fee_left }
{}


TransactionStatus::operator bool() const noexcept
{
    return _status == TransactionStatus::StatusCode::Success;
}


bool TransactionStatus::operator!() const noexcept
{
    return _status != TransactionStatus::StatusCode::Success;
}


const std::string& TransactionStatus::getMessage() const noexcept
{
    return _message;
}

std::string& TransactionStatus::getMessage() noexcept
{
    return _message;
}


TransactionStatus::StatusCode TransactionStatus::getStatus() const noexcept
{
    return _status;
}


TransactionStatus::ActionType TransactionStatus::getType() const noexcept
{
    return _action;
}

std::uint64_t TransactionStatus::getFeeLeft() const noexcept
{
    return _fee_left;
}

} // namespace lk
