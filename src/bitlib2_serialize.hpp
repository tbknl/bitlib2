#ifndef _BITLIB2_SERIALIZE_HPP_
#define _BITLIB2_SERIALIZE_HPP_


#include "bitlib2.hpp"


namespace bitlib2 {
namespace serialize {


    /**
     * Simple stream serializer.
     */
    class StreamSerializer : public ISerializer
    {
        public:
            enum {
                SEGMENTTYPE_START = 0,
                SEGMENTTYPE_INVERTED = 1,
                SEGMENTTYPE_EMPTYBLOCK_32BIT = 2, // 32 bit length
                SEGMENTTYPE_BLOCK_32BIT = 3, // 32 bit length
                SEGMENTTYPE_END = 255,
            };

            /**
             * Constructor.
             * @param stream Output stream.
             */
            StreamSerializer(std::ostream& stream) :
                outputStream(stream)
            {
            }


            /**
             * Start serialization.
             */
            virtual void start() {
                const byte data = SEGMENTTYPE_START;
                this->outputStream.write(reinterpret_cast<const char*>(&data), 1);
            }


            /**
             * End serialization.
             */
            virtual void end() {
                const byte data = SEGMENTTYPE_END;
                this->outputStream.write(reinterpret_cast<const char*>(&data), 1);
            }


            /**
             * Check whether something failed during serialization upto now.
             * @return Failure (true) or no failure (false).
             */
            virtual bool failed() const {
                return this->outputStream.fail();
            }


            /**
             * Set the inverted flag on or off.
             * @param inverted Inverted (true) or not inverted (false).
             */
            virtual void setInverted(bool inverted) {
                byte data[2] = { SEGMENTTYPE_INVERTED };
                data[1] = inverted ? 1 : 0;
                this->outputStream.write(reinterpret_cast<const char*>(data), 2);
            }


            /**
             * Add bytes.
             * @param bitData Byte data buffer.
             * @param bytesCount Number of bytes in the data buffer.
             * @param onBitCount Number of 'ON' bits in the buffer.
             */
            virtual void addBytes(const byte* bitData, std::size_t bytesCount, std::size_t onBitCount) {
                byte data[5] = { SEGMENTTYPE_BLOCK_32BIT };
                writeIntToByteBuffer(bytesCount, &data[1]);
                this->outputStream.write(reinterpret_cast<const char*>(data), 5);
                this->outputStream.write(reinterpret_cast<const char*>(bitData), bytesCount);
            }


            /**
             * Add empty bytes.
             * @param bytesCount Number of empty bytes.
             */
            virtual void addEmptyBytes(std::size_t bytesCount) {
                byte data[5] = { SEGMENTTYPE_EMPTYBLOCK_32BIT };
                writeIntToByteBuffer(bytesCount, &data[1]);
                this->outputStream.write(reinterpret_cast<const char*>(data), 5);
            }


            /**
             * Write a 32 bit integer to a byte array.
             * @param n Integer.
             * @param buf Data buffer.
             */
            template <typename IntType> static void writeIntToByteBuffer(const IntType& n, byte* buf) {
                for (int bitOffset = 0; bitOffset < 32; bitOffset += 8) {
                    *(buf++) = (n >> bitOffset) & 0xFF;
                }
            }

        private:
            std::ostream& outputStream;
    };


    /**
     * Simple stream deserializer. Corresponds to StreamSerializer.
     */
    class StreamDeserializer : public IDeserializer
    {
        public:
            /**
             * Constructor.
             * @param stream Input stream.
             */
            StreamDeserializer(std::istream& stream) :
                inputStream(stream),
                fail(false),
                inverted(false),
                currentSegmentType(StreamSerializer::SEGMENTTYPE_START),
                currentSegmentDataLeft(0)
            {
            }


            /**
             * Start deserialization.
             */
            virtual void start() {
                this->fail = false;
                this->inverted = false;
                this->readFromInput(&this->currentSegmentType, 1);
                if (this->currentSegmentType != StreamSerializer::SEGMENTTYPE_START) {
                    this->fail = true;
                    this->currentSegmentType = StreamSerializer::SEGMENTTYPE_END;
                }
            }


