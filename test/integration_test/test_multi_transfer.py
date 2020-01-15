from tester import NodeRunner, NodeId, Client, TEST_CHECK, test_case
import time


def check_test_received(log_line):
    if "Node received in {test}" in log_line:
        return True
    else:
        return False


def check_connect_asepted(log_line):
    if "Connection accepted:" in log_line:
        return True
    else:
        return False

def check_connection_established_builder(node_info):
    def check_connect_established(log_line):
        if f"Connection established: {node_info.connect_sync_address}" in log_line:
            return True
        else:
            return False
    return check_connect_established

def check_transaction_check_builder(from_address, to_address, amount):
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


@test_case("test_multi_transfer")
def main(node_exec_path, rpc_client_exec_path):

    node_id_1 = NodeId(sync_port=20206, rpc_port=50056)
    node_id_2 = NodeId(sync_port=20202, rpc_port=50053)

    client = Client(rpc_client_exec_path, "client")

    with NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_id_1), "node_1") as node_1:

        TEST_CHECK(client.run_check_test(host_id=node_id_1))
        TEST_CHECK(node_1.check(check_test_received))

        with NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_id_2, nodes_id_list=[node_id_1, ]), "node_2") as node_2:

            TEST_CHECK(client.run_check_test(host_id=node_id_1))
            TEST_CHECK(client.run_check_test(host_id=node_id_2))
            TEST_CHECK(node_2.check(check_test_received))

            TEST_CHECK(node_1.check(check_connect_asepted))
            TEST_CHECK(node_2.check(check_connection_established_builder(node_id_1)))

            target_address = "1" * 32

            TEST_CHECK(client.run_check_balance(address=target_address, host_id=node_id_2, target_balance=0))

            from_address = "0" * 32
            amount = 333
            transaction_wait = 2
            TEST_CHECK(client.run_check_transfer(from_address=from_address, host_id=node_id_2, to_address=target_address, amount=amount, wait=transaction_wait))

            TEST_CHECK(node_2.check(check_transaction_check_builder(from_address, target_address, amount)))
            TEST_CHECK(node_2.check(check_block_add))
            TEST_CHECK(client.run_check_balance(address=target_address, host_id=node_id_2, target_balance=amount))
            TEST_CHECK(node_1.check(check_block_add))
            TEST_CHECK(client.run_check_balance(address=target_address, host_id=node_id_1, target_balance=amount))

    return 0


@test_case("test_multi_transfer_connected_with_everything")
def main(node_exec_path, rpc_client_exec_path):

    start_sync_port = 20206
    start_rpc_port = 50056
    nodes_time = 3
    count_nodes = 5
    nodes_id = [NodeId(sync_port = start_sync_port, rpc_port = start_rpc_port, absolute_address = "127.0.0.1")]
    address = [str(i) * 32 for i in range(count_nodes)]

    client = Client(rpc_client_exec_path, "client")

    nodes = [NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id = nodes_id[0]), "node_1", start_up_time = nodes_time)]
    nodes[0].start()
    TEST_CHECK(client.run_check_test(host_id = nodes_id[0]))
    TEST_CHECK(nodes[0].check(check_test_received))

    for i in range(1, count_nodes):
        node_info = NodeId(sync_port=start_sync_port + i, rpc_port=start_rpc_port + i, absolute_address = "127.0.0." + str(i + 1))
        nodes.append(NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id = node_info,
         nodes_id_list=nodes_id), "node_" + str(i + 1), start_up_time = nodes_time))
        nodes[i].start()
        nodes_id.append(node_info)

        for j in range(i + 1):
            TEST_CHECK(client.run_check_test(host_id=nodes_id[j]))
        
        TEST_CHECK(nodes[i].check(check_test_received))
        TEST_CHECK(nodes[i - 1].check(check_connect_asepted))
        for j in range(i):
            TEST_CHECK(nodes[i].check(check_connection_established_builder(nodes_id[j])))

    first_amount = 1000
    for i in range(1, len(address)):
        from_address = address[0]
        to_address = address[i]
        
        for j in nodes_id:
            TEST_CHECK(client.run_check_balance(address=to_address, host_id=j, target_balance=0))

        transaction_wait = 4
        TEST_CHECK(client.run_check_transfer(from_address=from_address, host_id=nodes_id[0], to_address=to_address, amount=first_amount, wait=transaction_wait))
        
        for j in nodes_id:
            TEST_CHECK(client.run_check_balance(address=to_address, host_id=j, target_balance=first_amount))

    for i in range(1, len(address) - 1):
        from_address = address[i]
        to_address = address[i + 1]
        amount = i * 100
        transaction_wait = 4
        TEST_CHECK(client.run_check_transfer(from_address=from_address, host_id=nodes_id[i], to_address=to_address, amount=amount, wait=transaction_wait))

        for j in nodes_id:
            TEST_CHECK(client.run_check_balance(address=to_address, host_id=j, target_balance = first_amount + amount))
            TEST_CHECK(client.run_check_balance(address=from_address, host_id=j, target_balance = first_amount - amount  + (i - 1) * 100))


    for i in range(count_nodes):
        nodes[i].close()
    return 0


