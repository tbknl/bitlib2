#ifndef _BITLIB2_HPP_
#define _BITLIB2_HPP_


#include <vector>
#include <cstring>
#include <memory>


namespace bitlib2 {

    typedef unsigned char byte;


    namespace util {

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
         * Count the number of 'ON' bits in the block.
         * @param bitLength Include only 'bitLength' bits in the count.
         * @return Number of 'ON' bits.
         */
        static std::size_t countBits(const byte* data, std::size_t bitLength) {
            std::size_t count = 0;
            const byte* const end = data + (bitLength / 8);
            const byte* part = data;
            for (; part != end; ++part) {
                count += countLUT[*part];
            }

            const byte mask = ((byte)1 << (bitLength % 8)) - 1;
            if (mask && part <= end) {
                const byte maskedPart = *part & mask;
                count += countLUT[maskedPart];
            }

            return count;
        }


        /**
         * Calculate the greatest common divisor.
         */
        template <typename T> T gcd(T a, T b) {
            while (b > 0) {
                T t = b;
                b = a % b;
                a = t;
            }
            return a;
        }

    } // namespace util


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


    /**
     * Serializer interface
     */
    class ISerializer
    {
        public:
            typedef bitlib2::byte byte;

            virtual ~ISerializer() {}

            /**
             * Start serialization.
             */
            virtual void start() = 0;

            /**
             * End serialization.
             */
            virtual void end() = 0;

            /**
             * Check whether something failed during serialization upto now.
             * @return Failure (true) or no failure (false).
             */
            virtual bool failed() const = 0;

            /**
             * Serialize the inverted flag.
             * @param inverted Inverted (true) or not inverted (false).
             */
            virtual void setInverted(bool inverted) = 0;

            /**
             * Add bit data in a byte buffer.
             * @param bitData Byte data buffer.
             * @param bytesCount Number of bytes in the data buffer.
             * @param onBitCount Number of 'ON' bits in the buffer.
             */
            virtual void addBytes(const byte* data, std::size_t bytesCount, std::size_t onBitCount) = 0;

            /**
             * Add empty bit data.
             * @param bytesCount Number of empty bytes.
             */
            virtual void addEmptyBytes(std::size_t bytesCount) = 0;
    };


    /**
     * Deserializer interface
     */
    class IDeserializer
    {
        public:
            typedef bitlib2::byte byte;

            virtual ~IDeserializer() {}

            /**
             * Start deserialization.
             */
            virtual void start() = 0;

            /**
             * Check whether the inverted flag is set.
             * Note: This information is only available once the (optional)
             * inversion segment is read or the deserialization has finished.
             * @return Inverted (true) or not inverted (false).
             */
            virtual bool isInverted() = 0;

            /**
             * Deserialize bits into a byte block buffer.
             * @param data Data buffer.
             * @param bytesCount Size of data buffer.
             * @param empty Set to true if an empty block is retrieved.
             * @return Data has been read (true) or no data is read (false).
             */
            virtual bool getBytes(byte* data, std::size_t bytesCount, bool& empty) = 0;

            /**
             * Check whether something failed during deserialization upto now.
             * @return Failure (true) or no failure (false).
             */
            virtual bool failed() const = 0;

            /**
             * Return whether deserialization data is finished (or failed).
             * @return Finished (true) or not (false).
             */
            virtual bool finished() const = 0;
    };


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
        template <int BS, int OTS, typename AS> friend class BitBlock;

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
            std::size_t count(const IndexType length = ActualBlockSize) const {
                const byte* const block = this->data.getData();
                if (!block) {
                    return 0;
                }
                return util::countBits(block, std::min((IndexType)ActualBlockSize, length));
            }


