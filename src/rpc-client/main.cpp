#include <base/error.hpp>
#include "rpc/rpc.hpp"

#include "base/log.hpp"

int main()
{
    try {
        rpc::RpcClient client("localhost:50051");
        LOG_INFO << "Remote call of balance [" << client.balance("temp") << "]";
        LOG_INFO << "Remote call of transaction [" << client.transaction(1, "fake from address", "fake to address")
                 << "]";
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return -1;
    }
    return 0;
}