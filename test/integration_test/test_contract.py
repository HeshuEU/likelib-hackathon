import os

from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL, get_distributor_address_path, \
    ClientType, TransactionStatusCode


@test_case("push_contract_grpc_legacy")
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

    node_id = Id(20301, grpc_port=50301)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]
    target_value = 8888
    test_contract_init_message = client.encode_message(code=test_contract, message=f"constructor({target_value})")
    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    deployed_contract_status = client.push_contract(from_address=distributor_address, code=test_contract, amount=0,
                                                    fee=100000, init_message=test_contract_init_message)

    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=deployed_contract_status))
    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
#    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)

    call_message = client.encode_message(code=test_contract, message="get()")
    contract_address = deployed_contract_status.data
    message_call_status = client.message_call(from_address=distributor_address, to_address=contract_address,
                                              fee=100000, amount=0, message=call_message)

    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=message_call_status))

    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
#    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    contract_data_message = message_call_status.data
    res = client.decode_message(code=test_contract, method="get", message=contract_data_message)
    TEST_CHECK(res['data'] == target_value)

    message_result = client.call_view(from_address=distributor_address, to_address=contract_address,
                                      message=call_message)
    res = client.decode_message(code=test_contract, method="get", message=message_result)
    TEST_CHECK(res['data'] == target_value)

    return 0


@test_case("push_contract_http_legacy")
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

    node_id = Id(20302, http_port=50302)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_HTTP, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]
    target_value = 8888
    test_contract_init_message = client.encode_message(code=test_contract, message=f"constructor({target_value})")
    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    deployed_contract_status = client.push_contract(from_address=distributor_address, code=test_contract, amount=0,
                                                    fee=100000, init_message=test_contract_init_message)

    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=deployed_contract_status))

    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
#    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)

    call_message = client.encode_message(code=test_contract, message="get()")
    contract_address = deployed_contract_status.data
    message_call_status = client.message_call(from_address=distributor_address, to_address=contract_address,
                                              fee=100000, amount=0, message=call_message)

    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=message_call_status))

    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
#    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    contract_data_message = message_call_status.data
    res = client.decode_message(code=test_contract, method="get", message=contract_data_message)
    TEST_CHECK(res['data'] == target_value)

    message_result = client.call_view(from_address=distributor_address, to_address=contract_address,
                                      message=call_message)
    res = client.decode_message(code=test_contract, method="get", message=message_result)
    TEST_CHECK(res['data'] == target_value)

    return 0


@test_case("push_contract_http_python")
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

    node_id = Id(20303, http_port=50303)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.PYTHON_HTTP, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]
    target_value = 8888
    test_contract_init_message = client.encode_message(code=test_contract, message=f"constructor({target_value})")
    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    deployed_contract_status = client.push_contract(from_address=distributor_address, code=test_contract, amount=0,
                                                    fee=100000, init_message=test_contract_init_message)

    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=deployed_contract_status))

    deployed_contract_status = client.get_transaction_status(tx_hash=deployed_contract_status.tx_hash)
#    TEST_CHECK_EQUAL(deployed_contract_status.status_code, TransactionStatusCode.SUCCESS)

    call_message = client.encode_message(code=test_contract, message="get()")
    contract_address = deployed_contract_status.data
    message_call_status = client.message_call(from_address=distributor_address, to_address=contract_address,
                                              fee=100000, amount=0, message=call_message)

    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.PENDING)
    TEST_CHECK(client.transaction_success_wait(transaction=message_call_status))

    message_call_status = client.get_transaction_status(tx_hash=message_call_status.tx_hash)
#    TEST_CHECK_EQUAL(message_call_status.status_code, TransactionStatusCode.SUCCESS)

    contract_data_message = message_call_status.data
    res = client.decode_message(code=test_contract, method="get", message=contract_data_message)
    TEST_CHECK(res['data'] == target_value)

    message_result = client.call_view(from_address=distributor_address, to_address=contract_address,
                                      message=call_message)
    res = client.decode_message(code=test_contract, method="get", message=message_result)
    TEST_CHECK(res['data'] == target_value)

    return 0
