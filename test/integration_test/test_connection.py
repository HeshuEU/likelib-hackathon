from tester import test_case, Node


@test_case("connection")
def main(env, logger):
    node_settings = Node.Settings(Node.Id(20205, 50055))
    with Node(env, node_settings, logger) as node:
        node.run_check_test()
    return 0
