#include <boost/test/unit_test.hpp>

#include "bc/transactions_set.hpp"

namespace
{
struct TransInfo
{
    std::string from;
    std::string to;
    bc::Balance amount;
    base::Time time;
};


TransInfo trans1{"from1 vjS247DGFSv\n ", "to1 ()#%DSOJ\n", 12398, base::Time()};
TransInfo trans2{"from2 vj^Hs47DGFSv\n ", "to2 ()#%Dsdg\n", 5825285, base::Time::now()};
TransInfo trans3{"from3 vjS2%#&DGF\n ", "to3 ()#%DdfOJ\n", 12245398, base::Time()};
TransInfo trans4{"from4 vjS247sdgFSv\n ", "to4 {#%DSOJ ", 168524347, base::Time()};
TransInfo trans5{"from5 vjS2  DGFSv\n ", "to5 ()#%DSdsJ\n", 1434457, base::Time::now()};

bc::TransactionsSet getTestSet()
{
    bc::TransactionsSet set;
    set.add(bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time));
    set.add(bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time));
    set.add(bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time));
    set.add(bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time));
    set.add(bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time));
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

    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));

    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address(trans1.from), bc::Address("()#%DSOJ\n"), trans1.amount, trans1.time)));
    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, base::Time::now())));
}

BOOST_AUTO_TEST_CASE(transactions_set_remove)
{
    auto tx_set = getTestSet();

    tx_set.remove(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time));
    tx_set.remove(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time));

    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));


    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));
}


BOOST_AUTO_TEST_CASE(transactions_set_remove_set1)
{
    auto tx_set = getTestSet();
    bc::TransactionsSet rem_set;
    rem_set.add(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time));
    rem_set.add(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time));

    tx_set.remove(rem_set);

    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));


    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));
}


BOOST_AUTO_TEST_CASE(transactions_set_remove_set2)
{
    auto tx_set = getTestSet();
    auto rem_set = getTestSet();

    tx_set.remove(rem_set);

    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));
    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));
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
    oa << tx_set;

    base::SerializationIArchive ia(oa.getBytes());
    bc::TransactionsSet tx_set2;
    ia >> tx_set2;

    BOOST_CHECK(tx_set2.find(
        bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(tx_set2.find(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(tx_set2.find(
        bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(tx_set2.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));
    BOOST_CHECK(tx_set2.find(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));
}
