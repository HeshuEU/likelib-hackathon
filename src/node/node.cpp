#include "node.hpp"

#include <functional>

Node::Node(const base::PropertyTree& config)
    : _config{config}, _core{_config}
{
    auto service = std::make_shared<node::GeneralServerService>(_core);
    _rpc = std::make_unique<rpc::RpcServer>(_config.get<std::string>("rpc.address"), service);

    auto miner_callback = std::bind(&Node::onBlockMine, this, std::placeholders::_1);
    _miner = std::make_unique<Miner>(_config, miner_callback);
}


void Node::run()
{
    _core.run(); // run before all others

    _rpc->run();
    LOG_INFO << "RPC server started: " << _config.get<std::string>("rpc.address");
}


void Node::onBlockMine(bc::Block&& block)
{

}
