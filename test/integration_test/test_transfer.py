from tester import test_case, Node


@test_case("base_transfer")
def main(logger, node_exec_path, client_exec_path, evm_path):
    node_settings = Node.Settings(
        node_exec_path, client_exec_path, evm_path, Node.Id(20206, 50056))
    with Node(node_settings, logger) as node:
        node.run_check_test()

        target_address = node.create_new_address("keys1")
        node.run_check_balance(target_address, 0)

        distriburot_address = node.load_address(node.DISTRIBUTOR_ADDRESS_PATH)
        amount = 333
        node.run_check_transfer(to_address=target_address, amount=amount,
                                from_address=distriburot_address, fee=0)
        node.run_check_balance(target_address, amount)
    return 0
