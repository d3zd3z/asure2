#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(framework)
BOOST_AUTO_TEST_CASE(simple)
{
  BOOST_REQUIRE_EQUAL(1, 1);
  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(failed)
{
//  BOOST_CHECK(false);
}

BOOST_AUTO_TEST_SUITE_END()
