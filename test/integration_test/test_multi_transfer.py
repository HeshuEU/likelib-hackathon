from tester import Address, Log, test_case, NodeId, NodeTester, TEST_CHECK, NodePoll
import concurrent.futures


@test_case("multi_transfer")
def main(node_exec_path, rpc_client_exec_path):

    logger = Log("test.log")

    with NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=20300, rpc_port=50150), logger) as node_1:
        node_1.run_check_test()

        with NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=20301, rpc_port=50151), logger, nodes_id_list=[node_1.id, ]) as node_2:
            node_2.run_check_test()
            target_address = Address(node_1, "keys1")

            node_1.run_check_balance(address=target_address.address, target_balance=0)
            node_2.run_check_balance(address=target_address.address, target_balance=0)

            amount = 333
            transaction_wait = 4
            node_2.run_check_transfer(to_address=target_address.address, amount=amount, keys_path=node_1.DISTRIBUTOR_ADDRESS_PATH, fee=0, wait=transaction_wait)

            node_2.run_check_balance(address=target_address.address, target_balance=amount)
            node_1.run_check_balance(address=target_address.address, target_balance=amount)
        
    return 0


@test_case("multi_transfer_connected_with_everything")
def main(node_exec_path, rpc_client_exec_path):
    logger = Log("test.log")

    count_nodes = 10
    start_sync_port = 20302
    start_rpc_port = 50152
    waiting_time = 5
    transaction_wait = 5

    with NodePoll() as pool:
        pool.append(NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=start_sync_port, rpc_port=start_rpc_port), logger))
        pool.last.start_node(waiting_time)
        pool.last.run_check_test()

        # initializing connections with nodes
        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=curent_sync_port, rpc_port=curent_rpc_port), logger, nodes_id_list=pool.ids))
            pool.last.start_node(waiting_time)
            for node in pool:
                node.run_check_test()

        addresses = [Address(pool[0], f"keys{i}") for i in range(1, len(pool))]
        init_amount = 1000

        # init addresses with amount
        for to_address in addresses:
            pool.last.run_check_balance(address=to_address.address, target_balance=0)
            pool.last.run_check_transfer(to_address=to_address.address, amount=init_amount, keys_path=node.DISTRIBUTOR_ADDRESS_PATH, fee=0, wait=transaction_wait)
            for node in pool:
                node.run_check_balance(address=to_address.address, target_balance=init_amount)
        
        for i in range(1, len(addresses) - 1):
            from_address = addresses[i]
            to_address = addresses[i + 1]
            amount = i * 100
            pool.last.run_check_transfer(to_address=to_address.address, amount=amount, keys_path=from_address.key_path, fee=0, wait=transaction_wait)
            for node in pool:
                node.run_check_balance(address=to_address.address, target_balance=amount + init_amount)
        
        first_address = addresses[0]
        first_address_balance = init_amount
        for node in pool:
                node.run_check_balance(address=first_address.address, target_balance=first_address_balance)

    return 0


@test_case("multi_transfer_connected_one_by_one")
def main(node_exec_path, rpc_client_exec_path):
    logger = Log("test.log")

    count_nodes = 10
    start_sync_port = 20310
    start_rpc_port = 50160
    waiting_time = 5
    transaction_wait = 5

    with NodePoll() as pool:
        pool.append(NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=start_sync_port, rpc_port=start_rpc_port), logger))
        pool.last.start_node(waiting_time)
        pool.last.run_check_test()

        # initializing connections with nodes
        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=curent_sync_port, rpc_port=curent_rpc_port), logger, nodes_id_list=[pool.last.id, ]))
            pool.last.start_node(waiting_time)
            for node in pool:
                node.run_check_test()

        addresses = [Address(pool[0], f"keys{i}") for i in range(1, len(pool))]
        init_amount = 1000

        # init addresses with amount
        for to_address in addresses:
            pool.last.run_check_balance(address=to_address.address, target_balance=0)
            pool.last.run_check_transfer(to_address=to_address.address, amount=init_amount, keys_path=node.DISTRIBUTOR_ADDRESS_PATH, fee=0, wait=transaction_wait)
            for node in pool:
                node.run_check_balance(address=to_address.address, target_balance=init_amount)
        
        for i in range(1, len(addresses) - 1):
            from_address = addresses[i]
            to_address = addresses[i + 1]
            amount = i * 100
            pool.last.run_check_transfer(to_address=to_address.address, amount=amount, keys_path=from_address.key_path, fee=0, wait=transaction_wait)
            for node in pool:
                node.run_check_balance(address=to_address.address, target_balance=amount + init_amount)
        
        first_address = addresses[0]
        first_address_balance = init_amount
        for node in pool:
                node.run_check_balance(address=first_address.address, target_balance=first_address_balance)

    return 0


def node_transfers(node, addresses, transaction_wait):
    shift = len(addresses) - 1
    pos = 0
    from_address = addresses[pos]
    amount = 300
    for _ in range(len(addresses) * 5):
        pos = (pos + shift) % len(addresses)
        to_address = addresses[pos]
        node.run_check_transfer(to_address=to_address.address, amount=amount, keys_path=from_address.key_path, fee=0, wait=transaction_wait, timeout=3)
        from_address = to_address
    

@test_case("parallel_transfer_connected_with_everything")
def main(node_exec_path, rpc_client_exec_path):

    logger = Log("test.log")

    count_nodes = 7
    start_sync_port = 20330
    start_rpc_port = 50180
    node_startup_time = 5
    transaction_wait = 8

    init_amount = 1000
    address_per_nodes = 3

    with NodePoll() as pool:
        pool.append(NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=start_sync_port, rpc_port=start_rpc_port), logger))
        pool.last.start_node(transaction_wait)
        pool.last.run_check_test()

        # initializing connections with nodes
        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=curent_sync_port, rpc_port=curent_rpc_port), logger, nodes_id_list=pool.ids))
            pool.last.start_node(transaction_wait)
            for node in pool:
                node.run_check_test()

        addresses = [Address(pool[0], f"keys{i}") for i in range(1, count_nodes * address_per_nodes + 1)]

        # init addresses with amount
        for to_address in addresses:
            pool.last.run_check_balance(address=to_address.address, target_balance=0)
            pool.last.run_check_transfer(to_address=to_address.address, amount=init_amount, keys_path=node.DISTRIBUTOR_ADDRESS_PATH, fee=0, wait=transaction_wait)
            for node in pool:
                node.run_check_balance(address=to_address.address, target_balance=init_amount)
        
        with concurrent.futures.ThreadPoolExecutor(len(pool)) as executor:
            threads = []
            for i in range(len(pool)):
                first_address_number = i * address_per_nodes
                last_address_number = (i * address_per_nodes) + address_per_nodes
                threads.append(executor.submit(node_transfers, pool[i], addresses[first_address_number:last_address_number], transaction_wait))
            for i in threads:
                i.result()

        for address in addresses:
            for node in pool:
                node.run_check_balance(address=address.address, target_balance = init_amount)

    return 0
