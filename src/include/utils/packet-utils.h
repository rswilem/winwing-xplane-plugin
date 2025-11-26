#ifndef PACKET_UTILS_H
#define PACKET_UTILS_H

#include <cstdint>
#include <vector>

namespace PacketUtils {

    // Common packet numbering for Winwing devices
    class PacketNumber {
        private:
            uint8_t number = 1;

        public:
            uint8_t next() {
                uint8_t current = (number == 0) ? 1 : number;
                number++;
                if (number == 0) {
                    number = 1;
                }
                return current;
            }

            uint8_t current() const {
                return (number == 0) ? 1 : number;
            }

            void reset() {
                number = 1;
            }
    };

}

#endif