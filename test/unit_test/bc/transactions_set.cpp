#include <boost/test/unit_test.hpp>

#include "bc/transactions_set.hpp"

#include <vector>

namespace
{

bc::Transaction trans1{bc::Address{base::Bytes("from1 vjS247DGFSv\n ").toHex()},
    bc::Address{base::Bytes("to1 ()#%DSOJ\n").toHex()}, 12398, 11, base::Time()};
bc::Transaction trans2{bc::Address{base::Bytes("from2 vj^Hs47DGFSv\n ").toHex()},
    bc::Address{base::Bytes("to2 ()#%Dsdg\n").toHex()}, 5825285, 22, base::Time::now()};
bc::Transaction trans3{bc::Address{base::Bytes("from3 vjS2%#&DGF\n ").toHex()},
    bc::Address{base::Bytes("to3 ()#%DdfOJ\n").toHex()}, 12245398, 33, base::Time()};
bc::Transaction trans4{bc::Address{base::Bytes("from4 vjS247sdgFSv\n ").toHex()},
    bc::Address{base::Bytes("to4 {#%DSOJ ").toHex()}, 168524347, 44, base::Time()};
bc::Transaction trans5{bc::Address{base::Bytes("from5 vjS2  DGFSv\n ").toHex()},
    bc::Address{base::Bytes("to5 ()#%DSdsJ\n").toHex()}, 1434457, 55, base::Time::now()};

bc::TransactionsSet getTestSet()
{
    bc::TransactionsSet set;
    set.add(trans1);
    set.add(trans2);
    set.add(trans3);
    set.add(trans4);
    set.add(trans5);
    return set;
}
} // namespace

BOOST_AUTO_TEST_CASE(transactions_set_constructor)
{
    bc::TransactionsSet tx_set;

    BOOST_CHECK(tx_set.isEmpty());
}


BOOST_AUTO_TEST_CASE(transactions_set_find)
{
    auto tx_set = getTestSet();

    BOOST_CHECK(tx_set.find(trans1));
    BOOST_CHECK(tx_set.find(trans2));
    BOOST_CHECK(tx_set.find(trans3));
    BOOST_CHECK(tx_set.find(trans4));
    BOOST_CHECK(tx_set.find(trans5));

    BOOST_CHECK(!tx_set.find(bc::Transaction(trans1.getFrom(), bc::Address{base::Bytes("()#%DSOJ\n").toHex()},
        trans1.getAmount(), 0, trans1.getTimestamp())));
    BOOST_CHECK(
        !tx_set.find(bc::Transaction(trans3.getFrom(), trans3.getTo(), trans3.getAmount(), 0, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(transactions_set_find_sha)
{
    auto tx_set = getTestSet();

    BOOST_CHECK(tx_set.find(base::Sha256::compute(base::toBytes(trans1))).value() == trans1);
    BOOST_CHECK(tx_set.find(base::Sha256::compute(base::toBytes(trans2))).value() == trans2);
    BOOST_CHECK(tx_set.find(base::Sha256::compute(base::toBytes(trans3))).value() == trans3);
    BOOST_CHECK(tx_set.find(base::Sha256::compute(base::toBytes(trans4))).value() == trans4);
    BOOST_CHECK(tx_set.find(base::Sha256::compute(base::toBytes(trans5))).value() == trans5);
}


BOOST_AUTO_TEST_CASE(transactions_set_remove)
{
    auto tx_set = getTestSet();

    tx_set.remove(trans2);
    tx_set.remove(trans5);

    BOOST_CHECK(tx_set.find(trans1));
    BOOST_CHECK(tx_set.find(trans3));
    BOOST_CHECK(tx_set.find(trans4));


    BOOST_CHECK(!tx_set.find(trans2));
    BOOST_CHECK(!tx_set.find(trans5));
}


BOOST_AUTO_TEST_CASE(transactions_set_remove_set1)
{
    auto tx_set = getTestSet();
    bc::TransactionsSet rem_set;
    rem_set.add(trans2);
    rem_set.add(trans5);

    tx_set.remove(rem_set);

    BOOST_CHECK(tx_set.find(trans1));
    BOOST_CHECK(tx_set.find(trans3));
    BOOST_CHECK(tx_set.find(trans4));


    BOOST_CHECK(!tx_set.find(trans2));
    BOOST_CHECK(!tx_set.find(trans5));
}


BOOST_AUTO_TEST_CASE(transactions_set_remove_set2)
{
    auto tx_set = getTestSet();
    auto rem_set = getTestSet();

    tx_set.remove(rem_set);

    BOOST_CHECK(!tx_set.find(trans1));
    BOOST_CHECK(!tx_set.find(trans2));
    BOOST_CHECK(!tx_set.find(trans3));
    BOOST_CHECK(!tx_set.find(trans4));
    BOOST_CHECK(!tx_set.find(trans5));
}


BOOST_AUTO_TEST_CASE(transaction_set_inEmpty)
{
    bc::TransactionsSet tx_set;
    BOOST_CHECK(tx_set.isEmpty());

    tx_set.add(bc::Transaction(
        bc::Address{base::Bytes("1").toHex()}, bc::Address{base::Bytes("2").toHex()}, 111, 0, base::Time()));
    BOOST_CHECK(!tx_set.isEmpty());

    tx_set.remove(bc::Transaction(
        bc::Address{base::Bytes("1").toHex()}, bc::Address{base::Bytes("2").toHex()}, 111, 0, base::Time()));
    BOOST_CHECK(tx_set.isEmpty());
}


BOOST_AUTO_TEST_CASE(transactions_set_iterators_usage1)
{
    auto tx_set = getTestSet();
    bool is_ok = true;
    for(auto it = tx_set.begin(); it != tx_set.end(); it++) {
        is_ok = is_ok && (tx_set.find(*it));
    }
    BOOST_CHECK(is_ok);
}


BOOST_AUTO_TEST_CASE(transactions_set_iterators_usage2)
{
    const auto tx_set = getTestSet();
    bool is_ok = true;
    for(auto it = tx_set.begin(); it != tx_set.end(); it++) {
        is_ok = is_ok && (tx_set.find(*it));
    }
    BOOST_CHECK(is_ok);
}


BOOST_AUTO_TEST_CASE(transactions_set_iterators_usage3)
{
    auto tx_set = getTestSet();
    bool is_ok = true;
    for(auto& it: tx_set) {
        is_ok = is_ok && (tx_set.find(it));
    }
    BOOST_CHECK(is_ok);
}


BOOST_AUTO_TEST_CASE(transactions_set_iterators_usage4)
{
    const auto tx_set = getTestSet();
    bool is_ok = true;
    for(auto& it: tx_set) {
        is_ok = is_ok && (tx_set.find(it));
    }
    BOOST_CHECK(is_ok);
}


BOOST_AUTO_TEST_CASE(transactions_set_serialixation)
{
    auto tx_set = getTestSet();
    base::SerializationOArchive oa;
    oa.serialize(tx_set);

    base::SerializationIArchive ia(oa.getBytes());
    auto tx_set2 = ia.deserialize<bc::TransactionsSet>();

    BOOST_CHECK(tx_set2.find(trans1));
    BOOST_CHECK(tx_set2.find(trans2));
    BOOST_CHECK(tx_set2.find(trans3));
    BOOST_CHECK(tx_set2.find(trans4));
    BOOST_CHECK(tx_set2.find(trans5));
}
