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


TEST_CASE("bitvector/equals", "[bitvector]") {
    bitlib2::BitVector<bitlib2::BitBlock<512> > bv1;
    bitlib2::BitVector<bitlib2::BitBlock<512> > bv2;
    bitlib2::BitVector<bitlib2::BitBlock<64> > bv3;

    REQUIRE(bv1 == bv2); // Empty bitvectors with same block size are equal.
    REQUIRE(bv1 == bv3); // Empty bitvectors with different block size are equal.

    bv1.set(513, true);
    REQUIRE(bv1 != bv2); // Bitvectors with same block size but with different bits set to 'ON' are not equal.
    REQUIRE(bv1 != bv3); // Bitvectors with different block size but with different bits set to 'ON' are not equal.

    bv2.set(513, true);
    bv3.set(513, true);
    bv1.set(42, true);
    bv2.set(42, true);
    bv3.set(42, true);
    REQUIRE(bv1 == bv2); // Bitvectors with same block size and the same bits set to 'ON' are equal.
    REQUIRE(bv1 == bv3); // Bitvectors with different block size and the same bits set to 'ON' are equal.

    bv1.invert();
    REQUIRE(bv1 != bv2); // Inverted bitvector is not equal to non-inverted bitvector with same block size.
    REQUIRE(bv1 != bv3); // Inverted bitvector is not equal to non-inverted bitvector with different block size.

    bv2.invert();
    bv3.invert();
    REQUIRE(bv1 == bv2); // Inverted bitvector is equal to inverted bitvector with same block size.
    REQUIRE(bv1 == bv3); // Inverted bitvector is equal to inverted bitvector with different block size.
}


TEST_CASE("bitvector/copying", "[bitvector]") {
    bitlib2::BitVector<bitlib2::BitBlock<256> > bvOrig;
    bvOrig.set(512, true);
    bitlib2::BitVector<bitlib2::BitBlock<256> > bvCopy(bvOrig);

    REQUIRE(bvOrig == bvCopy);

    bvOrig.set(1, true);
    REQUIRE(bvOrig != bvCopy);

    bvOrig.invert();
    bvCopy = bvOrig;
    REQUIRE(bvOrig == bvCopy);

    bvCopy.set(2, false);
    REQUIRE(bvOrig != bvCopy);
}


