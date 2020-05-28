from tester import test_case, Env, NodeConfig, Id, TEST_CHECK, TEST_CHECK_EQUAL, ClientType


@test_case("connection_legacy_grpc")
def main(env: Env) -> int:
    node_id = Id(20101, grpc_port=50101)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    TEST_CHECK(client.connection_test())
    return 0


@test_case("connection_legacy_http")
def main(env: Env) -> int:
    node_id = Id(20102, http_port=50102)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_HTTP, node_id)
    TEST_CHECK(client.connection_test())
    return 0


@test_case("connection_python_http")
def main(env: Env) -> int:
    node_id = Id(20103, http_port=50103)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.PYTHON_HTTP, node_id)
    TEST_CHECK(client.connection_test())
    return 0


@test_case("get_node_info_legacy_grpc")
def main(env: Env) -> int:
    node_id = Id(20104, grpc_port=50104)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_GRPC, node_id)
    info = client.node_info()
    TEST_CHECK_EQUAL(info.top_block_number, 0)
    return 0


@test_case("get_node_info_legacy_http")
def main(env: Env) -> int:
    node_id = Id(20105, http_port=50105)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.LEGACY_HTTP, node_id)
    info = client.node_info()
    TEST_CHECK_EQUAL(info.top_block_number, 0)
    return 0


@test_case("get_node_info_python_http")
def main(env: Env) -> int:
    node_id = Id(20106, http_port=50106)
    env.start_node(NodeConfig(node_id))
    client = env.get_client(ClientType.PYTHON_HTTP, node_id)
    info = client.node_info()
    TEST_CHECK_EQUAL(info.top_block_number, 0)
    return 0
