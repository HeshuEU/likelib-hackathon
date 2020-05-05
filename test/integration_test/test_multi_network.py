from tester import test_case, Node, NodePoll
import concurrent.futures


@test_case("multi_network_connection_base")
def main(env, logger):
    with Node(env, Node.Settings(Node.Id(sync_port=20207, rpc_port=50057)), logger) as node_1:
        node_1.run_check_test()

        with Node(env, Node.Settings(Node.Id(sync_port=20207, rpc_port=50057), nodes=[node_1.settings.id, ]),
                  logger) as node_2:
            node_1.run_check_test()
            node_2.run_check_test()
    return 0


@test_case("multi_network_connection_one_by_one")
def main(env, logger):
    start_sync_port = 20209
    start_rpc_port = 50059
    waiting_time = 3
    count_nodes = 4

    with NodePoll() as pool:
        pool.append(Node(env, Node.Settings(Node.Id(start_sync_port, start_rpc_port)), logger))
        pool.last.start_node(waiting_time)
        pool.last.run_check_test()

        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(
                Node(env, Node.Settings(Node.Id(curent_sync_port, curent_rpc_port), nodes=[pool.last.settings.id, ]),
                     logger))

            pool.last.start_node(waiting_time)
            for node in pool:
                node.run_check_test()

    return 0


@test_case("multi_network_connection_with_everything")
def main(env, logger):
    start_sync_port = 20215
    start_rpc_port = 50065
    waiting_time = 5
    count_nodes = 4

    with NodePoll() as pool:
        pool.append(Node(env, Node.Settings(Node.Id(start_sync_port, start_rpc_port)), logger))
        pool.last.start_node(waiting_time)
        pool.last.run_check_test()

        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(Node(env, Node.Settings(Node.Id(curent_sync_port, curent_rpc_port), nodes=pool.ids), logger))
            pool.last.start_node(waiting_time)
            for node in pool:
                node.run_check_test()

    return 0


def init_nodes(env, initializing_node_ids, first_node_id, waiting_time, logger):
    nodes = list()
    for node_id in initializing_node_ids:
        node = Node(env, Node.Settings(node_id, nodes=[first_node_id, ]), logger)
        node.start_node(waiting_time)
        nodes.append(node)
    return nodes


@test_case("multi_network_parallel_connection_stress_test")
def main(env, logger):
    start_sync_port = 20230
    start_rpc_port = 50080
    waiting_time = 20
    count_threads = 5
    count_nodes_per_thread = 5

    amount = 1000
    transaction_wait = 20
    transaction_timeout = 40

    node_ids = list()
    for i in range(count_threads * count_nodes_per_thread + 1):
        node_ids.append(Node.Id(start_sync_port + i, start_rpc_port + i))

    with NodePoll() as pool:

        # start first node
        first_node = Node(env, Node.Settings(node_ids[0]), logger)
        pool.append(first_node)
        pool.last.start_node(2)
        pool.last.run_check_test()

        # parallel initialize nodes
        with concurrent.futures.ThreadPoolExecutor(count_threads) as executor:
            threads = []
            for i in range(count_threads):
                threads.append(executor.submit(init_nodes, env,
                                               node_ids[(count_nodes_per_thread * i) + 1: (count_nodes_per_thread * (
                                                       i + 1)) + 1], first_node.settings.id, waiting_time, logger))
            for i in threads:
                for node in i.result():
                    pool.append(node)

        # test for node initialization
        for node in pool:
            node.run_check_test()

        addresses = [first_node.create_new_address(keys_path=f"keys{i}") for i in range(1, len(pool))]
        distributor_address = first_node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)

        for to_address, node in zip(addresses, pool):
            for other_node in pool:
                other_node.run_check_balance(address=to_address, balance=0)

            node.run_check_transfer(to_address=to_address, amount=amount, from_address=distributor_address, fee=0,
                                    wait=transaction_wait, timeout=transaction_timeout)
            for other_node in pool:
                other_node.run_check_balance(address=to_address, balance=amount)

    return 0
