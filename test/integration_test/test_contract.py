import os

from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL, DISTRIBUTOR_ADDRESS_PATH


@test_case("push_contract")
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
