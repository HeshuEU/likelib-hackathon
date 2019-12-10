#include <boost/test/unit_test.hpp>

#include "bc/transactions_set.hpp"

namespace
{
bc::TransactionsSet getTestSet()
{
    bc::TransactionsSet set;
    set.add(bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 12398, base::Time()));
    set.add(bc::Transaction(bc::Address("vj^Hs47DGFSv\n "), bc::Address("()#%Dsdg\n"), 5825285, base::Time()));
    set.add(bc::Transaction(bc::Address("vjS2%#&DGF\n "), bc::Address("()#%DdfOJ\n"), 12245398, base::Time()));
    set.add(bc::Transaction(bc::Address("vjS247sdgFSv\n "), bc::Address("{#%DSOJ "), 168524347, base::Time()));
    set.add(bc::Transaction(bc::Address("vjS2  DGFSv\n "), bc::Address("()#%DSdsJ\n"), 1434457, base::Time()));
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
        tx_set.find(bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 12398, base::Time())));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vj^Hs47DGFSv\n "), bc::Address("()#%Dsdg\n"), 5825285, base::Time())));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS2%#&DGF\n "), bc::Address("()#%DdfOJ\n"), 12245398, base::Time())));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS247sdgFSv\n "), bc::Address("{#%DSOJ "), 168524347, base::Time())));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS2  DGFSv\n "), bc::Address("()#%DSdsJ\n"), 1434457, base::Time())));

    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 146987, base::Time())));
    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address("vjS247DG\n "), bc::Address("()#%DOJ\n"), 12398, base::Time::now())));
}

BOOST_AUTO_TEST_CASE(transactions_set_remove)
{
    auto tx_set = getTestSet();

    tx_set.remove(bc::Transaction(bc::Address("vj^Hs47DGFSv\n "), bc::Address("()#%Dsdg\n"), 5825285, base::Time()));
    tx_set.remove(bc::Transaction(bc::Address("vjS2  DGFSv\n "), bc::Address("()#%DSdsJ\n"), 1434457, base::Time()));

    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 12398, base::Time())));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS2%#&DGF\n "), bc::Address("()#%DdfOJ\n"), 12245398, base::Time())));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS247sdgFSv\n "), bc::Address("{#%DSOJ "), 168524347, base::Time())));


    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address("vj^Hs47DGFSv\n "), bc::Address("()#%Dsdg\n"), 5825285, base::Time())));
    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address("vjS2  DGFSv\n "), bc::Address("()#%DSdsJ\n"), 1434457, base::Time())));
}


BOOST_AUTO_TEST_CASE(transactions_set_remove_set1)
{
    auto tx_set = getTestSet();
    bc::TransactionsSet rem_set;
    rem_set.add(bc::Transaction(bc::Address("vj^Hs47DGFSv\n "), bc::Address("()#%Dsdg\n"), 5825285, base::Time()));
    rem_set.add(bc::Transaction(bc::Address("vjS2  DGFSv\n "), bc::Address("()#%DSdsJ\n"), 1434457, base::Time()));

    tx_set.remove(rem_set);

    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 12398, base::Time())));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS2%#&DGF\n "), bc::Address("()#%DdfOJ\n"), 12245398, base::Time())));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS247sdgFSv\n "), bc::Address("{#%DSOJ "), 168524347, base::Time())));


    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address("vj^Hs47DGFSv\n "), bc::Address("()#%Dsdg\n"), 5825285, base::Time())));
    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address("vjS2  DGFSv\n "), bc::Address("()#%DSdsJ\n"), 1434457, base::Time())));
}


BOOST_AUTO_TEST_CASE(transactions_set_remove_set2)
{
    auto tx_set = getTestSet();
    auto rem_set = getTestSet();

    tx_set.remove(rem_set);

    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 12398, base::Time())));
    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address("vjS2%#&DGF\n "), bc::Address("()#%DdfOJ\n"), 12245398, base::Time())));
    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address("vjS247sdgFSv\n "), bc::Address("{#%DSOJ "), 168524347, base::Time())));
    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address("vj^Hs47DGFSv\n "), bc::Address("()#%Dsdg\n"), 5825285, base::Time())));
    BOOST_CHECK(!tx_set.find(
        bc::Transaction(bc::Address("vjS2  DGFSv\n "), bc::Address("()#%DSdsJ\n"), 1434457, base::Time())));
    BOOST_CHECK(tx_set.isEmpty());
}


BOOST_AUTO_TEST_CASE(transactions_set_iterators_usage1)
{
    auto tx_set = getTestSet();
    bool is_ok = true;
    for(auto it = tx_set.begin(); it != tx_set.end(); it++)
    {
        is_ok = is_ok && (tx_set.find(*it));
    }
    BOOST_CHECK(is_ok);
}


BOOST_AUTO_TEST_CASE(transactions_set_iterators_usage2)
{
    const auto tx_set = getTestSet();
    bool is_ok = true;
    for(auto it = tx_set.begin(); it != tx_set.end(); it++)
    {
        is_ok = is_ok && (tx_set.find(*it));
    }
    BOOST_CHECK(is_ok);
}


BOOST_AUTO_TEST_CASE(transactions_set_iterators_usage3)
{
    auto tx_set = getTestSet();
    bool is_ok = true;
    for(auto& it : tx_set)
    {
        is_ok = is_ok && (tx_set.find(it));
    }
    BOOST_CHECK(is_ok);
}


BOOST_AUTO_TEST_CASE(transactions_set_iterators_usage4)
{
    const auto tx_set = getTestSet();
    bool is_ok = true;
    for(auto& it : tx_set)
    {
        is_ok = is_ok && (tx_set.find(it));
    }
    BOOST_CHECK(is_ok);
}
