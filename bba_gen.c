/* bba_gen.c */
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
    OP_GEN_SCALE = 600,
    OP_GEN_LENTH,
    OP_GEN_WIDTH,
    OP_GEN_RATE,
    OP_GEN_CHANNEL,
    OP_GEN_FREQ,
    OP_GEN_RAMPUP,
    OP_GEN_TONE,
    OP_GEN_SWEEP,
    OP_GEN_WHITENOISE,
    OP_GEN_SECTION,
    OP_GEN_MAX
};

const char gen_help[] = "\n"
"busybox audio generator\n"
"usage:\n"
"bba gen [-h|--help]\n"
"        [-l|--list]\n"
"        [-v|--verbose]\n"
"        [-o <filename>] [--scale <value>] [--length <value>]\n"
"        [--width <value>] [--rate <value>] [--channel <value>]\n"
"        [--freq <value>] [--rampup <value>]\n"
"        [--tone] [--sweep <value pair>] [--whitenoise]\n"
"        [--section <value pairs>]\n"
"description:\n"
"    -h  --help\n"
"    -l  --list                  list supported encoder.\n"
"    -v  --verbose               enable verbose print.\n"
"    -o <filename>               output file name.\n"
"        --scale <value>         volume control, dBFS: 0, -3.3, -12, etc.\n"
"        --length <value>        seconds: 1, 600, etc.\n"
"\n"
"        --width <value>         bits: 8, 16, etc.\n"
"        --rate <value>          Hz: 8000, 44100, etc.\n"
"        --channel <value>       number: 1, 2, etc.\n"
"\n"
"        --freq <value>          Hz: 300, 1000, etc.\n"
"        --rampup <value>        ramp up time when gain=MAX: 5, 10, etc.\n"
"        --tone                  genarate tone of sin wav\n"
"        --sweep <value pair>    start freq, end freq: 20,22000, etc.\n"
"        --whitenoise            generate white noise.\n"
"        --section <value pairs> generate sections with demanded time & gain.\n"
"                                the (start time, gain) pairs be seperated by\n"
"                                '/': 0,12/5,0/7,12, etc. the number of \n"
"                                section is limited by SECTION_PAIRS.\n";

int argproc_gen(s_audinfo_t *inf, int argc, char *argv[])
{
    int ret = 0;
    int c;

    /* parse parameters */
    if (argc < 2) {
        usage(GEN_HELP);
        return ret;
    }

    while (1) {
        int option_index = 0;
		static const char short_options[] = "hlvo:";
        static const struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"list",        0, 0, 'l'},
            {"verbose",     0, 0, 'v'},
            {"scale",       1, 0, OP_GEN_SCALE},
            {"length",      1, 0, OP_GEN_LENTH},
            {"width",       1, 0, OP_GEN_WIDTH},
            {"rate",        1, 0, OP_GEN_RATE},
            {"channel",     1, 0, OP_GEN_CHANNEL},
            {"freq",        1, 0, OP_GEN_FREQ},
            {"rampup",      1, 0, OP_GEN_RAMPUP},
            {"tone",        0, 0, OP_GEN_TONE},
            {"sweep",       1, 0, OP_GEN_SWEEP},
            {"whitenoise",  0, 0, OP_GEN_WHITENOISE},
            {"section",     1, 0, OP_GEN_SECTION},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, short_options,
                 long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'h':
            usage(GEN_HELP);
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
        case OP_GEN_SCALE:
            inf->en_tone = 1;
            inf->voldb = (double)atoi(optarg);
            break;
        case OP_GEN_LENTH:
            inf->en_tone = 1;
            inf->length = (double)atoi(optarg);
            break;
        case OP_GEN_WIDTH:
            inf->en_tone = 1;
            inf->width = atoi(optarg);
            break;
        case OP_GEN_RATE:
            inf->en_tone = 1;
            inf->rate = atoi(optarg);
            break;
        case OP_GEN_CHANNEL:
            inf->en_tone = 1;
            inf->nchan = atoi(optarg);
            break;
        case OP_GEN_FREQ:
            inf->en_tone = 1;
            inf->freq = atoi(optarg);
            break;
        case OP_GEN_RAMPUP:
            inf->en_tone = 1;
            inf->rampup = atoi(optarg);
            break;
        case OP_GEN_TONE:
            inf->en_tone = 1;
            break;
        case OP_GEN_SWEEP:
            inf->en_sweep = 1;
            ret = parse_pair_done(optarg, inf->sweep);
            if (ret < 0)
                return ret;
            break;
        case OP_GEN_WHITENOISE:
            inf->en_whitenoise = 1;
            break;
        case OP_GEN_SECTION:
            inf->en_tone = 1;
            ret = parse_pairs_done(optarg, inf->section);
            if (ret < 0)
                return ret;
            break;
        default:
            break;
        }
    }

    ret = generate_process(inf);
    return ret;
}

