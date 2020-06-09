from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL,\
                   ClientType, get_distributor_address_path, TransactionStatusCode
from time import sleep
import concurrent.futures


@test_case("multi_transfer_connected_with_everything")
def main(env: Env) -> int:
    count_nodes = 10
    start_sync_port = 20302
    start_rpc_port = 50152
    waiting_time = 10
    transaction_timeout = 20
    transaction_wait = 10

    pool = []
    node_ids = []
    id = Id(start_sync_port, grpc_port = start_rpc_port)
    env.start_node(NodeConfig(id))
    node_ids.append(id)
    pool.append(env.get_client(ClientType.LEGACY_GRPC, Id(start_sync_port, grpc_port = start_rpc_port)))
    TEST_CHECK(pool[0].connection_test())


    # initializing connections with nodes
    for i in range(1, count_nodes):
      current_sync_port = start_sync_port + i
      current_rpc_port = start_rpc_port + i

      id = Id(current_sync_port, grpc_port = current_rpc_port)
      env.start_node(NodeConfig(id, nodes = node_ids))
      pool.append(env.get_client(ClientType.LEGACY_GRPC, id))
      node_ids.append(id)
      for node in pool:
        TEST_CHECK(node.connection_test())
    print("Init SUCCESS")
    addresses = [pool[-1].generate_keys(keys_path=f"keys{i}") for i in range(1, len(pool))]
    init_amount = 1000

    distributor_address = pool[-1].load_address(keys_path=get_distributor_address_path())

    # init addresses with amount
    for to_address in addresses:
      TEST_CHECK_EQUAL(pool[-1].get_balance(address=to_address.address, timeout=2, wait=1), 0)
      transaction = pool[-1].transfer(to_address=to_address.address, amount=init_amount,
                              from_address=distributor_address, fee=0, wait=0.2, timeout=2)
      TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
      #stat = pool[-1].get_transaction_status(tx_hash=transaction.tx_hash)
      #TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)

    sleep(3)
    for node in pool:
        TEST_CHECK_EQUAL(node.get_balance(address=to_address.address, timeout=2, wait=1),
                         init_amount)
    for i in range(1, len(addresses) - 1):
      from_address = addresses[i]
      to_address = addresses[i + 1]
      amount = i * 100
      transaction = pool[-1].transfer(to_address=to_address.address, amount=amount,
                                       from_address=from_address, fee=0,
                                       wait=transaction_wait, timeout=transaction_timeout)
      TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
      stat = pool[-1].get_transaction_status(tx_hash=transaction.tx_hash)
      TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)
      for node in pool:
        TEST_CHECK_EQUAL(node.get_balance(address=to_address.address,
                                  timeout=2, wait=1), amount + init_amount)
        first_address = addresses[0]
        first_address_balance = init_amount
        for node in pool:
            TEST_CHECK_EQUAL(node.get_balance(address=first_address.address,
                                  timeout=2, wait=1), first_address_balance)

    return 0

