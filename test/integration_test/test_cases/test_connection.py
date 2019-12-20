from .helper import *


def main(node_exec_path, rpc_client_exec_path):
    work_dir = os.path.join(os.path.dirname(
        os.path.abspath(__file__)), "test_connection")

    node = NodeRunner(node_exec_path, os.path.join(work_dir, "node"))
    node_info = NodeId(sync_port=20205, rpc_port=50055)
    node.start(node.generate_config(current_node_info=node_info))

    client = Client(rpc_client_exec_path,
                    os.path.join(work_dir, "client"))
    TEST_CHECK(client.run_check_test(host_port=node_info.rpc_port,
                                     host_ip=node_info.absolute_address))
    node.close()

    return 0
