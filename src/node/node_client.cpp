#include "rpc/rpc.hpp"

#include "base/log.hpp"

int main() {

    auto client = rpc::RpcClient("localhost:50051");
    LOG_INFO << "Remote call of balance [" << client.balance("temp") << "]";
    LOG_INFO << "Remote call of transaction [" << client.transaction(1, "fake from address", "fake to address") << "]";

}