import os

#from tester import test_case, Node, TEST_CHECK
from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL,\
                   ClientType, get_distributor_address_path, TransactionStatusCode


CONTRACTS_FOLDER = os.path.realpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "doc", "contracts"))

if not os.path.exists(CONTRACTS_FOLDER):
    print("Contracts folder was not found", flush=True)
    exit(1)


@test_case("get_balance_contract_grpc")
def main(env: Env) -> int:
    contract_text = '''
pragma solidity >=0.4.0 <0.7.0;

contract Balance {

    function getBalance() public view returns (uint256 balance){
        return msg.sender.balance;
    }

}
'''
    contract_file_path = os.path.abspath("contract.sol")
    with open(contract_file_path, "wt", encoding='utf8') as f:
        f.write(contract_text)

    node_id = Id(20301, grpc_port=50301)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]

    distributor_address = client.load_address(keys_path=get_distributor_address_path())

    deployed_contract_status = client.push_contract(from_address=distributor_address,
                                                    code=test_contract, amount=0, fee=100000,
                                                    init_message="")

    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)

    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)

    current_balance = client.get_balance(address=distributor_address.address)
    gas_for_call = 10000000

    call_message = client.encode_message(code=test_contract, message="getBalance()")
    contract_address = deployed_contract_status.data
    message_call_status = client.message_call(from_address=distributor_address,
                                              to_address=contract_address, fee=gas_for_call,
                                              amount=0, message=call_message)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    contract_data_message = message_call_status.data
    res = client.decode_message(code=test_contract, method="getBalance", message=contract_data_message)
    TEST_CHECK(res['balance'] + gas_for_call == current_balance)

    return 0

@test_case("address_send_contract")
def main(env: Env) -> int:
    contract_text = '''
pragma solidity >=0.4.0 <0.7.0;

contract AddressSend {
    uint256 coins_store;
    address payable minter;

    constructor() public {
        minter = msg.sender;
    }

    function testAddressSend(uint256 amount) public payable returns(bool is_success) {
        coins_store += msg.value;
        if(coins_store >= amount){
            bool result = minter.send(coins_store);
            if(result){
                coins_store = 0;
            }
	    return result;
        }
        return true;
    }

}
'''
    contract_file_path = os.path.abspath("contract.sol")
    with open(contract_file_path, "wt", encoding='utf8') as f:
        f.write(contract_text)

    node_id = Id(20217, grpc_port=50067)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]

    distributor_address = client.load_address(keys_path=get_distributor_address_path())

    deployed_contract_status = client.push_contract(from_address=distributor_address,
                                                    code=test_contract, amount=0, fee=100000,
                                                    init_message="")

    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)

    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)

    current_balance = client.get_balance(address=distributor_address.address)

    new_test_account = client.generate_keys(keys_path="account_1")

    initializing_amount = 100000000
    transaction = client.transfer(to_address=new_test_account.address, amount=initializing_amount,
                         from_address=distributor_address, fee=0)
    TEST_CHECK_EQUAL(transaction.status_code, TransactionStatusCode.PENDING)
    stat = client.get_transaction_status(tx_hash=transaction.tx_hash)
    TEST_CHECK_EQUAL(stat.status_code, TransactionStatusCode.SUCCESS)

    TEST_CHECK_EQUAL(client.get_balance(address=new_test_account.address), initializing_amount)

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]

    deployed_contract_status = client.push_contract(from_address=new_test_account,
                                                    code=test_contract, amount=0, fee=100000,
                                                    init_message="", timeout=5)
    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)
    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)

    current_balance = client.get_balance(address=new_test_account.address)

    gas_for_call = 10000000
    amount_for_call = 5000000

    call_message = client.encode_message(code=test_contract, message=f"testAddressSend({amount_for_call})")
    contract_address = deployed_contract_status.data
    message_call_status = client.message_call(from_address=distributor_address,
                                              to_address=contract_address, fee=gas_for_call,
                                              amount=amount_for_call, message=call_message, timeout=5)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    res = client.decode_message(code=test_contract, method="testAddressSend", message=message_call_status.data)
    TEST_CHECK(res['is_success'])

    TEST_CHECK_EQUAL(client.get_balance(address=new_test_account.address, timeout=2, wait=1),
                    current_balance + amount_for_call)

    return 0

#next
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
