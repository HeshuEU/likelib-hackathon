#include "node.hpp"

Node::Node(const base::PropertyTree& config)
    : _config{config}, _core{_config}, _miner{_config, [](bc::Block&& o) { /* FIXME */ }}
{}


void Node::run()
{
    /*
    auto service = std::make_shared<node::GeneralServerService>(node);
    rpc::RpcServer rpc(exe_config.get<std::string>("rpc.address"), service);
    rpc.run();
    LOG_INFO << "RPC server started: " << exe_config.get<std::string>("rpc.address");
    */

}
