#include <boost/test/unit_test.hpp>

#include "base/database.hpp"

BOOST_AUTO_TEST_CASE(data_base_test_1)
{
    std::filesystem::path path_to_data_base_folder("local_test_base");

    base::Bytes target_bytes(
        "sgfabvduflalfgfdnjknv fvfdvdbgdyfuobv7359t4-79898^#$%@#%&^*(*)afua   lcjnajfhvbadg ksd weufib34g 8vb");
    base::Bytes target_key("test key");
    auto data_base = base::createClearDatabaseInstance(path_to_data_base_folder);

    data_base->put(target_key, target_bytes);

    BOOST_CHECK(data_base->exists(target_key));

    data_base->remove(target_key);

    BOOST_CHECK(!data_base->exists(target_key));

    data_base->put(target_key, target_bytes);

    BOOST_CHECK(data_base->exists(target_key));

    base::Bytes target_bytes_2;
    data_base->get(target_key, target_bytes_2);

    BOOST_CHECK_EQUAL(target_bytes.toString(), target_bytes_2.toString());

    std::filesystem::remove_all(path_to_data_base_folder);
}