@test_case("test_multi_transfer_connected_one_by_one")
def main(node_exec_path, rpc_client_exec_path):

    start_sync_port = 20206
    start_rpc_port = 50056
    waiting_time = 4
    count_nodes = 5
    nodes_id = [NodeId(sync_port = start_sync_port, rpc_port = start_rpc_port, absolute_address = "127.0.0.1")]
    address = [str(i) * 32 for i in range(count_nodes)]

    client = Client(rpc_client_exec_path, "client")

    nodes = [NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id = nodes_id[0]), "node_1", start_up_time = waiting_time)]
    nodes[0].start()
    TEST_CHECK(client.run_check_test(host_id = nodes_id[0]))
    TEST_CHECK(nodes[0].check(check_test_received))

    for i in range(1, count_nodes):
        nodes_id.append(NodeId(sync_port=start_sync_port + i, rpc_port=start_rpc_port + i, absolute_address = "127.0.0." + str(i + 1)))
        nodes.append(NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id = nodes_id[i],
         nodes_id_list=[nodes_id[i - 1]]), "node_" + str(i + 1), start_up_time = waiting_time))
        nodes[i].start()

        for j in range(i + 1):
            TEST_CHECK(client.run_check_test(host_id=nodes_id[j]))
        
        TEST_CHECK(nodes[i].check(check_test_received))
        TEST_CHECK(nodes[i - 1].check(check_connect_asepted))
        TEST_CHECK(nodes[i].check(check_connection_established_builder(nodes_id[i - 1])))

    first_amount = 1000
    for i in range(1, len(address)):
        from_address = address[0]
        to_address = address[i]
        
        for j in nodes_id:
            TEST_CHECK(client.run_check_balance(address=to_address, host_id=j, target_balance=0))

        transaction_wait = 4
        TEST_CHECK(client.run_check_transfer(from_address=from_address, host_id=nodes_id[0], to_address=to_address, amount=first_amount, wait=transaction_wait))
        
        for j in nodes_id:
            TEST_CHECK(client.run_check_balance(address=to_address, host_id=j, target_balance=first_amount))

    for i in range(1, len(address) - 1):
        from_address = address[i]
        to_address = address[i + 1]
        amount = i * 100
        transaction_wait = 4
        TEST_CHECK(client.run_check_transfer(from_address=from_address, host_id=nodes_id[i], to_address=to_address, amount=amount, wait=transaction_wait))

        for j in nodes_id:
            TEST_CHECK(client.run_check_balance(address=to_address, host_id=j, target_balance = first_amount + amount))
            TEST_CHECK(client.run_check_balance(address=from_address, host_id=j, target_balance = first_amount - amount  + (i - 1) * 100))


    for i in range(count_nodes):
        nodes[i].close()
    return 0