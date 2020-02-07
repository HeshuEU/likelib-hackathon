from tester import Log, test_case, NodeId, NodeTester, TEST_CHECK


@test_case("base_transfer")
def main(node_exec_path, rpc_client_exec_path):

    logger = Log("test.log")
    with NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=20206, rpc_port=50056), logger) as node:
        node.run_check_test()

        target_address = "1" * 32
        node.run_check_balance(address=target_address, target_balance=0)

        amount = 333
        transaction_wait = 3
        node.run_check_transfer(from_address=NodeTester.DISTRIBUTOR_ADDRESS, to_address=target_address, 
        amount=amount, wait=transaction_wait)

        node.run_check_balance(address=target_address, target_balance=amount)
    return 0