            /**
             * Check whether something failed during deserialization upto now.
             * @return Failure (true) or no failure (false).
             */
            virtual bool failed() const {
                return this->fail || this->inputStream.fail();
            }


            /**
             * Return whether deserialization data is finished (or failed).
             * @return Finished (true) or not (false).
             */
            virtual bool finished() const {
                return this->currentSegmentType == StreamSerializer::SEGMENTTYPE_END;
            }


            /**
             * Check whether the inverted flag is set.
             * Note: This information is only available once the (optional)
             * inversion segment is read or the deserialization has finished.
             * @return Inverted (true) or not inverted (false).
             */
            virtual bool isInverted() {
                return this->inverted;
            }


            /**
             * Deserialize bits into a byte block buffer.
             * @param data Data buffer.
             * @param bytesCount Size of data buffer.
             * @param empty Set to true if an empty block is retrieved.
             * @return Data has been read (true) or no data is read (false).
             */
            virtual bool getBytes(byte* data, std::size_t bytesCount, bool& empty) {
                bool isDataRead = false;
                empty = true;
                while (bytesCount > 0 && this->currentSegmentType != StreamSerializer::SEGMENTTYPE_END) {
                    switch (this->currentSegmentType) {
                        case StreamSerializer::SEGMENTTYPE_EMPTYBLOCK_32BIT:
                            if (this->currentSegmentDataLeft > bytesCount) {
                                this->currentSegmentDataLeft -= bytesCount;
                                return true;
                            }
                            else if (this->currentSegmentDataLeft > 0) {
                                data += this->currentSegmentDataLeft;
                                bytesCount -= this->currentSegmentDataLeft;
                                isDataRead = true;
                            }
                            break;

                        case StreamSerializer::SEGMENTTYPE_BLOCK_32BIT:
                            if (this->currentSegmentDataLeft > bytesCount) {
                                this->readFromInput(data, bytesCount);
                                this->currentSegmentDataLeft -= bytesCount;
                                empty = false;
                                return true;
                            }
                            else if (this->currentSegmentDataLeft > 0) {
                                this->readFromInput(data, this->currentSegmentDataLeft);
                                data += this->currentSegmentDataLeft;
                                bytesCount -= this->currentSegmentDataLeft;
                                empty = false;
                                isDataRead = true;
                            }
                            break;

                        default:
                            break;
                    }

                    this->readSegmentHead();
                }
                return isDataRead;
            }


            /**
             * Read a 32 bit integer from a byte array.
             * @param n Integer.
             * @param buf Data buffer.
             */
            template <typename IntType> static void readIntFromByteBuffer(IntType& n, const byte* buf) {
                n = 0;
                for (int bitOffset = 0; bitOffset < 32; bitOffset += 8) {
                    n |= static_cast<IntType>(*(buf++)) << bitOffset;
                }
            }

        private:
            void readSegmentHead() {
                byte buf[4];
                this->readFromInput(&this->currentSegmentType, 1);
                switch (this->currentSegmentType) {
                    case StreamSerializer::SEGMENTTYPE_INVERTED:
                        this->readFromInput(buf, 1);
                        this->inverted = buf[0] != 0;
                        break;

                    case StreamSerializer::SEGMENTTYPE_EMPTYBLOCK_32BIT:
                    case StreamSerializer::SEGMENTTYPE_BLOCK_32BIT:
                        this->readFromInput(buf, 4);
                        readIntFromByteBuffer(this->currentSegmentDataLeft, buf);
                        break;

                    case StreamSerializer::SEGMENTTYPE_END:
                        break;

                    default:
                        this->fail = true;
                        this->currentSegmentType = StreamSerializer::SEGMENTTYPE_END;
                }
            }


            /**
             * Read data from input.
             * @param buf Data buffer pointer.
             * @param size Number of bytes to read.
             */
            void readFromInput(void* const buf, const std::size_t size) {
                char* const charbuf = reinterpret_cast<char*>(buf);
                this->inputStream.read(charbuf, size);
            }


        private:
            std::istream& inputStream;
            bool fail;
            bool inverted;
            byte currentSegmentType;
            std::size_t currentSegmentDataLeft;
    };
    

} // namespace serialize
} // namespace bitlib2


#endif // _BITLIB2_SERIALIZE_HPP_
