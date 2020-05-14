#include <boost/test/unit_test.hpp>

#include "core/transaction.hpp"


BOOST_AUTO_TEST_CASE(transaction_constructor1)
{
    lk::Address from = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Address to = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Balance amount = 1239823409;
    auto time = base::Time::now();
    std::uint64_t fee = 42;
    lk::Transaction tx(from, to, amount, fee, time, base::Bytes{});

    BOOST_CHECK(tx.getFrom().toString() == from.toString());
    BOOST_CHECK(tx.getTo().toString() == to.toString());
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getAmount() == amount);
    BOOST_CHECK(tx.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_copy)
{
    lk::Address from = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Address to = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Balance amount = 1239823409;
    auto time = base::Time::now();
    std::uint64_t fee = 123;
    lk::Transaction tx1(from, to, amount, fee, time, base::Bytes{});
    lk::Transaction tx2(tx1);

    BOOST_CHECK(tx2.getFrom().toString() == from.toString());
    BOOST_CHECK(tx2.getTo().toString() == to.toString());
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
    BOOST_CHECK(tx2.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_move)
{
    lk::Address from = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Address to = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Balance amount = 1239823409;
    auto time = base::Time::now();
    std::uint64_t fee = 123;
    lk::Transaction tx1(from, to, amount, fee, time, base::Bytes{});
    lk::Transaction tx2(std::move(tx1));

    BOOST_CHECK(tx2.getFrom().toString() == from.toString());
    BOOST_CHECK(tx2.getTo().toString() == to.toString());
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
    BOOST_CHECK(tx2.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_operator_equal_copy)
{
    lk::Address from = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Address to = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Balance amount = 1239823409;
    auto time = base::Time::now();
    std::uint64_t fee{ 23213213 };
    lk::Transaction tx1(from, to, amount, fee, time, base::Bytes{});
    lk::Transaction tx2(from, to, 821481368, fee, time, base::Bytes{});

    BOOST_CHECK(tx1 != tx2);
    tx2 = tx1;
    BOOST_CHECK(tx2.getFrom().toString() == from.toString());
    BOOST_CHECK(tx2.getTo().toString() == to.toString());
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
    BOOST_CHECK(tx2.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_operator_equal_move)
{
    lk::Address from = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Address to = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Balance amount = 1239823409;
    auto time = base::Time::now();
    std::uint64_t fee{ 213213 };
    lk::Transaction tx1(from, to, amount, fee, time, base::Bytes{});
    lk::Transaction tx2(from, to, 821481368, fee, time, base::Bytes{});

    BOOST_CHECK(tx1 != tx2);
    tx2 = std::move(tx1);
    BOOST_CHECK(tx2.getFrom().toString() == from.toString());
    BOOST_CHECK(tx2.getTo().toString() == to.toString());
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
    BOOST_CHECK(tx2.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_sign)
{
    auto priv_key = base::Secp256PrivateKey();
    lk::Address from = lk::Address(priv_key.toPublicKey());
    lk::Address to = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Balance amount = 1239823409;
    auto time = base::Time::now();
    std::uint64_t fee = 42;
    lk::Transaction tx(from, to, amount, fee, time, base::Bytes{});
    tx.sign(priv_key);

    BOOST_CHECK(tx.checkSign());
}


BOOST_AUTO_TEST_CASE(transaction_serialization1)
{
    lk::Address from = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Address to = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Balance amount = 1239823409;
    auto time = base::Time::now();
    std::uint64_t fee = 123;
    lk::Transaction tx1(from, to, amount, fee, time, base::Bytes{});

    base::SerializationOArchive oa;
    oa.serialize(tx1);

    base::SerializationIArchive ia(oa.getBytes());
    auto tx2 = ia.deserialize<lk::Transaction>();
    BOOST_CHECK(tx1 == tx2);
}


BOOST_AUTO_TEST_CASE(transaction_serialization2)
{
    lk::Address from = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Address to = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Balance amount = 1239823409;
    auto time = base::Time::now();
    std::uint64_t fee{ 506 };
    lk::Transaction tx1(from, to, amount, fee, time, base::Bytes{});

    base::SerializationOArchive oa;
    oa.serialize(tx1);

    base::SerializationIArchive ia(oa.getBytes());
    auto tx2 = ia.deserialize<lk::Transaction>();
    BOOST_CHECK(tx1 == tx2);
}


BOOST_AUTO_TEST_CASE(transaction_builder_set_all1)
{
    lk::Address from = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Address to = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Balance amount = 1239823409;
    auto time = base::Time::now();
    std::uint64_t fee = 150;
    lk::TransactionBuilder txb;

    txb.setFrom(from);
    txb.setTo(to);
    txb.setTimestamp(time);
    txb.setAmount(amount);
    txb.setData(base::Bytes());
    txb.setFee(fee);

    auto tx = std::move(txb).build();

    BOOST_CHECK(tx.getFrom().toString() == from.toString());
    BOOST_CHECK(tx.getTo().toString() == to.toString());
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getAmount() == amount);
    BOOST_CHECK(tx.getData() == base::Bytes());
    BOOST_CHECK(tx.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_builder_set_all2)
{
    lk::Address from = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Address to = lk::Address(base::Secp256PrivateKey().toPublicKey());
    lk::Balance amount = 1239823409;
    auto time = base::Time::now();
    std::uint64_t fee{ 229 };

    lk::TransactionBuilder txb;
    txb.setFrom(from);
    txb.setTo(to);
    txb.setTimestamp(time);
    txb.setAmount(amount);
    txb.setData(base::Bytes());
    txb.setFee(fee);

    auto tx1 = txb.build();

    BOOST_CHECK(tx1.getFrom().toString() == from.toString());
    BOOST_CHECK(tx1.getTo().toString() == to.toString());
    BOOST_CHECK(tx1.getTimestamp() == time);
    BOOST_CHECK(tx1.getAmount() == amount);
    BOOST_CHECK(tx1.getData() == base::Bytes());
    BOOST_CHECK(tx1.getFee() == fee);
}
