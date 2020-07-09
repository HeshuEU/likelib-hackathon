import os
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
    res = client.decode_message(code=test_contract, message=contract_data_message)
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

    res = client.decode_message(code=test_contract, message=message_call_status.data)
    TEST_CHECK(res['is_success'])

    TEST_CHECK_EQUAL(client.get_balance(address=new_test_account.address, timeout=2, wait=1),
                    current_balance + amount_for_call)

    return 0

@test_case("get_previous_block_contract")
def main(env: Env) -> int:
    contract_text = '''
pragma solidity >=0.4.0 <0.7.0;

contract GetPreviousBlock {

    function get() public view returns (bytes32 previous_block_hash) {
        return blockhash(block.number - 1);
    }

}
'''
    contract_file_path = os.path.abspath("contract.sol")
    with open(contract_file_path, "wt", encoding='utf8') as f:
        f.write(contract_text)

    node_id = Id(20218, grpc_port=50068)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]

    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    deployed_contract_status = client.push_contract(from_address=distributor_address,
                                                    code=test_contract, amount=0, fee=10000000,
                                                    init_message="")
    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)
    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)


    for i in range(0,1):
      current_block_hash = client.node_info().top_block_hash
      contract_address = deployed_contract_status.data
      call_message = client.encode_message(code=test_contract, message="get()")
      message_call_status = client.message_call(from_address=distributor_address,
                                              to_address=contract_address, fee=10000000,
                                              amount=0, message=call_message, timeout=5)
      TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
      message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
      TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

      contract_data_message = message_call_status.data
      res = client.decode_message(code=test_contract, message=contract_data_message)
      TEST_CHECK(res["previous_block_hash"] == current_block_hash)

    call_message = client.encode_message(code=test_contract, message="get()")
    message_call_status = client.message_call(from_address=distributor_address,
                                              to_address=contract_address, fee=10000000,
                                              amount=0, message=call_message, timeout=5)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    contract_data_message = message_call_status.data
    res = client.decode_message(code=test_contract, message=contract_data_message)
    TEST_CHECK(res["previous_block_hash"] != current_block_hash)

    return 0

@test_case("selfdestruct_contract")
def main(env: Env) -> int:
    contract_text = '''
pragma solidity >=0.4.0 <0.7.0;

contract PayMe {
    uint256 coins_store;
    address payable target;

    constructor(address payable addr) public payable{
        coins_store = msg.value;
        target = addr;
    }

    function payMe() public payable returns(uint256 current_balance){
        coins_store += msg.value;
        return coins_store;
    }

    function initDelete() public {
        selfdestruct(target);
    }

}
'''
    contract_file_path = os.path.abspath("contract.sol")
    with open(contract_file_path, "wt", encoding='utf8') as f:
        f.write(contract_text)

    node_id = Id(20219, grpc_port=50069)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]

    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    new_account = client.generate_keys(keys_path="new_account")
    init_message = client.encode_message(code=test_contract, message=f"constructor(Address({new_account.address}))")
    deployed_contract_status = client.push_contract(from_address=distributor_address,
                                                    code=test_contract, amount=50, fee=10000000,
                                                    init_message=init_message, timeout=7)
    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)
    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)

    amount_for_call = 100000

    pay_message = client.encode_message(code=test_contract, message="payMe()")

    contract_address = deployed_contract_status.data
    message_call_status = client.message_call(from_address=distributor_address,
                                              to_address=contract_address, fee=10000000,
                                              amount=amount_for_call, message=pay_message, timeout=7)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    contract_data_message = message_call_status.data
    res = client.decode_message(code=test_contract, message=contract_data_message)
    TEST_CHECK(client.get_balance(address=contract_address, timeout=2, wait=1) == res["current_balance"])

    message_call_status = client.message_call(from_address=distributor_address,
                                              to_address=contract_address, fee=10000000,
                                              amount=amount_for_call, message=pay_message, timeout=7)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    contract_data_message = message_call_status.data
    res = client.decode_message(code=test_contract, message=contract_data_message)
    TEST_CHECK(client.get_balance(address=contract_address, timeout=2, wait=1) ==
                    res["current_balance"])
    TEST_CHECK(0 == client.get_balance(address=new_account.address, timeout=2, wait=1))
    last_balance = client.get_balance(address=contract_address)
    delete_message = client.encode_message(code=test_contract, message="initDelete()")

    message_call_status = client.message_call(from_address=distributor_address,
                                              to_address=contract_address, fee=10000000,
                                              amount=0, message=delete_message, timeout=7)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    TEST_CHECK(last_balance == client.get_balance(address=new_account.address))
    TEST_CHECK(0 == client.get_balance(address=contract_address))

    message_call_status = client.message_call(from_address=distributor_address,
                                              to_address=contract_address, fee=10000000,
                                              amount=1, message=pay_message, timeout=7)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    return 0

