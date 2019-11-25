#include <boost/test/unit_test.hpp>

#include "base/serialization.hpp"
#include "bc/transaction.hpp"

#include <iostream>

BOOST_AUTO_TEST_CASE(transaction_serialization_test)
{
    bc::Transaction tx;
    tx.setAmount(123);
    tx.setFrom("AAAAA3fewfeifjoewf3afa;jAAAAAAAA");
    tx.setTo("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");
    bc::Transaction tx1 = base::fromBytes<bc::Transaction>(base::toBytes(tx));
    BOOST_CHECK( tx.getFrom() == tx1.getFrom());
}
