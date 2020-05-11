from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL


@test_case("grpc_connection")
def main(env: Env) -> int:
    node_id = Id(20205, grpc_port=50055)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_GRPC_TYPE, node_id)
    TEST_CHECK(client.connection_test())
    return 0


@test_case("http_connection")
def main(env: Env) -> int:
    node_id = Id(20206, http_port=50056)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_HTTP_TYPE, node_id)
    TEST_CHECK(client.connection_test())
    return 0


@test_case("node_info_grpc_connection")
def main(env: Env) -> int:
    node_id = Id(20207, grpc_port=50057)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_GRPC_TYPE, node_id)
    info = client.node_info()
    TEST_CHECK_EQUAL(info.top_block_number, 0)
    return 0


@test_case("node_info_http_connection")
def main(env: Env) -> int:
    node_id = Id(20208, http_port=50058)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_HTTP_TYPE, node_id)
    info = client.node_info()
    TEST_CHECK_EQUAL(info.top_block_number, 0)
    return 0
