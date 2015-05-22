
#include <iostream>
#include <vector>
#include <cstring>
#include <memory>

using std::cout;
using std::cerr;
using std::endl;


namespace bitlib2 {


    /**
     * Lookup table for bit-counts of all 8-bit unsigned integers.
     */
    static const char countLUT[] = {
        0, 1, 1, 2, 1, 2, 2, 3,
        1, 2, 2, 3, 2, 3, 3, 4,
        1, 2, 2, 3, 2, 3, 3, 4,
        2, 3, 3, 4, 3, 4, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4,
        2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5,
        3, 4, 4, 5, 4, 5, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4,
        2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5,
        3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5,
        3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6,
        4, 5, 5, 6, 5, 6, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4,
        2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5,
        3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5,
        3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6,
        4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5,
        3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6,
        4, 5, 5, 6, 5, 6, 6, 7,
        3, 4, 4, 5, 4, 5, 5, 6,
        4, 5, 5, 6, 5, 6, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7,
        5, 6, 6, 7, 6, 7, 7, 8
    };


    /**
     * Selector for different types of allocators used within BitVector.
     */
    struct StdAllocatorSelector
    {
        template <typename _BitBlock> struct BitBlockContainerAllocator {
            typedef std::allocator<_BitBlock> type;
        };
        template <typename _Block> struct BitBlockDataAllocator {
            typedef std::allocator<_Block> type;
        };
        template <typename _RefCounter> struct RefCounterAllocator {
            typedef std::allocator<_RefCounter> type;
        };
    };


    /**
     * Reference counter.
     * Note: Only needs to allocate memory when counter > 1.
     */
    template < typename _AllocatorSelector >
    class RefCounter
    {
        public:
            typedef std::size_t CounterType;
            typedef typename _AllocatorSelector::template RefCounterAllocator<CounterType>::type CounterAllocator;

            RefCounter() : count(NULL) {
            }

            RefCounter(const RefCounter& other) : count(NULL) {
                this->combine(other);
            }

            ~RefCounter() {
                this->decrease();
            }

            RefCounter& operator=(const RefCounter& other) {
                this->combine(other);
            }

            CounterType getCount() const {
                return this->count ? *this->count : 1;
            }

        private:
            void combine(const RefCounter& other) {
                this->decrease();
                if (!other.count) {
                    other.increase();
                }
                this->count = other.count;
                this->increase();
            }

            void increase() const {
                if (!this->count) {
                    this->count = getCounterAllocator().allocate(1);
                    *this->count = 1;
                }
                else {
                    *this->count += 1;
                }
            }

            void decrease() const {
                if (this->count) {
                    *this->count -= 1;
                    if (*this->count == 0) {
                        getCounterAllocator().deallocate(this->count, 1);
                    }
                }
            }

            static CounterAllocator& getCounterAllocator() {
                static CounterAllocator allocator;
                return allocator;
            }

            mutable CounterType* count;
    };


    template <int BlockSize>
    struct BitBlockType {
        typedef typename BitBlockType::INVALID_BIT_SIZE type;
    };

    // TODO! Check at compile time that these types indeed have this size.
    template <> struct BitBlockType<64> { typedef unsigned long long type; };
    template <> struct BitBlockType<32> { typedef unsigned int type; };
    template <> struct BitBlockType<16> { typedef unsigned short type; };
    template <> struct BitBlockType<8> { typedef unsigned char type; };


    /**
     * A block of bit data managed with a reference counter.
     */
    template <
        typename BlockType,
        int BlockLength,
        typename _AllocatorSelector
    >
    class BitBlockData
    {
        struct Block {
            BlockType data[BlockLength];
        };

        typedef typename _AllocatorSelector::template BitBlockDataAllocator<Block>::type BlockDataAllocator;
        typedef RefCounter<_AllocatorSelector> _RefCounter;

        public:
            /**
             * @constructor
             */
            BitBlockData() :
                block(NULL)
            {
            }


            /**
             * @constructor
             * Copy constructor.
             * @param other Other bit-block.
             */
            BitBlockData(const BitBlockData& other) :
                block(other.block),
                refCounter(other.block ? other.refCounter : _RefCounter())
            {
            }


