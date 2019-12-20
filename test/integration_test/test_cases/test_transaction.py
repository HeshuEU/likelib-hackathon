from .helper import *


def main(node_exec_path, rpc_client_exec_path):
    work_dir = os.path.join(os.path.dirname(
        os.path.abspath(__file__)), "test_transfer")

    node = NodeRunner(node_exec_path, os.path.join(work_dir, "node"))

    node_info = NodeId(sync_port=20206, rpc_port=50056)

    node.start(node.generate_config(current_node_info=node_info))

    client = Client(rpc_client_exec_path,
                        os.path.join(work_dir, "client"))

    TEST_CHECK(client.run_check_test(
            host_port=node_info.rpc_port, host_ip=node_info.absolute_address))

    target_address = "11111111111111111111111111111111"

    TEST_CHECK(client.run_check_balance(address=target_address, host_ip=node_info.absolute_address,
                                            host_port=node_info.rpc_port, target_balance=0))

    from_address = "00000000000000000000000000000000"
    amount = 333
    transaction_wait = 2
    TEST_CHECK(client.run_check_transfer(from_address=from_address, host_ip=node_info.absolute_address,
                                             to_address=target_address, amount=amount, host_port=node_info.rpc_port, wait=transaction_wait))

    TEST_CHECK(client.run_check_balance(address=target_address, host_ip=node_info.absolute_address,
                                            host_port=node_info.rpc_port, target_balance=amount))
    node.close()

    return 0
