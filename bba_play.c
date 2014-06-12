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

int parsewav(s_audinfo_t *inf)
{
    int ret = 0;

    if ((inf->fpi = fopen(inf->fnamei, "rb")) == NULL){
        fprintf (stderr, "Input file '%s' does not exist !!\n", inf->fnamei);
        return -1;
    } else {
        fseek(inf->fpi, 0L, SEEK_END);
        inf->filebytes = ftell(inf->fpi);
        rewind(inf->fpi);
        Log("Open input '%s' [%d bytes] succeed.", inf->fnamei, inf->filebytes);
    }

    /* read wav info to inf->wav structure */
    ret = fread(inf->wavhead, sizeof(char), WAVHEAD_SIZE, inf->fpi);
    if (ret != WAVHEAD_SIZE) {
        Loge("read %d of %ld", ret, WAVHEAD_SIZE);
        ret = -1;
        goto end;
    } else
        ret = 0;

    int idx = 0;
    inf->wave.riffid = str2int32(inf->wavhead + idx, 1);
    idx += 4;
    if (inf->wave.riffid != WAV_RIFF_ID) {
        Loge("riffid %x not match %x", inf->wave.riffid, WAV_RIFF_ID);
        goto end;
    }

    inf->wave.riffsz = str2int32(inf->wavhead + idx, inf->ibn);
    idx += 4;

    inf->wave.waveid = str2int32(inf->wavhead + idx, 1);
    idx += 4;
    if (inf->wave.waveid != WAV_WAVE_ID) {
        Loge("waveid %x not match %x", inf->wave.waveid, WAV_WAVE_ID);
        goto end;
    }

    inf->wave.fmtid = str2int32(inf->wavhead + idx, 1);
    idx += 4;
    if (inf->wave.fmtid != WAV_FMT_ID) {
        Loge("fmtid %x not match %x", inf->wave.fmtid, WAV_FMT_ID);
        goto end;
    }

    inf->wave.fmtsz     = str2int32(inf->wavhead + idx, inf->ibn);
    idx += 4;

    inf->wave.ftag      = str2int16(inf->wavhead + idx, inf->ibn);
    idx += 2;

    inf->wave.nchan     = str2int16(inf->wavhead + idx, inf->ibn);
    idx += 2;

    inf->wave.samplerate= str2int32(inf->wavhead + idx, inf->ibn);
    idx += 4;

    inf->wave.byte_rate = str2int32(inf->wavhead + idx, inf->ibn);
    idx += 4;

    inf->wave.blockalign= str2int16(inf->wavhead + idx, inf->ibn);
    idx += 2;

    inf->wave.width     = str2int16(inf->wavhead + idx, inf->ibn);
    idx += 2;

    inf->wave.dataid = str2int32(inf->wavhead + idx, 1);
    idx += 4;
    if (inf->wave.dataid != WAV_DATA_ID) {
        Loge("dataid %x not match %x", inf->wave.dataid, WAV_DATA_ID);
        goto end;
    }

    inf->wave.datasz = str2int32(inf->wavhead + idx, inf->ibn);
    idx += 4;

    /* refresh inf structure from inf->wav structure */
    inf->ibn = 0;
    inf->voldb = 0.0;
    inf->volgain = 0.0;
    inf->length = (double)inf->wave.datasz / inf->wave.nchan /\
                  (inf->wave.width >> 3) / inf->wave.samplerate;
    inf->recsize = 0;
    inf->width = inf->wave.width;
    inf->rate = inf->wave.samplerate;
    inf->nchan = inf->wave.nchan;
    inf->nsample = inf->wave.datasz / inf->wave.nchan / (inf->wave.width >> 3);
    inf->databytes = inf->wave.datasz;
    inf->filebytes = inf->wave.riffsz + 8;

end:
    fclose(inf->fpi);
    audinfo(inf);
    return ret;
}

