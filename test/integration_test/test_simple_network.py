from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL,\
                   ClientType, get_distributor_address_path, TransactionStatusCode
from time import sleep
import concurrent.futures

@test_case("two_nodes")
def main(env: Env) -> int:
    amount = 100
    wait = 1
    timeout = 2
    sync_time = 1
    node_1_id = Id(20101, grpc_port=50101, http_port=50102)
    node_2_id = Id(20102, grpc_port=50201, http_port=50202)
    env.start_node(NodeConfig(node_1_id))
    env.start_node(NodeConfig(node_2_id, nodes=[node_1_id]))
    client_1_grpc = env.get_client(ClientType.LEGACY_GRPC, node_1_id)
    client_1_http = env.get_client(ClientType.LEGACY_HTTP, node_1_id)
    client_2_grpc = env.get_client(ClientType.LEGACY_GRPC, node_2_id)
    client_2_http = env.get_client(ClientType.LEGACY_HTTP, node_2_id)
    TEST_CHECK(client_1_grpc.connection_test())
    TEST_CHECK(client_2_grpc.connection_test())
    TEST_CHECK(client_1_http.connection_test())
    TEST_CHECK(client_2_http.connection_test())
    env.logger.info(f"All clients checked")

    distributor_address = client_1_grpc.load_address(keys_path=get_distributor_address_path())
    test_address = client_1_grpc.generate_keys(keys_path="test_keys")

    env.logger.info(f"Start transaction 1")
    transaction = client_1_grpc.transfer(to_address=test_address.address, amount=amount, from_address=distributor_address,
                                         fee=0, wait=wait, timeout=timeout)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client_1_grpc.transaction_success_wait(transaction=transaction))

    env.logger.info(f"Start transaction 2")
    transaction = client_2_http.transfer(to_address=test_address.address, amount=amount, from_address=distributor_address,
                                         fee=0, wait=wait, timeout=timeout)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client_2_http.transaction_success_wait(transaction=transaction))

    sleep(sync_time)
    env.logger.info(f"Check balance on client 1")
    TEST_CHECK_EQUAL(client_1_grpc.get_balance(address=test_address.address, timeout=timeout, wait=wait), 2*amount)
    TEST_CHECK_EQUAL(client_1_http.get_balance(address=test_address.address, timeout=timeout, wait=wait), 2*amount)
    env.logger.info(f"Check balance on client 2")
    TEST_CHECK_EQUAL(client_2_grpc.get_balance(address=test_address.address, timeout=timeout, wait=wait), 2*amount)
    TEST_CHECK_EQUAL(client_2_http.get_balance(address=test_address.address, timeout=timeout, wait=wait), 2*amount)

    return 0

@test_case("connecting_to_yourself")
def main(env: Env) -> int:
    amount = 100
    node = Id(20101, grpc_port=50101)
    env.logger.info("Start node")
    env.start_node(NodeConfig(node, nodes=[node]))
    client = env.get_client(ClientType.LEGACY_GRPC, node)
    env.logger.info("Test node")
    TEST_CHECK(client.connection_test())

    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    test_address = client.generate_keys(keys_path="test_keys")
    env.logger.info("Start transaction")
    transaction = client.transfer(to_address=test_address.address, amount=amount, from_address=distributor_address,
                                         fee=0, wait=1, timeout=2)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=transaction))
    env.logger.info("Check balance")
    TEST_CHECK_EQUAL(client.get_balance(address=test_address.address, timeout=2, wait=1), amount)

    return 0

@test_case("all_to_one")
def main(env: Env) -> int:
    amount=100
    node_1_id = Id(20101, grpc_port=50101)
    node_2_id = Id(20102, grpc_port=50201)
    node_3_id = Id(20103, grpc_port=50301)
    env.start_node(NodeConfig(node_1_id))
    env.start_node(NodeConfig(node_2_id, nodes=[node_1_id]))
    env.start_node(NodeConfig(node_3_id, nodes=[node_1_id]))
    client_1_grpc = env.get_client(ClientType.LEGACY_GRPC, node_1_id)
    client_2_grpc = env.get_client(ClientType.LEGACY_GRPC, node_2_id)
    client_3_grpc = env.get_client(ClientType.LEGACY_GRPC, node_3_id)
    TEST_CHECK(client_1_grpc.connection_test())
    TEST_CHECK(client_2_grpc.connection_test())
    TEST_CHECK(client_3_grpc.connection_test())

    distributor_address = client_1_grpc.load_address(keys_path=get_distributor_address_path())
    test_address = client_1_grpc.generate_keys(keys_path="test_keys")
    transaction = client_1_grpc.transfer(to_address=test_address.address, amount=amount, from_address=distributor_address,
                                         fee=0, wait=1, timeout=2)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    stat = client_1_grpc.get_transaction_status(tx_hash=transaction.tx_hash)
    TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)

    TEST_CHECK_EQUAL(client_1_grpc.get_balance(address=test_address.address, timeout=2, wait=1), amount)
    TEST_CHECK_EQUAL(client_2_grpc.get_balance(address=test_address.address, timeout=2, wait=1), amount)
    TEST_CHECK_EQUAL(client_3_grpc.get_balance(address=test_address.address, timeout=2, wait=1), amount)

    return 0


