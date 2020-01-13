from tester import NodeRunner, NodeId, Client, TEST_CHECK, test_case


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


@test_case("test_multi_network")
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

    return 0


@test_case("test_multi_network_one_by_one")
def main(node_exec_path, rpc_client_exec_path):

    start_sync_port = 20206
    start_rpc_port = 50056
    waiting_time = 3
    count_nodes = 4
    nodes_id = [NodeId(sync_port = start_sync_port, rpc_port = start_rpc_port, absolute_address = "127.0.0.1")]

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

    for i in range(count_nodes):
        nodes[i].close()
    return 0


@test_case("test_multi_network_with_everything")
def main(node_exec_path, rpc_client_exec_path):

    start_sync_port = 20206
    start_rpc_port = 50056
    nodes_time = 3
    count_nodes = 4
    nodes_id = [NodeId(sync_port = start_sync_port, rpc_port = start_rpc_port, absolute_address = "127.0.0.1")]

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

    for i in range(count_nodes):
        nodes[i].close()
    return 0
