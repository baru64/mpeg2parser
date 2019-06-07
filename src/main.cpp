#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

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
#define PORT 8001

int main(int argc, char* argv[]) {
    // fstream input_fs;
    fstream output_fs;
    int pids[MAX_PIDS] = {0};
    char packet_buff[PACKET_SIZE];
    char input_file[128] = "224.0.0.1";
    char output_file[128] = "audio.mp2";
    
    int sd;
    struct ip_mreq group;
    struct sockaddr_in addr;
    memset((char*) &addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    uint16_t pid_to_extract = 0xcb;

    if (argc < 4) {
        cout << "Usage: mpeg2parser OUTPUT_FILE INPUT_STREAM_IP PID_TO_EXTRACT" << endl;
        return EXIT_FAILURE;
    } else {
        strcpy(output_file, argv[1]);
        strcpy(input_file, argv[2]);
        pid_to_extract = (uint16_t) atoi(argv[3]);
    }

    cout << "Input: "   << input_file
         << " Output: " << output_file
         << " PID: "    << pid_to_extract << endl;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0) { perror("Opening datagram socket error"); exit(EXIT_FAILURE); }

    int reuse = 1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
        perror("Setting SO_REUSEADDR error");
        close(sd);
        exit(EXIT_FAILURE);
    }


    group.imr_multiaddr.s_addr = inet_addr(input_file);
    group.imr_interface.s_addr = inet_addr("172.20.128.22");
    if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0) {
        perror("Adding multicast group error");
        close(sd);
        exit(1);
    }


    // input_fs.open(input_file, fstream::in | fstream::binary);
    // if (!input_fs.good()) {
    //     cout << "Error opening " << input_file << "!" << endl;
    //     return 1;
    // }
    output_fs.open(output_file, fstream::out | fstream::binary);
    if (!output_fs.good()) {
        cout << "Error opening " << output_file << "!" << endl;
        return 1;
    }

    // int i = 0;
    PES_Parser* parser = new PES_Parser(&output_fs, true);
//    while(input_fs.read(packet_buff, PACKET_SIZE)) {
    while(true) {
        int readlen = read(sd, packet_buff, PACKET_SIZE);
        if (readlen > 0) {
            cout << "dlugosc: " << readlen << endl;
            for(int i = 0; i < readlen; i++) cout << packet_buff[i];
        }
        
        // if (input_fs.gcount() < 188) {
        //     cout << "Read less than 188 bytes, exiting." << endl;
        //     break;
        // }
        TS_Packet *packet = new TS_Packet(packet_buff);
        // if (packet->pid == pid_to_extract && i < 1000) {
        //     packet->desc();
        //     cout << "num: " << i++ << endl;
        // }

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
    // input_fs.close();
    output_fs.close();
    cout << "\n all pids: ";
    for(int i = 0; i < MAX_PIDS; i++)
        if(pids[i] != 0) cout << pids[i] << " ";
    cout << endl;
    return EXIT_SUCCESS;
}