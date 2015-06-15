#include "catch.hpp"

#include <map>
#include "bitlib2.hpp"


typedef bitlib2::RefCounter<bitlib2::StdAllocatorSelector> RefCounter;


TEST_CASE("refcounter/interface", "[refcounter]") {
    RefCounter rc1;

    REQUIRE(rc1.getCount() == 1); // Counter starts at 1.

    {
        RefCounter rc2;
        rc2 = rc1;

        REQUIRE(rc1.getCount() == 2); // After assignment counter is incremented and shared.
        REQUIRE(rc2.getCount() == 2); // After assignment counter is incremented and shared.

        {
            RefCounter rc3;
            rc3 = rc1;

            REQUIRE(rc1.getCount() == 3); // After assignment counter is incremented and shared.
            REQUIRE(rc2.getCount() == 3); // After assignment counter is incremented and shared.
            REQUIRE(rc3.getCount() == 3); // After assignment counter is incremented and shared.

            rc2 = rc3;

            REQUIRE(rc2.getCount() == 3); // After assignment to refcounter with already shared counter nothing happens.
            REQUIRE(rc3.getCount() == 3); // After assignment to refcounter with already shared counter nothing happens.

            rc2.reset();

            REQUIRE(rc2.getCount() == 1); // After reset of the counter, its count becomes 1.
            REQUIRE(rc3.getCount() == 2); // After reset of a shared counter, count decreases by 1.
        }
    }

    REQUIRE(rc1.getCount() == 1); // After deletion of other refcounter counter is decremented.

    rc1 = rc1;

    REQUIRE(rc1.getCount() == 1); // Assignment to itself does not increment the counter.

    {
        RefCounter rc4(rc1);

        REQUIRE(rc1.getCount() == 2); // After copy construct counter is incremented and shared.
        REQUIRE(rc4.getCount() == 2); // After copy construct counter is incremented and shared.
    }
}


