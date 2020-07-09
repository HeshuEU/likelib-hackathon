import os
import datetime
import time

from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL, get_distributor_address_path, \
    ClientType, TransactionStatusCode, TEST_CHECK_NOT_EQUAL

CONTRACTS_FOLDER = os.path.realpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "doc", "contracts"))

if not os.path.exists(CONTRACTS_FOLDER):
    print("Contracts folder was not found", flush=True)
    exit(1)

TRANSACTION_WAIT = 10


@test_case("auction_contract_legacy_grpc")
def main(env: Env):
    contract_file_path = os.path.join(CONTRACTS_FOLDER, "auction", "contract.sol")
    if not os.path.exists(contract_file_path):
        TEST_CHECK(False, message="Contracts file was not found")

    node_id = Id(20401, grpc_port=50401)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    auction_compiled_contract = contracts[0]

    beneficiary_keys = client.generate_keys(keys_path="user_1")

    beneficiary_init_balance = client.get_balance(address=beneficiary_keys.address)
    account_seconds_duration = 200
    auction_init_message = client.encode_message(code=auction_compiled_contract,
                                                 message=f"constructor({account_seconds_duration}, Address({beneficiary_keys.address}))")
    start_time = datetime.datetime.now()

    distributor_keys = client.load_address(keys_path=get_distributor_address_path())
    env.logger.info("Deploy contract")
    deployed_contract_status = client.push_contract(from_address=distributor_keys, code=auction_compiled_contract,
                                                    amount=0, fee=10000000, init_message=auction_init_message,
                                                    wait=TRANSACTION_WAIT)

    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=deployed_contract_status))
    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
#    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)
    deployed_contract = deployed_contract_status.data

    client_1 = client.generate_keys(keys_path="client_1")
    client_2 = client.generate_keys(keys_path="client_2")
    client_1_init_amount = 5000000
    client_2_init_amount = 5000000

    client.transfer(from_address=distributor_keys, to_address=client_1.address, amount=client_1_init_amount, fee=5,
                    wait=TRANSACTION_WAIT)
    client.transfer(from_address=distributor_keys, to_address=client_2.address, amount=client_2_init_amount, fee=5,
                    wait=TRANSACTION_WAIT)

    bid_message = client.encode_message(code=auction_compiled_contract, message="bid()")
    max_bid_message = client.encode_message(code=auction_compiled_contract, message="highestBid()")

    client_1_bid_1 = 10000
    client_1_bid_1_gas = 100000
    env.logger.info("Bid 1 from client 1")
    bid_status = client.message_call(from_address=client_1, to_address=deployed_contract, fee=client_1_bid_1_gas,
                                     amount=client_1_bid_1, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    client_1_bid_2 = 15000
    client_1_bid_2_gas = 100000
    env.logger.info("Bid 2 from client 1")
    bid_status = client.message_call(from_address=client_1, to_address=deployed_contract, fee=client_1_bid_2_gas,
                                     amount=client_1_bid_2, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)
    TEST_CHECK(current_max_bid[''] == client_1_bid_2)

    client_2_bid_1 = 10000
    client_2_bid_1_gas = 100000
    env.logger.info("Bid 1 from client 2")
    bid_status = client.message_call(from_address=client_2, to_address=deployed_contract, fee=client_2_bid_1_gas,
                                     amount=client_2_bid_1, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_NOT_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)
    TEST_CHECK(current_max_bid[''] == client_1_bid_2)

    client_2_bid_2 = 20000
    client_2_bid_2_gas = 100000
    env.logger.info("Bid 2 from client 2")
    bid_status = client.message_call(from_address=client_2, to_address=deployed_contract, fee=client_2_bid_2_gas,
                                     amount=client_2_bid_2, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)
    TEST_CHECK(current_max_bid[''] == client_2_bid_2)

    time.sleep(account_seconds_duration - (datetime.datetime.now() - start_time).total_seconds())

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)

    auction_end_message = client.encode_message(code=auction_compiled_contract, message="auctionEnd()")
    env.logger.info("End auction")
    auction_end_status = client.message_call(from_address=distributor_keys, to_address=deployed_contract, fee=100000,
                                             amount=0, message=auction_end_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=auction_end_status))
