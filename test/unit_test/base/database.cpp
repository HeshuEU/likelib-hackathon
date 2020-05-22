#include <boost/test/unit_test.hpp>

#include "base/database.hpp"
#include "base/error.hpp"

BOOST_AUTO_TEST_CASE(data_base_test_1)
{
    std::filesystem::path path_to_data_base_folder("local_test_base");

    base::Bytes target_bytes(
      "sgfabvduflalfgfdnjknv fvfdvdbgdyfuobv7359t4-79898^#$%@#%&^*(*)afua   lcjnajfhvbadg ksd weufib34g 8vb");
    base::Bytes target_key("test key");
    auto data_base = base::createClearDatabaseInstance(path_to_data_base_folder);

    data_base.put(target_key, target_bytes);

    BOOST_CHECK(data_base.exists(target_key));

    data_base.remove(target_key);

    BOOST_CHECK(!data_base.exists(target_key));

    data_base.put(target_key, target_bytes);

    BOOST_CHECK(data_base.exists(target_key));

    auto target_bytes_2 = data_base.get(target_key);

    BOOST_CHECK(target_bytes_2);
    BOOST_CHECK_EQUAL(target_bytes.toString(), target_bytes_2.value().toString());

    std::filesystem::remove_all(path_to_data_base_folder);
}


BOOST_AUTO_TEST_CASE(data_base_test_with_empty_key_and_value)
{
    std::filesystem::path path_to_data_base_folder("local_test_base");

    base::Bytes bytes1("sgfabvduflalfgfdnjknv  lcjnajfhvbadg ksd weufib34g 8vb");
    base::Bytes bytes2("");
    base::Bytes bytes3("empty _ key");
    base::Bytes key1("test key");
    base::Bytes key2("empty value");
    base::Bytes key3("");

    auto data_base = base::createClearDatabaseInstance(path_to_data_base_folder);

    data_base.put(key1, bytes1);
    data_base.put(key2, bytes2);
    data_base.put(key3, bytes3);
    BOOST_CHECK(data_base.exists(key1));
    BOOST_CHECK(data_base.exists(key2));
    BOOST_CHECK(data_base.exists(key3));

    BOOST_CHECK_EQUAL(data_base.get(key1).value().toString(), bytes1.toString());
    BOOST_CHECK_EQUAL(data_base.get(key2).value().toString(), bytes2.toString());
    BOOST_CHECK_EQUAL(data_base.get(key3).value().toString(), bytes3.toString());

    std::filesystem::remove_all(path_to_data_base_folder);
}


BOOST_AUTO_TEST_CASE(data_base_double_put)
{
    std::filesystem::path path_to_data_base_folder("local_test_base");

    base::Bytes bytes1("sgfabvduflalfgfdnjknv  lcjnajfhvbadg ksd weufib34g 8vb");
    base::Bytes bytes2("");
    base::Bytes bytes3("if*^Tsdg v460");
    base::Bytes key1("test key");
    base::Bytes key2("empty value");

    auto data_base = base::createClearDatabaseInstance(path_to_data_base_folder);

    data_base.put(key1, bytes1);
    data_base.put(key2, bytes2);
    BOOST_CHECK(data_base.exists(key1));
    BOOST_CHECK(data_base.exists(key2));

    BOOST_CHECK_EQUAL(data_base.get(key1).value().toString(), bytes1.toString());
    BOOST_CHECK_EQUAL(data_base.get(key2).value().toString(), bytes2.toString());

    data_base.put(key2, bytes3); // must change its meaning
    BOOST_CHECK(data_base.exists(key2));
    BOOST_CHECK_EQUAL(data_base.get(key2).value().toString(), bytes3.toString());

    std::filesystem::remove_all(path_to_data_base_folder);
}


BOOST_AUTO_TEST_CASE(data_base_remove_not_exist_object)
{
    std::filesystem::path path_to_data_base_folder("local_test_base");

    base::Bytes bytes1("sgfabvduflalfgfdnjknv  lcjnajfhvbadg ksd weufib34g 8vb");
    base::Bytes key1("test key");

    auto data_base = base::createClearDatabaseInstance(path_to_data_base_folder);

    data_base.put(key1, bytes1);
    BOOST_CHECK(data_base.exists(key1));
    BOOST_CHECK_EQUAL(data_base.get(key1).value().toString(), bytes1.toString());

    data_base.remove(key1);
    BOOST_CHECK(!data_base.exists(key1));

    BOOST_WARN_THROW(data_base.remove(key1), base::DatabaseError);

    std::filesystem::remove_all(path_to_data_base_folder);
}


