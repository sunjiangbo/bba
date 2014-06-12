/* bba_conv.c */
/*
 ** Copyright (C) 2014 Han Lu
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#include "bba.h"

enum {
    OP_CONV_WIDTH = 900,
    OP_CONV_RATE,
    OP_CONV_CHANNEL,
    OP_CONV_ENHANCE,
    OP_CONV_VOL,
    OP_CONV_MAX
};

const char conv_help[] = "\n"
"usage:\n"
"bba conv [-h|--help]\n"
"         [-l|--list]\n"
"         [-v|--verbose]\n"
"         [-i <filename>] [-o <filename>]\n"
"         [--width <value>] [--rate <value>] [--channel <value>]\n"
"         [--enhance] [--vol <value>]\n"
"    -h  --help\n"
"    -l  --list                  list supported encoder/decoder.\n"
"    -v  --verbose               enable verbose print.\n"
"    -o <filename>\n"
"\n"
"        --width <value>         bits: 8, 16, etc.\n"
"        --rate <value>          Hz: 8000, 44100, etc.\n"
"        --channel <value>       number: 1, 2, etc.\n"
"\n"
"        --enhance               ALC(audio level control) effect.\n"
"        --vol <value>           volume control, dB: -3, 3, etc.\n";

int argproc_conv(s_audinfo_t *inf, int argc, char *argv[])
{
    int ret = 0;
    int c;

    if (argc < 2) {
        usage(CONV_HELP);
        return ret;
    }

    while (1) {
        int option_index = 0;
		static const char short_options[] = "hlvi:o:";
        static const struct option long_options[] = {
            {"help",    0, 0, 'h'},
            {"list",    0, 0, 'l'},
            {"verbose", 0, 0, 'v'},
            {"width",   1, 0, OP_CONV_WIDTH},
            {"rate",    1, 0, OP_CONV_RATE},
            {"channel", 1, 0, OP_CONV_CHANNEL},
            {"enhance", 0, 0, OP_CONV_ENHANCE},
            {"vol",     1, 0, OP_CONV_VOL},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, short_options,
                 long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'h':
            usage(CONV_HELP);
            break;    
        case 'l':
            break;
        case 'v':
            debug_log = 1;
            break;
        case 'i':
            ret = strlen(optarg);
            if (ret < sizeof(inf->fnamei)) {
                memset(inf->fnamei, 0, sizeof(inf->fnamei));
                strncpy(inf->fnamei, optarg, ret);
            } else
                return -1;
            break;
        case 'o':
            ret = strlen(optarg);
            if (ret < sizeof(inf->fnameo)) {
                memset(inf->fnameo, 0, sizeof(inf->fnameo));
                strncpy(inf->fnameo, optarg, ret);
            } else
                return -1;
            break;
        case OP_CONV_WIDTH:
            break;
        case OP_CONV_RATE:
            break;
        case OP_CONV_CHANNEL:
            break;
        case OP_CONV_ENHANCE:
            break;
        case OP_CONV_VOL:
            break;
        default:
            break;
        }
    }

    convert_process(inf);
    return ret;
}

int convert_process(s_audinfo_t *inf)
{
    int ret = 0;
    int input_filetype, output_filetype;
    int input_compressed, output_compressed;

    /* misc check */
    input_filetype = parse_filename(inf->fnamei, FILENAME_SIZE);
    input_compressed = file_is_compressed(input_filetype);
    output_filetype = parse_filename(inf->fnameo, FILENAME_SIZE);
    output_compressed = file_is_compressed(output_filetype);

    /* decode check */
    if (input_compressed && !output_compressed) {
        inf->codec_type = file_to_codec(input_filetype);
        audio_file_decode(inf);
    }

    /* encode check */
    if (input_compressed && !output_compressed) {
        inf->codec_type = file_to_codec(output_filetype);
        audio_file_encode(inf);
    }

    return ret;
}

int file_is_compressed(int filetype)
{
    int ret = -1;
    switch (filetype) {
    case FILE_RAW:
    case FILE_PCM:
    case FILE_WAV:
        ret = 0;
    case FILE_MP3:
    case FILE_AAC:
    case FILE_AMR:
    case FILE_AWB:
        ret = 1;
    }
    return ret;
}

int file_to_codec(int filetype)
{
    int ret = -1;
    switch (filetype) {
    case FILE_RAW:
    case FILE_PCM:
    case FILE_WAV:
        ret = CODEC_PCM;
    case FILE_MP3:
        ret = CODEC_MP3;
    case FILE_AAC:
        ret = CODEC_AAC;
    case FILE_AMR:
        ret = CODEC_AMR;
    case FILE_AWB:
        ret = CODEC_AWB;
    }
    return ret;
}