#    auction_end_status = client.get_transaction_status(tx_hash=auction_end_status.tx_hash)
#    TEST_CHECK_EQUAL(auction_end_status.status_code, TransactionStatusCode.SUCCESS)
    TEST_CHECK(
        client.decode_message(code=auction_compiled_contract, method="auctionEnd", message=auction_end_status.data)[
            'is_end'])

    beneficiary_current_balance = client.get_balance(address=beneficiary_keys.address)
    TEST_CHECK(beneficiary_init_balance + current_max_bid[''] == beneficiary_current_balance)

    return 0


@test_case("auction_contract_legacy_http")
def main(env: Env):
    contract_file_path = os.path.join(CONTRACTS_FOLDER, "auction", "contract.sol")
    if not os.path.exists(contract_file_path):
        TEST_CHECK(False, message="Contracts file was not found")

    node_id = Id(20402, http_port=50402)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_HTTP, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    auction_compiled_contract = contracts[0]

    beneficiary_keys = client.generate_keys(keys_path="user_1")

    beneficiary_init_balance = client.get_balance(address=beneficiary_keys.address)
    account_seconds_duration = 200
    auction_init_message = client.encode_message(code=auction_compiled_contract,
                                                 message=f"constructor({account_seconds_duration}, Address({beneficiary_keys.address}))")
    start_time = datetime.datetime.now()

    distributor_keys = client.load_address(keys_path=get_distributor_address_path())
    deployed_contract_status = client.push_contract(from_address=distributor_keys, code=auction_compiled_contract,
                                                    amount=0, fee=10000000, init_message=auction_init_message,
                                                    wait=TRANSACTION_WAIT)

    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=deployed_contract_status))
    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
#    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)
    deployed_contract = deployed_contract_status.data

    client_1 = client.generate_keys(keys_path="client_1")
    client_2 = client.generate_keys(keys_path="client_2")
    client_1_init_amount = 5000000
    client_2_init_amount = 5000000

    client.transfer(from_address=distributor_keys, to_address=client_1.address, amount=client_1_init_amount, fee=5,
                    wait=TRANSACTION_WAIT)
    client.transfer(from_address=distributor_keys, to_address=client_2.address, amount=client_2_init_amount, fee=5,
                    wait=TRANSACTION_WAIT)

    bid_message = client.encode_message(code=auction_compiled_contract, message="bid()")
    max_bid_message = client.encode_message(code=auction_compiled_contract, message="highestBid()")

    client_1_bid_1 = 10000
    client_1_bid_1_gas = 100000
    bid_status = client.message_call(from_address=client_1, to_address=deployed_contract, fee=client_1_bid_1_gas,
                                     amount=client_1_bid_1, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    client_1_bid_2 = 15000
    client_1_bid_2_gas = 100000
    bid_status = client.message_call(from_address=client_1, to_address=deployed_contract, fee=client_1_bid_2_gas,
                                     amount=client_1_bid_2, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)
    TEST_CHECK(current_max_bid[''] == client_1_bid_2)

    client_2_bid_1 = 10000
    client_2_bid_1_gas = 100000
    bid_status = client.message_call(from_address=client_2, to_address=deployed_contract, fee=client_2_bid_1_gas,
                                     amount=client_2_bid_1, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_NOT_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)
    TEST_CHECK(current_max_bid[''] == client_1_bid_2)

    client_2_bid_2 = 20000
    client_2_bid_2_gas = 100000
    bid_status = client.message_call(from_address=client_2, to_address=deployed_contract, fee=client_2_bid_2_gas,
                                     amount=client_2_bid_2, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)
    TEST_CHECK(current_max_bid[''] == client_2_bid_2)

    time.sleep(account_seconds_duration - (datetime.datetime.now() - start_time).total_seconds())

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)

    auction_end_message = client.encode_message(code=auction_compiled_contract, message="auctionEnd()")
    auction_end_status = client.message_call(from_address=distributor_keys, to_address=deployed_contract, fee=100000,
                                             amount=0, message=auction_end_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=auction_end_status_status))
#    auction_end_status = client.get_transaction_status(tx_hash=auction_end_status.tx_hash)
#    TEST_CHECK_EQUAL(auction_end_status.status_code, TransactionStatusCode.SUCCESS)
    TEST_CHECK(
        client.decode_message(code=auction_compiled_contract, method="auctionEnd", message=auction_end_status.data)[
            'is_end'])

    beneficiary_current_balance = client.get_balance(address=beneficiary_keys.address)
    TEST_CHECK(beneficiary_init_balance + current_max_bid[''] == beneficiary_current_balance)

    return 0


