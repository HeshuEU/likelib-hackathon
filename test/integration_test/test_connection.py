from tester import NodeRunner, NodeId, Client, TEST_CHECK, test_case


@test_case("test_connection")
def main(node_exec_path, rpc_client_exec_path):
    
    node_id = NodeId(sync_port=20205, rpc_port=50055)

    with NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_id), "node") as node:

        client = Client(rpc_client_exec_path, "client")

        TEST_CHECK(client.run_check_test(host_id=node_id))

    return 0
