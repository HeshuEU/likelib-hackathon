#include <boost/test/unit_test.hpp>

#include "core/consensus.hpp"

// BOOST_AUTO_TEST_CASE(consensus_inital_condition_check)
//{
//    lk::Consensus c;
//
//    base::FixedBytes<32> b;
//    for (std::size_t i = 0; i < 32; ++i) {
//        b[i] = 0xFF;
//    }
//
//    BOOST_CHECK(c.getComplexity().getComparer() == b);
//}
//
//
// BOOST_AUTO_TEST_CASE(consensus_check1)
//{
//    lk::Consensus c;
//
//    lk::Block b1{ 0, base::Sha256::null(), base::Time::now(), lk::Address::null(), {} };
//    BOOST_CHECK(c.checkBlock(b1));
//
//    c.applyBlock(b1);
//
//    base::FixedBytes<32> b;
//    for (std::size_t i = 0; i < 32; ++i) {
//        b[i] = 0xFF;
//    }
//
//    BOOST_CHECK(base::config::BC_DIFFICULTY_RECALCULATION_RATE > 1 || b == c.getComplexity().getComparer());
//}