/* bba_play.c */
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
    OP_PLAY_SCALE = 700,
    OP_PLAY_MAX
};

const char play_help[] = "\n"
"busybox audio player\n"
"usage:\n"
"bba play [-h|--help]\n"
"         [-l|--list]\n"
"         [-v|--verbose]\n"
"         [-i <filename>] [--scale <value>]\n"
"description:\n"
"    -h  --help\n"
"    -l  --list                  list supported decoder.\n"
"    -v  --verbose               enable verbose print.\n"
"    -i <filename>               input file name\n"
"        --scale <value>         volume control, dBFS: 0, -3.3, -12, etc.\n";

int argproc_play(s_audinfo_t *inf, int argc, char *argv[])
{
    int ret = 0;
    int c;

    if (argc < 2) {
        usage(PLAY_HELP);
        return ret;
    }

    while (1) {
        int option_index = 0;
		static const char short_options[] = "hlvi:";
        static const struct option long_options[] = {
            {"help",    0, 0, 'h'},
            {"list",    0, 0, 'l'},
            {"verbose", 0, 0, 'v'},
            {"vol",     1, 0, OP_PLAY_SCALE},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, short_options,
                 long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'h':
            usage(PLAY_HELP);
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
        case OP_PLAY_SCALE:
            inf->volgain = (double)atoi(optarg);
            break;
        default:
            break;
        }
    }

    ret = playwav(inf);
    return ret;
}

int playwav(s_audinfo_t *inf)
{
    int ret = 0;
    /* init playing parameters */
    if ((ret = initplay(inf)) != 0) {
        fprintf (stderr, "initplay error(%s)\n",
            snd_strerror (ret));
        return -1;
    }

    /* read wav data to structure */
    if ((ret = parsewav(inf)) != 0) {
        fprintf (stderr, "parsewav error(%s)\n",
             snd_strerror (ret));
        return -1;
    }

    /* allocate buffer */
    if ((inf->data = calloc(sizeof(char), inf->databytes)) == NULL) {
        Loge("allocate data buffer of %d bytes failed, exit.", inf->databytes);
        return -1;
    }

    /* read pcm data to buffer */    
    if ((inf->fpi = fopen(inf->fnamei, "rb")) == NULL){
        fprintf (stderr, "Input file '%s' does not exist !!\n", inf->fnamei);
        ret = -1;
        goto end;
    }
    ret = fread(inf->data, sizeof(char), inf->databytes, inf->fpi);
    fclose(inf->fpi);
    if (ret != inf->databytes) {
        Loge("read %d of %d", ret, inf->databytes);
        ret = -1;
        goto end;
    } else
        ret = 0;

    /* play pcm through alsa api */
    if ((ret = alsa_play(inf)) != 0) {
        fprintf (stderr, "alsa_play error(%s)\n",
            snd_strerror (ret));
        goto end;
    }
end:
    free(inf->data);
    inf->data = NULL;
    return ret;
}

int initplay(s_audinfo_t *inf)
{
    int ret = 0;
    inf->alsa.stream = SND_PCM_STREAM_PLAYBACK;
    inf->alsa.access = SND_PCM_ACCESS_RW_INTERLEAVED;
    inf->alsa.format = SND_PCM_FORMAT_S16_LE;
    return ret;
}

