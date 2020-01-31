from tester import Log, NodeRunner, NodeId, Client, TEST_CHECK, test_case
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


@test_case("test_multi_network")
def main(node_exec_path, rpc_client_exec_path):

    logger = Log("test.log")
    node_id_1 = NodeId(sync_port=20207, rpc_port=50057)
    node_id_2 = NodeId(sync_port=20208, rpc_port=50058)

    try:
        client = Client(rpc_client_exec_path, "client", logger=logger)

        with NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_id_1), "node_"+str(node_id_1.sync_port),
                        logger=logger) as node_1:

            TEST_CHECK(client.run_check_test(host_id=node_id_1))
            TEST_CHECK(node_1.check(check_test_received))

            with NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_id_2, nodes_id_list=[node_id_1, ]),
                            "node_"+str(node_id_1.sync_port), logger=logger) as node_2:

                TEST_CHECK(client.run_check_test(host_id=node_id_1))
                TEST_CHECK(client.run_check_test(host_id=node_id_2))
                TEST_CHECK(node_2.check(check_test_received))

                TEST_CHECK(node_1.check(check_connect_asepted))
                TEST_CHECK(node_2.check(
                    check_connection_established_builder(node_id_1)))
    except Exception as exs:
        print(exs)
        return 1

    return 0


def close_nodes(nodes):
    for node in nodes:
        node.close()


@test_case("test_multi_network_one_by_one")
def main(node_exec_path, rpc_client_exec_path):
    logger = Log("test.log")
    start_sync_port = 20209
    start_rpc_port = 50059
    waiting_time = 3
    count_nodes = 4
    nodes_id = [NodeId(sync_port=start_sync_port,
                       rpc_port=start_rpc_port, absolute_address="127.0.0.1")]

    try:
        client = Client(rpc_client_exec_path, "client", logger=logger)

        nodes = [NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=nodes_id[0]), "node_" + str(start_sync_port),
                            logger=logger, start_up_time=waiting_time)]
        nodes[0].start()
        TEST_CHECK(client.run_check_test(host_id=nodes_id[0]))
        TEST_CHECK(nodes[0].check(check_test_received))

        for i in range(1, count_nodes):
            nodes_id.append(NodeId(sync_port=start_sync_port + i,
                                   rpc_port=start_rpc_port + i, absolute_address="127.0.0.1"))
            nodes.append(NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=nodes_id[i],
                                                                               nodes_id_list=[nodes_id[i - 1]]), "node_" + str(start_sync_port + i), logger=logger, start_up_time=waiting_time))
            nodes[i].start()

            for j in range(i + 1):
                TEST_CHECK(client.run_check_test(host_id=nodes_id[j]))

            TEST_CHECK(nodes[i].check(check_test_received))
            TEST_CHECK(nodes[i - 1].check(check_connect_asepted))
            TEST_CHECK(nodes[i].check(
                check_connection_established_builder(nodes_id[i - 1])))
    except Exception as exs:
        print(exs)
        close_nodes(nodes)
        return 1
    close_nodes(nodes)
    return 0


@test_case("test_multi_network_with_everything")
def main(node_exec_path, rpc_client_exec_path):
    logger = Log("test.log")
    start_sync_port = 20215
    start_rpc_port = 50065
    waiting_time = 5
    count_nodes = 4
    nodes_id = [NodeId(sync_port=start_sync_port,
                       rpc_port=start_rpc_port, absolute_address="127.0.0.1")]

    try:
        client = Client(rpc_client_exec_path, "client", logger=logger)

        nodes = [NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=nodes_id[0]), "node_" + str(start_sync_port),
                            logger=logger, start_up_time=waiting_time)]
        nodes[0].start()
        TEST_CHECK(client.run_check_test(host_id=nodes_id[0]))
        TEST_CHECK(nodes[0].check(check_test_received))

        for i in range(1, count_nodes):
            node_info = NodeId(sync_port=start_sync_port + i,
                               rpc_port=start_rpc_port + i, absolute_address="127.0.0.1")
            nodes.append(NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_info,
                                                                               nodes_id_list=nodes_id), "node_" + str(i + start_sync_port), logger=logger, start_up_time=waiting_time))
            nodes[i].start()
            nodes_id.append(node_info)

            for j in range(i + 1):
                TEST_CHECK(client.run_check_test(host_id=nodes_id[j]))

            TEST_CHECK(nodes[i].check(check_test_received))
            TEST_CHECK(nodes[i - 1].check(check_connect_asepted))
            for j in range(i):
                TEST_CHECK(nodes[i].check(
                    check_connection_established_builder(nodes_id[j])))
    except Exception as exs:
        print(exs)
        close_nodes(nodes)
        return 1

    close_nodes(nodes)
    return 0


