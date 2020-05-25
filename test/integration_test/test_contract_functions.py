import os

from tester import test_case, Node, TEST_CHECK

CONTRACTS_FOLDER = os.path.realpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "doc", "contracts"))

if not os.path.exists(CONTRACTS_FOLDER):
    print("Contracts folder was not found", flush=True)
    exit(1)


@test_case("simple_storage_contract")
def main(env, logger):
    contract_file_path = os.path.join(CONTRACTS_FOLDER, "simple_storage_contract", "contract.sol")
    if not os.path.exists(contract_file_path):
        TEST_CHECK(False, message="Contracts folder was not found")

    node_settings = Node.Settings(Node.Id(20215, 50065), start_up_time=2)
    with Node(env, node_settings, logger) as node:
        node.run_check_test()

        contracts = node.compile_contract(code=contract_file_path)
        test_contract = contracts[0]
        target_value = 8888
        test_contract_init_message = node.encode_message(code=test_contract, message=f"constructor({target_value})")

        distributor_address = node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)
        deployed_contract = node.push_contract(from_address=distributor_address, code=test_contract, amount=0,
                                               fee=10000000, init_message=test_contract_init_message, timeout=5)

        call_message = node.encode_message(code=test_contract, message="get()")
        message_result = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                                  fee=10000000, amount=0, message=call_message, timeout=5)
        res = node.decode_message(code=test_contract, method="get", message=message_result)
        TEST_CHECK(res['stored_data'] == target_value)

        target_value_2 = 5555
        call_message_2 = node.encode_message(code=test_contract, message=f"set({target_value_2})")
        node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                 fee=10000000, amount=0, message=call_message_2, timeout=5)

        message_result_3 = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                                    fee=10000000, amount=0, message=call_message, timeout=5)

        res = node.decode_message(code=test_contract, method="get", message=message_result_3)
        TEST_CHECK(res['stored_data'] == target_value_2)

    return 0


@test_case("get_balance_contract")
def main(env, logger):
    contract_file_path = os.path.join(CONTRACTS_FOLDER, "get_balance_contract", "contract.sol")
    if not os.path.exists(contract_file_path):
        TEST_CHECK(False, message="Contracts folder was not found")

    node_settings = Node.Settings(Node.Id(20216, 50066), start_up_time=2)
    with Node(env, node_settings, logger) as node:
        node.run_check_test()

        contracts = node.compile_contract(code=contract_file_path)
        test_contract = contracts[0]
        distributor_address = node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)
        deployed_contract = node.push_contract(from_address=distributor_address, code=test_contract, amount=0,
                                               fee=10000000, init_message="", timeout=5)

        current_balance = node.get_balance(address=distributor_address)
        gas_for_call = 10000000

        call_message = node.encode_message(code=test_contract, message="getBalance()")
        message_result = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                                  fee=gas_for_call, amount=0, message=call_message, timeout=5)
        res = node.decode_message(code=test_contract, method="getBalance", message=message_result)
        TEST_CHECK((res["balance"] + gas_for_call) == current_balance)

    return 0


@test_case("address_send_contract")
def main(env, logger):
    contract_file_path = os.path.join(CONTRACTS_FOLDER, "address_send_contract", "contract.sol")
    if not os.path.exists(contract_file_path):
        TEST_CHECK(False, message="Contracts folder was not found")

    node_settings = Node.Settings(Node.Id(20217, 50067), start_up_time=2)
    with Node(env, node_settings, logger) as node:
        node.run_check_test()

        distributor_address = node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)
        new_test_account = node.create_new_address(keys_path="account_1")

        initializing_amount = 100000000
        TEST_CHECK(
            node.transfer(to_address=new_test_account, from_address=distributor_address, amount=initializing_amount,
                          fee=0))

        node.run_check_balance(new_test_account, initializing_amount)

        contracts = node.compile_contract(code=contract_file_path)
        test_contract = contracts[0]

        deployed_contract = node.push_contract(from_address=new_test_account, code=test_contract, amount=0,
                                               fee=100000, init_message="", timeout=5)

        current_balance = node.get_balance(address=new_test_account)

        gas_for_call = 10000000
        amount_for_call = 5000000
        call_message = node.encode_message(code=test_contract, message=f"testAddressSend({amount_for_call})")
        message_result = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                                  fee=gas_for_call, amount=amount_for_call, message=call_message,
                                                  timeout=5)
        res = node.decode_message(code=test_contract, method="testAddressSend", message=message_result)
        TEST_CHECK(res["is_success"])

        new_balance = node.get_balance(address=new_test_account)
        TEST_CHECK(new_balance == (current_balance + amount_for_call))

    return 0