@test_case("all_to_all")
def main(env: Env) -> int:
    amount=100
    sync_time = 1
    n=3
    node_ids=[]
    clients=[]
    for i in range(0,n):
      node_ids.append(Id(20100 + i, grpc_port = 50100 + i))
    for i in range(0,n):
      env.logger.info(f"Start node {i}")
      env.start_node(NodeConfig(node_ids[i], nodes=node_ids))
      clients.append(env.get_client(ClientType.LEGACY_GRPC, node_ids[i]))
      TEST_CHECK(clients[i].connection_test())

    env.logger.info("All nodes started")
    distributor_address = clients[0].load_address(keys_path=get_distributor_address_path())
    test_address = clients[0].generate_keys(keys_path="test_keys")
    env.logger.info("Start transaction")
    transaction = clients[0].transfer(to_address=test_address.address, amount=amount,
                                      from_address=distributor_address, fee=0, wait=1, timeout=2)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(clients[0].transaction_success_wait(transaction=transaction))
    env.logger.info("Transaction success")

    sleep(sync_time)
    for i in range(0,n):
      env.logger.info(f"Check balance (node {i})")
      TEST_CHECK_EQUAL(clients[i].get_balance(address=test_address.address, timeout=2, wait=1), amount)

    return 0


def init_nodes(env, initializing_node_ids, first_node_id):
  nodes = list()
  for node_id in initializing_node_ids:
    env.start_node(NodeConfig(node_id, nodes=[first_node_id, ]))
    nodes.append(env.get_client(ClientType.LEGACY_GRPC, node_id))
  return nodes


@test_case("multi_network_parallel_connection_stress_test")
def main(env: Env) -> int:
    start_sync_port = 20230
    start_rpc_port = 50080
    waiting_time = 20
    count_threads = 5
    count_nodes_per_thread = 5

    amount = 1000
    transaction_wait = 20
    transaction_timeout = 40

    node_ids = list()
    pool = list()

    for i in range(count_threads * count_nodes_per_thread + 1):
      node_ids.append(Id(start_sync_port + i, grpc_port = start_rpc_port + i))

    # start first node
    env.start_node(NodeConfig(node_ids[0]))
    first_node = env.get_client(ClientType.LEGACY_GRPC, node_ids[0])
    pool.append(first_node)
    TEST_CHECK(first_node.connection_test())

    # parallel initialize nodes
    with concurrent.futures.ThreadPoolExecutor(count_threads) as executor:
      threads = []
      for i in range(count_threads):
        threads.append(executor.submit(init_nodes, env,
                 node_ids[(count_nodes_per_thread * i) + 1: (count_nodes_per_thread * (
                 i + 1)) + 1], node_ids[0]))

    # test for node initialization
    for node in pool:
      TEST_CHECK(node.connection_test())
    addresses = [first_node.generate_keys(keys_path=f"keys{i}") for i in range(1, len(pool))]
    distributor_address = first_node.load_address(keys_path=get_distributor_address_path())

    for to_address, node in zip(addresses, pool):
      for other_node in pool:
        TEST_CHECK_EQUAL(other_node.get_balance(address=to_address.address, timeout=2, wait=1), 0)

      transaction = node.transfer(to_address=to_address.address, amount=amount,
                                      from_address=distributor_address, fee=0,
                                      wait=transaction_wait, timeout=transaction_timeout)

      for other_node in pool:
        TEST_CHECK_EQUAL(other_node.get_balance(address=to_address.address, timeout=2, wait=1), amount)

    return 0