def create_node_name(node_id):
    return "node_" + str(node_id.rpc_port)


def init_nodes(node_exec_path, start_sync_port, start_rpc_port, nodes_id, first_node_id, waiting_time, logger):
    nodes = []
    for node_id in nodes_id:
        node = NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=node_id,
                                                                     nodes_id_list=[first_node_id]), create_node_name(node_id), logger=logger, start_up_time=waiting_time)
        node.start()
        nodes.append(node)
    return nodes


DISTRIBUTOR_ADDRESS = '0' * 32


def create_address_list(addresses_count):
    addresses = ['0' * (32 - len(str(i))) + str(i)
                 for i in range(1, addresses_count+1)]
    return addresses


@test_case("test_multi_network_parallel_stress_test")
def main(node_exec_path, rpc_client_exec_path):
    logger = Log("test.log")
    start_sync_port = 20230
    start_rpc_port = 50080
    waiting_time = 20
    count_threads = 10
    count_nodes_per_thread = 5
    amount = 1000
    transaction_wait = 10

    first_node_id = NodeId(sync_port=start_sync_port, rpc_port=start_rpc_port)

    node_ids = list()
    for i in range(1, count_threads * count_nodes_per_thread+1):
        node_ids.append(NodeId(sync_port=start_sync_port +
                               i, rpc_port=start_rpc_port + i))

    client = Client(rpc_client_exec_path, "client", logger=logger)
    first_node = NodeRunner(node_exec_path, NodeRunner.generate_config(current_node_id=first_node_id), create_node_name(first_node_id),
                            logger=logger, start_up_time=waiting_time)
    nodes = list()
    try:
        # start first node
        first_node.start()
        TEST_CHECK(client.run_check_test(host_id=first_node_id))
        TEST_CHECK(first_node.check(check_test_received))

        # initialize nodes
        with concurrent.futures.ThreadPoolExecutor(count_threads) as executor:
            threads = []
            for i in range(count_threads):
                threads.append(executor.submit(init_nodes, node_exec_path, start_sync_port, start_rpc_port,
                                               node_ids[count_nodes_per_thread*i: count_nodes_per_thread*(i+1)], first_node_id, waiting_time, logger))
            for i in threads:
                nodes += i.result()

         # map nodes by node_id
        if(len(nodes) != len(node_ids)):
            raise Exception("Logic error")
        nodes_map = dict()
        for i in range(len(nodes)):
            nodes_map[node_ids[i]] = nodes[i]

        # test for node initialization
        for i in node_ids:
            try:
                TEST_CHECK(client.run_check_test(host_id=i))
            except Exception as e:
                pid = nodes_map[i].pid
                print(f"Node in dead lock {pid}")
                raise Exception(e)

        addresses = create_address_list(len(nodes))

        for to_address, node_id in zip(addresses, nodes_map):
            for j in nodes_map:
                TEST_CHECK(client.run_check_balance(
                    address=to_address, host_id=j, target_balance=0))

            TEST_CHECK(client.run_check_transfer(from_address=DISTRIBUTOR_ADDRESS,
                                                 host_id=node_id, to_address=to_address, amount=amount, wait=transaction_wait))

            for node_id in nodes_map:
                try:
                    TEST_CHECK(client.run_check_balance(
                        address=to_address, host_id=node_id, target_balance=amount))
                except Exception as e:
                    pid = nodes_map[node_id].pid
                    print(f"Node in dead lock {pid}")
                    raise Exception(e)

    except Exception as exs:
        print(exs)
        for node in nodes:
            node.close()
        first_node.close()
        return 1

    for node in nodes:
        node.close()
    first_node.close()
    return 0
