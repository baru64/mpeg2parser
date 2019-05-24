#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "packet.h"
using namespace std;

int arrFindInt(int* arr, int x, int len)
{
    for(int i = 0; i < len; i++)
        if (arr[i] == x) return i;
    return -1;
}

bool arrAppendInt(int* arr, int x, int len) {
    for(int i = 0; i < len; i++)
        if (arr[i] == 0) {
             arr[i] = x;
             return false; // no error
        }
    return true; // error
}

#define MAX_PIDS 8
#define PACKET_SIZE 188
#define BUFFER_LEN 4096

int main(int argc, char* argv[]) {
    fstream input_fs;
    fstream output_fs;
    int pids[MAX_PIDS] = {0};
    char packet_buff[PACKET_SIZE];
    char input_file[128] = "example.ts";
    char output_file[128] = "audio.mp2";
    uint16_t pid_to_extract = 0xcb;

    if (argc < 4) {
        cout << "Usage: mpeg2parser OUTPUT_FILE INPUT_FILE PID_TO_EXTRACT" << endl;
    } else {
        strcpy(output_file, argv[1]);
        strcpy(input_file, argv[2]);
        pid_to_extract = (uint16_t) atoi(argv[3]);
    }

    cout << "Input: "   << input_file
         << " Output: " << output_file
         << " PID: "    << pid_to_extract << endl;

    input_fs.open(input_file, fstream::in | fstream::binary);
    if (!input_fs.good()) {
        cout << "Error opening " << input_file << "!" << endl;
        return 1;
    }
    output_fs.open(output_file, fstream::out | fstream::binary);
    if (!output_fs.good()) {
        cout << "Error opening " << output_file << "!" << endl;
        return 1;
    }

    int i = 0;
    int offset;
    bool start_writing = false;
    char buffer[BUFFER_LEN];
    char* cpy_ptr = buffer;
    while(input_fs.read(packet_buff, PACKET_SIZE)) {
        if (input_fs.gcount() < 188) {
            cout << "Read less than 188 bytes, exiting." << endl;
            break;
        }
        Packet *packet = new Packet(packet_buff);
        if (packet->pid == pid_to_extract && i < 1000) {
            packet->desc();
            cout << "num: " << i++ << endl;
        }
        
        if (packet->pid == pid_to_extract) {
            if (packet->payload_struct_ind == 1) {
                cout    << "BUFOR: "
                        << (int) buffer[0] << " | "
                        << (int) buffer[1] << " | "
                        << (int) buffer[2] << " | "
                        << (int) ((uint16_t)buffer[3]) << " | "
                        << (int) ((uint16_t)buffer[4]) << " | "
                        << (int) ((uint16_t)buffer[5]) << " | "
                        << endl;
                if (start_writing) {
                    cout << "writing!" << endl;
                    uint16_t len = __builtin_bswap16(*((uint16_t*) &buffer[4]));
                    output_fs.write(buffer, len);
                } else start_writing = true;
                offset = 0;
                if (packet->afc != 1)
                    offset += 1 + ((int) packet->content[0]);
                cpy_ptr = &buffer[0];
                memcpy(cpy_ptr, &(packet->content[offset]), PACKET_SIZE - offset);
            } else {
                offset = 4;
                if (packet->afc == 1) {
                    memcpy(cpy_ptr, packet->content, PACKET_SIZE - offset);
                } else {
                    offset += 1 + ((int) packet->content[0]);
                    memcpy(cpy_ptr, &packet->content[offset - 4], PACKET_SIZE - offset);
                }
            }
            cpy_ptr += PACKET_SIZE - offset;
        }

        if (arrFindInt(pids, packet->pid, MAX_PIDS) == -1) {
            arrAppendInt(pids, packet->pid, MAX_PIDS);
        }

        if (!output_fs.good()) {
            cout << "Output stream error!\n";
            break;
        }
    }
    input_fs.close();
    output_fs.close();
    cout << "\n all pids: ";
    for(int i = 0; i < MAX_PIDS; i++)
        if(pids[i] != 0) cout << pids[i] << " ";
    cout << endl;
    return 0;
}