from tester import NodeRunner, NodeId, Client, TEST_CHECK, test_case


def check_test_received(log_line):
    if "Node received in {test}" in log_line:
        return True
    else:
        return False

def transaction_check_builder(from_address, to_address, amount):
    def check_transaction_receives(log_line):
        if " Node received in {transaction}: from_address["+from_address+"], to_address["+to_address+"], amount["+str(amount)+"]" in log_line:
            return True
        else:
            return False
    return check_transaction_receives


def check_block_add(log_line):
    if "Adding block. Block hash" in log_line:
        return True
    else:
        return False


@test_case("test_transfer")
def main(node_exec_path, rpc_client_exec_path):

    node_id = NodeId(sync_port=20206, rpc_port=50056)

    with NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_id), "node") as node:

        client = Client(rpc_client_exec_path, "client")

        TEST_CHECK(client.run_check_test(host_id=node_id))
        TEST_CHECK(node.check(check_test_received))

        target_address = "11111111111111111111111111111111"

        TEST_CHECK(client.run_check_balance(address=target_address, host_id=node_id, target_balance=0))

        from_address = "00000000000000000000000000000000"
        amount = 333
        transaction_wait = 2
        TEST_CHECK(client.run_check_transfer(from_address=from_address, host_id=node_id, to_address=target_address, amount=amount, wait=transaction_wait))

        TEST_CHECK(node.check(transaction_check_builder(from_address, target_address, amount)))
        TEST_CHECK(node.check(check_block_add))
        TEST_CHECK(client.run_check_balance(address=target_address, host_id=node_id, target_balance=amount))

    return 0