@test_case("call_other_contract")
def main(env: Env) -> int:
    contract_a_text = '''
pragma solidity >=0.4.0 <0.7.0;

import "./abstract_a.sol";

contract A is AbstractA {
    function sum(uint arg1, uint arg2) external override returns(uint) {
        return arg1 + arg2;
    }
}
'''
    contract_b_text = '''
pragma solidity >=0.4.0 <0.7.0;

import "./abstract_a.sol";

contract B {
    function doYourThing(address addressOfA, uint arg1, uint arg2) public returns (uint result) {
        AbstractA my_a = AbstractA(addressOfA);
        return my_a.sum(arg1, arg2);
    }
}
'''
    contract_abstract_a_text = '''
pragma solidity >=0.4.0 <0.7.0;

interface AbstractA {
    function sum(uint arg1, uint arg2) external returns(uint);
}
'''
    contract_a_file_path = os.path.abspath("a.sol")
    with open(contract_a_file_path, "wt", encoding='utf8') as f:
        f.write(contract_a_text)
    contract_b_file_path = os.path.abspath("b.sol")
    with open(contract_b_file_path, "wt", encoding='utf8') as f:
        f.write(contract_b_text)
    contract_abstract_a_file_path = os.path.abspath("abstract_a.sol")
    with open(contract_abstract_a_file_path, "wt", encoding='utf8') as f:
        f.write(contract_abstract_a_text)

    node_id = Id(20216, grpc_port=50066)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    TEST_CHECK(client.connection_test())

    contracts_a = client.compile_file(code=contract_a_file_path)
    test_contract_a = "A"

    contracts_b = client.compile_file(code=contract_b_file_path)
    test_contract_b = "B"

    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    deployed_contract_a_status = client.push_contract(from_address=distributor_address,
                                                    code=test_contract_a, amount=0, fee=10000000,
                                                    init_message="", timeout=5)
    TEST_CHECK_EQUAL(deployed_contract_a_status.status_code, TransactionStatusCode.PENDING)
    deployed_contract_a_status = client.get_transaction_status(tx_hash=deployed_contract_a_status.tx_hash)
    TEST_CHECK_EQUAL(deployed_contract_a_status.status_code, TransactionStatusCode.SUCCESS)

    deployed_contract_b_status = client.push_contract(from_address=distributor_address,
                                                    code=test_contract_b, amount=0, fee=10000000,
                                                    init_message="", timeout=5)
    TEST_CHECK_EQUAL(deployed_contract_b_status.status_code, TransactionStatusCode.PENDING)
    deployed_contract_b_status = client.get_transaction_status(tx_hash=deployed_contract_b_status.tx_hash)
    TEST_CHECK_EQUAL(deployed_contract_b_status.status_code, TransactionStatusCode.SUCCESS)

    arg1 = 5
    arg2 = 6
    call_message = client.encode_message(code=test_contract_b,
                      message=f"doYourThing(Address({deployed_contract_a_status.data}), {arg1}, {arg2})")

    message_call_status = client.message_call(from_address=distributor_address,
                                              to_address=deployed_contract_b_status.data, fee=10000000,
                                              amount=0, message=call_message, timeout=5)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    contract_data_message = message_call_status.data
    res = client.decode_message(code=test_contract_b, message=contract_data_message)
    TEST_CHECK(res['result'] == (arg1 + arg2))

    return 0
