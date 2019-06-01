#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string.h>
using namespace std;

#ifndef PACKET_HPP
#define PACKET_HPP

class TS_Packet {
    public:
        uint8_t sync_byte;
        bool error_ind;
        bool payload_struct_ind;
        bool transport_priority;
        uint16_t pid;
        uint8_t tsc;
        uint8_t afc;
        uint8_t cc;
        uint8_t content[184];
        TS_Packet(char* bytes);
        void desc();
};

#endif