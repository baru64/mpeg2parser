#include "wav_utils.hpp"
// these need fstream insted of using fds directly

void print_wav_header(struct riff_header *hdr) {
    printf("file_size: %d | format_len: %d | format_type: %d | rate: %d |"
           "channels: %d | srbpsch: %d | bpsxch: %d | bps: %d | data_size: %d\n",
            hdr->file_size, hdr->format_len, hdr->format_type,
            hdr->sample_rate, hdr->channels, hdr->srxbpsxch, hdr->bpsxch,
            hdr->bits_per_sample, hdr->data_size);
}

void overwrite_wav_header(fstream* fs, struct riff_header *hdr) {
    fs->seekp(0, ios::beg);
    fs->write((char*) hdr, sizeof (*hdr));
    fs->seekp(0, ios::end);
}
