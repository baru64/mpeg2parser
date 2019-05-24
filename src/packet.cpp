#include "packet.h"


/* MPEG-2 TS
*/
Packet :: Packet (char* bytes) {
    // uint32_t header = (bytes[0] << 24) + (bytes[1] << 16) +  (bytes[2] << 8) + bytes[3];
    uint32_t header = __builtin_bswap32(*((uint32_t*) bytes));
    sync_byte = header >> 24;
    error_ind = header & 0x80000;           //(header >> 23) & 0x01;
    payload_struct_ind = header & 0x400000; //(header >> 22) & 0x01;
    transport_priority = header & 0x200000; //(header >> 21) & 0x01;
    pid = (header & 0x1fff00) >> 8;         //(header >> 8) & 8191;
    tsc = (header & 0xc0) >> 6;             //(header >> 6) & 3;
    afc = (header & 0x30) >> 4;             //(header >> 4) & 3;
    cc = header & 0xf;
    memcpy(content, &bytes[4], 184);
}

void Packet :: desc () {
    cout << "sync: " << sync_byte << " | ";
    cout << "PID: " << pid << " | ";
    cout << "CC: " << (int) cc << "\t| ";
    cout << "S:" << payload_struct_ind << " | ";
    cout << "AFC: " << (int) afc << " | ";
    cout << "error: " << error_ind << " | ";
    cout << "t_pr: " << transport_priority << " | ";
    if (afc == 2 || afc == 3)
        cout << "AF_LEN: " << (int) content[0] << " | ";
    // if (payload_struct_ind == 1) {
    //     cout << "start: " << (int) content[2] << '|' <<(int) content[3] << '|' << (int) content[4] <<  endl;
    // }
}
