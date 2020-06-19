from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL,\
                   ClientType, get_distributor_address_path, TransactionStatusCode
from time import sleep
import subprocess, os
from random import randrange


def log_subprocess_output(pipe, env):
    for line in iter(pipe.readline, b''): # b'\n'-separated lines
      env.logger.info('start_bad_network.sh: {}'.format(line))

@test_case("connect_node_to_bad_network")
def main(env: Env) -> int:
    sync_port = 20100
    grpc_port = 50100
    amount = randrange(1000)
    update_time = 0.5
    transaction_update_time = 2
    max_update_request = 10
    timeout = 2
    wait_time = 1

    env.logger.debug('Random amount for test = {}'.format(amount))
    script_path = os.path.realpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                      "..", "..", "test", "integration_test", "tester", "start_bad_network.sh"))
    process=subprocess.Popen(['{}'.format(script_path)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    with process.stdout:
      log_subprocess_output(process.stdout, env)
    exitcode=process.wait()
    if exitcode != 0:
      env.logger.debug('Script exit code = {}'.format(exitcode))
      return 1

    bad_client_pool = []
    bad_node_ids = []
    bad_node_ids.append(Id(20203, grpc_port=50051, absolute_address="192.168.100.141"))
    bad_node_ids.append(Id(20203, grpc_port=50051, absolute_address="192.168.100.142"))
    bad_node_ids.append(Id(20203, grpc_port=50051, absolute_address="192.168.100.143"))
    env.logger.info("Get client from bad network pool:")
    for id in bad_node_ids:
      bad_client_pool.append(env.get_grpc_client_to_outside_node(id))
    main_id = Id(sync_port, grpc_port = grpc_port)
    env.logger.info("Start main node with connecting to bad network nodes:")
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
    stat = main_client.get_transaction_status(tx_hash=transaction.tx_hash)
    env.logger.info("Wait transaction (transaction_update_time = {}, max_update_request = {}).".format(
                                       transaction_update_time,      max_update_request))
    request_count = 0
    while stat.status_code != TransactionStatusCode.SUCCESS:
      sleep(transaction_update_time)
      stat = main_client.get_transaction_status(tx_hash=transaction.tx_hash)
      request_count += 1
      env.logger.info("Wait transaction request_count = {}".format(request_count))
      if request_count > max_update_request: return 1
    env.logger.info("Transaction success.")
    TEST_CHECK_EQUAL(main_client.get_balance(address=address.address, timeout=timeout, wait=wait_time), amount)
    env.logger.info("Main client transaction checked success.")
    for client in bad_client_pool:
      TEST_CHECK_EQUAL(client.get_balance(address=address.address, timeout=timeout, wait=wait_time), amount)
    env.logger.info("Test ended success.")

    return 0
