/* bba_info.c */
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
    OP_INFO_VOL = 1000,
    OP_INFO_MAX
};

const char info_help[] = "\n"
"busybox audio information\n"
"usage:\n"
"bba info [-h|--help]\n"
"         [-v|--verbose]\n"
"         [-i <filename>\n"
"    -h  --help\n"
"    -l  --list                  info list\n"
"    -v  --verbose               enable verbose print.\n"
"    -i <filename>               input filename\n";

int argproc_info(s_audinfo_t *inf, int argc, char *argv[])
{
    int ret = 0;
    int c;

    if (argc < 2) {
        usage(INFO_HELP);
        return ret;
    }

    while (1) {
        int option_index = 0;
		static const char short_options[] = "hlvi:";
        static const struct option long_options[] = {
            {"help",    0, 0, 'h'},
            {"list",    0, 0, 'l'},
            {"verbose", 0, 0, 'v'},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, short_options,
                 long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'h':
            usage(INFO_HELP);
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
        default:
            break;
        }
    }

    ret = info_process(inf);
    return ret;
}

int info_process(s_audinfo_t *inf)
{
	int ret = 0;
    int filetype;

    /* misc check */
    filetype = parse_filename(inf->fnamei, strlen(inf->fnamei));
    Log("%d", filetype);

    /* parse file */
    switch (filetype) {
    case FILE_RAW:
    case FILE_PCM:
        Log("raw data, can not parse");
        ret = 0;
        break;
    case FILE_WAV:
        ret = parsewav(inf);
        break;
    case FILE_MP3:
        ret = parsemp3(inf);
        break;
    case FILE_AAC:
        ret = parseaac(inf);
        break;
    case FILE_AMR:
        ret = parseamr(inf);
        break;
    case FILE_AWB:
        ret = parseawb(inf);
        break;
    default:
        Loge("Unsupported file type");
        break;
    }

    return ret;
}

int search_tags(s_audinfo_t *inf)
{
	int ret = 0;
	return ret;
}

int search_frames(s_audinfo_t *inf)
{
	int ret = 0;
	int idx = 0;

	do {
		if (is_frame_valid(inf))
			inf->parser.idx++;
		idx++;
	} while (idx < inf->filebytes);

	frame_info(inf);
	return ret;
}

int is_frame_valid(s_audinfo_t *inf)
{
	int ret = 0;
	u_framehead_t head;
	head.value = str2int32(inf->data, inf->ibn);

	if (head.bits_mp3.sync != 0xFFF ||
		head.bits_mp3.id != 1 ||
		head.bits_mp3.layer != 0x1 ||
		head.bits_mp3.bitrate == 0xF ||
		head.bits_mp3.samplerate == 0x3 ) {
		//printf("Not supported head");
		return ret;
	}

	inf->parser.head.value = head.value;
	inf->parser.idx++;

	return ret;
}

int frame_info(s_audinfo_t *inf)
{
	int tmp = 0;
	printf("Last frame of Valid MPEG Audio:\n");
	printf("Layer:                %5d\n", 4 - inf->parser.head.bits_mp3.layer);
	printf("Protection bit:       %5d\n", inf->parser.head.bits_mp3.protect);
	tmp = inf->parser.head.bits_mp3.bitrate << 5;
	if (tmp == 0)
		printf("Bitrate:              free format\n");
	else
		printf("Bitrate(kbit/s):      %5d\n", tmp);
	switch (inf->parser.head.bits_mp3.samplerate) {
	case 0: tmp = 44100; break;
	case 1: tmp = 48000; break;
	case 2: tmp = 32000; break;
	}
	printf("Sample rate:          %5d\n", tmp);
	printf("Padding bit:          %5d\n", inf->parser.head.bits_mp3.padding);
	printf("Private bit:          %5d\n", inf->parser.head.bits_mp3.private);
	tmp = inf->parser.head.bits_mp3.mode;
	switch (tmp) {
	case 0:
		printf("Mode:                stereo\n");
		break;
	case 1:
		printf("Mode:          joint_stereo\n");
		break;
	case 2:
		printf("Mode:          dual channel\n");
		break;
	case 3:
		printf("Mode:        single channel\n");
		break;
	}
	printf("Mode extension:       %5d\n", inf->parser.head.bits_mp3.mode_ext);
	printf("Copy right protected: %5d\n", inf->parser.head.bits_mp3.copyright);
	printf("Bitstream is original:%5d\n", inf->parser.head.bits_mp3.original);
	printf("Mode extension:       %5d\n", inf->parser.head.bits_mp3.mode_ext);
	printf("Emphasis:             %5d\n", inf->parser.head.bits_mp3.emphasis);
	printf("Number of valid frame:%5d\n", inf->parser.idx);

	return 0;
}

