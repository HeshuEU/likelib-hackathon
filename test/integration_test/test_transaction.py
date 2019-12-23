from tester import NodeRunner, NodeId, Client, TEST_CHECK, test_case


@test_case("test_transfer")
def main(node_exec_path, rpc_client_exec_path):

    node_id = NodeId(sync_port=20206, rpc_port=50056)

    with NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_id), "node") as node:

        client = Client(rpc_client_exec_path, "client")

        TEST_CHECK(client.run_check_test(host_id=node_id))

        target_address = "11111111111111111111111111111111"

        TEST_CHECK(client.run_check_balance(address=target_address, host_id=node_id, target_balance=0))

        from_address = "00000000000000000000000000000000"
        amount = 333
        transaction_wait = 2
        TEST_CHECK(client.run_check_transfer(from_address=from_address, host_id=node_id, to_address=target_address, amount=amount, wait=transaction_wait))

        TEST_CHECK(client.run_check_balance(address=target_address, host_id=node_id, target_balance=amount))

    return 0
