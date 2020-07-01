//#include <boost/test/unit_test.hpp>
//
//#include "core/block.hpp"
//
//
// namespace
//{
//
// lk::Transaction trans1{ lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                        lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                        12398,
//                        0,
//                        base::Time(),
//                        base::Bytes{} };
// lk::Transaction trans2{ lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                        lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                        5825285,
//                        0,
//                        base::Time::now(),
//                        base::Bytes{} };
// lk::Transaction trans3{ lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                        lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                        12245398,
//                        0,
//                        base::Time(),
//                        base::Bytes{} };
// lk::Transaction trans4{ lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                        lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                        168524347,
//                        0,
//                        base::Time(),
//                        base::Bytes{} };
// lk::Transaction trans5{ lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                        lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                        1434457,
//                        0,
//                        base::Time::now(),
//                        base::Bytes{} };
//
// lk::TransactionsSet getTestSet()
//{
//    lk::TransactionsSet set;
//    set.add(trans1);
//    set.add(trans2);
//    set.add(trans3);
//    set.add(trans4);
//    set.add(trans5);
//    return set;
//}
//
// lk::Address miner_address(base::Secp256PrivateKey().toPublicKey());
//} // namespace
//
//
// BOOST_AUTO_TEST_CASE(block_constructor1)
//{
//    auto tx_set = getTestSet();
//    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
//    lk::Block block(
//      12, base::Sha256::compute(base::Bytes(str_for_sha)), base::Time(), miner_address, std::move(tx_set));
//
//    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
//    auto block_tx_set = block.getTransactions();
//
//    BOOST_CHECK(block_tx_set.find(trans1));
//    BOOST_CHECK(block_tx_set.find(trans2));
//    BOOST_CHECK(block_tx_set.find(trans3));
//    BOOST_CHECK(block_tx_set.find(trans4));
//    BOOST_CHECK(block_tx_set.find(trans5));
//
//    BOOST_CHECK(!block_tx_set.find(
//      lk::Transaction(trans2.getFrom(), trans2.getTo(), 146987, 0, trans2.getTimestamp(), base::Bytes{})));
//    BOOST_CHECK(!block_tx_set.find(
//      lk::Transaction(trans4.getFrom(), trans4.getTo(), trans4.getAmount(), 0, base::Time::now(), base::Bytes{})));
//}
//
//
// BOOST_AUTO_TEST_CASE(block_constructor2)
//{
//    auto tx_set = getTestSet();
//    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
//    lk::Block block(
//      112, base::Sha256::compute(base::Bytes(str_for_sha)), base::Time(), miner_address, std::move(tx_set));
//
//    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
//    auto block_tx_set = block.getTransactions();
//
//    BOOST_CHECK(block_tx_set.find(trans1));
//    BOOST_CHECK(block_tx_set.find(trans2));
//    BOOST_CHECK(block_tx_set.find(trans3));
//    BOOST_CHECK(block_tx_set.find(trans4));
//    BOOST_CHECK(block_tx_set.find(trans5));
//
//    BOOST_CHECK(!block_tx_set.find(
//      lk::Transaction(trans2.getFrom(), trans2.getTo(), 146987, 0, trans2.getTimestamp(), base::Bytes{})));
//    BOOST_CHECK(!block_tx_set.find(
//      lk::Transaction(trans4.getFrom(), trans4.getTo(), trans4.getAmount(), 0, base::Time::now(), base::Bytes{})));
//}
//
//
// BOOST_AUTO_TEST_CASE(block_operator_equal)
//{
//    auto tx_set = getTestSet();
//    tx_set.remove(trans1);
//
//    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
//    lk::Block block1(
//      113, base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f a%")), base::Time(), miner_address,
//      std::move(tx_set));
//    lk::Block block2(114, base::Sha256::compute(base::Bytes(str_for_sha)), base::Time(), miner_address, getTestSet());
//    block1 = block2;
//
//    BOOST_CHECK(block1.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
//    auto block_tx_set = block1.getTransactions();
//
//    BOOST_CHECK(block_tx_set.find(trans1));
//    BOOST_CHECK(block_tx_set.find(trans2));
//    BOOST_CHECK(block_tx_set.find(trans3));
//    BOOST_CHECK(block_tx_set.find(trans4));
//    BOOST_CHECK(block_tx_set.find(trans5));
//
//    BOOST_CHECK(!block_tx_set.find(
//      lk::Transaction(trans2.getFrom(), trans2.getTo(), 146987, 0, trans2.getTimestamp(), base::Bytes{})));
//    BOOST_CHECK(!block_tx_set.find(
//      lk::Transaction(trans4.getFrom(), trans4.getTo(), trans4.getAmount(), 0, base::Time::now(), base::Bytes{})));
//}
//
//
// BOOST_AUTO_TEST_CASE(block_operator_move)
//{
//    auto tx_set = getTestSet();
//    tx_set.remove(trans1);
//
//    std::string str_for_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
//    lk::Block block1(
//      115, base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f a%")), base::Time(), miner_address,
//      std::move(tx_set));
//    lk::Block block2(116, base::Sha256::compute(base::Bytes(str_for_sha)), base::Time(), miner_address, getTestSet());
//    block1 = std::move(block2);
//
//    BOOST_CHECK(block1.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_for_sha)));
//    auto block_tx_set = block1.getTransactions();
//
//    BOOST_CHECK(block_tx_set.find(trans1));
//    BOOST_CHECK(block_tx_set.find(trans2));
//    BOOST_CHECK(block_tx_set.find(trans3));
//    BOOST_CHECK(block_tx_set.find(trans4));
//    BOOST_CHECK(block_tx_set.find(trans5));
//
//    BOOST_CHECK(!block_tx_set.find(
//      lk::Transaction(trans2.getFrom(), trans2.getTo(), 146987, 0, trans2.getTimestamp(), base::Bytes{})));
//    BOOST_CHECK(!block_tx_set.find(
//      lk::Transaction(trans4.getFrom(), trans4.getTo(), trans4.getAmount(), 0, base::Time::now(), base::Bytes{})));
//}
//
//
// BOOST_AUTO_TEST_CASE(block_sets)
//{
//    lk::Block block(117,
//                    base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")),
//                    base::Time(),
//                    miner_address,
//                    getTestSet());
//    auto tx_set = getTestSet();
//
//    lk::Transaction new_trans{ lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                               lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                               67805678,
//                               0,
//                               base::Time(),
//                               base::Bytes{} };
//    tx_set.remove(trans3);
//    tx_set.remove(trans5);
//    tx_set.add(new_trans);
//
//    block.setNonce(lk::NonceInt(5678969));
//    std::string new_str_for_sha = "SDGK\nsfj$^DG Ldfj34/GHOJ ";
//    block.setPrevBlockHash(base::Sha256::compute(base::Bytes(new_str_for_sha)));
//    block.setTransactions(std::move(tx_set));
//
//    BOOST_CHECK(block.getNonce() == lk::NonceInt(5678969));
//    BOOST_CHECK(block.getPrevBlockHash() == base::Sha256::compute(base::Bytes(new_str_for_sha)));
//    auto block_tx_set = block.getTransactions();
//
//    BOOST_CHECK(block_tx_set.find(trans1));
//    BOOST_CHECK(block_tx_set.find(trans2));
//    BOOST_CHECK(block_tx_set.find(trans4));
//    BOOST_CHECK(block_tx_set.find(new_trans));
//
//    BOOST_CHECK(!block_tx_set.find(trans3));
//    BOOST_CHECK(!block_tx_set.find(trans5));
//}
//
//
// BOOST_AUTO_TEST_CASE(block_add_transaction)
//{
//    lk::Block block(118,
//                    base::Sha256::compute(base::Bytes("#%*(D VASGL/n\n\f asdeGDH#%")),
//                    base::Time(),
//                    miner_address,
//                    getTestSet());
//    lk::Transaction new_trans1{ lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                                lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                                67805678,
//                                0,
//                                base::Time(),
//                                base::Bytes{} };
//    lk::Transaction new_trans2{ lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                                lk::Address(base::Secp256PrivateKey().toPublicKey()),
//                                2078911,
//                                0,
//                                base::Time(),
//                                base::Bytes{} };
//
//    block.addTransaction(new_trans1);
//    block.addTransaction(new_trans2);
//
//    auto block_tx_set = block.getTransactions();
//
//    BOOST_CHECK(block_tx_set.find(new_trans1));
//    BOOST_CHECK(block_tx_set.find(new_trans2));
//}
//
//
// BOOST_AUTO_TEST_CASE(block_serialization1)
//{
//    std::string str_fro_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
//    lk::Block block1(119, base::Sha256::compute(base::Bytes(str_fro_sha)), base::Time(), miner_address, getTestSet());
//
//    block1.setNonce(lk::NonceInt(6706744));
//    base::SerializationOArchive oa;
//    block1.serialize(oa);
//
//    base::SerializationIArchive ia(oa.getBytes());
//    lk::Block block2(120, base::Sha256::compute(base::Bytes("")), base::Time(), miner_address, lk::TransactionsSet());
//    block2 = lk::Block::deserialize(ia);
//
//    BOOST_CHECK(block2.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_fro_sha)));
//    BOOST_CHECK(block2.getNonce() == lk::NonceInt(6706744));
//    auto block_tx_set = block1.getTransactions();
//
//    BOOST_CHECK(block_tx_set.find(trans1));
//    BOOST_CHECK(block_tx_set.find(trans2));
//    BOOST_CHECK(block_tx_set.find(trans3));
//    BOOST_CHECK(block_tx_set.find(trans4));
//    BOOST_CHECK(block_tx_set.find(trans5));
//}
//
//
// BOOST_AUTO_TEST_CASE(block_serialization2)
//{
//    std::string str_fro_sha = "#%*(D VASGL/n\n\f asdeGDH#%";
//    lk::Block block1(119, base::Sha256::compute(base::Bytes(str_fro_sha)), base::Time(), miner_address, getTestSet());
//
//    block1.setNonce(lk::NonceInt(6706744));
//    base::SerializationOArchive oa;
//    oa.serialize(block1);
//
//    base::SerializationIArchive ia(oa.getBytes());
//    lk::Block block2(120, base::Sha256::compute(base::Bytes("")), base::Time(), miner_address, lk::TransactionsSet());
//    block2 = ia.deserialize<lk::Block>();
//
//    BOOST_CHECK(block2.getPrevBlockHash() == base::Sha256::compute(base::Bytes(str_fro_sha)));
//    BOOST_CHECK(block2.getNonce() == lk::NonceInt(6706744));
//    auto block_tx_set = block1.getTransactions();
//
//    BOOST_CHECK(block_tx_set.find(trans1));
//    BOOST_CHECK(block_tx_set.find(trans2));
//    BOOST_CHECK(block_tx_set.find(trans3));
//    BOOST_CHECK(block_tx_set.find(trans4));
//    BOOST_CHECK(block_tx_set.find(trans5));
//}
