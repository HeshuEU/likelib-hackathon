#include <boost/test/unit_test.hpp>

#include "bc/transaction.hpp"


BOOST_AUTO_TEST_CASE(transaction_constructor1)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 42;
    bc::Transaction tx(from, to, amount, time, fee);

    BOOST_CHECK(tx.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getAmount() == amount);
    BOOST_CHECK(tx.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_copy)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 123;
    bc::Transaction tx1(from, to, amount, time, fee);
    bc::Transaction tx2(tx1);

    BOOST_CHECK(tx2.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx2.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
    BOOST_CHECK(tx2.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_move)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 123;
    bc::Transaction tx1(from, to, amount, time, fee);
    bc::Transaction tx2(std::move(tx1));

    BOOST_CHECK(tx2.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx2.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
    BOOST_CHECK(tx2.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_operator_equal_copy)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee{23213213};
    bc::Transaction tx1(from, to, amount, time, fee);
    bc::Transaction tx2(from, to, 821481368, time, fee);

    BOOST_CHECK(tx1 != tx2);
    tx2 = tx1;
    BOOST_CHECK(tx2.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx2.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
    BOOST_CHECK(tx2.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_operator_equal_move)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee{213213};
    bc::Transaction tx1(from, to, amount, time, fee);
    bc::Transaction tx2(from, to, 821481368, time, fee);

    BOOST_CHECK(tx1 != tx2);
    tx2 = std::move(tx1);
    BOOST_CHECK(tx2.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx2.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
    BOOST_CHECK(tx2.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_set_all1)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 150;
    bc::TransactionBuilder txb;

    txb.setFrom(bc::Address(str_from));
    txb.setTo(bc::Address(str_to));
    txb.setTimestamp(time);
    txb.setAmount(amount);
    txb.setFee(fee);

    auto tx = std::move(txb).build();

    BOOST_CHECK(tx.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getAmount() == amount);
    BOOST_CHECK(tx.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_set_all2)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee{229};

    bc::TransactionBuilder txb;
    txb.setFrom(bc::Address(str_from));
    txb.setTo(bc::Address(str_to));
    txb.setTimestamp(time);
    txb.setAmount(amount);
    txb.setFee(fee);

    auto tx1 = txb.build();

    BOOST_CHECK(tx1.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx1.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx1.getTimestamp() == time);
    BOOST_CHECK(tx1.getAmount() == amount);
    BOOST_CHECK(tx1.getFee() == fee);
}


BOOST_AUTO_TEST_CASE(transaction_serialization1)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee = 123;
    bc::Transaction tx1(from, to, amount, time, fee);

    base::SerializationOArchive oa;
    oa.serialize(tx1);

    base::SerializationIArchive ia(oa.getBytes());
    auto tx2 = ia.deserialize<bc::Transaction>();
    BOOST_CHECK(tx1 == tx2);
}


BOOST_AUTO_TEST_CASE(transaction_serialization2)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Balance fee{506};
    bc::Transaction tx1(from, to, amount, time, fee);

    base::SerializationOArchive oa;
    oa.serialize(tx1);

    base::SerializationIArchive ia(oa.getBytes());
    auto tx2 = ia.deserialize<bc::Transaction>();
    BOOST_CHECK(tx1 == tx2);
}
