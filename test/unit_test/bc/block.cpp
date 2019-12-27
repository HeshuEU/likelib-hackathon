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
    auto tx_set = getTestSet();
    bc::Block block(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), std::move(tx_set));

    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")));
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

    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 146987, base::Time())));
    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address("vjS247DG\n "), bc::Address("()#%DOJ\n"), 12398, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_constructor2)
{
    auto tx_set = getTestSet();
    bc::Block block(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), std::move(tx_set));

    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")));
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

    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 146987, base::Time())));
    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address("vjS247DG\n "), bc::Address("()#%DOJ\n"), 12398, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_operator_equal)
{
    auto tx_set = getTestSet();
    tx_set.remove(bc::Transaction(bc::Address("from1 vjS247DGFSv\n "), bc::Address("to1 ()#%DSOJ\n"), 12398, base::Time()));
    bc::Block block1(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f a%")), std::move(tx_set));
    bc::Block block2(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), getTestSet());
    block1 = block2;

    BOOST_CHECK(block1.getPrevBlockHash() == base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")));
    auto block_tx_set = block1.getTransactions();

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

    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 146987, base::Time())));
    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address("vjS247DG\n "), bc::Address("()#%DOJ\n"), 12398, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_operator_move)
{
    auto tx_set = getTestSet();
    tx_set.remove(bc::Transaction(bc::Address("from1 vjS247DGFSv\n "), bc::Address("to1 ()#%DSOJ\n"), 12398, base::Time()));
    bc::Block block1(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f a%")), std::move(tx_set));
    bc::Block block2(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), getTestSet());
    block1 = std::move(block2);

    BOOST_CHECK(block1.getPrevBlockHash() == base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")));
    auto block_tx_set = block1.getTransactions();

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

    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address("vjS247DGFSv\n "), bc::Address("()#%DSOJ\n"), 146987, base::Time())));
    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address("vjS247DG\n "), bc::Address("()#%DOJ\n"), 12398, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_sets)
{
    bc::Block block(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), getTestSet());
    auto tx_set = getTestSet();
    tx_set.remove(
        bc::Transaction(bc::Address("from3 vjS2%#&DGF\n "), bc::Address("to3 ()#%DdfOJ\n"), 12245398, base::Time()));
    tx_set.remove(
        bc::Transaction(bc::Address("from5 vjS2  DGFSv\n "), bc::Address("to5 ()#%DSdsJ\n"), 1434457, base::Time()));
    tx_set.add(bc::Transaction(bc::Address("SD#%),/n\' \n"), bc::Address("#(vm496LDF "), 67805678, base::Time()));
    block.setNonce(bc::NonceInt(5678969));
    block.setPrevBlockHash(base::Sha256::compute(base::Bytes("SDGK\nsfj$^DG Ldfj34/GHOJ ")));
    block.setTransactions(std::move(tx_set));

    BOOST_CHECK(block.getNonce() == bc::NonceInt(5678969));
    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes("SDGK\nsfj$^DG Ldfj34/GHOJ ")));
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


BOOST_AUTO_TEST_CASE(block_add_transaction)
{
    bc::Block block(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), getTestSet());
    block.addTransaction(
        bc::Transaction(bc::Address("SD#%),/n\' \n"), bc::Address("#(vm496LDF "), 67805678, base::Time()));
    block.addTransaction(
        bc::Transaction(bc::Address("(#583 Ks%3\n\\  sf )"), bc::Address("# sd  FS34$"), 2078911, base::Time()));

    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("(#583 Ks%3\n\\  sf )"), bc::Address("# sd  FS34$"), 2078911, base::Time())));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address("SD#%),/n\' \n"), bc::Address("#(vm496LDF "), 67805678, base::Time())));
}


BOOST_AUTO_TEST_CASE(block_serialization1)
{
    bc::Block block1(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), getTestSet());
    block1.setNonce(bc::NonceInt(6706744));
    base::SerializationOArchive oa;
    bc::Block::serialize(oa, block1);

    base::SerializationIArchive ia(oa.getBytes());
    bc::Block block2(base::Sha256::compute(base::Bytes("")), bc::TransactionsSet());
    block2 = bc::Block::deserialize(ia);
    BOOST_CHECK(block2.getPrevBlockHash() == base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")));
    BOOST_CHECK(block2.getNonce() == bc::NonceInt(6706744));
    auto block_tx_set = block1.getTransactions();

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
}


BOOST_AUTO_TEST_CASE(block_serialization2)
{
    bc::Block block1(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), getTestSet());
    block1.setNonce(bc::NonceInt(6706744));
    base::SerializationOArchive oa;
    oa << block1;

    base::SerializationIArchive ia(oa.getBytes());
    bc::Block block2(base::Sha256::compute(base::Bytes("")), bc::TransactionsSet());
    ia >> block2;
    BOOST_CHECK(block2.getPrevBlockHash() == base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")));
    BOOST_CHECK(block2.getNonce() == bc::NonceInt(6706744));
    auto block_tx_set = block1.getTransactions();

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
}