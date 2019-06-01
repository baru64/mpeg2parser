#include "pesparser.hpp"

PES_Parser::PES_Parser(fstream* fs, bool enable_desc) {
    is_ready = false;
    len = 0;
    offset = 0;
    cpy_ptr = buffer;
    printinfo = enable_desc;
    header_len = 0;
    output_fs = fs;
}

void PES_Parser::next_packet(TS_Packet* ts_packet) {
    if (ts_packet->payload_struct_ind == 1) {
        if (is_ready) {
            // cout    << "got full packet! buff[3]: "<< (int) buffer[3] 
                    // << " buff[8]: " << (unsigned) buffer[8] << endl;
            uint8_t sid = (uint8_t) buffer[3];
            if (sid == 0xBD ||
                (sid >= 0xC0 && sid <= 0xDF) ||
                (sid >= 0xE0 && sid <= 0xEF))
                header_len = (uint8_t) buffer[8];
            else header_len = 0;
            len = __builtin_bswap16(*((uint16_t*) &buffer[4]));
            desc();
            append_file();
        } else is_ready = true;
        offset = 0;
        if (ts_packet->afc != 1)
            offset += 1 + ((int) ts_packet->content[0]);
        cpy_ptr = &buffer[0];
        memcpy(cpy_ptr, &(ts_packet->content[offset]), PACKET_SIZE - offset);
    } else {
        offset = 4;
        if (ts_packet->afc == 1) {
            memcpy(cpy_ptr, ts_packet->content, PACKET_SIZE - offset);
        } else {
            offset += 1 + ((int) ts_packet->content[0]);
            memcpy(cpy_ptr, &ts_packet->content[offset - 4], PACKET_SIZE - offset);
        }
    }
    cpy_ptr += PACKET_SIZE - offset;
}

void PES_Parser::desc() {
    if(printinfo) {
        cout    << "pakiecig: "
                << (int) buffer[0] << " | "
                << (int) buffer[1] << " | "
                << (int) buffer[2] << " | "
                << (int) ((uint8_t)buffer[3]) << " | "
                << (int) ((uint16_t)buffer[4]) << " | "
                << (int) ((uint16_t)buffer[5]) << " | "
                << "len: " << (int) len << " | "
                << "header_len:" << (int) header_len
                << endl;
    } else cout << "Not full packet in buffer" << endl;
    
}

bool PES_Parser::append_file() {
    if (!output_fs->good()) {
        cout << "Error file not good!" << endl;
        return false;
    }
    cout << "len to write: " << (len+6)-(6+3+header_len) << endl;
    output_fs->write(&buffer[6+3+header_len], (len+6)-(6+3+header_len));
    return true;
}