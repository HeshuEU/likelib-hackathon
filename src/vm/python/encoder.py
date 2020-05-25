import os
import json
import argparse
import copy
import web3


def create_hash(function):
    name = function.abi['name']
    str_view = str(function)
    signature = str_view[str_view.find(" ") + 1: len(str_view) - 1]
    sha3_hash = web3.Web3.solidityKeccak(
        ['bytes'], [signature.encode('ascii')]).hex()
    return name, signature, sha3_hash[:10]


def encode_call(compiled_sol, call):

    new_contract_data_abi = copy.deepcopy(compiled_sol['metadata']['output']['abi'])
    for item in new_contract_data_abi:
        for input_item in item["inputs"]:
            if input_item["type"] == "address":
                input_item["internalType"] = "bytes32"
                input_item["type"] = "bytes32"

    new_contract = web3.Web3().eth.contract(
        abi=new_contract_data_abi, bytecode=compiled_sol['bytecode'])

    if call['method'] == "constructor":
        call_data = new_contract.constructor(*call["args"]).data_in_transaction
    else:
        call_data = new_contract.encodeABI(fn_name=call["method"], args=call["args"])

        original = web3.Web3().eth.contract(abi=compiled_sol['metadata']['output']['abi'])
        target_hash = create_hash(original.get_function_by_name(call["method"]))
        call_data = target_hash[2] + call_data[10:]

    return call_data


def load_contract_data(path_to_contract_folder):
    compiled_contract_file_path = os.path.join(
        path_to_contract_folder, "compiled_code.bin")
    if not os.path.exists(compiled_contract_file_path):
        raise Exception(
            f"contract file not exists by path: {compiled_contract_file_path}")
    with open(compiled_contract_file_path, "rt") as f:
        bytecode = f.read()

    contract_metadata_file_path = os.path.join(
        path_to_contract_folder, "metadata.json")
    if not os.path.exists(contract_metadata_file_path):
        raise Exception(
            f"contract metadata file not exists by path: {contract_metadata_file_path}")
    with open(contract_metadata_file_path, "rt") as f:
        metadata = json.loads(f.read())

    return {"bytecode": bytecode, "metadata": metadata}


def parse_call(call_string):
    bracket_index = call_string.find('(')
    method_name = call_string[0: bracket_index]
    argument_data = f"[{call_string[bracket_index + 1: len(call_string) - 1]}]"
    arguments = json.loads(argument_data)
    return {"method": method_name, "args": arguments}


if __name__ == "__main__":
    argument_parser = argparse.ArgumentParser()
    argument_parser.add_argument('--contract_path', help='path to compiled contract folder')
    argument_parser.add_argument('--call', help='call function code')
    args = argument_parser.parse_args()

    contract_data = load_contract_data(args.contract_path)
    parsed_call = parse_call(args.call)

    call_data = encode_call(contract_data, parsed_call)
    if call_data:
        print(call_data[2:])
    else:
        exit(2)