            /**
             * Test equality of this and other bitblock (other can have different BitBlock type).
             * @param other Other bitblock.
             * @return Bitblocks are equal (true) or different (false).
             */
            template <int BS, int OTS, typename AS>
            bool operator==(const BitBlock<BS, OTS, AS>& other) const {
                if ((int)ActualBlockSize != (int)BitBlock<BS, OTS, AS>::ActualBlockSize) {
                    return false;
                }

                const byte* myData = this->data.getData();
                const byte* otherData = other.data.getData();

                if (myData && otherData) {
                    return 0 == std::memcmp(myData, otherData, BlockByteCount);
                }
                else if (myData) {
                    return this->count() == 0;
                }
                else if (otherData) {
                    return other.count() == 0;
                }

                return true;
            }


            /**
             * Test equality of this and other bitblock (other can have different BitBlock type).
             * @param other Other bitblock.
             * @return Bitblocks are equal (true) or different (false).
             */
            template <int BS, int OTS, typename AS>
            bool operator!=(const BitBlock<BS, OTS, AS>& other) const {
                return !(*this == other);
            }


            /**
             * Check for equality of the specified ranges.
             * @param myOffset Byte offset of this block.
             * @param other Other block.
             * @param otherOffset Byte offset of other block.
             * @param rangeSize Range size in bytes.
             * @return Equal (true) or different (false).
             */
            template <int BS, int OTS, typename AS>
            bool equalRange(std::size_t myOffset, const BitBlock<BS, OTS, AS>* other, std::size_t otherOffset, std::size_t rangeSize) const {
                const byte* const myData = this->data.getData() ? this->data.getData() + myOffset : NULL;
                const byte* const otherData = other && other->data.getData() ? other->data.getData() + otherOffset : NULL;

                if (myData && otherData) {
                    return 0 == std::memcmp(myData, otherData, rangeSize);
                }
                else if (myData) {
                    return 0 == util::countBits(myData, rangeSize);
                }
                else if (otherData) {
                    return 0 == util::countBits(otherData, rangeSize);
                }
                else {
                    return true;
                }
            }


            /**
             * Serialize the block of data.
             * @param serializer Serializer.
             */
            void serialize(ISerializer& serializer) const {
                const std::size_t onBitCount = this->count();
                if (onBitCount == 0) {
                    serializer.addEmptyBytes(BlockByteCount);
                }
                else {
                    serializer.addBytes(this->data.getData(), BlockByteCount, onBitCount);
                }
            }