            /**
             * @destructor
             */
            ~BitBlockData() {
                if (this->block && this->refCounter.getCount() == 1) {
                    //cout << "Deallocate " << this->block << endl;
                    getDataAllocatorInstance().deallocate(this->block, 1);
                }
            }


            /**
             * Assignment.
             * @param other Other bit-block.
             * @return This instance.
             */
            BitBlockData& operator=(const BitBlockData& other) {
                if (this->block && this->refCounter.getCount() == 1) {
                    //cout << "Deallocate " << this->block << endl;
                    getDataAllocatorInstance().deallocate(this->block, 1);
                }
                this->block = other.block;
                if (this->block) {
                    this->refCounter = other.refCounter;
                }
            }


            /**
             * Get the block data.
             * @return The data block or NULL.
             */
            const BlockType* getData() const {
                return this->block ? this->block->data : NULL;
            }


            /**
             * Get the (mutable) block data.
             * Note: If there is no data yet or data is shared with another block, then memory will be allocated.
             * @return The data block.
             */
            BlockType* getMutableData() {
                if (!this->block) {
                    this->allocate();
                }
                else if (this->refCounter.getCount() != 1) {
                    this->allocate();
                    this->refCounter = _RefCounter();
                }
                return this->block->data;
            }

        private:
            /**
             * Allocate memory for the block data.
             * Note: Existing (shared) data will be copied over.
             */
            void allocate() {
                const Block* const oldBlock = this->block;
                void* blockMem = getDataAllocatorInstance().allocate(1, NULL);
                this->block = new(blockMem) Block();
                //cout << "Allocate " << this->block << endl;
                // TODO: Check for memory allocation error.
                if (oldBlock) {
                    std::memcpy(this->block, oldBlock, sizeof(Block));
                }
            }


            /**
             * Return the singleton allocator instance for block data.
             */
            static BlockDataAllocator& getDataAllocatorInstance() {
                static BlockDataAllocator allocator;
                return allocator;
            }

            Block* block;
            _RefCounter refCounter;
    };


    /**
     * A block of bit data with bit-operations.
     */
    template <
        int _BlockSize = 65536,
        int BlockTypeSize = 64,
        typename _AllocatorSelector = StdAllocatorSelector
    >
    class BitBlock
    {
        public:
            typedef _AllocatorSelector AllocatorSelector;
            typedef std::size_t IndexType;
            typedef typename BitBlockType<BlockTypeSize>::type BlockType;
            enum {
                BlockSize = _BlockSize,
                BlockLength = 1 + ((BlockSize - 1) / (sizeof(BlockType) * 8)),
            };
            typedef BitBlockData<BlockType, BlockLength, _AllocatorSelector> _BitBlockData;


            /**
             * @constructor
             */
            BitBlock()
            {
            }


            /**
             * Set the value of a bit by index.
             * @param index Bit index.
             * @param value On (true) or off (false).
             */
            void set(IndexType index, bool value) {
                if (!this->data.getData() && !value) {
                    return;
                }
                BlockType* block = this->data.getMutableData();
                BlockType& part = block[index / BlockTypeSize];
                if (value) {
                    part |= ((BlockType)1 << (index % BlockTypeSize));
                }
                else {
                    part &= ~((BlockType)1 << (index % BlockTypeSize));
                }
            }


            /**
             * Get the value of a bit by index.
             * @param index Bit index.
             * @return On (true) or off (false).
             */
            bool get(IndexType index) const {
                const BlockType* block = this->data.getData();
                if (!block) {
                    return false;
                }
                return 0 != (block[index / BlockTypeSize] & ((BlockType)1 << (index % BlockTypeSize)));
            }


            /**
             * Count the number of 'ON' bits in the block.
             * @param length Include only 'length' bits in the count (default: all bits).
             * @return Number of 'ON' bits.
             */
            int count(const IndexType length = BlockSize) const {
                const BlockType* block = this->data.getData();
                if (!block) {
                    return 0;
                }

                int count = 0;
                const BlockType* const end = block + std::min((IndexType)BlockLength, length / BlockTypeSize);
                const BlockType* part = block;
                for (; part != end; ++part) {
                    for (IndexType bitIndex = 0; bitIndex < BlockTypeSize; bitIndex += 8) {
                        count += countLUT[(*part >> bitIndex) & 0xFF];
                    }
                }
                
                if (length / BlockTypeSize < BlockLength) {
                    const BlockType mask = ((BlockType)1 << (length % BlockTypeSize)) - 1;
                    const BlockType maskedPart = *part & mask;
                    for (IndexType bitIndex = 0; bitIndex < BlockTypeSize; bitIndex += 8) {
                        count += countLUT[(maskedPart >> bitIndex) & 0xFF];
                    }
                }

                return count;
            }

