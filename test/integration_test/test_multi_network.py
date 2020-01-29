from tester import NodeRunner, NodeId, Client, TEST_CHECK, test_case
import concurrent.futures


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

def close_nodes(nodes):
    for node in nodes:
        node.close()


@test_case("test_multi_network")
def main(node_exec_path, rpc_client_exec_path):

    node_id_1 = NodeId(sync_port=20206, rpc_port=50056)
    node_id_2 = NodeId(sync_port=20202, rpc_port=50053)

    try:
        client = Client(rpc_client_exec_path, "client")

        with NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_id_1), "node_"+str(node_id_1.sync_port)) as node_1:

            TEST_CHECK(client.run_check_test(host_id=node_id_1))
            TEST_CHECK(node_1.check(check_test_received))

            with NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_id_2, nodes_id_list=[node_id_1, ]), "node_"+str(node_id_1.sync_port)) as node_2:

                TEST_CHECK(client.run_check_test(host_id=node_id_1))
                TEST_CHECK(client.run_check_test(host_id=node_id_2))
                TEST_CHECK(node_2.check(check_test_received))

                TEST_CHECK(node_1.check(check_connect_asepted))
                TEST_CHECK(node_2.check(check_connection_established_builder(node_id_1)))
    except Exception as exs:
        print(exs)
        return 1
        
    return 0


@test_case("test_multi_network_one_by_one")
def main(node_exec_path, rpc_client_exec_path):

    start_sync_port = 20206
    start_rpc_port = 50056
    waiting_time = 3
    count_nodes = 4
    nodes_id = [NodeId(sync_port = start_sync_port, rpc_port = start_rpc_port, absolute_address = "127.0.0.1")]

    try:
        client = Client(rpc_client_exec_path, "client")

        nodes = [NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id = nodes_id[0]), "node_" + str(start_sync_port), start_up_time = waiting_time)]
        nodes[0].start()
        TEST_CHECK(client.run_check_test(host_id = nodes_id[0]))
        TEST_CHECK(nodes[0].check(check_test_received))

        for i in range(1, count_nodes):
            nodes_id.append(NodeId(sync_port=start_sync_port + i, rpc_port=start_rpc_port + i, absolute_address = "127.0.0.1"))
            nodes.append(NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id = nodes_id[i],
            nodes_id_list=[nodes_id[i - 1]]), "node_" + str(start_sync_port + i), start_up_time = waiting_time))
            nodes[i].start()

            for j in range(i + 1):
                TEST_CHECK(client.run_check_test(host_id=nodes_id[j]))
            
            TEST_CHECK(nodes[i].check(check_test_received))
            TEST_CHECK(nodes[i - 1].check(check_connect_asepted))
            TEST_CHECK(nodes[i].check(check_connection_established_builder(nodes_id[i - 1])))
    except Exception as exs:
        print(exs)
        close_nodes(nodes)
        return 1
    close_nodes(nodes)   
    return 0


@test_case("test_multi_network_with_everything")
def main(node_exec_path, rpc_client_exec_path):

    start_sync_port = 20206
    start_rpc_port = 50056
    waiting_time = 5
    count_nodes = 4
    nodes_id = [NodeId(sync_port = start_sync_port, rpc_port = start_rpc_port, absolute_address = "127.0.0.1")]

    try:
        client = Client(rpc_client_exec_path, "client")

        nodes = [NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id = nodes_id[0]), "node_" + str(start_sync_port), start_up_time = waiting_time)]
        nodes[0].start()
        TEST_CHECK(client.run_check_test(host_id = nodes_id[0]))
        TEST_CHECK(nodes[0].check(check_test_received))

        for i in range(1, count_nodes):
            node_info = NodeId(sync_port=start_sync_port + i, rpc_port=start_rpc_port + i, absolute_address = "127.0.0.1")
            nodes.append(NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id = node_info,
            nodes_id_list=nodes_id), "node_" + str(i + start_sync_port), start_up_time = waiting_time))
            nodes[i].start()
            nodes_id.append(node_info)

            for j in range(i + 1):
                TEST_CHECK(client.run_check_test(host_id=nodes_id[j]))
                
            TEST_CHECK(nodes[i].check(check_test_received))
            TEST_CHECK(nodes[i - 1].check(check_connect_asepted))
            for j in range(i):
                TEST_CHECK(nodes[i].check(check_connection_established_builder(nodes_id[j])))
    except Exception as exs:
        print(exs)
        close_nodes(nodes)
        return 1
        
    close_nodes(nodes)
    return 0


def init_nodes(node_exec_path, start_sync_port, start_rpc_port, nodes_id, first_node_id, waiting_time):
    nodes = []
    for node_id in nodes_id:
        node = NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id = node_id,
         nodes_id_list=[first_node_id]), "node_" + str(node_id.sync_port), start_up_time = waiting_time)
        node.start()
        nodes.append(node)
    return nodes


@test_case("test_multi_network_parallel_stress_test")
def main(node_exec_path, rpc_client_exec_path):

    start_sync_port = 20206
    start_rpc_port = 50056
    waiting_time = 20
    count_threads = 10
    count_nodes_per_thread = 5
    nodes_id = [NodeId(sync_port = start_sync_port, rpc_port = start_rpc_port, absolute_address = "127.0.0.1")]

    try:
        client = Client(rpc_client_exec_path, "client")

        nodes = [NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id = nodes_id[0]), "node_" + str(start_sync_port), start_up_time = waiting_time)]
        nodes[0].start()
        TEST_CHECK(client.run_check_test(host_id = nodes_id[0]))
        TEST_CHECK(nodes[0].check(check_test_received))

        for i in range(1, count_threads * count_nodes_per_thread+1):
            nodes_id.append(NodeId(sync_port=start_sync_port + i, rpc_port=start_rpc_port + i, absolute_address = "127.0.0.1"))

        with concurrent.futures.ThreadPoolExecutor(count_threads) as executor:
            threads = []
            for i in range(count_threads):
                threads.append(executor.submit(init_nodes, node_exec_path, start_sync_port, start_rpc_port, nodes_id[count_nodes_per_thread*i+1 : count_nodes_per_thread*(i+1)+1], nodes_id[0], waiting_time))
            for i in threads:
                nodes += i.result()

        addresses = ['0' * (32 - len(str(i))) + str(i) for i in range(len(nodes)+1)]
        amount = 1000
        transaction_wait = 8
        for i in range(1, len(addresses)):
            from_address = addresses[0]
            to_address = addresses[i]   
            for j in nodes_id:
                TEST_CHECK(client.run_check_balance(address=to_address, host_id=j, target_balance=0))
            
            TEST_CHECK(client.run_check_transfer(from_address=from_address, host_id=nodes_id[i-1], to_address=to_address, amount=amount, wait=transaction_wait)) 
            
            for j in nodes_id:
                TEST_CHECK(client.run_check_balance(address=to_address, host_id=j, target_balance=amount))
    except Exception as exs:
        print(exs)
        close_nodes(nodes)
        return 1

    close_nodes(nodes)
    return 0
