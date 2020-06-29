from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL,\
                   ClientType, get_distributor_address_path, TransactionStatusCode
from time import sleep
import concurrent.futures
from random import randrange


@test_case("multi_transfer_connected_with_everything")
def main(env: Env) -> int:
    init_amount = 1000
    count_nodes = 10
    start_sync_port = 20302
    start_rpc_port = 50152
    waiting_time = 0.5
    timeout = 2
    transaction_wait = 1
    transaction_update_time = 1
    max_update_request = 10
    sync_time = 3

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
    addresses = [pool[-1].generate_keys(keys_path=f"keys{i}") for i in range(1, len(pool))]
    distributor_address = pool[-1].load_address(keys_path=get_distributor_address_path())

    init_transactions = []
    # init addresses with amount
    for to_address in addresses:
      TEST_CHECK_EQUAL(pool[-1].get_balance(address=to_address.address, timeout=timeout, wait=waiting_time), 0)
      transaction = pool[-1].transfer(to_address=to_address.address, amount=init_amount,
                        from_address=distributor_address, fee=0, wait=transaction_wait, timeout=timeout)
      TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
      init_transactions.append(transaction)

    for transaction in init_transactions:
      TEST_CHECK(pool[-1].transaction_success_wait(transaction=transaction))
    env.logger.info("Init transactions success. Wait synchronization")
    sleep(sync_time)
    for node in pool:
      TEST_CHECK_EQUAL(node.get_balance(address=to_address.address, timeout=timeout, wait=waiting_time),
                         init_amount)
    env.logger.info("Init check balance success.")
    for i in range(1, len(addresses) - 1):
      from_address = addresses[i]
      to_address = addresses[i + 1]
      amount = i * 100
      transaction = pool[-1].transfer(to_address=to_address.address, amount=amount,
                                       from_address=from_address, fee=0,
                                       wait=transaction_wait, timeout=timeout)
      TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
      TEST_CHECK(pool[-1].transaction_success_wait(transaction=transaction))
      env.logger.info(f"Transaction success, check balance {to_address.address}")

      for node in pool:
        TEST_CHECK_EQUAL(node.get_balance(address=to_address.address,
                                  timeout=timeout, wait=waiting_time), amount + init_amount)
      first_address = addresses[0]
      first_address_balance = init_amount
      env.logger.info(f"Check balance of first address {first_address.address}")
      for node in pool:
        TEST_CHECK_EQUAL(node.get_balance(address=first_address.address,
                                  timeout=timeout, wait=waiting_time), first_address_balance)

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

def node_transfers(node, addresses, transaction_wait, env):
    shift = len(addresses) - 1
    pos = 0
    from_address = addresses[pos]
    amount = 300
    transaction_timeout = 40
    env.logger.info(f"Node transfers start")
    for _ in range(len(addresses) * 5):
        pos = (pos + shift) % len(addresses)
        to_address = addresses[pos]

        transaction = node.transfer(to_address=to_address.address, amount=amount,
           from_address=from_address, fee=0, wait=transaction_wait, timeout=transaction_timeout)
        TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
        TEST_CHECK(node.transaction_success_wait(transaction=transaction))

        from_address = to_address

@test_case("parallel_transfer_connected_with_everything")
def main(env: Env) -> int:
    count_nodes = 5
    start_sync_port = 20330
    start_rpc_port = 50180
    node_startup_time = 5
    transaction_wait = 2
    transaction_timeout = 10

    init_amount = 1000
    address_per_nodes = 3


    pool = []
    node_ids = []

    id = Id(start_sync_port, grpc_port = start_rpc_port)
    env.start_node(NodeConfig(id))
    node_ids.append(id)
    pool.append(env.get_client(ClientType.LEGACY_GRPC, id))
    TEST_CHECK(pool[0].connection_test())

    env.logger.info(f"First node started success")
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
    env.logger.info(f"All nodes in pool checked")

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
    env.logger.info(f"Init balance checked")

    with concurrent.futures.ThreadPoolExecutor(len(pool)) as executor:
      threads = []
      for i in range(len(pool)):
        first_address_number = i * address_per_nodes
        last_address_number = (i * address_per_nodes) + address_per_nodes
        threads.append(
                 executor.submit(node_transfers, pool[i],
                 addresses[first_address_number:last_address_number], transaction_wait, env))
      for i in threads:
        i.result()

    env.logger.info(f"All thread finished")
    for address in addresses:
      for node in pool:
        TEST_CHECK_EQUAL(node.get_balance(address=address.address, timeout=2, wait=1),
                         init_amount)

    return 0

@test_case("transfer_to_myself")
def main(env: Env) -> int:
    sync_port = 20100
    grpc_port = 50100
    amount = randrange(1000)
    update_time = 0.5
    timeout = 2
    wait_time = 1
    transaction_update_time=2
    max_update_request=10

    env.logger.debug(f"Random amount for test = {amount}")

    id = Id(sync_port, grpc_port = grpc_port)
    env.start_node(NodeConfig(id))
    client = env.get_client(ClientType.LEGACY_GRPC, id)

    TEST_CHECK(client.connection_test())
    env.logger.info("Node started success.")

    address = client.generate_keys(keys_path=f"keys")
    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    TEST_CHECK_EQUAL(client.get_balance(address=address.address, timeout=timeout, wait=wait_time), 0)
    env.logger.info("New address created.")

    transaction = client.transfer(to_address=address.address, amount=amount,
                              from_address=distributor_address, fee=0, wait=wait_time, timeout=timeout)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=transaction))
    TEST_CHECK_EQUAL(client.get_balance(address=address.address, timeout=timeout, wait=wait_time), amount)
    env.logger.info("Initialaze transaction success.")

    transaction = client.transfer(to_address=address.address, amount=amount,
                              from_address=address, fee=0, wait=wait_time, timeout=timeout)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=transaction))
    TEST_CHECK_EQUAL(client.get_balance(address=address.address, timeout=timeout, wait=wait_time), amount)
    env.logger.info("Transaction to myself success.")

    return 0