        private:
            _BitBlockData data;
    };


    /**
     * A bit-vector of infinite length with bit-operations.
     */
    template < typename _BitBlock = BitBlock<> >
    class BitVector
    {
        public:
            enum { BlockSize = _BitBlock::BlockSize };
            typedef typename _BitBlock::IndexType IndexType;
            typedef typename _BitBlock::AllocatorSelector::template BitBlockContainerAllocator<_BitBlock>::type BitBlockContainerAllocator;
            typedef std::vector< _BitBlock, BitBlockContainerAllocator > BitBlockContainer;

            /**
             * @constructor
             */
            BitVector() :
                inverted(false)
            {
            }


            /**
             * Set the bit at the specified index.
             * @param index Bit index.
             * @param value On (true) or off (false).
             */
            void set(IndexType index, bool value) {
                const typename BitBlockContainer::size_type blockIndex = index / BlockSize;
                const bool indexContained = blockIndex < this->blocks.size();
                if (!indexContained) {
                    if (value == this->inverted) {
                        return;
                    }
                    this->blocks.resize(blockIndex + 1);
                }
                this->blocks[blockIndex].set(index % BlockSize, inverted ? !value : value);
            }


            /**
             * Get the bit value at the specified index.
             * @param index Bit index.
             * @return On (true) or off (false).
             */
            bool get(IndexType index) const {
                const typename BitBlockContainer::size_type blockIndex = index / BlockSize;
                const bool indexContained = blockIndex < this->blocks.size();
                if (!indexContained) {
                    return this->inverted;
                }
                const bool value = this->blocks[blockIndex].get(index % BlockSize);
                return this->inverted ? !value : value;
            }


            /**
             * Invert the whole bitvector.
             * Note: Instant operation, no data is touched expect the 'inverted' flag.
             * @return This instance.
             */
            BitVector& invert() {
                this->inverted = !this->inverted;
                return *this;
            }


            /**
             * Count the number of 'ON' bits in the vector.
             * Note: If length parameter is 0 (default) and the bitvector is inverted then the
             *       return value will be -1.
             * @param length Assumed length of the bitvector (or no assumed length if 0).
             * @return Number of 'ON' bits or -1 if infinite.
             */
            int count(IndexType length = 0) const {
                int count = 0;
                if (length == 0) {
                    if (this->inverted) {
                        return -1;
                    }
                    for (typename BitBlockContainer::const_iterator it = blocks.begin(); it != blocks.end(); ++it) {
                        count += it->count();
                    }
                }
                else {
                    const typename BitBlockContainer::size_type endBlockIndex = length / BlockSize;
                    const bool indexContained = endBlockIndex < this->blocks.size();
                    typename BitBlockContainer::const_iterator it = this->blocks.begin();
                    const typename BitBlockContainer::const_iterator end = indexContained ? it + endBlockIndex : this->blocks.end();
                    for (; it != end; ++it) {
                        count += it->count();
                    }

                    const IndexType bitsLeft = length % BlockSize;
                    if (it != this->blocks.end() && bitsLeft != 0) {
                        count += it->count(bitsLeft);
                    }

                    count = this->inverted ? length - count : count;
                }
                return count;
            }


        private:
            bool inverted;
            BitBlockContainer blocks;
    };


} // namespace bitlib2


using namespace bitlib2;

typedef BitBlock<512> BB;
typedef BitVector<BB> BV;


int main(int argc, char* argv[]) {
    BV bv1;
    for (int i = 0; i < 1000000000; i += 1000) {
        bv1.set(i, 1);
    }
    int total = 0;
    for (int r = 0; r < 100; ++r) {
        bv1.set(r * 12345, 1);
        total += bv1.count();
    }
    cout << total << endl;
    return 0;
}

