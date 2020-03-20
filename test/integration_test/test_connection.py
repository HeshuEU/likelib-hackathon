from tester import test_case, Node


@test_case("connection")
def main(logger, node_exec_path, rpc_client_exec_path, evm_path):
    node_settings = Node.Settings(
        node_exec_path, rpc_client_exec_path, evm_path, Node.Id(20205, 50055))
    with Node(node_settings, logger) as node:
        node.run_check_test()
    return 0
