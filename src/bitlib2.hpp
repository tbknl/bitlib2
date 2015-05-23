#ifndef _BITLIB2_HPP_
#define _BITLIB2_HPP_


#include <vector>
#include <cstring>
#include <memory>


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
                return *this;
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
                    this->count = getCounterAllocator().allocate(1, NULL);
                    *this->count = 1;
                }
                else {
                    *this->count += 1;
                }
            }

            void decrease() const {
                if (this->count) {
                    if (*this->count == 1) {
                        getCounterAllocator().deallocate(this->count, 1);
                        this->count = NULL;
                    }
                    else {
                        *this->count -= 1;
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

    typedef unsigned char byte;


    /**
     * A block of bit data managed with a reference counter.
     */
    template <
        int BlockByteCount,
        typename _AllocatorSelector
    >
    class BitBlockData
    {
        private:
            struct Block {
                byte data[BlockByteCount];
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
                    getDataAllocatorInstance().deallocate(this->block, 1);
                }
                this->block = other.block;
                if (this->block) {
                    this->refCounter = other.refCounter;
                }
                return *this;
            }


            /**
             * Get the block data.
             * @return The data block or NULL.
             */
            const byte* getData() const {
                return this->block ? this->block->data : NULL;
            }


            /**
             * Get the (mutable) block data.
             * Note: If there is no data yet or data is shared with another block, then memory will be allocated.
             * @return The data block.
             */
            byte* getMutableData() {
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
                // TODO: Check for memory allocation error.
                if (oldBlock) {
                    std::memcpy(this->block->data, oldBlock->data, BlockByteCount);
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
        int OperationTypeSize = 64,
        typename _AllocatorSelector = StdAllocatorSelector
    >
    class BitBlock
    {
        public:
            typedef _AllocatorSelector AllocatorSelector;
            typedef std::size_t IndexType;
            typedef typename BitBlockType<OperationTypeSize>::type OperationType;
            enum {
                OperationTypeLength = 1 + ((_BlockSize - 1) / (sizeof(OperationType) * 8)),
                BlockByteCount = OperationTypeLength * sizeof(OperationType),
                ActualBlockSize = BlockByteCount * 8,
            };
            typedef BitBlockData<BlockByteCount, _AllocatorSelector> _BitBlockData;


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
                byte* block = this->data.getMutableData();
                byte& part = block[index / 8];
                if (value) {
                    part |= ((byte)1 << (index % 8));
                }
                else {
                    part &= ~((byte)1 << (index % 8));
                }
            }


            /**
             * Get the value of a bit by index.
             * @param index Bit index.
             * @return On (true) or off (false).
             */
            bool get(IndexType index) const {
                const byte* block = this->data.getData();
                if (!block) {
                    return false;
                }
                return 0 != (block[index / 8] & ((byte)1 << (index % 8)));
            }


            /**
             * Count the number of 'ON' bits in the block.
             * @param length Include only 'length' bits in the count (default: all bits).
             * @return Number of 'ON' bits.
             */
            int count(const IndexType length = ActualBlockSize) const {
                const byte* const block = this->data.getData();
                if (!block) {
                    return 0;
                }

                int count = 0;
                const byte* const end = block + std::min((IndexType)BlockByteCount, length / 8);
                const byte* part = block;
                for (; part != end; ++part) {
                    count += countLUT[*part];
                }
                
                const byte mask = ((byte)1 << (length % 8)) - 1;
                if (mask && part < block + BlockByteCount) {
                    const byte maskedPart = *part & mask;
                    count += countLUT[maskedPart];
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
            enum { BlockSize = _BitBlock::ActualBlockSize };
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
                this->blocks[blockIndex].set(index % BlockSize, this->inverted ? !value : value);
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


#endif // _BITLIB2_HPP_