BOOST_AUTO_TEST_CASE(data_base_constructor_from_file)
{
    std::filesystem::path path_to_data_base_folder("local_test_base");

    base::Bytes bytes1("sgfabvduflalfgfdnjknv  lcjnajfhvbadg ksd weufib34g 8vb");
    base::Bytes bytes2("SGJ$( GDSN3tdgjs#)(u35");
    base::Bytes key1("test key");
    base::Bytes key2("test key too");
    base::Bytes key3("I will remove it in the future");
    {
        auto data_base = base::createClearDatabaseInstance(path_to_data_base_folder);

        data_base.put(key1, bytes1);
        data_base.put(key2, bytes2);
        data_base.put(key3, base::Bytes("rem"));
        BOOST_CHECK(data_base.exists(key1));
        BOOST_CHECK(data_base.exists(key2));
        BOOST_CHECK(data_base.exists(key3));

        data_base.remove(key3);
        BOOST_CHECK(!data_base.exists(key3));
    }

    base::Database data_base2(path_to_data_base_folder);
    BOOST_CHECK(data_base2.exists(key1));
    BOOST_CHECK(data_base2.exists(key2));
    BOOST_CHECK(!data_base2.exists(key3));

    BOOST_CHECK_EQUAL(data_base2.get(key1).value().toString(), bytes1.toString());
    BOOST_CHECK_EQUAL(data_base2.get(key2).value().toString(), bytes2.toString());

    std::filesystem::remove_all(path_to_data_base_folder);
}


BOOST_AUTO_TEST_CASE(data_base_open)
{
    std::filesystem::path path_to_data_base_folder("local_test_base");

    base::Bytes bytes1("sgfabvduflalfgfdnjknv  lcjnajfhvbadg ksd weufib34g 8vb");
    base::Bytes bytes2("SGJ$( GDSN3tdgjs#)(u35");
    base::Bytes key1("test key");
    base::Bytes key2("test key too");
    base::Bytes key3("I will remove it in the future");
    {
        auto data_base = base::createClearDatabaseInstance(path_to_data_base_folder);

        data_base.put(key1, bytes1);
        data_base.put(key2, bytes2);
        data_base.put(key3, base::Bytes("rem"));
        BOOST_CHECK(data_base.exists(key1));
        BOOST_CHECK(data_base.exists(key2));
        BOOST_CHECK(data_base.exists(key3));

        data_base.remove(key3);
        BOOST_CHECK(!data_base.exists(key3));
    }

    base::Database data_base2;
    data_base2.open(path_to_data_base_folder);
    BOOST_CHECK(data_base2.exists(key1));
    BOOST_CHECK(data_base2.exists(key2));
    BOOST_CHECK(!data_base2.exists(key3));

    BOOST_CHECK_EQUAL(data_base2.get(key1).value().toString(), bytes1.toString());
    BOOST_CHECK_EQUAL(data_base2.get(key2).value().toString(), bytes2.toString());

    std::filesystem::remove_all(path_to_data_base_folder);
}


BOOST_AUTO_TEST_CASE(data_base_createDefaultDatabaseInstance)
{
    std::filesystem::path path_to_data_base_folder("local_test_base");

    base::Bytes bytes1("sgfabvduflalfgfdnjknv  lcjnajfhvbadg ksd weufib34g 8vb");
    base::Bytes bytes2("SGJ$( GDSN3tdgjs#)(u35");
    base::Bytes key1("test key");
    base::Bytes key2("test key too");
    base::Bytes key3("I will remove it in the future");
    {
        auto data_base = base::createClearDatabaseInstance(path_to_data_base_folder);

        data_base.put(key1, bytes1);
        data_base.put(key2, bytes2);
        data_base.put(key3, base::Bytes("rem"));
        BOOST_CHECK(data_base.exists(key1));
        BOOST_CHECK(data_base.exists(key2));
        BOOST_CHECK(data_base.exists(key3));

        data_base.remove(key3);
        BOOST_CHECK(!data_base.exists(key3));
    }

    auto data_base2 = base::createDefaultDatabaseInstance(path_to_data_base_folder);
    BOOST_CHECK(data_base2.exists(key1));
    BOOST_CHECK(data_base2.exists(key2));
    BOOST_CHECK(!data_base2.exists(key3));

    BOOST_CHECK_EQUAL(data_base2.get(key1).value().toString(), bytes1.toString());
    BOOST_CHECK_EQUAL(data_base2.get(key2).value().toString(), bytes2.toString());

    std::filesystem::remove_all(path_to_data_base_folder);
}


BOOST_AUTO_TEST_CASE(data_base_with_long_path)
{
    std::filesystem::path path_to_data_base_folder("local/test/base");

    base::Bytes bytes1("sgfabvduflalfgfdnjknv  lcjnajfhvbadg ksd weufib34g 8vb");
    base::Bytes key1("test key");
    {
        auto data_base = base::createClearDatabaseInstance(path_to_data_base_folder);

        data_base.put(key1, bytes1);
        BOOST_CHECK(data_base.exists(key1));
    }

    auto data_base2 = base::createDefaultDatabaseInstance(path_to_data_base_folder);
    BOOST_CHECK(data_base2.exists(key1));

    BOOST_CHECK_EQUAL(data_base2.get(key1).value().toString(), bytes1.toString());

    std::filesystem::remove_all(path_to_data_base_folder);
}