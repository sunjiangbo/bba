/* bba_record.c */
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
    OP_RECORD_LENTH = 800,
    OP_RECORD_SIZE,
    OP_RECORD_WIDTH,
    OP_RECORD_RATE,
    OP_RECORD_CHANNEL,
    OP_RECORD_MAX
};

const char record_help[] = "\n"
"busybox audio recorder\n"
"usage:\n"
"bba record [-h|--help]\n"
"           [-l|--list]\n"
"           [-v|--verbose]\n"
"           [-o <filename>] [--length <value>] [--size <value>]\n"
"           [--width <value>] [--rate <value>] [--channel <value>]\n"
"    -h  --help\n"
"    -l  --list                  list supported encoder.\n"
"    -v  --verbose               enable verbose print.\n"
"    -o <filename>               output file name\n"
"        --length <value>        seconds: 1, 600, etc.\n"
"        --size <value>          limited record bytes: 1024, 8k, etc.\n"
"\n"
"        --width <value>         bits: 8, 16, etc.\n"
"        --rate <value>          Hz: 8000, 44100, etc.\n"
"        --channel <value>       number: 1, 2, etc.\n";

int stopsign = 0;

int argproc_record(s_audinfo_t *inf, int argc, char *argv[])
{
    int ret = 0;
    int c;

    if (argc < 2) {
        usage(RECORD_HELP);
        return ret;
    }

    while (1) {
        int option_index = 0;
		static const char short_options[] = "hlvo:";
        static const struct option long_options[] = {
            {"help",    0, 0, 'h'},
            {"list",    0, 0, 'l'},
            {"verbose", 0, 0, 'v'},
            {"length",  1, 0, OP_RECORD_LENTH},
            {"size",    1, 0, OP_RECORD_SIZE},
            {"width",   1, 0, OP_RECORD_WIDTH},
            {"rate",    1, 0, OP_RECORD_RATE},
            {"channel", 1, 0, OP_RECORD_CHANNEL},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, short_options,
                 long_options, &option_index);
        if (c == -1)
            break;
    
        switch (c) {
        case 'h':
            usage(RECORD_HELP);
            break;    
        case 'l':
            break;
        case 'v':
            debug_log = 1;
            break;
        case 'o':
            ret = strlen(optarg);
            if (ret < sizeof(inf->fnameo)) {
                memset(inf->fnameo, 0, sizeof(inf->fnameo));
                strncpy(inf->fnameo, optarg, ret);
            } else
                return -1;
            break;
        case OP_RECORD_LENTH:
            inf->length = (double)atoi(optarg);
            inf->recsize = 0;
            break;
        case OP_RECORD_SIZE:
            inf->recsize= atoi(optarg);
            inf->length = 0.0;
            break;
        case OP_RECORD_WIDTH:
            inf->width= atoi(optarg);
            break;
        case OP_RECORD_RATE:
            inf->rate= atoi(optarg);
            break;
        case OP_RECORD_CHANNEL:
            inf->nchan= atoi(optarg);
            break;
        default:
            break;
        }
    }

    ret = recwav(inf);
    return ret;
}

void signal_handler(int sig)
{
    Loge("\nReceived signal: %d.", sig);
    Log("You can find more information with 'stty -a'.");
    if (sig == SIGINT) {
        stopsign = 1;
    }
}

int recwav(s_audinfo_t *inf)
{
    int ret = 0;

    /* init capture parameters */
    if ((ret = initrec(inf)) != 0) {
        fprintf (stderr, "initcapt error(%s)\n",
            snd_strerror (ret));
        goto end;
    }

    /* create wav to write */
    if ((inf->fpo = fopen(inf->fnameo, "wb+")) == NULL){
        fprintf (stderr, "Open file '%s' fail !!\n", inf->fnameo);
        ret = -1;
        goto end;
    } else {
        Log("Open file '%s' succeed.", inf->fnameo);
    }

    /* capture pcm through alsa api and write data to output file*/
    fseek(inf->fpo, WAVHEAD_SIZE, SEEK_SET);
    if ((ret = alsa_record(inf)) != 0) {
        fprintf (stderr, "alsa_record error(%s)\n",
            snd_strerror (ret));
        goto end;
    }

    /* update file wav head */
    rewind(inf->fpo);    
    ret = fwrite(inf->wavhead, sizeof(char), WAVHEAD_SIZE, inf->fpo);
    if (ret == WAVHEAD_SIZE) ret = 0;

    fclose(inf->fpo);
end:
    audinfo(inf);
    stopsign = 0;
    return ret;
}

int initrec(s_audinfo_t *inf)
{
    int ret = 0;
    inf->alsa.stream = SND_PCM_STREAM_CAPTURE;
    inf->alsa.access = SND_PCM_ACCESS_RW_INTERLEAVED;
    inf->alsa.format = SND_PCM_FORMAT_S16_LE;
    signal(SIGINT, signal_handler);
    return ret;
}

