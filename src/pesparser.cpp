#include "pesparser.hpp"

PES_Parser::PES_Parser() {
    is_ready = false;
    len = 0;
    offset = 0;
}

bool PES_Parser::next_packet(TS_Packet* ts_packet) {
    if (ts_packet->payload_struct_ind == 1) {
        if (is_ready) {
            cout << "got full packet!" << endl;
            len = __builtin_bswap16(*((uint16_t*) &buffer[4]));
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
    if(is_ready) {
        cout    << "pakiecig: "
                << (int) buffer[0] << " | "
                << (int) buffer[1] << " | "
                << (int) buffer[2] << " | "
                << (int) ((uint16_t)buffer[3]) << " | "
                << (int) ((uint16_t)buffer[4]) << " | "
                << (int) ((uint16_t)buffer[5]) << " | "
                << "dlugosc: " << (int) len << " | "
                << endl;
    } else cout << "Not full packet in buffer" << endl;
    
}

bool PES_Parser::ready() {
    return is_ready;
}

bool PES_Parser::append_file(char* filename) {
    fstream output_fs;
    output_fs.open(filename, fstream::out | fstream::binary | fstream::app);
    if (!output_fs.good()) {
        cout << "Error opening " << filename << "!" << endl;
        return false;
    }
    output_fs.write(buffer, len);
    output_fs.close();
    return true;
}