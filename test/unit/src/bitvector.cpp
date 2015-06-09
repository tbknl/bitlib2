#include "catch.hpp"

#include "bitlib2.hpp"


TEST_CASE("bitvector/set_and_get", "[bitvector]") {
    bitlib2::BitVector<> bv;

    REQUIRE(bv.get(0) == false); // Bitvector starts with all bits off.
    REQUIRE(bv.get(1) == false); // Bitvector starts with all bits off.
    REQUIRE(bv.get(2) == false); // Bitvector starts with all bits off.

    REQUIRE(&bv.set(0, true) == &bv); // Set method returns itself.
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


TEST_CASE("bitvector/bitwise_and", "[bitvector]") {
    // Bitwise AND with empty bitvector clears first:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;

        bv1.set(0, true).set(123, true).set(1234, true);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.count() == 0); // Bitwise and with empty bitvector results in empty bitvector.
    }

    // Bitwise AND with first bitvector empty results in empty bitvector:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;

        bv2.set(0, true).set(123, true).set(1234, true);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.count() == 0); // Bitwise and with empty first bitvector results in empty bitvector.
    }

    // Bitvectors with equal number of blocks:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;

        bv1.set(0, true).set(1, true);
        bv2.set(0, true).set(2, true);
        bv1.set(256 * 3 + 123, true);
        bv2.set(256 * 3 + 123, true);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.get(0) == true); // Bitwise and with both bits on results in on.
        REQUIRE(bv1.get(1) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(2) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(3) == false); // Bitwise and with both bits off results in off.
        REQUIRE(bv1.get(256 * 3 + 123) == true); // Bitwise and with both bits on outside of first block results in on.
        REQUIRE(bv1.get(256 * 3 + 124) == false); // Bitwise and with both bits off outside of first block results in off.
        REQUIRE(bv1.isInverted() == false); // First bitvector should not be inverted after bitwise and.
    }

    // First bitvector is longer:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;

        bv1.set(0, true).set(1, true);
        bv2.set(0, true).set(2, true);
        bv1.set(256 * 3 + 456, true);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.get(0) == true); // Bitwise and with both bits on results in on.
        REQUIRE(bv1.get(1) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(2) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(3) == false); // Bitwise and with both bits off results in off.
        REQUIRE(bv1.get(256 * 3 + 456) == false); // Bitwise and with only one bit on outside of range of second bitvector results in off.
        REQUIRE(bv1.get(256 * 3 + 457) == false); // Bitwise and with both bits off outside of range of second bitvector results in off.
    }

    // Second bitvector is longer:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;

        bv1.set(0, true).set(1, true);
        bv2.set(0, true).set(2, true);
        bv2.set(256 * 3 + 456, true);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.get(0) == true); // Bitwise and with both bits on results in on.
        REQUIRE(bv1.get(1) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(2) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(3) == false); // Bitwise and with both bits off results in off.
        REQUIRE(bv1.get(256 * 3 + 456) == false); // Bitwise and with only one bit on outside of range of second bitvector results in off.
        REQUIRE(bv1.get(256 * 3 + 457) == false); // Bitwise and with both bits off outside of range of second bitvector results in off.
    }

    // First bitvector is inverted, first bitvector has more blocks:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;
    
        bv1.invert();
        bv1.set(0, true).set(1, true).set(2, false).set(3, false).set(256 * 3 + 789, false);
        bv2.set(0, true).set(1, false).set(2, true).set(3, false);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.get(0) == true); // Bitwise and with both bits on results in on.
        REQUIRE(bv1.get(1) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(2) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(3) == false); // Bitwise and with both bits off results in off.
        REQUIRE(bv1.get(256 * 3 + 789) == false); // Bitwise and with bit off outside of range of second bitvector results in off.
        REQUIRE(bv1.get(256 * 3 + 790) == false); // Bitwise and with bit on outside of range of second bitvector results in off.
        REQUIRE(bv1.isInverted() == false); // First bitvector should not be inverted after bitwise and.
    }

    // First bitvector is inverted, second bitvector has more blocks:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;
    
        bv1.invert();
        bv1.set(0, true).set(1, true).set(2, false).set(3, false);
        bv2.set(0, true).set(1, false).set(2, true).set(3, false).set(256 * 3 + 789, true);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.get(0) == true); // Bitwise and with both bits on results in on.
        REQUIRE(bv1.get(1) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(2) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(3) == false); // Bitwise and with both bits off results in off.
        REQUIRE(bv1.get(256 * 3 + 789) == true); // Bitwise and with bit on outside of range of first bitvector results in on.
        REQUIRE(bv1.get(256 * 3 + 790) == false); // Bitwise and with bit off outside of range of first bitvector results in off.
        REQUIRE(bv1.isInverted() == false); // First bitvector should not be inverted after bitwise and.
    }

    // Second bitvector is inverted, first bitvector has more blocks:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;
    
        bv2.invert();
        bv1.set(0, true).set(1, true).set(2, false).set(3, false).set(256 * 3 + 789, true);
        bv2.set(0, true).set(1, false).set(2, true).set(3, false);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.get(0) == true); // Bitwise and with both bits on results in on.
        REQUIRE(bv1.get(1) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(2) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(3) == false); // Bitwise and with both bits off results in off.
        REQUIRE(bv1.get(256 * 3 + 789) == true); // Bitwise and with bit on outside of range of second bitvector results in on.
        REQUIRE(bv1.get(256 * 3 + 790) == false); // Bitwise and with bit off outside of range of second bitvector results in off.
        REQUIRE(bv1.isInverted() == false); // First bitvector should not be inverted after bitwise and.
    }

    // Second bitvector is inverted, second bitvector has more blocks:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;
    
        bv2.invert();
        bv1.set(0, true).set(1, true).set(2, false).set(3, false);
        bv2.set(0, true).set(1, false).set(2, true).set(3, false).set(256 * 3 + 789, false);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.get(0) == true); // Bitwise and with both bits on results in on.
        REQUIRE(bv1.get(1) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(2) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(3) == false); // Bitwise and with both bits off results in off.
        REQUIRE(bv1.get(256 * 3 + 789) == false); // Bitwise and with bit off outside of range of first bitvector results in off.
        REQUIRE(bv1.get(256 * 3 + 790) == false); // Bitwise and with bit on outside of range of first bitvector results in off.
        REQUIRE(bv1.isInverted() == false); // First bitvector should not be inverted after bitwise and.
    }

    // Both bitvectors are inverted, first bitvector has more blocks:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;
    
        bv1.invert();
        bv2.invert();
        bv1.set(0, true).set(1, true).set(2, false).set(3, false).set(256 * 3 + 789, false);
        bv2.set(0, true).set(1, false).set(2, true).set(3, false);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.get(0) == true); // Bitwise and with both bits on results in on.
        REQUIRE(bv1.get(1) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(2) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(3) == false); // Bitwise and with both bits off results in off.
        REQUIRE(bv1.get(256 * 3 + 789) == false); // Bitwise and with bit off outside of range of second bitvector results in off.
        REQUIRE(bv1.get(256 * 3 + 790) == true); // Bitwise and with bit on outside of range of second bitvector results in off.
        REQUIRE(bv1.isInverted() == true); // First bitvector should not be inverted after bitwise and.
    }

    // Both bitvectors are inverted, second bitvector has more blocks:
    {
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv1;
        bitlib2::BitVector<bitlib2::BitBlock<256> > bv2;
    
        bv1.invert();
        bv2.invert();
        bv1.set(0, true).set(1, true).set(2, false).set(3, false);
        bv2.set(0, true).set(1, false).set(2, true).set(3, false).set(256 * 3 + 789, false);
        bv1.bitAnd(bv2);
        REQUIRE(bv1.get(0) == true); // Bitwise and with both bits on results in on.
        REQUIRE(bv1.get(1) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(2) == false); // Bitwise and with one bit on results in off.
        REQUIRE(bv1.get(3) == false); // Bitwise and with both bits off results in off.
        REQUIRE(bv1.get(256 * 3 + 789) == false); // Bitwise and with bit off outside of range of first bitvector results in off.
        REQUIRE(bv1.get(256 * 3 + 790) == true); // Bitwise and with bit on outside of range of first bitvector results in on.
        REQUIRE(bv1.isInverted() == true); // First bitvector should not be inverted after bitwise and.
    }

}


