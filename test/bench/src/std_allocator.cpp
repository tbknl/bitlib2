#include <iostream>
#include "bitlib2.hpp"


using std::cout;
using std::cerr;
using std::endl;


typedef bitlib2::BitBlock<4096> BitBlock;
typedef bitlib2::BitVector<BitBlock> BitVector;


#include "allocator_base.hpp"

int main(int argc, char* argv[]) {
    int count = doManyAllocations();

    std::cout << "Count: " << count;
    return 0;
}

