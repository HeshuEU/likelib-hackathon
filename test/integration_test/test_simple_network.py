from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL,\
                   ClientType, get_distributor_address_path, TransactionStatusCode
from time import sleep

@test_case("two_nodes")
def main(env: Env) -> int:
    amount=100
    node_1_id = Id(20101, grpc_port=50101, http_port=50102)
    node_2_id = Id(20102, grpc_port=50201, http_port=50202)
    env.start_node(NodeConfig(node_1_id))
    env.start_node(NodeConfig(node_2_id, nodes=[node_1_id]))
    client_1_grpc = env.get_client(ClientType.LEGACY_GRPC, node_1_id)
    client_1_http = env.get_client(ClientType.LEGACY_HTTP, node_1_id)
    client_2_grpc = env.get_client(ClientType.LEGACY_GRPC, node_2_id)
    client_2_http = env.get_client(ClientType.LEGACY_HTTP, node_2_id)
    TEST_CHECK(client_1_grpc.connection_test())

    distributor_address = client_1_grpc.load_address(keys_path=get_distributor_address_path())
    test_address = client_1_grpc.generate_keys(keys_path="test_keys")

    transaction = client_1_grpc.transfer(to_address=test_address.address, amount=amount, from_address=distributor_address,
                                         fee=0, wait=1, timeout=2)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    stat = client_1_grpc.get_transaction_status(tx_hash=transaction.tx_hash)
    TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)

    transaction = client_2_http.transfer(to_address=test_address.address, amount=amount, from_address=distributor_address,
                                         fee=0, wait=1, timeout=2)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    stat = client_1_grpc.get_transaction_status(tx_hash=transaction.tx_hash)
    TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)

    TEST_CHECK_EQUAL(client_1_grpc.get_balance(address=test_address.address, timeout=2, wait=1), 2*amount)
    TEST_CHECK_EQUAL(client_1_http.get_balance(address=test_address.address, timeout=2, wait=1), 2*amount)
    TEST_CHECK_EQUAL(client_2_grpc.get_balance(address=test_address.address, timeout=2, wait=1), 2*amount)
    TEST_CHECK_EQUAL(client_2_http.get_balance(address=test_address.address, timeout=2, wait=1), 2*amount)

    return 0

@test_case("connect_to_myself")
def main(env: Env) -> int:
    amount = 100
    node = Id(20101, grpc_port=50101)
    client = env.get_client(ClientType.LEGACY_GRPC, node)
    env.start_node(NodeConfig(node, nodes=[node]))
    TEST_CHECK(client.connection_test())

    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    test_address = client.generate_keys(keys_path="test_keys")
    transaction = client.transfer(to_address=test_address.address, amount=amount, from_address=distributor_address,
                                         fee=0, wait=1, timeout=2)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    stat = client.get_transaction_status(tx_hash=transaction.tx_hash)
    TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)
    TEST_CHECK_EQUAL(client.get_balance(address=test_address.address, timeout=2, wait=1), amount)

    return 0