int generate_process(s_audinfo_t *inf)
{
    int ret = 0;
    int val = 0;
    int cnt = 0;

    double t = 0.0;
    double dt = (double)1.0 / inf->rate;
    int wid = inf->width >> 3;
    int ch = inf->nchan;

    double current_freq = (double)inf->sweep[0];
    double df = 0.0;

    double gain = pow(10, (0 - inf->voldb)/20);

    int idx_section = 0;
    int gain_current = 0;
    double gain_sec1 = 0.0;
    double gain_sec2 = 0.0;
    double gain_sec3 = 0.0;

    /* check parameters */
    if (inf->sweep[1] < 0 || inf->sweep[1] < inf->sweep[0]) {
        Loge("Invalid sweep setting: %d %d",inf->sweep[0],inf->sweep[1]);
        return -1;
    } else if ((inf->sweep[0] || inf->sweep[1]) != 0) {
        df = (double)(inf->sweep[1] - inf->sweep[0]) / inf->nsample;
        inf->en_sweep = 1;
    }

    if ((inf->section[2] || inf->section[4] || inf->section[6]) != 0) {
        inf->section_valid = 1;
        do {
            if (gain_current > inf->section[2 * idx_section]) {
                Loge("Invalid sector setting: %d > %d",\
                    gain_current, inf->section[2 * idx_section]);
                return -1;
            }
            else
               gain_current = inf->section[2 * idx_section];
            idx_section++;
        }while (idx_section < SECTION_PAIRS);
        gain_sec1 = pow(10, (0 - (double)inf->section[1])/20);
        gain_sec2 = pow(10, (0 - (double)inf->section[3])/20);
        gain_sec3 = pow(10, (0 - (double)inf->section[5])/20);
        Log("%f %f %f",gain_sec1, gain_sec2, gain_sec3);
    }

    /* allocate buffer */
    inf->databytes = inf->length * inf->rate * inf->nchan * (inf->width >> 3);
    inf->nsample = inf->length * inf->rate;
    if ((inf->data = calloc(sizeof(char), inf->databytes)) == NULL) {
        Loge("allocate data buffer failed, exit.");
        return -1;
    }

    /* generate tones */
    while (cnt < inf->nsample) {

        /* generate basic tone */
        if (inf->en_tone)
            val = RANGE_16 * sin(2 * PI * t * inf->freq);
        else if (inf->en_whitenoise)
            val = RANGE_16 * (double)rand() / RAND_MAX * 2.0 - 1.0;
        else if (inf->en_sweep) {
            val = RANGE_16 * sin(1 * PI * t * current_freq); //still not clear
            current_freq += df;
        } else {
            Loge("no designated operation");
            return -1;
        }
        t += dt;

        /* effect for gain */
        val *= gain;

        /* effect for rampup */
        if (t < inf->rampup)
            val *= t / inf->rampup;

        /* effect for section */
        if (!inf->section_valid)
            ;
        else if (t > inf->section[0] && t < inf->section[2])
            val *= gain_sec1;
        else if (t > inf->section[2] && t < inf->section[4])
            val *= gain_sec2;
        else if (t > inf->section[4])
            val *= gain_sec3;

        switch (ch) {
            case 1:
                int2str16(inf->data + cnt * wid, val, inf->ibn);
                cnt++;
                break;
            case 2:
                int2str16(inf->data + (cnt * wid + 0) * ch, val, inf->ibn);
                int2str16(inf->data + (cnt * wid + 1) * ch, val, inf->ibn);
                cnt++;
                break;
            default:
                Loge("Not supported channel number %d", ch);
                goto end;
        }
    }

    /* save output */
    ret = save_output(inf);

    /* release buffer */
end:
    free(inf->data);
    inf->data = NULL;
    return ret;
}

int parse_pair_done(char *args, int *pair)
{
    int ret = 0;
    char c = 0;
    int offset = 0;

    if (args == NULL || *args == 0)
        return -1;

    if (pair == NULL)
        return -2;

    pair[0] = pair[1] = 0;

    do {
        c = args[offset];
        if (c == ',') {
            offset++;
            break;
        } else if (c > '9' || c < '0' || pair[0] > RANGE_16) {
            pair[0] = -1;
            return -3;
        }
        pair[0] = pair[0] * 10 + atoi(&c);
        offset++;
    }while (1);

    do {
        c = args[offset];
        if (c == 0) {
            return 0;
            break;
        } else if (c == '/') {
            offset++;
            return offset;
            break;
        } else if (c > '9' || c < '0' || pair[1] > RANGE_16) {
            pair[1] = -1;
            return -4;
        }
        pair[1] = pair[1] * 10 + atoi(&c);
        offset++;
    }while (1);

    return ret;
}

int parse_pairs_done(char *args, int *pairs)
{
    int ret = 0;
    int args_offset = 0;
    int pairs_offset = 0;

    do {
        ret = parse_pair_done(args + args_offset, pairs + pairs_offset);
        if (ret < 0)
            return ret;
        else if (ret == 0)
            break;
        else {
            pairs_offset += 2;
            args_offset += ret;
        }
    }while(1);

    return ret;
}

int save_output(s_audinfo_t *inf)
{
    int ret = 0;
    int filetype = 0;

    if ((inf->fpo = fopen(inf->fnameo, "wb+")) == NULL){
        fprintf (stderr, "Open output file '%s' fail !!\n", inf->fnameo);
        ret = -1;
        goto end;
    } else {
        Log("Open output file '%s' succeed.", inf->fnameo);
    }

    filetype = parse_filename(inf->fnameo, strlen(inf->fnameo));
    if (filetype == FILE_WAV)
        ret = fwrite(inf->wavhead, sizeof(char), WAVHEAD_SIZE, inf->fpo);
    if (filetype == FILE_WAV || filetype == FILE_RAW || filetype == FILE_PCM)
        ret = fwrite(inf->data, sizeof(char), inf->databytes, inf->fpo);

    fclose(inf->fpo);
end:
    return ret;
}