@test_case("multi_transfer_connected_one_by_one")
def main(env: Env) -> int:
    count_nodes = 10
    start_sync_port = 20310
    start_rpc_port = 50160
    waiting_time = 5
    transaction_timeout = 7
    transaction_wait = 4

    pool = []
    node_ids = []

    id = Id(start_sync_port, grpc_port = start_rpc_port)
    env.start_node(NodeConfig(id))
    node_ids.append(id)
    pool.append(env.get_client(ClientType.LEGACY_GRPC, id))
    TEST_CHECK(pool[0].connection_test())


    # initializing connections with nodes
    for i in range(1, count_nodes):
      current_sync_port = start_sync_port + i
      current_rpc_port = start_rpc_port + i
      last_id = id
      id = Id(current_sync_port, grpc_port = current_rpc_port)
      env.start_node(NodeConfig(id, nodes = [last_id, ]))
      node_ids.append(id)
      pool.append(env.get_client(ClientType.LEGACY_GRPC, id))

      for node in pool:
        TEST_CHECK(node.connection_test())

    addresses = [pool[-1].generate_keys(keys_path=f"keys{i}") for i in range(1, len(pool))]
    init_amount = 1000

    distributor_address = pool[-1].load_address(keys_path=get_distributor_address_path())

    # init addresses with amount
    for to_address in addresses:
      TEST_CHECK_EQUAL(pool[-1].get_balance(address=to_address.address, timeout=2, wait=1), 0)
      transaction = pool[-1].transfer(to_address=to_address.address, amount=init_amount,
           from_address=distributor_address, fee=0, wait=transaction_wait, timeout=transaction_timeout)
      TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
      stat = pool[-1].get_transaction_status(tx_hash=transaction.tx_hash)
      TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)

      for node in pool:
        TEST_CHECK_EQUAL(node.get_balance(address=to_address.address, timeout=2, wait=1),
                         init_amount)

    for i in range(1, len(addresses) - 1):
      from_address = addresses[i]
      to_address = addresses[i + 1]
      amount = i * 100

      transaction = pool[-1].transfer(to_address=to_address.address, amount=amount,
           from_address=from_address, fee=0, wait=transaction_wait, timeout=transaction_timeout)
      TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
      stat = pool[-1].get_transaction_status(tx_hash=transaction.tx_hash)
      TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)

      for node in pool:
        TEST_CHECK_EQUAL(node.get_balance(address=to_address.address, timeout=2, wait=1),
                         amount + init_amount)

    first_address = addresses[0]
    first_address_balance = init_amount
    for node in pool:
      TEST_CHECK_EQUAL(node.get_balance(address=first_address.address, timeout=2, wait=1),
                         first_address_balance)

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

        transaction = node.transfer(to_address=to_address.address, amount=amount,
           from_address=from_address, fee=0, wait=transaction_wait, timeout=transaction_timeout)
        TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
        stat = node.get_transaction_status(tx_hash=transaction.tx_hash)
        TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)

        from_address = to_address

# next
@test_case("parallel_transfer_connected_with_everything")
def main(env: Env) -> int:
    count_nodes = 7
    start_sync_port = 20330
    start_rpc_port = 50180
    node_startup_time = 5
    transaction_wait = 10
    transaction_timeout = 42

    init_amount = 1000
    address_per_nodes = 3


    pool = []
    node_ids = []

    id = Id(start_sync_port, grpc_port = start_rpc_port)
    env.start_node(NodeConfig(id))
    node_ids.append(id)
    pool.append(env.get_client(ClientType.LEGACY_GRPC, id))
    TEST_CHECK(pool[0].connection_test())

    # initializing connections with nodes
    for i in range(1, count_nodes):
      current_sync_port = start_sync_port + i
      current_rpc_port = start_rpc_port + i
      id = Id(current_sync_port, grpc_port = current_rpc_port)
      env.start_node(NodeConfig(id, nodes = node_ids))
      node_ids.append(id)
      pool.append(env.get_client(ClientType.LEGACY_GRPC, id))

      for node in pool:
        TEST_CHECK(node.connection_test())

    addresses = [pool[-1].generate_keys(keys_path=f"keys{i}") for i in range(1, count_nodes * address_per_nodes + 1)]
    distributor_address = pool[-1].load_address(keys_path=get_distributor_address_path())

    # init addresses with amount
    for to_address in addresses:
      TEST_CHECK_EQUAL(pool[-1].get_balance(address=to_address.address, timeout=2, wait=1), 0)
      transaction = pool[-1].transfer(to_address=to_address.address, amount=init_amount,
           from_address=distributor_address, fee=0, wait=transaction_wait, timeout=transaction_timeout)
      TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
      stat = pool[-1].get_transaction_status(tx_hash=transaction.tx_hash)
      TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)

      for node in pool:
        TEST_CHECK_EQUAL(node.get_balance(address=to_address.address, timeout=2, wait=1),
                         init_amount)

    with concurrent.futures.ThreadPoolExecutor(len(pool)) as executor:
      threads = []
      for i in range(len(pool)):
        first_address_number = i * address_per_nodes
        last_address_number = (i * address_per_nodes) + address_per_nodes
        threads.append(
                 executor.submit(node_transfers, pool[i],
                 addresses[first_address_number:last_address_number], transaction_wait))
      for i in threads:
        i.result()

    for address in addresses:
      for node in pool:
        TEST_CHECK_EQUAL(node.get_balance(address=address.address, timeout=2, wait=1),
                         init_amount)

    return 0
