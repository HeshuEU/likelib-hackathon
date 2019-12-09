#include <boost/test/unit_test.hpp>

#include "base/database.hpp"

BOOST_AUTO_TEST_CASE(data_base_interafce_type_check_convertion)
{
    base::BaseDataType target_type = base::BaseDataType::TRANSACTION;
    base::Bytes target_bytes(std::string("hfsdagblsafghlw5q34tfgbsfnewafb*&R23gbv"));

    base::DatabaseKey key_1(target_type, target_bytes);
    // BOOST_CHECK_EQUAL(target_type, key_1.type());
    if(target_type == key_1.type()) {
        BOOST_CHECK(true);
    }
    else {
        BOOST_CHECK(false);
    }
    // BOOST_CHECK_EQUAL(target_bytes, key_1.key());
    if(target_bytes == key_1.key()) {
        BOOST_CHECK(true);
    }
    else {
        BOOST_CHECK(false);
    }
    auto temp = key_1.toBytes();

    base::DatabaseKey key_2(temp);
    // BOOST_CHECK_EQUAL(target_type, key_2.type());
    if(target_type == key_2.type()) {
        BOOST_CHECK(true);
    }
    else {
        BOOST_CHECK(false);
    }
    // BOOST_CHECK_EQUAL(target_bytes, key_2.key());
    if(target_bytes == key_2.key()) {
        BOOST_CHECK(true);
    }
    else {
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_CASE(data_base_test_1)
{
    std::filesystem::path path_to_data_base_folder("./local_test_base");

    if(std::filesystem::exists(path_to_data_base_folder)) {
        std::filesystem::remove_all(path_to_data_base_folder);
    }

    base::Database data_base(path_to_data_base_folder);
    base::DatabaseKey target_key(base::BaseDataType::BLOCK, base::Bytes("testBlockKey"));
    base::Bytes target_bytes(
        "sgfabvduflalfgfdnjknv fvfdvdbgdyfuobv7359t4-79898^#$%@#%&^*(*)afua   lcjnajfhvbadg ksd weufib34g 8vb");

    data_base.put(target_key, target_bytes);

    BOOST_CHECK(data_base.exists(target_key));

    data_base.remove(target_key);

    BOOST_CHECK(!data_base.exists(target_key));

    data_base.put(target_key, target_bytes);

    BOOST_CHECK(data_base.exists(target_key));

    base::Bytes target_bytes_2 = data_base.get(target_key);

    BOOST_CHECK_EQUAL(target_bytes.toString(), target_bytes_2.toString());
}