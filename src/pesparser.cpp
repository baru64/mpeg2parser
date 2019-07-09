#include "pesparser.hpp"

PES_Parser::PES_Parser(fstream* fs, bool enable_desc) {
    is_ready = false;
    len = 0;
    offset = 0;
    cpy_ptr = buffer;
    printinfo = enable_desc;
    header_len = 0;
    output_fs = fs;
    outc = 0;

	// write header
    memset(&hdr, 0, sizeof hdr);
    memcpy(hdr.riff_mark, "RIFF", 4);
    memcpy(hdr.file_type, "WAVE", 4);
    memcpy(hdr.format_marker, "fmt ", 4);
    memcpy(hdr.data_marker, "data", 4);
    hdr.format_len = 16U; // length of format data
    hdr.format_type = 1; // 1 - PCM

	output_fs->write((char*) &hdr, sizeof hdr);	

    // mpg123 init
    mpg123_init();
    m = mpg123_new(NULL, &ret);
    if (m == NULL)
    {
        fprintf(stderr,"Unable to create mpg123 handle: %s\n",
                mpg123_plain_strerror(ret));
        exit(EXIT_FAILURE);
    }
    mpg123_param(m, MPG123_VERBOSE, 2, 0);

    mpg123_open_feed(m);
    if (m == NULL) exit(EXIT_FAILURE);
}

void PES_Parser::next_packet(TS_Packet* ts_packet) {
    if (ts_packet->payload_struct_ind == 1) {
        if (is_ready) {
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
        memcpy(cpy_ptr, &(ts_packet->content[offset]), PACKET_SIZE - 4 - offset);
    } else {
        offset = 0;
        if (ts_packet->afc == 1) {
            memcpy(cpy_ptr, ts_packet->content, PACKET_SIZE - 4);
        } else {
            offset += 1 + ((int) ts_packet->content[0]);
            memcpy(cpy_ptr, &ts_packet->content[offset], PACKET_SIZE - 4 - offset);
        }
    }
    cpy_ptr += PACKET_SIZE - 4 - offset;
}

void PES_Parser::desc() {
    if(printinfo) {
        cout    << "pakiecig: "
                << (int) buffer[0] << " | "
                << (int) buffer[1] << " | "
                << (int) buffer[2] << " | "
                << (int) ((uint8_t)buffer[3]) << " | "
                << "len: " << (int) len << " | "
                << "header_len:" << (int) header_len
                << endl;
    }
}

bool PES_Parser::append_file() {
    if (!output_fs->good()) {
        cout << "Error file not good!" << endl;
        return false;
    }

    size_t size;
    ret = mpg123_decode(m, (unsigned char*) &buffer[6+3+header_len],
                        (len+6)-(6+3+header_len), out, OUTBUFF, &size);
    if(ret == MPG123_NEW_FORMAT) {
        long rate;
        int channels, enc;

        mpg123_getformat(m, &rate, &channels, &enc);
        fprintf(stderr, "New format: %li Hz, %i channels, encoding value %i\n",
                rate, channels, enc);

        hdr.channels = (uint16_t) channels;
        hdr.sample_rate = (uint32_t) rate;
        if (enc & MPG123_ENC_8) hdr.bits_per_sample = 8U;
        else if (enc & MPG123_ENC_16) hdr.bits_per_sample = 16U;
        else if (enc & MPG123_ENC_24) hdr.bits_per_sample = 24U;
        else if (enc & MPG123_ENC_32) hdr.bits_per_sample = 32U;
        hdr.srxbpsxch = (hdr.sample_rate * hdr.bits_per_sample
                        * hdr.channels) / 8;
        hdr.bpsxch = (hdr.bits_per_sample * hdr.channels) / 8;

        //print_wav_header(&hdr);
        overwrite_wav_header(output_fs, &hdr);
    }
	output_fs->write((char*) out, size);
    outc += size;
    while (ret != MPG123_ERR && ret != MPG123_NEED_MORE) {
    // Get all decoded audio that is available now before feeding more input
        ret = mpg123_decode(m,NULL,0,out,OUTBUFF,&size);
	    output_fs->write((char*) out, size);
        outc += size;
    }
    if (ret == MPG123_ERR) {
        fprintf(stderr, "MPG123 ERROR: %s", mpg123_strerror(m)); return false;
    }

    return true;
}

bool PES_Parser::close_fs(char* filename) {
    struct stat file_info;
    int err;
    err = stat(filename, &file_info);
    if (err == -1) {
        fprintf(stderr,"Stat error");
        output_fs->close();
        return false;
    }
    hdr.file_size = ((uint32_t) file_info.st_size ) - 8;
    hdr.data_size = (uint32_t) outc;
    print_wav_header(&hdr);
    overwrite_wav_header(output_fs, &hdr);
    output_fs->close();
    return true;
}
