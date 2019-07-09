#pragma once
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "riff.h"

using namespace std;

void print_wav_header(struct riff_header*);
void overwrite_wav_header(fstream*, struct riff_header*);
