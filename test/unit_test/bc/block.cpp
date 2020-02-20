#include <boost/test/unit_test.hpp>

#include "bc/block.hpp"


namespace
{

bc::Transaction trans1{
    base::Bytes("from1 vjS247DGFSv\n ").toHex(), base::Bytes("to1 ()#%DSOJ\n").toHex(), 12398, base::Time()};
bc::Transaction trans2{
    base::Bytes("from2 vj^Hs47DGFSv\n ").toHex(), base::Bytes("to2 ()#%Dsdg\n").toHex(), 5825285, base::Time::now()};
bc::Transaction trans3{
    base::Bytes("from3 vjS2%#&DGF\n ").toHex(), base::Bytes("to3 ()#%DdfOJ\n").toHex(), 12245398, base::Time()};
bc::Transaction trans4{
    base::Bytes("from4 vjS247sdgFSv\n ").toHex(), base::Bytes("to4 {#%DSOJ ").toHex(), 168524347, base::Time()};
bc::Transaction trans5{
    base::Bytes("from5 vjS2  DGFSv\n ").toHex(), base::Bytes("to5 ()#%DSdsJ\n").toHex(), 1434457, base::Time::now()};

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


BOOST_AUTO_TEST_CASE(block_constructor1)
{
    auto tx_set = getTestSet();
    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block(12, base::Sha256::compute(base::Bytes(str_for_sha)), std::move(tx_set));

    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(trans1));
    BOOST_CHECK(block_tx_set.find(trans2));
    BOOST_CHECK(block_tx_set.find(trans3));
    BOOST_CHECK(block_tx_set.find(trans4));
    BOOST_CHECK(block_tx_set.find(trans5));

    BOOST_CHECK(!block_tx_set.find(bc::Transaction(trans2.getFrom(), trans2.getTo(), 146987, trans2.getTimestamp())));
    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(trans4.getFrom(), trans4.getTo(), trans4.getAmount(), base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_constructor2)
{
    auto tx_set = getTestSet();
    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block(112, base::Sha256::compute(base::Bytes(str_for_sha)), std::move(tx_set));

    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(trans1));
    BOOST_CHECK(block_tx_set.find(trans2));
    BOOST_CHECK(block_tx_set.find(trans3));
    BOOST_CHECK(block_tx_set.find(trans4));
    BOOST_CHECK(block_tx_set.find(trans5));

