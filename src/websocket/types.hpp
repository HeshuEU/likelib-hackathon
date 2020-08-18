#pragma once

#include "base/hash.hpp"

namespace websocket
{

struct NodeInfo
{
    base::Sha256 top_block_hash;
    uint64_t top_block_number;
};

namespace Command
{

enum class Name : std::uint64_t
{
    LAST_BLOCK_INFO = 1,
    FIND_TRANSACTION = 2,
    FIND_TRANSACTION_STATUS = 4,
    PUSH_TRANSACTION = 8,
    FIND_BLOCK = 16,
    ACCOUNT_INFO = 32,
    RESERVED1 = 64,
    RESERVED2 = 128
};

enum class Type : std::uint64_t
{
    CALL = 256,
    SUBSCRIBE = 512,
    UNSUBSCRIBE = 1024,
    RESERVED = 2048
};

using Id = std::uint64_t;

constexpr Id NameMask = (static_cast<std::uint64_t>(Name::RESERVED2) * 2) - 1;            // 0000 0000 1111 1111
constexpr Id TypeMask = (static_cast<std::uint64_t>(Type::RESERVED) * 2) - NameMask - 1; // 0000 1111 0000 0000

constexpr Id CALL_LAST_BLOCK_INFO = websocket::Command::Id(websocket::Command::Type::CALL) |
                                    websocket::Command::Id(websocket::Command::Name::LAST_BLOCK_INFO);

constexpr Id CALL_ACCOUNT_INFO = websocket::Command::Id(websocket::Command::Type::CALL) |
                                 websocket::Command::Id(websocket::Command::Name::ACCOUNT_INFO);

constexpr Id CALL_FIND_TRANSACTION_STATUS = websocket::Command::Id(websocket::Command::Type::CALL) |
                                            websocket::Command::Id(websocket::Command::Name::FIND_TRANSACTION_STATUS);

constexpr Id CALL_FIND_TRANSACTION = websocket::Command::Id(websocket::Command::Type::CALL) |
                                            websocket::Command::Id(websocket::Command::Name::FIND_TRANSACTION);

constexpr Id CALL_FIND_BLOCK =
  websocket::Command::Id(websocket::Command::Type::CALL) | websocket::Command::Id(websocket::Command::Name::FIND_BLOCK);

constexpr Id SUBSCRIBE_PUSH_TRANSACTION = websocket::Command::Id(websocket::Command::Type::SUBSCRIBE) |
                                          websocket::Command::Id(websocket::Command::Name::PUSH_TRANSACTION);

constexpr Id SUBSCRIBE_LAST_BLOCK_INFO = websocket::Command::Id(websocket::Command::Type::SUBSCRIBE) |
                                         websocket::Command::Id(websocket::Command::Name::LAST_BLOCK_INFO);

constexpr Id SUBSCRIBE_ACCOUNT_INFO = websocket::Command::Id(websocket::Command::Type::SUBSCRIBE) |
                                      websocket::Command::Id(websocket::Command::Name::ACCOUNT_INFO);

constexpr Id UNSUBSCRIBE_PUSH_TRANSACTION = websocket::Command::Id(websocket::Command::Type::UNSUBSCRIBE) |
                                            websocket::Command::Id(websocket::Command::Name::PUSH_TRANSACTION);

constexpr Id UNSUBSCRIBE_LAST_BLOCK_INFO = websocket::Command::Id(websocket::Command::Type::UNSUBSCRIBE) |
                                           websocket::Command::Id(websocket::Command::Name::LAST_BLOCK_INFO);

constexpr Id UNSUBSCRIBE_ACCOUNT_INFO = websocket::Command::Id(websocket::Command::Type::UNSUBSCRIBE) |
                                        websocket::Command::Id(websocket::Command::Name::ACCOUNT_INFO);

}


using SessionId = std::uint64_t;

using QueryId = std::uint64_t;

}
