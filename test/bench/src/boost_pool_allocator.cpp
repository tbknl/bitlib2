#include <iostream>
#include <boost/pool/pool_alloc.hpp>
#include "bitlib2.hpp"


using std::cout;
using std::cerr;
using std::endl;


/**
 * Selector for different types of allocators used within BitVector.
 */
struct BoostPoolAllocatorSelector : public bitlib2::StdAllocatorSelector
{
    template <typename _BitBlock> struct BitBlockContainerAllocator {
        typedef boost::pool_allocator<_BitBlock> type;
    };
    template <typename _Block> struct BitBlockDataAllocator {
        typedef boost::fast_pool_allocator<_Block> type;
    };
    template <typename _RefCounter> struct RefCounterAllocator {
        typedef boost::fast_pool_allocator<_RefCounter> type;
    };
};


typedef bitlib2::BitBlock<4096, BoostPoolAllocatorSelector> BitBlock;
typedef bitlib2::BitVector<BitBlock> BitVector;


#include "allocator_base.hpp"

int main(int argc, char* argv[]) {
    int count = doManyAllocations();

    std::cout << "Count: " << count;
    return 0;
}

