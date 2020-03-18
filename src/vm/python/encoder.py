import os
import json
import argparse

import web3


def generate_call(compiled_sol, call):
    web3_interface = web3.Web3()
    contract = web3_interface.eth.contract(
        abi=compiled_sol['metadata']['output']['abi'], bytecode=compiled_sol['bytecode'])

    if call['method'] == "constructor":
        call_data = contract.constructor(*call["args"]).data_in_transaction
        call_data = call_data[len(compiled_sol['bytecode']):]
    else:
        call_data = contract.encodeABI(
            fn_name=call["method"], args=call["args"])

    return call_data


def load(path_to_contract_folder):
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
    argument_data = f"[{call_string[bracket_index+1: len(call_string) - 1]}]"
    arguments = json.loads(argument_data)
    return {"method": method_name, "args": arguments}


if __name__ == "__main__":
    argument_parser = argparse.ArgumentParser()
    argument_parser.add_argument(
        '--contract_path', type=str, help='path to compiled contract folder')
    argument_parser.add_argument(
        '--call', type=parse_call, help='call function code')
    args = argument_parser.parse_args()
    contract_data = load(args.contract_path)
    call_data = generate_call(contract_data, args.call)
    if call_data:
        print(call_data[2:])
    else:
        exit(2)
