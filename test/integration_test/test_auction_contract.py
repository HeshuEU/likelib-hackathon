import os
import datetime
import time

from tester import test_case, Node, TEST_CHECK

CONTRACTS_FOLDER = os.path.realpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "doc", "contracts"))

if not os.path.exists(CONTRACTS_FOLDER):
    print("Contracts folder was not found", flush=True)
    exit(1)


@test_case("auction_contract")
def main(env, logger):
    contract_file_path = os.path.join(CONTRACTS_FOLDER, "auction", "contract.sol")
    if not os.path.exists(contract_file_path):
        TEST_CHECK(False, message="Contracts folder was not found")

    node_settings = Node.Settings(Node.Id(20225, 50075), start_up_time=2)
    with Node(env, node_settings, logger) as node:
        node.run_check_test()

        contracts = node.compile_contract(code=contract_file_path)
        auction_contract_data = contracts[0]

        beneficiary_address = node.create_new_address(keys_path='beneficiary')
        beneficiary_init_balance = node.get_balance(address=beneficiary_address)
        account_seconds_duration = 40
        auction_init_message = node.encode_message(code=auction_contract_data,
                                                   message=f"constructor({account_seconds_duration}, Address({beneficiary_address.address}))")
        start_time = datetime.datetime.now()

        distributor_address = node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)
        deployed_contract = node.push_contract(from_address=distributor_address, code=auction_contract_data, amount=0,
                                               fee=10000000, init_message=auction_init_message, timeout=10)

        client_1 = node.create_new_address(keys_path='client_1')
        client_2 = node.create_new_address(keys_path='client_2')
        client_1_init_amount = 5000000
        client_2_init_amount = 5000000

        node.transfer(from_address=distributor_address, to_address=client_1, amount=client_1_init_amount, fee=5,
                      timeout=10)
        node.transfer(from_address=distributor_address, to_address=client_2, amount=client_2_init_amount, fee=5,
                      timeout=10)

        bid_message = node.encode_message(code=auction_contract_data, message="bid()")
        max_bid_message = node.encode_message(code=auction_contract_data, message="highestBid()")

        client_1_bid_1 = 10000
        client_1_bid_1_gas = 100000
        node.message_to_contract(from_address=client_1, to_address=deployed_contract,
                                 fee=client_1_bid_1_gas, amount=client_1_bid_1, message=bid_message,
                                 timeout=10)

        client_1_bid_2 = 15000
        client_1_bid_2_gas = 100000
        node.message_to_contract(from_address=client_1, to_address=deployed_contract,
                                 fee=client_1_bid_2_gas, amount=client_1_bid_2, message=bid_message,
                                 timeout=10)

        result = node.message_to_contract(from_address=distributor_address,
                                          to_address=deployed_contract,
                                          fee=100000, amount=0,
                                          message=max_bid_message, timeout=10)
        current_max_bid = node.decode_message(code=auction_contract_data, method="highestBid", message=result)
        TEST_CHECK(current_max_bid[''] == client_1_bid_2)

        client_2_bid_1 = 10000
        client_2_bid_1_gas = 100000
        node.message_to_contract(from_address=client_2, to_address=deployed_contract,
                                 fee=client_2_bid_1_gas, amount=client_2_bid_1,
                                 message=bid_message, timeout=10)

        result = node.message_to_contract(from_address=distributor_address,
                                          to_address=deployed_contract,
                                          fee=100000, amount=0,
                                          message=max_bid_message, timeout=10)
        current_max_bid = node.decode_message(code=auction_contract_data, method="highestBid", message=result)
        TEST_CHECK(current_max_bid[''] == client_1_bid_2)

        client_2_bid_2 = 20000
        client_2_bid_2_gas = 100000
        node.message_to_contract(from_address=client_2, to_address=deployed_contract,
                                 fee=client_2_bid_2_gas, amount=client_2_bid_2,
                                 message=bid_message, timeout=10)

        result = node.message_to_contract(from_address=distributor_address,
                                          to_address=deployed_contract,
                                          fee=100000, amount=0,
                                          message=max_bid_message, timeout=10)
        current_max_bid = node.decode_message(code=auction_contract_data, method="highestBid", message=result)
        TEST_CHECK(current_max_bid[''] == client_2_bid_2)

        time.sleep(account_seconds_duration - (datetime.datetime.now() - start_time).total_seconds())

        max_bid_message = node.encode_message(code=auction_contract_data, message="highestBid()")
        result = node.message_to_contract(from_address=distributor_address,
                                          to_address=deployed_contract,
                                          fee=100000, amount=0,
                                          message=max_bid_message, timeout=10)
        current_max_bid = node.decode_message(code=auction_contract_data, method="highestBid", message=result)

        auction_end_message = node.encode_message(code=auction_contract_data, message="auctionEnd()")
        result = node.message_to_contract(from_address=distributor_address,
                                          to_address=deployed_contract,
                                          fee=100000, amount=0,
                                          message=auction_end_message, timeout=10)
        if result.message:
            TEST_CHECK(
                node.decode_message(code=auction_contract_data, method="auctionEnd", message=result)['is_end'])

        beneficiary_current_balance = node.get_balance(address=beneficiary_address)
        TEST_CHECK(beneficiary_init_balance + current_max_bid[''] == beneficiary_current_balance)

    return 0
