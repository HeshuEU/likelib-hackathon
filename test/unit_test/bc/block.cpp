#include <boost/test/unit_test.hpp>

#include "bc/block.hpp"


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


BOOST_AUTO_TEST_CASE(block_constructor1)
{
    auto tx_set = getTestSet();
    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block(base::Sha256::compute(base::Bytes(str_for_sha)), std::move(tx_set));

    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));

    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), 146987, trans2.time)));
    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_constructor2)
{
    auto tx_set = getTestSet();
    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block(base::Sha256::compute(base::Bytes(str_for_sha)), std::move(tx_set));

    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));

    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), 146987, trans2.time)));
    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_operator_equal)
{
    auto tx_set = getTestSet();
    tx_set.remove(bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time));

    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block1(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f a%")), std::move(tx_set));
    bc::Block block2(base::Sha256::compute(base::Bytes(str_for_sha)), getTestSet());
    block1 = block2;

    BOOST_CHECK(block1.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
    auto block_tx_set = block1.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));

    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), 146987, trans2.time)));
    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_operator_move)
{
    auto tx_set = getTestSet();
    tx_set.remove(bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time));

    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block1(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f a%")), std::move(tx_set));
    bc::Block block2(base::Sha256::compute(base::Bytes(str_for_sha)), getTestSet());
    block1 = std::move(block2);

    BOOST_CHECK(block1.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
    auto block_tx_set = block1.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));

    BOOST_CHECK(
        !block_tx_set.find(bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), 146987, trans2.time)));
    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, base::Time::now())));
}


BOOST_AUTO_TEST_CASE(block_sets)
{
    bc::Block block(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), getTestSet());
    auto tx_set = getTestSet();

    TransInfo new_trans{"SD#%),/n\' \n", "#(vm496LDF ", 67805678, base::Time()};
    tx_set.remove(bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time));
    tx_set.remove(bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time));
    tx_set.add(
        bc::Transaction(bc::Address(new_trans.from), bc::Address(new_trans.to), new_trans.amount, new_trans.time));

    block.setNonce(bc::NonceInt(5678969));
    std::string new_str_for_sha = "SDGK\nsfj$^DG Ldfj34/GHOJ ";
    block.setPrevBlockHash(base::Sha256::compute(base::Bytes(new_str_for_sha)));
    block.setTransactions(std::move(tx_set));

    BOOST_CHECK(block.getNonce() == bc::NonceInt(5678969));
    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes(new_str_for_sha)));
    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(new_trans.from), bc::Address(new_trans.to), new_trans.amount, new_trans.time)));

    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(!block_tx_set.find(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));
}


BOOST_AUTO_TEST_CASE(block_add_transaction)
{
    bc::Block block(base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")), getTestSet());
    TransInfo new_trans1{"SD#%),/n\' \n", "#(vm496LDF ", 67805678, base::Time()};
    TransInfo new_trans2{"(#583 Ks%3\n\\  sf )", "# sd  FS34$", 2078911, base::Time()};

    block.addTransaction(
        bc::Transaction(bc::Address(new_trans1.from), bc::Address(new_trans1.to), new_trans1.amount, new_trans1.time));
    block.addTransaction(
        bc::Transaction(bc::Address(new_trans2.from), bc::Address(new_trans2.to), new_trans2.amount, new_trans2.time));

    auto block_tx_set = block.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(new_trans1.from), bc::Address(new_trans1.to), new_trans1.amount, new_trans1.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(new_trans2.from), bc::Address(new_trans2.to), new_trans2.amount, new_trans2.time)));
}


BOOST_AUTO_TEST_CASE(block_serialization1)
{
    std::string str_fro_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block1(base::Sha256::compute(base::Bytes(str_fro_sha)), getTestSet());

    block1.setNonce(bc::NonceInt(6706744));
    base::SerializationOArchive oa;
    bc::Block::serialize(oa, block1);

    base::SerializationIArchive ia(oa.getBytes());
    bc::Block block2(base::Sha256::compute(base::Bytes("")), bc::TransactionsSet());
    block2 = bc::Block::deserialize(ia);

    BOOST_CHECK(block2.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_fro_sha)));
    BOOST_CHECK(block2.getNonce() == bc::NonceInt(6706744));
    auto block_tx_set = block1.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));
}


BOOST_AUTO_TEST_CASE(block_serialization2)
{
    std::string str_fro_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
    bc::Block block1(base::Sha256::compute(base::Bytes(str_fro_sha)), getTestSet());

    block1.setNonce(bc::NonceInt(6706744));
    base::SerializationOArchive oa;
    oa << block1;

    base::SerializationIArchive ia(oa.getBytes());
    bc::Block block2(base::Sha256::compute(base::Bytes("")), bc::TransactionsSet());
    ia >> block2;

    BOOST_CHECK(block2.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_fro_sha)));
    BOOST_CHECK(block2.getNonce() == bc::NonceInt(6706744));
    auto block_tx_set = block1.getTransactions();

    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans1.from), bc::Address(trans1.to), trans1.amount, trans1.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans2.from), bc::Address(trans2.to), trans2.amount, trans2.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans3.from), bc::Address(trans3.to), trans3.amount, trans3.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans4.from), bc::Address(trans4.to), trans4.amount, trans4.time)));
    BOOST_CHECK(block_tx_set.find(
        bc::Transaction(bc::Address(trans5.from), bc::Address(trans5.to), trans5.amount, trans5.time)));
}