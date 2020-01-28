#include <boost/test/unit_test.hpp>

#include "bc/transaction.hpp"


BOOST_AUTO_TEST_CASE(transaction_constructor1)
{
    bc::Transaction tx1;
    bc::Transaction tx2;
    BOOST_CHECK(tx1.getFrom().toString() == "");
    BOOST_CHECK(tx1.getTo().toString() == "");
    BOOST_CHECK(tx1.getTimestamp() == base::Time());
    BOOST_CHECK(tx1.getFrom().toString() == tx2.getFrom().toString());
    BOOST_CHECK(tx1.getTo().toString() == tx2.getTo().toString());
    BOOST_CHECK(tx1.getTimestamp() == tx2.getTimestamp());
    // BOOST_CHECK(tx1.getAmount() == tx2.getAmount());  //TODO: not work now
    // BOOST_CHECK(tx1 == tx2);  //TODO: not work now
}


BOOST_AUTO_TEST_CASE(transaction_constructor2)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx(str_from, str_to, amount, time);

    BOOST_CHECK(tx.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_copy)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx1(str_from, str_to, amount, time);
    bc::Transaction tx2(tx1);

    BOOST_CHECK(tx2.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx2.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_move)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx1(str_from, str_to, amount, time);
    bc::Transaction tx2(std::move(tx1));

    BOOST_CHECK(tx2.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx2.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx2.getTimestamp() == time);
    BOOST_CHECK(tx2.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_operator_equal_copy)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx1(str_from, str_to, amount, time);
    bc::Transaction tx2(str_from, str_to, 821481368, time);

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
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx1(str_from, str_to, amount, time);
    bc::Transaction tx2(str_from, str_to, 821481368, time);

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
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx;

    tx.setFrom(bc::Address(str_from));
    tx.setTo(bc::Address(str_to));
    tx.setTimestamp(time);
    tx.setAmount(amount);

    BOOST_CHECK(tx.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_set_all2)
{
    bc::Transaction tx(base::Bytes("vjSSDGHS*#%/fg\f").toHex(), base::Bytes("()#%sdo#%KGD\n/Skg/dfe").toHex(),
        821481368, base::Time());

    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();

    tx.setFrom(bc::Address(str_from));
    tx.setTo(bc::Address(str_to));
    tx.setTimestamp(time);
    tx.setAmount(amount);

    BOOST_CHECK(tx.getFrom() == bc::Address(str_from));
    BOOST_CHECK(tx.getTo() == bc::Address(str_to));
    BOOST_CHECK(tx.getTimestamp() == time);
    BOOST_CHECK(tx.getAmount() == amount);
}


BOOST_AUTO_TEST_CASE(transaction_serialization1)
{
    bc::Transaction tx1;

    base::SerializationOArchive oa;
    oa << tx1;

    base::SerializationIArchive ia(oa.getBytes());
    bc::Transaction tx2;
    ia >> tx2;
    BOOST_CHECK(tx1 == tx2);
}


BOOST_AUTO_TEST_CASE(transaction_serialization2)
{
    std::string str_from = base::Bytes("vjS#%(247DGFSMKv\n sdf?//").toHex();
    std::string str_to = base::Bytes("()#%9vdmLDSOJ\n\n\\/Skg/dfe").toHex();
    bc::Balance amount = 1239823409;
    auto time = base::Time::now();
    bc::Transaction tx1(str_from, str_to, amount, time);

    base::SerializationOArchive oa;
    oa << tx1;

    base::SerializationIArchive ia(oa.getBytes());
    bc::Transaction tx2;
    ia >> tx2;
    BOOST_CHECK(tx1 == tx2);
}