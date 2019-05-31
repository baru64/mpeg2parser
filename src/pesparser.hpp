#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string.h>
#include "packet.hpp"
using namespace std;

#define BUFFER_LEN 4096
#define PACKET_SIZE 188

class PES_Parser {
    public:
        char buffer[BUFFER_LEN];
        
        bool ready();
        bool next_packet(TS_Packet* ts_packet);
        bool append_file(char* filename);
        void desc();
        PES_Parser();
    
    private:
        uint8_t magic[3];
        uint8_t stream_id;
        uint16_t len;
        int offset;
        char* cpy_ptr;
        bool is_ready;
};