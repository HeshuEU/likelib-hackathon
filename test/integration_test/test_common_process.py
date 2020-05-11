from tester import test_case, Node, Settings, Id, TEST_CHECK, TEST_CHECK_EQUAL, DISTRIBUTOR_ADDRESS_PATH, Transaction


@test_case("grpc_transfer")
def main(env, logger):
    node_settings = Settings(Id(20209, grpc_port=50059))
    with Node(env, node_settings, logger) as node:
        distributor_address = node.load_address(keys_path=DISTRIBUTOR_ADDRESS_PATH)
        target_address = node.generate_keys(keys_path="user_1")
        distributor_init_balance = node.get_balance(address=distributor_address)
        transaction_fee = 10
        target_amount = 500000
        transfer_result = node.transfer(to_address=target_address, amount=target_amount,
                                        from_address=distributor_address, fee=transaction_fee)
        TEST_CHECK(transfer_result.result)
        distributor_process_balance = node.get_balance(address=distributor_address)
        TEST_CHECK_EQUAL(distributor_process_balance + transaction_fee + target_amount, distributor_init_balance)

    return 0


@test_case("http_transfer")
def main(env, logger):
    node_settings = Settings(Id(20210, http_port=50060))
    with Node(env, node_settings, logger) as node:
        distributor_address = node.load_address(keys_path=DISTRIBUTOR_ADDRESS_PATH)
        target_address = node.generate_keys(keys_path="user_1")
        distributor_init_balance = node.get_balance(address=distributor_address, is_http=True)
        transaction_fee = 10
        target_amount = 500000
        transfer_result = node.transfer(to_address=target_address, amount=target_amount,
                                        from_address=distributor_address, fee=transaction_fee, is_http=True)
        TEST_CHECK(transfer_result.result)
        distributor_process_balance = node.get_balance(address=distributor_address, is_http=True)
        TEST_CHECK_EQUAL(distributor_process_balance + transaction_fee + target_amount, distributor_init_balance)

    return 0


@test_case("grpc_transaction_get")
def main(env, logger):
    node_settings = Settings(Id(20211, grpc_port=50061))
    with Node(env, node_settings, logger) as node:
        distributor_address = node.load_address(keys_path=DISTRIBUTOR_ADDRESS_PATH)
        target_address = node.generate_keys(keys_path="user_1")
        distributor_init_balance = node.get_balance(address=distributor_address)
        transaction_fee = 10
        target_amount = 500000
        transfer_result = node.transfer(to_address=target_address, amount=target_amount,
                                        from_address=distributor_address, fee=transaction_fee)
        TEST_CHECK(transfer_result.result)
        distributor_process_balance = node.get_balance(address=distributor_address)
        TEST_CHECK_EQUAL(distributor_process_balance + transaction_fee + target_amount, distributor_init_balance)

        transaction = node.get_transaction(tx_hash=transfer_result.transaction_hash)
        TEST_CHECK_EQUAL(transaction.value, target_amount)
        TEST_CHECK_EQUAL(transaction.fee, transaction_fee)
        TEST_CHECK_EQUAL(transaction.from_address, distributor_address.address)
        TEST_CHECK_EQUAL(transaction.to_address, target_address.address)
        TEST_CHECK_EQUAL(transaction.tx_type, transaction.TRANSFER_TYPE)
        TEST_CHECK(transaction.verified)
        print(transfer_result.transaction_hash)
        print(transaction.hash())
        print(transaction.value)
        print(transaction.fee)
        print(transaction.from_address)
        print(transaction.to_address)
        print(transaction.data)


    return 0


@test_case("transaction_hash")
def main(env, logger):

    # Transaction tx("transfer", )
    # print(tx.hash())
    return 0