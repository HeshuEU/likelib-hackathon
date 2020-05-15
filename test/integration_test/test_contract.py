import os

from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL, DISTRIBUTOR_ADDRESS_PATH


@test_case("grpc_legacy_push_contract")
def main(env: Env):
    contract_text = '''
pragma solidity >=0.4.0 <0.7.0;

contract SimpleStorage {
    uint storedData;

    constructor(uint x) public{
        storedData = x;
    }

    function set(uint x) public {
        storedData = x;
    }

    function get() public view returns (uint data) {
        return storedData;
    }
}

'''
    contract_file_path = os.path.abspath("contract.sol")
    with open(contract_file_path, "wt", encoding='utf8') as f:
        f.write(contract_text)

    node_id = Id(20215, grpc_port=50065)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_GRPC_TYPE, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]
    target_value = 8888
    test_contract_init_message = client.encode_message(code=test_contract, message=f"constructor({target_value})")
    distributor_address = client.load_address(keys_path=DISTRIBUTOR_ADDRESS_PATH)
    deployed_contract = client.push_contract(from_address=distributor_address, code=test_contract, amount=0,
                                             fee=10000000, init_message=test_contract_init_message)

    call_message = client.encode_message(code=test_contract, message="get()")
    message_result = client.message_call(from_address=distributor_address, to_address=deployed_contract.address,
                                         fee=10000000, amount=0, message=call_message)
    res = client.decode_message(code=test_contract, method="get", message=message_result.message)
    TEST_CHECK(res['data'] == target_value)

    message_result = client.call_view(from_address=distributor_address, to_address=deployed_contract.address,
                                      message=call_message)
    res = client.decode_message(code=test_contract, method="get", message=message_result)
    TEST_CHECK(res['data'] == target_value)

    return 0


@test_case("http_legacy_push_contract")
def main(env: Env):
    contract_text = '''
pragma solidity >=0.4.0 <0.7.0;

contract SimpleStorage {
    uint storedData;

    constructor(uint x) public{
        storedData = x;
    }

    function set(uint x) public {
        storedData = x;
    }

    function get() public view returns (uint data) {
        return storedData;
    }
}

'''
    contract_file_path = os.path.abspath("contract.sol")
    with open(contract_file_path, "wt", encoding='utf8') as f:
        f.write(contract_text)

    node_id = Id(20217, http_port=50067)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_HTTP_TYPE, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]
    target_value = 8888
    test_contract_init_message = client.encode_message(code=test_contract, message=f"constructor({target_value})")
    distributor_address = client.load_address(keys_path=DISTRIBUTOR_ADDRESS_PATH)
    deployed_contract = client.push_contract(from_address=distributor_address, code=test_contract, amount=0,
                                             fee=10000000, init_message=test_contract_init_message)

    call_message = client.encode_message(code=test_contract, message="get()")
    message_result = client.message_call(from_address=distributor_address, to_address=deployed_contract.address,
                                         fee=10000000, amount=0, message=call_message)
    res = client.decode_message(code=test_contract, method="get", message=message_result.message)
    TEST_CHECK(res['data'] == target_value)

    message_result = client.call_view(from_address=distributor_address, to_address=deployed_contract.address,
                                      message=call_message)
    res = client.decode_message(code=test_contract, method="get", message=message_result)
    TEST_CHECK(res['data'] == target_value)

    return 0


@test_case("http_python_push_contract")
def main(env: Env):
    init_message = "608060405234801561001057600080fd5b506040516101213803806101218339818101604052602081101561003357600080fd5b8101908080519060200190929190505050806000819055505060c78061005a6000396000f3fe6080604052348015600f57600080fd5b506004361060325760003560e01c806360fe47b11460375780636d4ce63c146062575b600080fd5b606060048036036020811015604b57600080fd5b8101908080359060200190929190505050607e565b005b60686088565b6040518082815260200191505060405180910390f35b8060008190555050565b6000805490509056fea264697066735822122045c91e12b3285e562afb26fd32a447ca2c5a98c5f29afa5cf61b9c84e20651c664736f6c6343000606003300000000000000000000000000000000000000000000000000000000000022b8"
    abi = '''{
    "abi": [
        {
            "inputs": [
                {
                    "internalType": "uint256",
                    "name": "x",
                    "type": "uint256"
                }
            ],
            "stateMutability": "nonpayable",
            "type": "constructor"
        },
        {
            "inputs": "",
            "name": "get",
            "outputs": [
                {
                    "internalType": "uint256",
                    "name": "data",
                    "type": "uint256"
                }
            ],
            "stateMutability": "view",
            "type": "function"
        },
        {
            "inputs": [
                {
                    "internalType": "uint256",
                    "name": "x",
                    "type": "uint256"
                }
            ],
            "name": "set",
            "outputs": "",
            "stateMutability": "nonpayable",
            "type": "function"
        }
    ]
}
'''

    node_id = Id(20216, http_port=50066)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_PYTHON_HTTP_TYPE, node_id)
    TEST_CHECK(client.connection_test())

    distributor_address = client.load_address(keys_path=DISTRIBUTOR_ADDRESS_PATH)
    deployed_contract = client.push_contract(from_address=distributor_address, code=abi, amount=0,
                                             fee=10000000, init_message=init_message)
    get_message = "6d4ce63c"
    message_result1 = client.message_call(from_address=distributor_address, to_address=deployed_contract.address,
                                          fee=10000000, amount=0, message=get_message)
    TEST_CHECK(message_result1.fee_left != 0 and message_result1.fee_left != 10000000)
    message_result2 = client.call_view(from_address=distributor_address, to_address=deployed_contract.address,
                                       message=get_message)

    TEST_CHECK_EQUAL(message_result1.message, message_result2)
    return 0
