#include <boost/test/unit_test.hpp>

#include "bc/transaction.hpp"


BOOST_AUTO_TEST_CASE(transaction_constructor1)
{
    bc::Address from = bc::Address(base::generateKeys().first);
    bc::Address to = bc::Address(base::generateKeys().first);
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 42;
    bc::Transaction tx(from, to, amount, fee, time, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{});

    BOOST_CHECK(tx.getFrom().toString() == from.toString());
    BOOST_CHECK(tx.getTo().toString() == to.toString());
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getAmount() == amount);
    BOOST_CHECK(tx.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_copy)
{
    bc::Address from = bc::Address(base::generateKeys().first);
    bc::Address to = bc::Address(base::generateKeys().first);
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 123;
    bc::Transaction tx1(from, to, amount, fee, time, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{});
    bc::Transaction tx2(tx1);

    BOOST_CHECK(tx2.getFrom().toString() == from.toString());
    BOOST_CHECK(tx2.getTo().toString() == to.toString());
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
    BOOST_CHECK(tx2.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_move)
{
    bc::Address from = bc::Address(base::generateKeys().first);
    bc::Address to = bc::Address(base::generateKeys().first);
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 123;
    bc::Transaction tx1(from, to, amount, fee, time, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{});
    bc::Transaction tx2(std::move(tx1));

    BOOST_CHECK(tx2.getFrom().toString() == from.toString());
    BOOST_CHECK(tx2.getTo().toString() == to.toString());
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
    BOOST_CHECK(tx2.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_operator_equal_copy)
{
    bc::Address from = bc::Address(base::generateKeys().first);
    bc::Address to = bc::Address(base::generateKeys().first);
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee{ 23213213 };
    bc::Transaction tx1(from, to, amount, fee, time, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{});
    bc::Transaction tx2(from, to, 821481368, fee, time, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{});

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
    bc::Address from = bc::Address(base::generateKeys().first);
    bc::Address to = bc::Address(base::generateKeys().first);
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee{ 213213 };
    bc::Transaction tx1(from, to, amount, fee, time, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{});
    bc::Transaction tx2(from, to, 821481368, fee, time, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{});

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
    auto [pub_key, priv_key] = base::generateKeys();
    bc::Address from = bc::Address(pub_key);
    bc::Address to = bc::Address(base::generateKeys().first);
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 42;
    bc::Transaction tx(from, to, amount, fee, time, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{});
    tx.sign(pub_key, priv_key);

    BOOST_CHECK(tx.checkSign());
}


BOOST_AUTO_TEST_CASE(transaction_serialization1)
{
    bc::Address from = bc::Address(base::generateKeys().first);
    bc::Address to = bc::Address(base::generateKeys().first);
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 123;
    bc::Transaction tx1(from, to, amount, fee, time, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{});

    base::SerializationOArchive oa;
    oa.serialize(tx1);

    base::SerializationIArchive ia(oa.getBytes());
    auto tx2 = ia.deserialize<bc::Transaction>();
    BOOST_CHECK(tx1 == tx2);
}


BOOST_AUTO_TEST_CASE(transaction_serialization2)
{
    bc::Address from = bc::Address(base::generateKeys().first);
    bc::Address to = bc::Address(base::generateKeys().first);
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee{ 506 };
    bc::Transaction tx1(from, to, amount, fee, time, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{});

    base::SerializationOArchive oa;
    oa.serialize(tx1);

    base::SerializationIArchive ia(oa.getBytes());
    auto tx2 = ia.deserialize<bc::Transaction>();
    BOOST_CHECK(tx1 == tx2);
}


BOOST_AUTO_TEST_CASE(transaction_builder_set_all1)
{
    bc::Address from = bc::Address(base::generateKeys().first);
    bc::Address to = bc::Address(base::generateKeys().first);
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 150;
    bc::TransactionBuilder txb;

    txb.setFrom(from);
    txb.setTo(to);
    txb.setTimestamp(time);
    txb.setAmount(amount);
    txb.setType(bc::Transaction::Type::MESSAGE_CALL);
    txb.setData(base::Bytes());
    txb.setFee(fee);

    auto tx = std::move(txb).build();

    BOOST_CHECK(tx.getFrom().toString() == from.toString());
    BOOST_CHECK(tx.getTo().toString() == to.toString());
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getType() == bc::Transaction::Type::MESSAGE_CALL);
    BOOST_CHECK(tx.getAmount() == amount);
    BOOST_CHECK(tx.getData() == base::Bytes());
    BOOST_CHECK(tx.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_builder_set_all2)
{
    bc::Address from = bc::Address(base::generateKeys().first);
    bc::Address to = bc::Address(base::generateKeys().first);
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee{ 229 };

    bc::TransactionBuilder txb;
    txb.setFrom(from);
    txb.setTo(to);
    txb.setTimestamp(time);
    txb.setAmount(amount);
    txb.setType(bc::Transaction::Type::MESSAGE_CALL);
    txb.setData(base::Bytes());
    txb.setFee(fee);

    auto tx1 = txb.build();

    BOOST_CHECK(tx1.getFrom().toString() == from.toString());
    BOOST_CHECK(tx1.getTo().toString() == to.toString());
    BOOST_CHECK(tx1.getTimestamp() == time);
    BOOST_CHECK(tx1.getType() == bc::Transaction::Type::MESSAGE_CALL);
    BOOST_CHECK(tx1.getAmount() == amount);
    BOOST_CHECK(tx1.getData() == base::Bytes());
    BOOST_CHECK(tx1.getFee() == fee);
}
