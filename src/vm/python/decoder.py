import os
import json
import argparse
import binascii

import web3


def create_hash(function):
    name = function.abi['name']
    str_view = str(function)
    signature = str_view[str_view.find(" ") + 1: len(str_view) - 1]
    sha3_hash = web3.Web3.solidityKeccak(
        ['bytes'], [signature.encode('ascii')]).hex()
    return name, signature, sha3_hash[2:10]


def decode_output(compiled_sol, method, data):
    web3_interface = web3.Web3()

    for abi_fn in compiled_sol['metadata']['output']['abi']:
        if abi_fn["type"] == "function":
            if abi_fn["name"] == method:
                remake_abi = abi_fn
                remake_abi['inputs'] = remake_abi['outputs']
                remake_abi['outputs'] = ""
                remake_contract = web3_interface.eth.contract(abi=[remake_abi])
                target_hash = create_hash(remake_contract.get_function_by_name(method))
                return remake_contract.decode_function_input(target_hash[2] + data)[1]
    return None


def load(path_to_contract_folder):
    compiled_contract_file_path = os.path.join(
        path_to_contract_folder, "compiled_code.bin")
    if not os.path.exists(compiled_contract_file_path):
        raise Exception(f"contract file not exists by path: {compiled_contract_file_path}")
    with open(compiled_contract_file_path, "rt") as f:
        bytecode = f.read()

    contract_metadata_file_path = os.path.join(
        path_to_contract_folder, "metadata.json")
    if not os.path.exists(contract_metadata_file_path):
        raise Exception(f"contract metadata file not exists by path: {contract_metadata_file_path}")
    with open(contract_metadata_file_path, "rt") as f:
        metadata = json.loads(f.read())

    return {"bytecode": bytecode, "metadata": metadata}


def prepare_for_serialize(decoded_data):
    if type(decoded_data) is list:
        for item_num in range(len(decoded_data)):
            decoded_data[item_num] = prepare_for_serialize(decoded_data[item_num])
    elif type(decoded_data) is bytes:
        b = binascii.hexlify(decoded_data)
        return b.decode('utf8')
    elif type(decoded_data) is dict:
        for item_key in decoded_data.keys():
            decoded_data[item_key] = prepare_for_serialize(decoded_data[item_key])
    return decoded_data


if __name__ == "__main__":
    argument_parser = argparse.ArgumentParser()
    argument_parser.add_argument(
        '--contract_path', type=str, help='path to compiled contract folder')
    argument_parser.add_argument(
        '--method', type=str, help='called method')
    argument_parser.add_argument(
        '--data', type=str, help='output data from method')
    args = argument_parser.parse_args()
    contract_data = load(args.contract_path)
    decoded_data = decode_output(contract_data, args.method, args.data)
    if decoded_data:
        print(json.dumps(prepare_for_serialize(decoded_data)))
    else:
        exit(2)
