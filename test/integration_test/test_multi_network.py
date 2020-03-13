from tester import Address, Log, test_case, NodeId, NodeTester, TEST_CHECK, NodePoll
import concurrent.futures, time


@test_case("multi_network_connection_base")
def main(node_exec_path, rpc_client_exec_path, evm_exec_path):

    logger = Log("test.log")
    
    with NodeTester(node_exec_path, rpc_client_exec_path, evm_exec_path, NodeId(sync_port=20207, rpc_port=50057), logger) as node_1:
        node_1.run_check_test()

        with NodeTester(node_exec_path, rpc_client_exec_path, evm_exec_path, NodeId(sync_port=20208, rpc_port=50058), logger, nodes_id_list=[node_1.id, ]) as node_2:
                node_1.run_check_test()
                node_2.run_check_test() 
    return 0


@test_case("multi_network_connection_one_by_one")
def main(node_exec_path, rpc_client_exec_path, evm_exec_path):
    logger = Log("test.log")

    start_sync_port = 20209
    start_rpc_port = 50059
    waiting_time = 3
    count_nodes = 4

    with NodePoll() as pool:
        pool.append(NodeTester(node_exec_path, rpc_client_exec_path, evm_exec_path, NodeId(sync_port=start_sync_port, rpc_port=start_rpc_port), logger))
        pool.last.start_node(waiting_time)
        pool.last.run_check_test()

        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(NodeTester(node_exec_path, rpc_client_exec_path, evm_exec_path, NodeId(sync_port=curent_sync_port, rpc_port=curent_rpc_port), logger, nodes_id_list=[pool.last.id, ]))
            pool.last.start_node(waiting_time)
            for node in pool:
                node.run_check_test()

    return 0


@test_case("multi_network_connection_with_everything")
def main(node_exec_path, rpc_client_exec_path, evm_exec_path):
    logger = Log("test.log")

    start_sync_port = 20215
    start_rpc_port = 50065
    waiting_time = 5
    count_nodes = 4

    with NodePoll() as pool:
        pool.append(NodeTester(node_exec_path, rpc_client_exec_path, evm_exec_path, NodeId(sync_port=start_sync_port, rpc_port=start_rpc_port), logger))
        pool.last.start_node(waiting_time)
        pool.last.run_check_test()

        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(NodeTester(node_exec_path, rpc_client_exec_path, evm_exec_path, NodeId(sync_port=curent_sync_port, rpc_port=curent_rpc_port), logger, nodes_id_list=pool.ids))
            pool.last.start_node(waiting_time)
            for node in pool:
                node.run_check_test()

    return 0


def init_nodes(node_exec_path, rpc_client_exec_path, initializing_node_ids, first_node_id, waiting_time, logger):
    nodes = list()
    for node_id in initializing_node_ids:
        node = NodeTester(node_exec_path, rpc_client_exec_path, node_id, logger, nodes_id_list=[first_node_id, ])
        node.start_node(waiting_time)
        nodes.append(node)
    return nodes

@test_case("multi_network_parallel_connection_stress_test")
def main(node_exec_path, rpc_client_exec_path, evm_exec_path):
    logger = Log("test.log")

    start_sync_port = 20230
    start_rpc_port = 50080
    waiting_time = 10
    count_threads = 5
    count_nodes_per_thread = 5
    
    amount = 1000
    transaction_wait = 2

    node_ids = list()
    for i in range(count_threads * count_nodes_per_thread+1):
        node_ids.append(NodeId(sync_port=start_sync_port +i, rpc_port=start_rpc_port + i))

    with NodePoll() as pool:

        # start first node
        first_node = NodeTester(node_exec_path, rpc_client_exec_path, evm_exec_path, node_ids[0], logger)
        pool.append(first_node)
        pool.last.start_node(2)
        pool.last.run_check_test()

        # parallel initialize nodes
        with concurrent.futures.ThreadPoolExecutor(count_threads) as executor:
            threads = []
            for i in range(count_threads):
                threads.append(executor.submit(init_nodes, node_exec_path, rpc_client_exec_path, evm_exec_path, node_ids[(count_nodes_per_thread*i)+1: (count_nodes_per_thread*(i+1)) + 1], first_node.id, waiting_time, logger))
            for i in threads:
                for node in i.result():
                    pool.append(node)

        # test for node initialization
        for node in pool:
            node.run_check_test()
            
        addresses = [Address(pool[0], f"keys{i}") for i in range(1, len(pool))]

        for to_address, node in zip(addresses, pool):
            for other_node in pool:
                other_node.run_check_balance(address=to_address.address, target_balance=0)

            node.run_check_transfer(to_address=to_address.address, amount=amount, keys_path=node.DISTRIBUTOR_ADDRESS_PATH, fee=0, wait=transaction_wait)
            for other_node in pool:
                other_node.run_check_balance(address=to_address.address, target_balance=amount)

    return 0
