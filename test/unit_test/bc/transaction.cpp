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
    bc::Transaction tx(from, to, amount, time);

    BOOST_CHECK(tx.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_copy)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx1(from, to, amount, time);
    bc::Transaction tx2(tx1);

    BOOST_CHECK(tx2.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx2.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_move)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx1(from, to, amount, time);
    bc::Transaction tx2(std::move(tx1));

    BOOST_CHECK(tx2.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx2.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_operator_equal_copy)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx1(from, to, amount, time);
    bc::Transaction tx2(from, to, 821481368, time);

    BOOST_CHECK(tx1 != tx2);
    tx2 = tx1;
    BOOST_CHECK(tx2.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx2.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_operator_equal_move)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx1(from, to, amount, time);
    bc::Transaction tx2(from, to, 821481368, time);

    BOOST_CHECK(tx1 != tx2);
    tx2 = std::move(tx1);
    BOOST_CHECK(tx2.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx2.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_set_all1)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::TransactionBuilder txb;

    txb.setFrom(bc::Address(str_from));
    txb.setTo(bc::Address(str_to));
    txb.setTimestamp(time);
    txb.setAmount(amount);

    auto tx = std::move(txb).build();

    BOOST_CHECK(tx.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_set_all2)
{
    bc::Transaction tx(bc::Address{base::Bytes("vjSSDGHS*#%/fg\f").toHex()}, bc::Address{base::Bytes("()#%sdo#%KGD\n/Skg/dfe").toHex()},
        821481368, base::Time());

    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();

    bc::TransactionBuilder txb;
    txb.setFrom(bc::Address(str_from));
    txb.setTo(bc::Address(str_to));
    txb.setTimestamp(time);
    txb.setAmount(amount);

    auto tx1 = txb.build();

    BOOST_CHECK(tx1.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx1.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx1.getTimestamp() == time);
    BOOST_CHECK(tx1.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_serialization1)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    bc::Address from{str_from};
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Address to{str_to};
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx1(from, to, amount, time);

    base::SerializationOArchive oa;
    oa << tx1;

    base::SerializationIArchive ia(oa.getBytes());
    bc::Transaction tx2 = bc::Transaction::deserialize(ia);
    BOOST_CHECK(tx1 == tx2);
}