    BOOST_CHECK(!block_tx_set.find(bc::Transaction(trans2.getFrom(), trans2.getTo(), 146987, trans2.getTimestamp())));
    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(trans4.getFrom(), trans4.getTo(), trans4.getAmount(), base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_operator_equal)
{
    auto tx_set = getTestSet();
    tx_set.remove(trans1);

    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block1(113, base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f a%")), std::move(tx_set));
    bc::Block block2(114, base::Sha256::compute(base::Bytes(str_for_sha)), getTestSet());
    block1 = block2;

    BOOST_CHECK(block1.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
    auto block_tx_set = block1.getTransactions();

    BOOST_CHECK(block_tx_set.find(trans1));
    BOOST_CHECK(block_tx_set.find(trans2));
    BOOST_CHECK(block_tx_set.find(trans3));
    BOOST_CHECK(block_tx_set.find(trans4));
    BOOST_CHECK(block_tx_set.find(trans5));

    BOOST_CHECK(!block_tx_set.find(bc::Transaction(trans2.getFrom(), trans2.getTo(), 146987, trans2.getTimestamp())));
    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(trans4.getFrom(), trans4.getTo(), trans4.getAmount(), base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_operator_move)
{
    auto tx_set = getTestSet();
    tx_set.remove(trans1);

    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block1(115, base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f a%")), std::move(tx_set));
    bc::Block block2(116, base::Sha256::compute(base::Bytes(str_for_sha)), getTestSet());
    block1 = std::move(block2);

    BOOST_CHECK(block1.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
    auto block_tx_set = block1.getTransactions();

    BOOST_CHECK(block_tx_set.find(trans1));
    BOOST_CHECK(block_tx_set.find(trans2));
    BOOST_CHECK(block_tx_set.find(trans3));
    BOOST_CHECK(block_tx_set.find(trans4));
    BOOST_CHECK(block_tx_set.find(trans5));

    BOOST_CHECK(!block_tx_set.find(bc::Transaction(trans2.getFrom(), trans2.getTo(), 146987, trans2.getTimestamp())));
    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(trans4.getFrom(), trans4.getTo(), trans4.getAmount(), base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_sets)
{
    bc::Block block(117, base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), getTestSet());
    auto tx_set = getTestSet();

    bc::Transaction new_trans{
        base::Bytes("SD#%),/n\' \n").toHex(), base::Bytes("#(vm496LDF ").toHex(), 67805678, base::Time()};
    tx_set.remove(trans3);
    tx_set.remove(trans5);
    tx_set.add(new_trans);

    block.setNonce(bc::NonceInt(5678969));
    std::string new_str_for_sha = "SDGK\nsfj$^DG Ldfj34/GHOJ ";
    block.setPrevBlockHash(base::Sha256::compute(base::Bytes(new_str_for_sha)));
    block.setTransactions(std::move(tx_set));

    BOOST_CHECK(block.getNonce() == bc::NonceInt(5678969));
    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes(new_str_for_sha)));
    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(trans1));
    BOOST_CHECK(block_tx_set.find(trans2));
    BOOST_CHECK(block_tx_set.find(trans4));
    BOOST_CHECK(block_tx_set.find(new_trans));

    BOOST_CHECK(!block_tx_set.find(trans3));
    BOOST_CHECK(!block_tx_set.find(trans5));
}


BOOST_AUTO_TEST_CASE(block_add_transaction)
{
    bc::Block block(118, base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), getTestSet());
    bc::Transaction new_trans1{
        base::Bytes("SD#%),/n\' \n").toHex(), base::Bytes("#(vm496LDF ").toHex(), 67805678, base::Time()};
    bc::Transaction new_trans2{
        base::Bytes("(#583 Ks%3\n\\  sf )").toHex(), base::Bytes("# sd  FS34$").toHex(), 2078911, base::Time()};

    block.addTransaction(new_trans1);
    block.addTransaction(new_trans2);

    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(new_trans1));
    BOOST_CHECK(block_tx_set.find(new_trans2));
}


BOOST_AUTO_TEST_CASE(block_serialization1)
{
    std::string str_fro_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block1(119, base::Sha256::compute(base::Bytes(str_fro_sha)), getTestSet());

    block1.setNonce(bc::NonceInt(6706744));
    base::SerializationOArchive oa;
    block1.serialize(oa);

    base::SerializationIArchive ia(oa.getBytes());
    bc::Block block2(120, base::Sha256::compute(base::Bytes("")), bc::TransactionsSet());
    block2 = bc::Block::deserialize(ia);

    BOOST_CHECK(block2.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_fro_sha)));
    BOOST_CHECK(block2.getNonce() == bc::NonceInt(6706744));
    auto block_tx_set = block1.getTransactions();

    BOOST_CHECK(block_tx_set.find(trans1));
    BOOST_CHECK(block_tx_set.find(trans2));
    BOOST_CHECK(block_tx_set.find(trans3));
    BOOST_CHECK(block_tx_set.find(trans4));
    BOOST_CHECK(block_tx_set.find(trans5));
}


BOOST_AUTO_TEST_CASE(block_serialization2)
{
    std::string str_fro_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block1(119, base::Sha256::compute(base::Bytes(str_fro_sha)), getTestSet());

    block1.setNonce(bc::NonceInt(6706744));
    base::SerializationOArchive oa;
    oa.serialize(block1);

    base::SerializationIArchive ia(oa.getBytes());
    bc::Block block2(120, base::Sha256::compute(base::Bytes("")), bc::TransactionsSet());
    block2 = ia.deserialize<bc::Block>();

    BOOST_CHECK(block2.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_fro_sha)));
    BOOST_CHECK(block2.getNonce() == bc::NonceInt(6706744));
    auto block_tx_set = block1.getTransactions();

    BOOST_CHECK(block_tx_set.find(trans1));
    BOOST_CHECK(block_tx_set.find(trans2));
    BOOST_CHECK(block_tx_set.find(trans3));
    BOOST_CHECK(block_tx_set.find(trans4));
    BOOST_CHECK(block_tx_set.find(trans5));
}
