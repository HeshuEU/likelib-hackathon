from tester import test_case, Node


@test_case("base_transfer")
def main(env, logger):
    node_settings = Node.Settings(Node.Id(20206, 50056))
    with Node(env, node_settings, logger) as node:
        node.run_check_test()

        target_address = node.create_new_address(keys_path="keys1")
        node.run_check_balance(target_address, 0)

        distributor_address = node.load_address(keys_path=node.DISTRIBUTOR_ADDRESS_PATH)
        amount = 333
        node.run_check_transfer(to_address=target_address, amount=amount,
                                from_address=distributor_address, fee=0, timeout=2)
        node.run_check_balance(target_address, amount)
    return 0
