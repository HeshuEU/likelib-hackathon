from tester import test_case, Node, NodePoll
import concurrent.futures


@test_case("multi_transfer")
def main(env, logger):
    settings_node_1 = Node.Settings(Node.Id(20300, 50150))
    settings_node_2 = Node.Settings(Node.Id(20301, 50151), nodes=[settings_node_1.id, ])

    with Node(env, settings_node_1, logger) as node_1:
        node_1.run_check_test()

        with Node(env, settings_node_2, logger) as node_2:
            node_2.run_check_test()
            target_address = node_1.create_new_address(keys_path="keys1")

            node_1.run_check_balance(address=target_address, balance=0)
            node_2.run_check_balance(address=target_address, balance=0)

            distributor_address = node_1.load_address(keys_path=node_1.DISTRIBUTOR_ADDRESS_PATH)
            amount = 333
            transaction_wait = 5
            transaction_timeout = 3
            node_2.run_check_transfer(to_address=target_address, amount=amount,
                                      from_address=distributor_address, fee=0, timeout=transaction_timeout, wait=transaction_wait)

            node_2.run_check_balance(address=target_address, balance=amount)
            node_1.run_check_balance(address=target_address, balance=amount)

    return 0


@test_case("multi_transfer_connected_with_everything")
def main(env, logger):
    count_nodes = 10
    start_sync_port = 20302
    start_rpc_port = 50152
    waiting_time = 10
    transaction_timeout = 20
    transaction_wait = 10

    with NodePoll() as pool:
        pool.append(Node(env, Node.Settings(Node.Id(start_sync_port, start_rpc_port)), logger))
        pool.last.start_node(waiting_time)
        pool.last.run_check_test()

        # initializing connections with nodes
        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(
                Node(env, Node.Settings(Node.Id(curent_sync_port, curent_rpc_port), nodes=pool.ids),
                     logger))

            pool.last.start_node(waiting_time)
            for node in pool:
                node.run_check_test()

        addresses = [pool.last.create_new_address(keys_path=f"keys{i}") for i in range(1, len(pool))]
        init_amount = 1000

        distributor_address = pool.last.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)

        # init addresses with amount
        for to_address in addresses:
            pool.last.run_check_balance(address=to_address, balance=0)
            pool.last.run_check_transfer(to_address=to_address, amount=init_amount,
                                         from_address=distributor_address, fee=0, timeout=transaction_timeout, wait=transaction_wait)
            for node in pool:
                node.run_check_balance(address=to_address, balance=init_amount)

        for i in range(1, len(addresses) - 1):
            from_address = addresses[i]
            to_address = addresses[i + 1]
            amount = i * 100
            pool.last.run_check_transfer(to_address=to_address, amount=amount, from_address=from_address,
                                         fee=0, timeout=transaction_timeout, wait=transaction_wait)
            for node in pool:
                node.run_check_balance(address=to_address, balance=amount + init_amount)

        first_address = addresses[0]
        first_address_balance = init_amount
        for node in pool:
            node.run_check_balance(address=first_address, balance=first_address_balance)

    return 0


@test_case("multi_transfer_connected_one_by_one")
def main(env, logger):
    count_nodes = 10
    start_sync_port = 20310
    start_rpc_port = 50160
    waiting_time = 5
    transaction_timeout = 7
    transaction_wait = 4

    with NodePoll() as pool:
        pool.append(Node(env, Node.Settings(Node.Id(start_sync_port, start_rpc_port)), logger))
        pool.last.start_node(waiting_time)
        pool.last.run_check_test()

        # initializing connections with nodes
        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(
                Node(env, Node.Settings(Node.Id(curent_sync_port, curent_rpc_port), nodes=[pool.last.settings.id, ]),
                     logger))

            pool.last.start_node(waiting_time)
            for node in pool:
                node.run_check_test()

        addresses = [pool.last.create_new_address(keys_path=f"keys{i}") for i in range(1, len(pool))]
        init_amount = 1000
        distributor_address = pool.last.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)

        # init addresses with amount
        for to_address in addresses:
            pool.last.run_check_balance(address=to_address, balance=0)
            pool.last.run_check_transfer(to_address=to_address, amount=init_amount,
                                         from_address=distributor_address, fee=0, timeout=transaction_timeout,
                                         wait=transaction_wait)
            for node in pool:
                node.run_check_balance(address=to_address, balance=init_amount)

        for i in range(1, len(addresses) - 1):
            from_address = addresses[i]
            to_address = addresses[i + 1]
            amount = i * 100
            pool.last.run_check_transfer(to_address=to_address, amount=amount, from_address=from_address,
                                         fee=0, timeout=transaction_timeout,
                                         wait=transaction_wait)
            for node in pool:
                node.run_check_balance(address=to_address, balance=amount + init_amount)

        first_address = addresses[0]
        first_address_balance = init_amount
        for node in pool:
            node.run_check_balance(address=first_address, balance=first_address_balance)

    return 0


def node_transfers(node, addresses, transaction_wait):
    shift = len(addresses) - 1
    pos = 0
    from_address = addresses[pos]
    amount = 300
    transaction_timeout = 40
    for _ in range(len(addresses) * 5):
        pos = (pos + shift) % len(addresses)
        to_address = addresses[pos]
        node.run_check_transfer(to_address=to_address, amount=amount, from_address=from_address, fee=0,
                                timeout=transaction_timeout, wait=transaction_wait)
        from_address = to_address


@test_case("parallel_transfer_connected_with_everything")
def main(env, logger):
    count_nodes = 7
    start_sync_port = 20330
    start_rpc_port = 50180
    node_startup_time = 5
    transaction_wait = 10
    transaction_timeout = 42

    init_amount = 1000
    address_per_nodes = 3

    with NodePoll() as pool:
        pool.append(Node(env, Node.Settings(Node.Id(start_sync_port, start_rpc_port)), logger))
        pool.last.start_node(node_startup_time)
        pool.last.run_check_test()

        # initializing connections with nodes
        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(
                Node(env, Node.Settings(Node.Id(curent_sync_port, curent_rpc_port), nodes=pool.ids),
                     logger))

            pool.last.start_node(node_startup_time)
            for node in pool:
                node.run_check_test()

        addresses = [pool.last.create_new_address(keys_path=f"keys{i}") for i in
                     range(1, count_nodes * address_per_nodes + 1)]
        distributor_address = pool.last.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)

        # init addresses with amount
        for to_address in addresses:
            pool.last.run_check_balance(address=to_address, balance=0)
            pool.last.run_check_transfer(to_address=to_address, amount=init_amount,
                                         from_address=distributor_address, fee=0, timeout=transaction_timeout,
                                         wait=transaction_wait)
            for node in pool:
                node.run_check_balance(address=to_address, balance=init_amount)

        with concurrent.futures.ThreadPoolExecutor(len(pool)) as executor:
            threads = []
            for i in range(len(pool)):
                first_address_number = i * address_per_nodes
                last_address_number = (i * address_per_nodes) + address_per_nodes
                threads.append(
                    executor.submit(node_transfers, pool[i], addresses[first_address_number:last_address_number],
                                    transaction_wait))
            for i in threads:
                i.result()

        for address in addresses:
            for node in pool:
                node.run_check_balance(address=address, balance=init_amount)

    return 0
