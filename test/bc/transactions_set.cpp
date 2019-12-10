#include <boost/test/unit_test.hpp>

#include "bc/transactions_set.hpp"

bc::TransactionsSet getTestSet()
{
    bc::TransactionsSet set;
    set.add(bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 12398, base::Time()));
    set.add(bc::Transaction(bc::Address("vj^Hs47DGFSv\n "), bc::Address("()#%Dsdg\n"), 5825285, base::Time()));
    set.add(bc::Transaction(bc::Address("vjS2%#&DGFSv\n "), bc::Address("()#%DdfOJ\n"), 12245398, base::Time()));
    set.add(bc::Transaction(bc::Address("vjS247sdgFSv\n "), bc::Address("{#%DSOJ "), 168524347, base::Time()));
    set.add(bc::Transaction(bc::Address("vjS2  DGFSv\n "), bc::Address("()#%DSdsJ\n"), 1434457, base::Time()));
    return set;
}

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
    BOOST_CHECK(tx_set.find(
        bc::Transaction(bc::Address("vjS2%#&DGFSv\n "), bc::Address("()#%DdfOJ\n"), 12245398, base::Time())));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS247sdgFSv\n "), bc::Address("{#%DSOJ "), 168524347, base::Time())));
    BOOST_CHECK(
        tx_set.find(bc::Transaction(bc::Address("vjS2  DGFSv\n "), bc::Address("()#%DSdsJ\n"), 1434457, base::Time())));

    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 146987, base::Time())));
    BOOST_CHECK(
        !tx_set.find(bc::Transaction(bc::Address("vjS247DG\n "), bc::Address("()#%DOJ\n"), 12398, base::Time::now())));
}