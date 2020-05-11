from tester import test_case, Node, Settings, Id, TEST_CHECK, TEST_CHECK_EQUAL


@test_case("grpc_connection")
def main(env, logger):
    node_settings = Settings(Id(20205, grpc_port=50055))
    with Node(env, node_settings, logger) as node:
        TEST_CHECK(node.connection_test())
    return 0


@test_case("http_connection")
def main(env, logger):
    node_settings = Settings(Id(20206, http_port=50056))
    with Node(env, node_settings, logger) as node:
        TEST_CHECK(node.connection_test(is_http=True))
    return 0


@test_case("node_info_grpc_connection")
def main(env, logger):
    node_settings = Settings(Id(20207, grpc_port=50057))
    with Node(env, node_settings, logger) as node:
        info = node.node_info()
        TEST_CHECK_EQUAL(info.top_block_number, 0)
    return 0


@test_case("node_info_http_connection")
def main(env, logger):
    node_settings = Settings(Id(20208, http_port=50058))
    with Node(env, node_settings, logger) as node:
        info = node.node_info(is_http=True)
        TEST_CHECK_EQUAL(info.top_block_number, 0)
    return 0
