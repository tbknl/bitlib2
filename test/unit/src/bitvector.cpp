#include "catch.hpp"

#include "bitlib2.hpp"


TEST_CASE("bitvector/set_and_get", "[bitvector]") {
    bitlib2::BitVector<> bv;

    REQUIRE(bv.get(0) == false); // Bitvector starts with all bits off.
    REQUIRE(bv.get(1) == false); // Bitvector starts with all bits off.
    REQUIRE(bv.get(2) == false); // Bitvector starts with all bits off.

    bv.set(0, true);
    bv.set(1, false);
    bv.set(2, true);
    REQUIRE(bv.get(0) == true); // This bit was set to on.
    REQUIRE(bv.get(1) == false); // This bit was set to off.
    REQUIRE(bv.get(2) == true); // This bit was set to on.
}


TEST_CASE("bitvector/invert", "[bitvector]") {
    bitlib2::BitVector<> bv;

    REQUIRE(&bv.invert() == &bv); // Invert returns itself.
    REQUIRE(bv.get(0) == true); // All bits of inverted empty bitvector are on.
    REQUIRE(bv.get(1) == true); // All bits of inverted empty bitvector are on.
    REQUIRE(bv.get(2) == true); // All bits of inverted empty bitvector are on.

    bv.set(0, false);
    bv.set(1, true);
    REQUIRE(bv.get(0) == false); // This bit was set to off.
    REQUIRE(bv.get(1) == true); // This bit was set to on.

    bv.invert();
    REQUIRE(bv.get(0) == true); // This bit was set to off, but is inverted now.
    REQUIRE(bv.get(1) == false); // This bit was set to on, but is inverted now.
    REQUIRE(bv.get(2) == false); // This bit was never set.
}

