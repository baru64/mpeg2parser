#include <mpg123.h>
#include <fmt123.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "packet.hpp"
#include "riff.h"
#include "wav_utils.hpp"

using namespace std;

#define BUFFER_LEN 4096
#define PACKET_SIZE 188
#define OUTBUFF 32768

class PES_Parser {
    public:
        char buffer[BUFFER_LEN];
        bool printinfo;
        fstream* output_fs;
        
        void next_packet(TS_Packet* ts_packet);
        bool append_file();
        bool close_fs(char* filename);
        void desc();
        PES_Parser(fstream* fs, bool enable_desc);
    
    private:
        uint8_t magic[3];
        uint8_t stream_id;
        uint16_t len;
        int offset;
        char* cpy_ptr;
        bool is_ready;
        uint8_t header_len;
        mpg123_handle *m;
        int ret;
        unsigned char out[OUTBUFF];
        struct riff_header hdr;
        ssize_t outc;
};
