#include "catch.hpp"

#include "bitlib2.hpp"


TEST_CASE("util/gcd", "[util]") {
    REQUIRE(bitlib2::util::gcd(3, 4) == 1);
    REQUIRE(bitlib2::util::gcd(444, 555) == 111);
    REQUIRE(bitlib2::util::gcd(512, 65536) == 512);
    REQUIRE(bitlib2::util::gcd(768, 16384) == 256);
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

