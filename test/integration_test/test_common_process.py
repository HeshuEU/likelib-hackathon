from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL, DISTRIBUTOR_ADDRESS_PATH


@test_case("grpc_transfer")
def main(env: Env) -> int:
    node_id = Id(20209, grpc_port=50059)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_GRPC_TYPE, node_id)
    distributor_address = client.load_address(keys_path=DISTRIBUTOR_ADDRESS_PATH)
    target_address = client.generate_keys(keys_path="user_1")
    distributor_init_balance = client.get_balance(address=distributor_address)
    transaction_fee = 10
    target_amount = 500000
    transfer_result = client.transfer(to_address=target_address, amount=target_amount,
                                      from_address=distributor_address, fee=transaction_fee)
    TEST_CHECK(transfer_result.result)
    distributor_process_balance = client.get_balance(address=distributor_address)
    TEST_CHECK_EQUAL(distributor_process_balance + transaction_fee + target_amount, distributor_init_balance)

    return 0


@test_case("http_transfer")
def main(env: Env) -> int:
    node_id = Id(20210, http_port=50060)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_HTTP_TYPE, node_id)
    distributor_address = client.load_address(keys_path=DISTRIBUTOR_ADDRESS_PATH)
    target_address = client.generate_keys(keys_path="user_1")
    distributor_init_balance = client.get_balance(address=distributor_address)
    transaction_fee = 10
    target_amount = 500000
    transfer_result = client.transfer(to_address=target_address, amount=target_amount,
                                      from_address=distributor_address, fee=transaction_fee)
    TEST_CHECK(transfer_result.result)
    distributor_process_balance = client.get_balance(address=distributor_address)
    TEST_CHECK_EQUAL(distributor_process_balance + transaction_fee + target_amount, distributor_init_balance)

    return 0


@test_case("grpc_transaction_get")
def main(env: Env) -> int:
    node_id = Id(20211, grpc_port=50061)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_GRPC_TYPE, node_id)

    distributor_address = client.load_address(keys_path=DISTRIBUTOR_ADDRESS_PATH)
    target_address = client.generate_keys(keys_path="user_1")
    distributor_init_balance = client.get_balance(address=distributor_address)
    transaction_fee = 10
    target_amount = 500000
    transfer_result = client.transfer(to_address=target_address, amount=target_amount,
                                      from_address=distributor_address, fee=transaction_fee)
    TEST_CHECK(transfer_result.result)
    distributor_process_balance = client.get_balance(address=distributor_address)
    TEST_CHECK_EQUAL(distributor_process_balance + transaction_fee + target_amount, distributor_init_balance)

    transaction = client.get_transaction(tx_hash=transfer_result.transaction_hash)
    TEST_CHECK_EQUAL(transaction.value, target_amount)
    TEST_CHECK_EQUAL(transaction.fee, transaction_fee)
    TEST_CHECK_EQUAL(transaction.from_address, distributor_address.address)
    TEST_CHECK_EQUAL(transaction.to_address, target_address.address)
    TEST_CHECK_EQUAL(transaction.tx_type, transaction.TRANSFER_TYPE)
    TEST_CHECK(transaction.verified)

    return 0


@test_case("http_transaction_get")
def main(env: Env) -> int:
    node_id = Id(20211, http_port=50061)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_HTTP_TYPE, node_id)

    distributor_address = client.load_address(keys_path=DISTRIBUTOR_ADDRESS_PATH)
    target_address = client.generate_keys(keys_path="user_1")
    distributor_init_balance = client.get_balance(address=distributor_address)
    transaction_fee = 10
    target_amount = 500000
    transfer_result = client.transfer(to_address=target_address, amount=target_amount,
                                      from_address=distributor_address, fee=transaction_fee)
    TEST_CHECK(transfer_result.result)
    distributor_process_balance = client.get_balance(address=distributor_address)
    TEST_CHECK_EQUAL(distributor_process_balance + transaction_fee + target_amount, distributor_init_balance)

    transaction = client.get_transaction(tx_hash=transfer_result.transaction_hash)
    TEST_CHECK_EQUAL(transaction.value, target_amount)
    TEST_CHECK_EQUAL(transaction.fee, transaction_fee)
    TEST_CHECK_EQUAL(transaction.from_address, distributor_address.address)
    TEST_CHECK_EQUAL(transaction.to_address, target_address.address)
    TEST_CHECK_EQUAL(transaction.tx_type, transaction.TRANSFER_TYPE)
    TEST_CHECK(transaction.verified)

    return 0
