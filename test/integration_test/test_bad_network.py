from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL,\
                   ClientType, get_distributor_address_path, TransactionStatusCode
from time import sleep
import subprocess, os
from random import randrange


def log_subprocess_output(pipe, env):
    for line in iter(pipe.readline, b''): # b'\n'-separated lines
      env.logger.info(f"start_bad_network.sh: {line}")

def run_bad_nodes(env):
    script_path = os.path.realpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                      "..", "..", "test", "integration_test", "tester", "start_bad_network.sh"))
    process=subprocess.Popen([f"{script_path}"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    with process.stdout:
      log_subprocess_output(process.stdout, env)
    exitcode=process.wait()
    if exitcode != 0:
      env.logger.debug(f"Script exit code = {exitcode}")
      return 1

    bad_client_pool = []
    bad_node_ids = []
    bad_node_ids.append(Id(20203, grpc_port=50051, absolute_address="192.168.100.141"))
    bad_node_ids.append(Id(20203, grpc_port=50051, absolute_address="192.168.100.142"))
    bad_node_ids.append(Id(20203, grpc_port=50051, absolute_address="192.168.100.143"))
    env.logger.info("Get client from bad network pool:")
    for id in bad_node_ids:
      bad_client_pool.append(env.get_grpc_client_to_outside_node(id))
    return bad_client_pool, bad_node_ids


@test_case("connect_node_to_bad_network")
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

    bad_client_pool, bad_node_ids = run_bad_nodes(env)
    main_id = Id(sync_port, grpc_port = grpc_port)
    env.logger.info("Start main node with connecting to bad network nodes:")
    # connect to only first node form bad pool, becouse it's IP from good network.
    # If connect to this ids, nodes in bad pool synchron across 2 network card
    env.start_node(NodeConfig(main_id, nodes=[bad_node_ids[0], ]))
    main_client = env.get_client(ClientType.LEGACY_GRPC, main_id)
    env.logger.info("Check all nodes:")
    TEST_CHECK(main_client.connection_test())
    for client in bad_client_pool:
      TEST_CHECK(client.connection_test())
    env.logger.info("All nodes started success.")

    address = main_client.generate_keys(keys_path=f"keys")
    distributor_address = main_client.load_address(keys_path=get_distributor_address_path())
    TEST_CHECK_EQUAL(main_client.get_balance(address=address.address, timeout=timeout, wait=wait_time), 0)
    env.logger.info("New address created.")

    transaction = main_client.transfer(to_address=address.address, amount=amount,
                              from_address=distributor_address, fee=0, wait=wait_time, timeout=timeout)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(main_client.transaction_success_wait(transaction=transaction))
    TEST_CHECK_EQUAL(main_client.get_balance(address=address.address, timeout=timeout, wait=wait_time), amount)
    env.logger.info("Main client transaction checked success.")
    for client in bad_client_pool:
      TEST_CHECK_EQUAL(client.get_balance(address=address.address, timeout=timeout, wait=wait_time), amount)
    env.logger.info("Test ended success.")

    return 0


@test_case("double_connection_in_bad_network")
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

    bad_client_pool, bad_node_ids = run_bad_nodes(env)
    main_id = Id(sync_port, grpc_port = grpc_port)
    env.logger.info("Start main node with connecting to bad network nodes:")
    # connect to all nodes form bad pool.
    # For nodes in bad pool synchron across 2 network card
    env.start_node(NodeConfig(main_id, nodes=bad_node_ids))
    main_client = env.get_client(ClientType.LEGACY_GRPC, main_id)
    env.logger.info("Check all nodes:")
    TEST_CHECK(main_client.connection_test())
    for client in bad_client_pool:
      TEST_CHECK(client.connection_test())
    env.logger.info("All nodes started success.")

    address = main_client.generate_keys(keys_path=f"keys")
    distributor_address = main_client.load_address(keys_path=get_distributor_address_path())
    TEST_CHECK_EQUAL(main_client.get_balance(address=address.address, timeout=timeout, wait=wait_time), 0)
    env.logger.info("New address created.")

    transaction = main_client.transfer(to_address=address.address, amount=amount,
                              from_address=distributor_address, fee=0, wait=wait_time, timeout=timeout)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(main_client.transaction_success_wait(transaction=transaction))
    TEST_CHECK_EQUAL(main_client.get_balance(address=address.address, timeout=timeout, wait=wait_time), amount)
    env.logger.info("Main client transaction checked success.")
    for client in bad_client_pool:
      TEST_CHECK_EQUAL(client.get_balance(address=address.address, timeout=timeout, wait=wait_time), amount)
    env.logger.info("Test ended success.")

    return 0

def node_transfers(client, addresses, transaction_wait, finish_address, amount, timeout,
                   wait_time, transaction_update_time, max_update_request):
    shift = len(addresses) - 1
    pos = 0
    from_address = addresses[pos]
    transactions = []
    for _ in range(len(addresses) * 5):
      pos = (pos + shift) % len(addresses)
      to_address = addresses[pos]

      transactions.append(node.transfer(to_address=finish_address.address, amount=amount,
         from_address=from_address, fee=0, wait=wait_time, timeout=timeout))
      TEST_CHECK_EQUAL(transactions[-1].status_code, TransactionStatusCode.PENDING)
      env.logger.info(f"Transaction {transactions[-1].tx_hash} is PENDING (from {from_address.address})")
    for transaction in transactions:
      TEST_CHECK(client.transaction_success_wait(transaction=transaction))


@test_case("transaction_stress_in_bad_network")
def main(env: Env) -> int:
    amount = randrange(1000)
    start_balance = 5*amount
    finish_balance = start_balance - amount
    update_time = 0.5
    timeout = 2
    wait_time = 1
    transaction_update_time=2
    max_update_request=10
    number_addresses_per_thread = 5

    env.logger.debug(f"Random amount for test = {amount}")

    bad_client_pool, bad_node_ids = run_bad_nodes(env)
    env.logger.info("Check all nodes:")
    for client in bad_client_pool:
      TEST_CHECK(client.connection_test())
    env.logger.info("All nodes started success.")

    addresses = [bad_client_pool[0].generate_keys(keys_path=f"keys{i}")
                          for i in range(1, number_addresses_per_thread * len(bad_client_pool) + 1)]
    distributor_address = bad_client_pool[0].load_address(keys_path=get_distributor_address_path())
    for address in addresses:
      TEST_CHECK_EQUAL(bad_client_pool[0].get_balance(address=address.address, timeout=timeout, wait=wait_time), 0)
      evn.logger.info(f"Balance of ${address.address} 0")
    env.logger.info(f"New {number_addresses} addresses created.")

    for address in addresses:
      transaction = bad_client_pool[0].transfer(to_address=address.address, amount=start_amount,
                              from_address=distributor_address, fee=0, wait=wait_time, timeout=timeout)
      TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
      TEST_CHECK(bad_client_pool[0].transaction_success_wait(transaction=transaction))
    for client in bad_client_pool:
      for address in addresses:
        TEST_CHECK_EQUAL(client.get_balance(address=address.address, timeout=timeout, wait=wait_time), start_balance)
      env.logger.info(f"Node {client.name} check initialize balance success")
    env.logger.info("Initialize transfers success, current balanse {start_balance}")

    with concurrent.futures.ThreadPoolExecutor(len(bad_client_pool)) as executor:
      threads = []
      for i in range(len(bad_client_pool)):
        first_address_number = i * number_addresses_per_thread
        last_address_number = (i * number_addresses_per_thread) + number_addresses_per_thread
        threads.append(
             executor.submit(node_transfers, bad_client_pool[i],
               addresses[first_address_number:last_address_number], transaction_wait,
               addresses[-1], amount, timeout, wait_time, transaction_update_time, max_update_request))

      for i in threads:
        i.result()

    env.logger.info("Check finish_balance (in this test {finish_balance})")
    for address in addresses[:-1]:
      TEST_CHECK_EQUAL(bad_client_pool[0].get_balance(address=address.address, timeout=timeout,
                                       wait=wait_time), finish_balance)
    last_address_finish_balance = start_balance + amount * len(bad_client_pool) * number_addresses_per_thread
    env.logger.info("Check balance on last address start_balance + all transfers {last_address_finish_balance}")
    TEST_CHECK_EQUAL(bad_client_pool[0].get_balance(address=addresses[-1].address, timeout=timeout,
            wait=wait_time), last_address_finish_balance)



    return 0

