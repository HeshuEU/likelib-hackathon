from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL


@test_case("legacy_grpc_connection")
def main(env: Env) -> int:
    node_id = Id(20205, grpc_port=50055)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_GRPC_TYPE, node_id)
    TEST_CHECK(client.connection_test())
    return 0


@test_case("legacy_http_connection")
def main(env: Env) -> int:
    node_id = Id(20206, http_port=50056)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_HTTP_TYPE, node_id)
    TEST_CHECK(client.connection_test())
    return 0


@test_case("http_connection")
def main(env: Env) -> int:
    node_id = Id(20207, http_port=50057)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_PYTHON_HTTP_TYPE, node_id)
    TEST_CHECK(client.connection_test())
    return 0


@test_case("node_info_legacy_grpc_connection")
def main(env: Env) -> int:
    node_id = Id(20208, grpc_port=50058)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_GRPC_TYPE, node_id)
    info = client.node_info()
    TEST_CHECK_EQUAL(info.top_block_number, 0)
    return 0


@test_case("node_info_legacy_http_connection")
def main(env: Env) -> int:
    node_id = Id(20209, http_port=50059)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_LEGACY_HTTP_TYPE, node_id)
    info = client.node_info()
    TEST_CHECK_EQUAL(info.top_block_number, 0)
    return 0


@test_case("node_info_http_connection")
def main(env: Env) -> int:
    node_id = Id(20210, http_port=50060)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(env.CLIENT_PYTHON_HTTP_TYPE, node_id)
    info = client.node_info()
    TEST_CHECK_EQUAL(info.top_block_number, 0)
    return 0
