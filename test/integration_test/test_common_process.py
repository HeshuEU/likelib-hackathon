from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL, ClientType, \
    get_distributor_address_path, AccountType, TEST_CHECK_NOT_EQUAL, TransactionStatusCode


@test_case("get_block_legacy_grpc")
def main(env: Env) -> int:
    node_id = Id(20201, grpc_port=50201)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)

    block = client.get_block(block_number=0)
    TEST_CHECK_EQUAL(block.depth, 0)

    return 0


@test_case("get_block_legacy_http")
def main(env: Env) -> int:
    node_id = Id(20202, http_port=50202)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_HTTP, node_id)

    block = client.get_block(block_number=0)
    TEST_CHECK_EQUAL(block.depth, 0)

    return 0


@test_case("get_block_python_http")
def main(env: Env) -> int:
    node_id = Id(20203, http_port=50203)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.PYTHON_HTTP, node_id)

    block = client.get_block(block_number=0)
    TEST_CHECK_EQUAL(block.depth, 0)

    return 0


@test_case("get_account_info_legacy_grpc")
def main(env: Env) -> int:
    node_id = Id(20204, grpc_port=50204)
    env.start_node(NodeConfig(node_id), 3)
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    account_info = client.get_account_info(address=distributor_address.address)
    TEST_CHECK_EQUAL(account_info.account_type, AccountType.CLIENT)
    TEST_CHECK_NOT_EQUAL(account_info.balance, 0)
    return 0


@test_case("get_account_info_legacy_http")
def main(env: Env) -> int:
    node_id = Id(20205, http_port=50205)
    env.start_node(NodeConfig(node_id), 3)
    client = env.get_client(ClientType.LEGACY_HTTP, node_id)
    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    account_info = client.get_account_info(address=distributor_address.address)
    TEST_CHECK_EQUAL(account_info.account_type, AccountType.CLIENT)
    TEST_CHECK_NOT_EQUAL(account_info.balance, 0)
    return 0


@test_case("get_account_info_python_http")
def main(env: Env) -> int:
    node_id = Id(20206, http_port=50206)
    env.start_node(NodeConfig(node_id), 3)
    client = env.get_client(ClientType.PYTHON_HTTP, node_id)
    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    account_info = client.get_account_info(address=distributor_address.address)
    TEST_CHECK_EQUAL(account_info.account_type, AccountType.CLIENT)
    TEST_CHECK_NOT_EQUAL(account_info.balance, 0)
    return 0


@test_case("transfer_legacy_grpc")
def main(env: Env) -> int:
    node_id = Id(20207, grpc_port=50207)
    env.start_node(NodeConfig(node_id), 3)
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    TEST_CHECK(client.connection_test())

    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    target_address = client.generate_keys(keys_path="user_1")
    distributor_init_balance = client.get_balance(address=distributor_address.address)
    transaction_fee = 10
    target_amount = 500000
    transfer_result = client.transfer(to_address=target_address.address, amount=target_amount,
                                      from_address=distributor_address, fee=transaction_fee)

    TEST_CHECK_EQUAL(transfer_result.status_code, TransactionStatusCode.PENDING)
    distributor_process_balance = client.get_balance(address=distributor_address.address)
    TEST_CHECK_EQUAL(distributor_process_balance + transaction_fee + target_amount, distributor_init_balance)

    target_process_balance = client.get_balance(address=target_address.address)
    TEST_CHECK_EQUAL(target_process_balance, target_amount)
    return 0


@test_case("transfer_legacy_http")
def main(env: Env) -> int:
    node_id = Id(20208, http_port=50208)
    env.start_node(NodeConfig(node_id), 3)
    client = env.get_client(ClientType.LEGACY_HTTP, node_id)
    TEST_CHECK(client.connection_test())

    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    target_address = client.generate_keys(keys_path="user_1")
    distributor_init_balance = client.get_balance(address=distributor_address.address)
    transaction_fee = 10
    target_amount = 500000
    transfer_result = client.transfer(to_address=target_address.address, amount=target_amount,
                                      from_address=distributor_address, fee=transaction_fee)

    TEST_CHECK_EQUAL(transfer_result.status_code, TransactionStatusCode.PENDING)

    transfer_status = client.get_transaction_status(tx_hash=transfer_result.tx_hash)
    TEST_CHECK_EQUAL(transfer_status.status_code, TransactionStatusCode.SUCCESS)

    distributor_process_balance = client.get_balance(address=distributor_address.address)
    TEST_CHECK_EQUAL(distributor_process_balance + transaction_fee + target_amount, distributor_init_balance)

    target_process_balance = client.get_balance(address=target_address.address)
    TEST_CHECK_EQUAL(target_process_balance, target_amount)
    return 0


@test_case("transfer_python_http")
def main(env: Env) -> int:
    node_id = Id(20209, http_port=50209)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.PYTHON_HTTP, node_id)
    TEST_CHECK(client.connection_test())

    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    target_address = client.generate_keys(keys_path="user_1")
    distributor_init_balance = client.get_balance(address=distributor_address.address)
    transaction_fee = 10
    target_amount = 500000
    transfer_result = client.transfer(to_address=target_address.address, amount=target_amount,
                                      from_address=distributor_address, fee=transaction_fee)

    TEST_CHECK_EQUAL(transfer_result.status_code, TransactionStatusCode.PENDING)

    transfer_status = client.get_transaction_status(tx_hash=transfer_result.tx_hash)
    TEST_CHECK_EQUAL(transfer_status.status_code, TransactionStatusCode.SUCCESS)

    distributor_process_balance = client.get_balance(address=distributor_address.address)
    TEST_CHECK_EQUAL(distributor_process_balance + transaction_fee + target_amount, distributor_init_balance)

    target_process_balance = client.get_balance(address=target_address.address)
    TEST_CHECK_EQUAL(target_process_balance, target_amount)
    return 0

