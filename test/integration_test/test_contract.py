import os

from tester import test_case, Node, TEST_CHECK


@test_case("push_contract")
def main(env, logger):
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

    node_settings = Node.Settings(Node.Id(20205, 50055), start_up_time=2)
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
        TEST_CHECK(res['data'] == target_value)

    return 0
