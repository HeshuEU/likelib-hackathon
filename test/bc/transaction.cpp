#include <boost/test/unit_test.hpp>

#include "base/serialization.hpp"
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
    //BOOST_CHECK(tx1.getAmount() == tx2.getAmount());  //TODO: not work now
    //BOOST_CHECK(tx1 == tx2);  //TODO: not work now
}


BOOST_AUTO_TEST_CASE(transaction_constructor2)
{
    bc::Transaction tx(bc::Address("vjS#%(247DGFSMKv\n sdf?//"), bc::Address("()#%9vdmLDSOJ\n\n\\/Skg/dfe"), 1239823409, base::Time::now());
    BOOST_CHECK(tx.getFrom() == bc::Address("vjS#%(247DGFSMKv\n sdf?//"));
    BOOST_CHECK(tx.getTo() == bc::Address("()#%9vdmLDSOJ\n\n\\/Skg/dfe"));
    BOOST_CHECK(tx.getTimestamp() == base::Time::now());
    BOOST_CHECK(tx.getAmount() == 1239823409);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_copy)
{
    bc::Transaction tx1(bc::Address("vjS#%(247DGFSMKv\n sdf?//"), bc::Address("()#%9vdmLDSOJ\n\n\\/Skg/dfe"), 1239823409, base::Time::now());
    bc::Transaction tx2(tx1);

    BOOST_CHECK(tx2.getFrom() == bc::Address("vjS#%(247DGFSMKv\n sdf?//"));
    BOOST_CHECK(tx2.getTo() == bc::Address("()#%9vdmLDSOJ\n\n\\/Skg/dfe"));
    BOOST_CHECK(tx2.getTimestamp() == base::Time::now());
    BOOST_CHECK(tx2.getAmount() == 1239823409);
}


BOOST_AUTO_TEST_CASE(transaction_constructor_move)
{
    bc::Transaction tx1(bc::Address("vjS#%(247DGFSMKv\n sdf?//"), bc::Address("()#%9vdmLDSOJ\n\n\\/Skg/dfe"), 1239823409, base::Time::now());
    bc::Transaction tx2(std::move(tx1));

    BOOST_CHECK(tx2.getFrom() == bc::Address("vjS#%(247DGFSMKv\n sdf?//"));
    BOOST_CHECK(tx2.getTo() == bc::Address("()#%9vdmLDSOJ\n\n\\/Skg/dfe"));
    BOOST_CHECK(tx2.getTimestamp() == base::Time::now());
    BOOST_CHECK(tx2.getAmount() == 1239823409);
}


BOOST_AUTO_TEST_CASE(transaction_operator_equal_copy)
{
    bc::Transaction tx1(bc::Address("vjS#%(247DGFSMKv\n sdf?//"), bc::Address("()#%9vdmLDSOJ\n\n\\/Skg/dfe"), 1239823409, base::Time::now());
    bc::Transaction tx2(bc::Address("vjSSDGHS*#%/fg\f"), bc::Address("()#%sdo#%KGD\n/Skg/dfe"), 821481368, base::Time::now());

    BOOST_CHECK(tx1 != tx2);
    tx2 = tx1;
    BOOST_CHECK(tx2.getFrom() == bc::Address("vjS#%(247DGFSMKv\n sdf?//"));
    BOOST_CHECK(tx2.getTo() == bc::Address("()#%9vdmLDSOJ\n\n\\/Skg/dfe"));
    BOOST_CHECK(tx2.getTimestamp() == base::Time::now());
    BOOST_CHECK(tx2.getAmount() == 1239823409);
}


BOOST_AUTO_TEST_CASE(transaction_operator_equal_move)
{
    bc::Transaction tx1(bc::Address("vjS#%(247DGFSMKv\n sdf?//"), bc::Address("()#%9vdmLDSOJ\n\n\\/Skg/dfe"), 1239823409, base::Time::now());
    bc::Transaction tx2(bc::Address("vjSSDGHS*#%/fg\f"), bc::Address("()#%sdo#%KGD\n/Skg/dfe"), 821481368, base::Time::now());

    BOOST_CHECK(tx1 != tx2);
    tx2 = std::move(tx1);
    BOOST_CHECK(tx2.getFrom() == bc::Address("vjS#%(247DGFSMKv\n sdf?//"));
    BOOST_CHECK(tx2.getTo() == bc::Address("()#%9vdmLDSOJ\n\n\\/Skg/dfe"));
    BOOST_CHECK(tx2.getTimestamp() == base::Time::now());
    BOOST_CHECK(tx2.getAmount() == 1239823409);
}