            /**
             * Deserialize the block of data.
             * @param deserializer Deserializer.
             * @return Success (true) or failure (false).
             */
            void deserialize(IDeserializer& deserializer) {
                static const std::size_t bufferSize = BlockByteCount > 4096 ? 4096 : BlockByteCount;
                byte buf[bufferSize] = {0}; // Initializes all array elements to 0.
                std::size_t dataOffset = 0;
                bool allEmpty = true;
                byte* data = buf;
                while (dataOffset < BlockByteCount) {
                    const std::size_t readSize = bufferSize > BlockByteCount - dataOffset ? BlockByteCount - dataOffset : bufferSize;
                    bool empty = false;
                    if (!deserializer.getBytes(data, readSize, empty)) {
                        break;
                    }
                    if (!empty && allEmpty) {
                        data = this->data.getMutableData();
                        std::memset(data, 0, BlockByteCount);
                        data += dataOffset;
                        std::memcpy(data, buf, readSize);
                        allEmpty = false;
                    }
                    if (!allEmpty) {
                        data += readSize;
                    }
                    dataOffset += readSize;
                }

                if (allEmpty && this->data.getData()) {
                    this->data = _BitBlockData();
                }
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
        template <typename BB> friend class BitVector;
        typedef _BitBlock _BitBlockType;

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
             * Clear the whole bitvector (i.e. set all bits to zero).
             * @return This instance.
             */
            BitVector& clear() {
                this->inverted = false;
                this->blocks.clear();
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


            /**
             * Test equality of this and other bitvector (other can have different BitBlock type).
             * @param other Other bitvector.
             * @return Bitvectors are equal (true) or different (false).
             */
            template <typename BB> bool operator==(const BitVector<BB>& other) const {
                if (this->inverted != other.inverted) {
                    return false;
                }

                // Optimized treatment if actual BitBlock sizes are equal:
                if ((int)BlockSize == (int)BB::ActualBlockSize) {
                    const std::size_t blockIndexCount = this->blocks.size() > other.blocks.size() ? this->blocks.size() : other.blocks.size();
                    for (std::size_t blockIndex = 0; blockIndex < blockIndexCount; ++blockIndex) {
                        const _BitBlock* myBlock = blockIndex < this->blocks.size() ? &this->blocks[blockIndex] : NULL;
                        const BB* otherBlock = blockIndex < other.blocks.size() ? &other.blocks[blockIndex] : NULL;
                        if (myBlock && otherBlock) {
                            if (*myBlock != *otherBlock) {
                                return false;
                            }
                        }
                        else if (myBlock) {
                            if (myBlock->count() > 0) {
                                return false;
                            }
                        }
                        else if (otherBlock) {
                            if (otherBlock->count() > 0) {
                                return false;
                            }
                        }
                    }
                }
                else {
                    const std::size_t byteIncrement = util::gcd(static_cast<std::size_t>(_BitBlock::BlockByteCount), static_cast<std::size_t>(BB::BlockByteCount));
                    std::size_t byteIndex = 0;
                    while (true) {
                        const std::size_t myBlockIndex = byteIndex / _BitBlock::BlockByteCount;
                        const std::size_t otherBlockIndex = byteIndex / BB::BlockByteCount;
                        const _BitBlock* myBlock = myBlockIndex < this->blocks.size() ? &this->blocks[myBlockIndex] : NULL;
                        const BB* otherBlock = otherBlockIndex < other.blocks.size() ? &other.blocks[otherBlockIndex] : NULL;
                        const std::size_t myBlockOffset = byteIndex % _BitBlock::BlockByteCount;
                        const std::size_t otherBlockOffset = byteIndex % BB::BlockByteCount;
                        if (myBlock) {
                            if (!myBlock->equalRange(myBlockOffset, otherBlock, otherBlockOffset, byteIncrement)) {
                                return false;
                            }
                        }
                        else if (otherBlock) {
                            if (!otherBlock->equalRange(otherBlockOffset, myBlock, myBlockOffset, byteIncrement)) {
                                return false;
                            }
                        }
                        else {
                            break;
                        }

                        byteIndex += byteIncrement;
                    }
                }

                return true;
            }


            /**
             * Test equality of this and other bitvector (other uses a different BitBlock type).
             * @param other Other bitvector.
             * @return Bitvectors are equal (true) or different (false).
             */
            template <typename BB> bool operator!=(const BitVector<BB>& other) const {
                return !(*this == other);
            }


            /**
             * Serialize the bitvector.
             * @param serializer Serializer.
             * @return Success (true) or failure (false).
             */
            bool serialize(ISerializer& serializer) const {
                serializer.start();
                if (serializer.failed()) {
                    return false;
                }
                serializer.setInverted(this->inverted);
                if (serializer.failed()) {
                    return false;
                }
                typename BitBlockContainer::const_iterator it = this->blocks.begin();
                for (; it != this->blocks.end(); ++it) {
                    it->serialize(serializer);
                    if (serializer.failed()) {
                        return false;
                    }
                }
                serializer.end();
                return !serializer.failed();
            }


            /**
             * Deserialize the bitvector.
             * @param deserializer Deserializer.
             * @return Success (true) or failure (false).
             */
            bool deserialize(IDeserializer& deserializer) {
                std::size_t blockIndex = 0;

                deserializer.start();
                while (!deserializer.finished()) {
                    if (blockIndex >= this->blocks.size()) {
                        this->blocks.resize(blockIndex + 1);
                    }
                    this->blocks[blockIndex].deserialize(deserializer);
                    blockIndex += 1;
                }

                if (deserializer.failed()) {
                    return false;
                }

                this->blocks.resize(blockIndex);
                this->inverted = deserializer.isInverted();
                return true;
            }


        private:
            bool inverted;
            BitBlockContainer blocks;
    };


} // namespace bitlib2


#endif // _BITLIB2_HPP_
