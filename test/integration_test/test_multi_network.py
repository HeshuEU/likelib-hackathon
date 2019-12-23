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


@test_case("test_milti_network")
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
