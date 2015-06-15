#include "catch.hpp"

#include "bitlib2.hpp"


TEST_CASE("util/gcd", "[util]") {
    const int gcd1 = bitlib2::util::GCD<3, 4>::value;
    REQUIRE(gcd1 == 1);
    const int gcd2 = bitlib2::util::GCD<444, 555>::value;
    REQUIRE(gcd2 == 111);
    const int gcd3 = bitlib2::util::GCD<512, 65536>::value;
    REQUIRE(gcd3 == 512);
    const int gcd4 = bitlib2::util::GCD<768, 16384>::value;
    REQUIRE(gcd4 == 256);
}


TEST_CASE("util/countBits", "[util]") {
    const bitlib2::byte data[] = {0x02, 0x0C, 0xA0};
    REQUIRE(bitlib2::util::countBits(data, 1) == 0);
    REQUIRE(bitlib2::util::countBits(data, 2) == 1);
    REQUIRE(bitlib2::util::countBits(data, 10) == 1);
    REQUIRE(bitlib2::util::countBits(data, 11) == 2);
    REQUIRE(bitlib2::util::countBits(data, 12) == 3);
    REQUIRE(bitlib2::util::countBits(data, 13) == 3);
    REQUIRE(bitlib2::util::countBits(data, 22) == 4);
    REQUIRE(bitlib2::util::countBits(data, 24) == 5);
}

