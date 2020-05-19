import os

from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL, get_distributor_address_path, \
    ClientType


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
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]
    target_value = 8888
    test_contract_init_message = client.encode_message(code=test_contract, message=f"constructor({target_value})")
    distributor_address = client.load_address(keys_path=get_distributor_address_path())
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
    client = env.get_client(ClientType.LEGACY_HTTP, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]
    target_value = 8888
    test_contract_init_message = client.encode_message(code=test_contract, message=f"constructor({target_value})")
    distributor_address = client.load_address(keys_path=get_distributor_address_path())
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
    client = env.get_client(ClientType.PYTHON_HTTP, node_id)
    TEST_CHECK(client.connection_test())

    contracts = client.compile_file(code=contract_file_path)
    test_contract = contracts[0]
    target_value = 8888
    test_contract_init_message = client.encode_message(code=test_contract, message=f"constructor({target_value})")
    distributor_address = client.load_address(keys_path=get_distributor_address_path())
    deployed_contract = client.push_contract(from_address=distributor_address, code=test_contract, amount=0,
                                             fee=100000, init_message=test_contract_init_message)

    call_message = client.encode_message(code=test_contract, message="get()")
    message_result = client.message_call(from_address=distributor_address, to_address=deployed_contract.address,
                                         fee=100000, amount=0, message=call_message)

    res = client.decode_message(code=test_contract, method="get", message=message_result.message)
    TEST_CHECK(res['data'] == target_value)

    message_result = client.call_view(from_address=distributor_address, to_address=deployed_contract.address,
                                      message=call_message)
    res = client.decode_message(code=test_contract, method="get", message=message_result)
    TEST_CHECK(res['data'] == target_value)

    return 0


import hashlib
import coincurve
import base64


@test_case("temp")
def main(env: Env):
    target = "gNGef0RHsbMnXnULJAV9hT6zJzf2Awv+GciKrXitRuQNBIBKJywAs7gJS4WEWm2FbApfgrIVAguJb/mmWoaRKgE="
    temp = "bUzmPA=="
    string_for_hash = "49cfqVfB1gTGw5XZSu6nZDrntLr1" + "3TjeuoZkrmxFDiGaMe4CEwsPZ7KZ" + str(0) + str(100000) + str(
        1589904067) + temp

    m = hashlib.sha256()
    m.update(string_for_hash.encode())
    hash_data = m.digest()
    from_address_private_key = coincurve.PrivateKey.from_hex(
        "2aef91bc6d2df7c41bd605caa267e8d357e18b741c4a785e06650d649d650409")

    sign = base64.b64encode(from_address_private_key.sign_recoverable(hash_data)).decode()
    TEST_CHECK_EQUAL(target, sign)

    temp2 = bytes.fromhex(
        "6a714c8300000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000002")
    string_for_hash2 = "4XXBBPDk6SpLxaRZstg6B9rPrPnQ" + "2iV6HUM5ELNDFeWBCQnneSJ6nVDQ" + str(0) + str(20000) + str(
        1589873155) + base64.b64encode(temp2).decode()

    print(string_for_hash2)
    m = hashlib.sha256()
    m.update(string_for_hash2.encode())
    hash_data2 = m.digest()
    print(hash_data2.hex())

    from_address_private_key2 = coincurve.PrivateKey.from_hex(
        "43d5d6f90fbe17c53e803a9ff8d5f419dc5553efaaddd4a20f9bb2f36b306933")

    sign2 = base64.b64encode(from_address_private_key2.sign_recoverable(hash_data2)).decode()
    print(sign2)

    return 0
