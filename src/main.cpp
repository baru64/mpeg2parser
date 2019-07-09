#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <cstdlib>

#include "packet.hpp"
#include "pesparser.hpp"

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
        return EXIT_FAILURE;
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

    PES_Parser* parser = new PES_Parser(&output_fs, false);
    while(input_fs.read(packet_buff, PACKET_SIZE)) {
        if (input_fs.gcount() < 188) {
            cout << "Read less than 188 bytes, exiting." << endl;
            break;
        }
        TS_Packet *packet = new TS_Packet(packet_buff);

        if (packet->pid == pid_to_extract) {
            parser->next_packet(packet);
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
    parser->close_fs(output_file);
    cout << "\n all pids: ";
    for(int i = 0; i < MAX_PIDS; i++)
        if(pids[i] != 0) cout << pids[i] << " ";
    cout << endl;
    return EXIT_SUCCESS;
}
