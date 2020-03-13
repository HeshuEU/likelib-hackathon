from tester import Log, test_case, NodeId, NodeTester, TEST_CHECK


@test_case("connection")
def main(node_exec_path, rpc_client_exec_path, evm_exec_path):
    logger = Log("test.log")
    with NodeTester(node_exec_path, rpc_client_exec_path, evm_exec_path, NodeId(sync_port=20205, rpc_port=50055), logger) as node:
        node.run_check_test()
    return 0
