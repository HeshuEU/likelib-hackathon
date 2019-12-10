#include <boost/test/unit_test.hpp>

#include "bc/block.hpp"


namespace
{
bc::TransactionsSet getTestSet()
{
    bc::TransactionsSet set;
    set.add(bc::Transaction(bc::Address("from1 vjS247DGFSv\n "), bc::Address("to1 ()#%DSOJ\n"), 12398, base::Time()));
    set.add(
        bc::Transaction(bc::Address("from2 vj^Hs47DGFSv\n "), bc::Address("to2 ()#%Dsdg\n"), 5825285, base::Time()));
    set.add(
        bc::Transaction(bc::Address("from3 vjS2%#&DGF\n "), bc::Address("to3 ()#%DdfOJ\n"), 12245398, base::Time()));
    set.add(
        bc::Transaction(bc::Address("from4 vjS247sdgFSv\n "), bc::Address("to4 {#%DSOJ "), 168524347, base::Time()));
    set.add(
        bc::Transaction(bc::Address("from5 vjS2  DGFSv\n "), bc::Address("to5 ()#%DSdsJ\n"), 1434457, base::Time()));
    return set;
}
} // namespace


BOOST_AUTO_TEST_CASE(block_constructor1)
{
    bc::Block block;

    BOOST_CHECK(block.getPrevBlockHash() == base::Bytes());
    BOOST_CHECK(block.getTransactions().isEmpty());
    // BOOST_CHECK(block.getNonce() == bc::NonceInt());  //TODO:not work now
}


BOOST_AUTO_TEST_CASE(block_constructor2)
{
    auto tx_set = getTestSet();
    base::Bytes prev_hash(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%"));
    bc::Block block(prev_hash, std::move(tx_set));

    BOOST_CHECK(block.getPrevBlockHash() == base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%"));
    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from1 vjS247DGFSv\n "), bc::Address("to1 ()#%DSOJ\n"), 12398, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from2 vj^Hs47DGFSv\n "), bc::Address("to2 ()#%Dsdg\n"), 5825285, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from3 vjS2%#&DGF\n "), bc::Address("to3 ()#%DdfOJ\n"), 12245398, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from4 vjS247sdgFSv\n "), bc::Address("to4 {#%DSOJ "), 168524347, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from5 vjS2  DGFSv\n "), bc::Address("to5 ()#%DSdsJ\n"), 1434457, base::Time())));

    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 146987, base::Time())));
    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(bc::Address("vjS247DG\n "), bc::Address("()#%DOJ\n"), 12398, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_constructor3)
{
    auto tx_set = getTestSet();
    bc::Block block(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%"), std::move(tx_set));

    BOOST_CHECK(block.getPrevBlockHash() == base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%"));
    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from1 vjS247DGFSv\n "), bc::Address("to1 ()#%DSOJ\n"), 12398, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from2 vj^Hs47DGFSv\n "), bc::Address("to2 ()#%Dsdg\n"), 5825285, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from3 vjS2%#&DGF\n "), bc::Address("to3 ()#%DdfOJ\n"), 12245398, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from4 vjS247sdgFSv\n "), bc::Address("to4 {#%DSOJ "), 168524347, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from5 vjS2  DGFSv\n "), bc::Address("to5 ()#%DSdsJ\n"), 1434457, base::Time())));

    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 146987, base::Time())));
    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(bc::Address("vjS247DG\n "), bc::Address("()#%DOJ\n"), 12398, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_sets_all)
{
    bc::Block block(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%"), getTestSet());
    auto tx_set = getTestSet();
    tx_set.remove(bc::Transaction(bc::Address("from3 vjS2%#&DGF\n "), bc::Address("to3 ()#%DdfOJ\n"), 12245398, base::Time()));
    tx_set.remove(bc::Transaction(bc::Address("from5 vjS2  DGFSv\n "), bc::Address("to5 ()#%DSdsJ\n"), 1434457, base::Time()));
    tx_set.add(bc::Transaction(bc::Address("SD#%),/n\' \n"), bc::Address("#(vm496LDF "), 67805678, base::Time()));
    block.setNonce(bc::NonceInt(5678969));
    block.setPrevBlockHash(base::Bytes("SDGK\nsfj$^DG Ldfj34/GHOJ "));
    block.setTransactions(std::move(tx_set));

    BOOST_CHECK(block.getNonce() == bc::NonceInt(5678969));
    BOOST_CHECK(block.getPrevBlockHash() == base::Bytes("SDGK\nsfj$^DG Ldfj34/GHOJ "));
    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from1 vjS247DGFSv\n "), bc::Address("to1 ()#%DSOJ\n"), 12398, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from2 vj^Hs47DGFSv\n "), bc::Address("to2 ()#%Dsdg\n"), 5825285, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("from4 vjS247sdgFSv\n "), bc::Address("to4 {#%DSOJ "), 168524347, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("SD#%),/n\' \n"), bc::Address("#(vm496LDF "), 67805678, base::Time())));

    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address("from3 vjS2%#&DGF\n "), bc::Address("to3 ()#%DdfOJ\n"), 12245398, base::Time())));
    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address("from5 vjS2  DGFSv\n "), bc::Address("to5 ()#%DSdsJ\n"), 1434457, base::Time())));
}