@test_case("auction_contract_python_grpc")
def main(env: Env):
    contract_file_path = os.path.join(CONTRACTS_FOLDER, "auction", "contract.sol")
    if not os.path.exists(contract_file_path):
        TEST_CHECK(False, message="Contracts file was not found")

    node_id = Id(20403, http_port=50403)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.PYTHON_HTTP, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    auction_compiled_contract = contracts[0]

    beneficiary_keys = client.generate_keys(keys_path="user_1")

    beneficiary_init_balance = client.get_balance(address=beneficiary_keys.address)
    account_seconds_duration = 200
    auction_init_message = client.encode_message(code=auction_compiled_contract,
                                                 message=f"constructor({account_seconds_duration}, Address({beneficiary_keys.address}))")
    start_time = datetime.datetime.now()

    distributor_keys = client.load_address(keys_path=get_distributor_address_path())
    deployed_contract_status = client.push_contract(from_address=distributor_keys, code=auction_compiled_contract,
                                                    amount=0, fee=10000000, init_message=auction_init_message,
                                                    wait=TRANSACTION_WAIT)

    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=deployed_contract_status))
    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
#    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)
    deployed_contract = deployed_contract_status.data

    client_1 = client.generate_keys(keys_path="client_1")
    client_2 = client.generate_keys(keys_path="client_2")
    client_1_init_amount = 5000000
    client_2_init_amount = 5000000

    client.transfer(from_address=distributor_keys, to_address=client_1.address, amount=client_1_init_amount, fee=5,
                    wait=TRANSACTION_WAIT)
    client.transfer(from_address=distributor_keys, to_address=client_2.address, amount=client_2_init_amount, fee=5,
                    wait=TRANSACTION_WAIT)

    bid_message = client.encode_message(code=auction_compiled_contract, message="bid()")
    max_bid_message = client.encode_message(code=auction_compiled_contract, message="highestBid()")

    client_1_bid_1 = 10000
    client_1_bid_1_gas = 100000
    bid_status = client.message_call(from_address=client_1, to_address=deployed_contract, fee=client_1_bid_1_gas,
                                     amount=client_1_bid_1, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    client_1_bid_2 = 15000
    client_1_bid_2_gas = 100000
    bid_status = client.message_call(from_address=client_1, to_address=deployed_contract, fee=client_1_bid_2_gas,
                                     amount=client_1_bid_2, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)
    TEST_CHECK(current_max_bid[''] == client_1_bid_2)

    client_2_bid_1 = 10000
    client_2_bid_1_gas = 100000
    bid_status = client.message_call(from_address=client_2, to_address=deployed_contract, fee=client_2_bid_1_gas,
                                     amount=client_2_bid_1, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_NOT_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)
    TEST_CHECK(current_max_bid[''] == client_1_bid_2)

    client_2_bid_2 = 20000
    client_2_bid_2_gas = 100000
    bid_status = client.message_call(from_address=client_2, to_address=deployed_contract, fee=client_2_bid_2_gas,
                                     amount=client_2_bid_2, message=bid_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=bid_status))
#    bid_status = client.get_transaction_status(tx_hash=bid_status.tx_hash)
#    TEST_CHECK_EQUAL(bid_status.status_code, TransactionStatusCode.SUCCESS)

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)
    TEST_CHECK(current_max_bid[''] == client_2_bid_2)

    time.sleep(account_seconds_duration - (datetime.datetime.now() - start_time).total_seconds())

    result = client.call_view(from_address=distributor_keys, to_address=deployed_contract, message=max_bid_message)
    current_max_bid = client.decode_message(code=auction_compiled_contract, method="highestBid", message=result)

    auction_end_message = client.encode_message(code=auction_compiled_contract, message="auctionEnd()")
    auction_end_status = client.message_call(from_address=distributor_keys, to_address=deployed_contract, fee=100000,
                                             amount=0, message=auction_end_message, wait=TRANSACTION_WAIT)
    TEST_CHECK(client.transaction_success_wait(transaction=auction_end_status))
#    auction_end_status = client.get_transaction_status(tx_hash=auction_end_status.tx_hash)
#    TEST_CHECK_EQUAL(auction_end_status.status_code, TransactionStatusCode.SUCCESS)
    TEST_CHECK(
        client.decode_message(code=auction_compiled_contract, method="auctionEnd", message=auction_end_status.data)[
            'is_end'])

    beneficiary_current_balance = client.get_balance(address=beneficiary_keys.address)
    TEST_CHECK(beneficiary_init_balance + current_max_bid[''] == beneficiary_current_balance)

    return 0