@test_case("get_previous_block_contract")
def main(env, logger):
    contract_file_path = os.path.join(CONTRACTS_FOLDER, "get_previous_block_contract", "contract.sol")
    if not os.path.exists(contract_file_path):
        TEST_CHECK(False, message="Contracts folder was not found")

    node_settings = Node.Settings(Node.Id(20218, 50068), start_up_time=2)
    with Node(env, node_settings, logger) as node:
        node.run_check_test()

        contracts = node.compile_contract(code=contract_file_path)
        test_contract = contracts[0]
        distributor_address = node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)
        deployed_contract = node.push_contract(from_address=distributor_address, code=test_contract, amount=0,
                                               fee=10000000, init_message="", timeout=5)

        current_block_hash = node.get_info()

        call_message = node.encode_message(code=test_contract, message="get()")
        message_result = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                                  fee=10000000, amount=0, message=call_message, timeout=5)
        res = node.decode_message(code=test_contract, method="get", message=message_result)
        TEST_CHECK(res["previous_block_hash"] == current_block_hash)

        current_block_hash = node.get_info()

        call_message = node.encode_message(code=test_contract, message="get()")
        message_result = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                                  fee=10000000, amount=0, message=call_message, timeout=5)
        res = node.decode_message(code=test_contract, method="get", message=message_result)
        TEST_CHECK(res["previous_block_hash"] == current_block_hash)

        call_message = node.encode_message(code=test_contract, message="get()")
        message_result = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                                  fee=10000000, amount=0, message=call_message, timeout=5)
        res = node.decode_message(code=test_contract, method="get", message=message_result)
        TEST_CHECK(res["previous_block_hash"] != current_block_hash)

    return 0


@test_case("selfdestruct_contract")
def main(env, logger):
    contract_file_path = os.path.join(CONTRACTS_FOLDER, "selfdestruct_contract", "contract.sol")
    if not os.path.exists(contract_file_path):
        TEST_CHECK(False, message="Contracts folder was not found")

    node_settings = Node.Settings(Node.Id(20219, 50069), start_up_time=2)
    with Node(env, node_settings, logger) as node:
        node.run_check_test()

        new_account = node.create_new_address(keys_path="new_account")
        contracts = node.compile_contract(code=contract_file_path)
        test_contract = contracts[0]
        distributor_address = node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)
        init_message = node.encode_message(code=test_contract, message=f"constructor(Address({new_account.address}))")
        deployed_contract = node.push_contract(from_address=distributor_address, code=test_contract, amount=50,
                                               fee=10000000, init_message=init_message, timeout=7)
        amount_for_call = 100000

        pay_message = node.encode_message(code=test_contract, message="payMe()")
        message_result = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                                  fee=10000000, amount=amount_for_call, message=pay_message, timeout=7)
        res = node.decode_message(code=test_contract, method="payMe", message=message_result)
        TEST_CHECK(res["current_balance"] == node.get_balance(address=deployed_contract))

        message_result = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                                  fee=10000000, amount=amount_for_call, message=pay_message, timeout=7)
        res = node.decode_message(code=test_contract, method="payMe", message=message_result)
        TEST_CHECK(res["current_balance"] == node.get_balance(address=deployed_contract))

        TEST_CHECK(0 == node.get_balance(address=new_account))
        last_balance = node.get_balance(address=deployed_contract)
        delete_message = node.encode_message(code=test_contract, message="initDelete()")
        node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                 fee=10000000, amount=0, message=delete_message, timeout=7)
        TEST_CHECK(last_balance == node.get_balance(address=new_account))
        TEST_CHECK(0 == node.get_balance(address=deployed_contract))

        message_result = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract,
                                                  fee=10000000, amount=1, message=pay_message, timeout=7)
        TEST_CHECK(not message_result.message)

    return 0


@test_case("call_other_contract")
def main(env, logger):
    contract_a_file_path = os.path.join(CONTRACTS_FOLDER, "call_other_contract", "a.sol")
    if not os.path.exists(contract_a_file_path):
        TEST_CHECK(False, message="Contracts folder was not found")

    contract_b_file_path = os.path.join(CONTRACTS_FOLDER, "call_other_contract", "b.sol")
    if not os.path.exists(contract_b_file_path):
        TEST_CHECK(False, message="Contracts folder was not found")

    node_settings = Node.Settings(Node.Id(20216, 50066), start_up_time=2)
    with Node(env, node_settings, logger) as node:
        node.run_check_test()

        contracts_a = node.compile_contract(code=contract_a_file_path)
        test_contract_a = "A"

        contracts_b = node.compile_contract(code=contract_b_file_path)
        test_contract_b = "B"

        distributor_address = node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)
        deployed_contract_a = node.push_contract(from_address=distributor_address, code=test_contract_a, amount=0,
                                                 fee=10000000, init_message="", timeout=5)

        deployed_contract_b = node.push_contract(from_address=distributor_address, code=test_contract_b, amount=0,
                                                 fee=10000000, init_message="", timeout=5)

        arg1 = 5
        arg2 = 6
        call_message = node.encode_message(code=test_contract_b,
                                           message=f"doYourThing(Address({deployed_contract_a.address}), {arg1}, {arg2})")
        message_result = node.message_to_contract(from_address=distributor_address, to_address=deployed_contract_b,
                                                  fee=10000000, amount=0, message=call_message, timeout=5)
        res = node.decode_message(code=test_contract_b, method="doYourThing", message=message_result)
        TEST_CHECK(res['result'] == (arg1 + arg2))

    return 0
