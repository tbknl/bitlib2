#include <sstream>

#include "catch.hpp"

#include "bitlib2.hpp"
#include "bitlib2_serialize.hpp"


TEST_CASE("serialize/serializer_interface", "[serialize]") {
    // TODO: Decide on a mocking framework first.
}


TEST_CASE("serialize/deserializer_interface", "[serialize]") {
    // TODO: Decide on a mocking framework first.
}


TEST_CASE("serialize/streamserializer", "[serialize]") {
    std::string serializedData;
    bitlib2::BitVector<bitlib2::BitBlock<64> > bv1;

    {
        std::ostringstream oss;
        bitlib2::serialize::StreamSerializer serializer(oss);

        bv1.set(123, true);
        bv1.set(345, true);
        bv1.serialize(serializer);
        REQUIRE(serializer.failed() == false);
        serializedData = oss.str();
        REQUIRE(serializedData.length() > 0);
    }

    {
        std::istringstream iss(serializedData);
        bitlib2::serialize::StreamDeserializer deserializer(iss);
        bitlib2::BitVector<bitlib2::BitBlock<128> > bv2;

        bv2.deserialize(deserializer);
        REQUIRE(deserializer.failed() == false);
        REQUIRE(bv2.get(0) == false);
        REQUIRE(bv2.get(123) == true);
        REQUIRE(bv2.get(345) == true);
        REQUIRE(bv2.get(567) == false);
        REQUIRE(bv1 == bv2);
    }

    {
        std::istringstream iss(serializedData);
        bitlib2::serialize::StreamDeserializer deserializer(iss);
        bitlib2::BitVector<bitlib2::BitBlock<64> > bv2;

        bv2.deserialize(deserializer);
        REQUIRE(bv1 == bv2);
    }

}

