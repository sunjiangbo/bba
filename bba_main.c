/* bba_main.c */
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

static const char main_help[] = "\n"
"busybox audio toolkit\n"
"usage:\n"
"bba [-h|--help]\n"
"    <command> [args]\n"
"\n"
"The most commonly used commands are:\n"
"    gen         generate tone or special signal\n"
"    play        play music file with supported decode type\n"
"    record      record sound till time limit or size limit or user interrupt\n"
"    conv        codec/width/src/channel/gain convert\n"
"    info        show current hardware informations\n"
"\n"
"See 'bba <command> for more information on a specific command.\n";

extern const char gen_help[];
extern const char play_help[];
extern const char record_help[];
extern const char conv_help[];
extern const char info_help[];

static const char command_gen[] = "gen";
static const char command_play[] = "play";
static const char command_record[] = "record";
static const char command_conv[] = "conv";
static const char command_info[] = "info";

int debug_log = 0;

int main(int argc, char *argv[])
{
    int ret = 0;
    s_audinfo_t inf;

    init(&inf);

    ret = argproc(&inf, argc, argv);
    if (ret != 0) {
        Loge("Error: %s(%d)", snd_strerror (ret), ret);
		goto finish;
    }

finish:
    uninit(&inf);
    return ret;
}

void init(s_audinfo_t *inf)
{
    clear_struct(inf);
    Log("%ld bytes cleared", sizeof(s_audinfo_t));
    inf->voldb          = 0.0;
    inf->volgain        = 0.0;
	inf->length         = DEFAULT_LEN;
    inf->recsize        = DEFAULT_RECSIZE;
    inf->width          = DEFAULT_WIDTH;
    inf->rate           = DEFAULT_RATE;
    inf->nchan          = DEFAULT_NCH;
    inf->nsample        = inf->length * inf->rate;
    inf->databytes      = inf->nsample * inf->nchan * (inf->width >> 3);
    inf->filebytes      = inf->databytes + WAVHEAD_SIZE;
    inf->freq           = DEFAULT_FREQ;
    strcpy(inf->fnamei,   DEFAULT_NAMEI);
    strcpy(inf->fnameo,   DEFAULT_NAMEO);
    strcpy(inf->alsa.dev, DEFAULT_ALSADEV);
    updatewavhead(inf);
}

void uninit(s_audinfo_t *inf)
{
    if (inf->data != NULL) {
        free(inf->data);
        inf->data = NULL;
    }
}

int argproc(s_audinfo_t *inf, int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
        usage(MAIN_HELP);
    else if (!strcmp (argv[1], "-h") || !strcmp (argv[1], "--help"))
        usage(MAIN_HELP);
    else if (!strcmp (argv[1], command_gen))
        ret = argproc_gen (inf, --argc, ++argv);
    else if (!strcmp (argv[1], command_play))
        ret = argproc_play (inf, --argc, ++argv);
    else if (!strcmp (argv[1], command_record))
        ret = argproc_record (inf, --argc, ++argv);
    else if (!strcmp (argv[1], command_conv))
        ret = argproc_conv (inf, --argc, ++argv);
    else if (!strcmp (argv[1], command_info))
        ret = argproc_info (inf, --argc, ++argv);
    else {
        Loge("Invalis parameter: %s", argv[1]);
    }

    if (ret != 0)
        Loge("Error: return %d", ret);
    return ret;
}

int clear_struct(s_audinfo_t *inf)
{
    int ret = 0;
    memset(inf, 0, sizeof(s_audinfo_t));    
    return ret;
}

void usage(int type)
{
    switch (type) {
    case MAIN_HELP:
        printf("%s", main_help);
        break;
    case GEN_HELP:
        printf("%s", gen_help);
        break;
    case PLAY_HELP: 
        printf("%s", play_help);
        break;
    case RECORD_HELP:
        printf("%s", record_help);
        break;
    case CONV_HELP:
        printf("%s", conv_help);
        break;
    case INFO_HELP:
        printf("%s", info_help);
        break;
    default:
        printf("Unsupported operation.\n");
        break;
    }
}

