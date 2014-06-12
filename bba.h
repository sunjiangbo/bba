/* bba.h */
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

#ifndef _BBA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <sys/signal.h>

#define RANGE_8         0x7F
#define RANGE_16        0x7FFF
#define RANGE_24        0x7FFFFF
#define RANGE_32        0x7FFFFFFF

#define PI              3.1415926535897932384626433832795

#define FILENAME_SIZE  	40
#define WAVHEAD_SIZE	44L
#define FORMAT_SIZE     0x10
#define FORMATTAG_PCM	0x0001

#define DEFAULT_WIDTH   16
#define DEFAULT_NCH     2
#define DEFAULT_RATE    44100
#define DEFAULT_LEN     10.0
#define DEFAULT_FREQ    1000
#define DEFAULT_NAMEI   "in.wav"
#define DEFAULT_NAMEO   "out.wav"
#define DEFAULT_RECSIZE ((DEFAULT_WIDTH >> 3) * DEFAULT_NCH * \
                        DEFAULT_RATE * DEFAULT_LEN)
#define DEFAULT_SWEEP   "20,22050"
#define DEFAULT_SECTION "0,12/5,0/8,12"
/*
 * alsa record and play device
 * review with aplay -l, cat /proc/asound/cards and cat /proc/asound/devices
 */
#define DEFAULT_ALSADEV "hw:0,0"
/* RIFF */
#define WAV_RIFF_ID     0x52494646
/* WAVE */
#define WAV_WAVE_ID     0x57415645
/* fmt  */
#define WAV_FMT_ID      0x666d7420
/* data */
#define WAV_DATA_ID     0x64617461

#define MAX_WIDTH       16
#define MAX_NCH         2
#define MAX_RATE        192000
#define MAX_LEN         600.0
#define MAX_FREQ        22000

#define SWEEP_PAIRS     1
#define SWEEP_SIZE      (SWEEP_PAIRS * 2)
#define SECTION_PAIRS   3
#define SECTION_SIZE    (SECTION_PAIRS * 2)

/*
 * AVCODEC third party library
 */
//#define USE_LIBAVCODEC_FFMPEG
#ifdef USE_LIBAVCODEC_FFMPEG
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>

#define INBUF_SIZE          4096
#define AUDIO_INBUF_SIZE    20480
#define AUDIO_REFILL_THRESH 4096
#endif

typedef __int8_t s8_t;
typedef __uint8_t u8_t;
typedef __int16_t s16_t;
typedef __uint16_t u16_t;
typedef __int32_t s32_t;
typedef __uint32_t u32_t;
typedef __int64_t s64_t;
typedef __uint64_t u64_t;

typedef struct {
	struct timeval start;
	struct timeval end;
	struct timezone tz;
} s_clock;

typedef struct {
	u32_t riffid;
	u32_t riffsz;
	u32_t waveid;
	u32_t fmtid;
	u32_t fmtsz;
	u16_t ftag;
	u16_t nchan;
	u32_t samplerate;
	u32_t byte_rate;
	u16_t blockalign;
	u16_t width;
	u32_t dataid;
	u32_t datasz;
} s_waveinfo_t;

typedef struct {
	char dev[FILENAME_SIZE];
	snd_pcm_stream_t stream;
	snd_pcm_access_t access;
	snd_pcm_format_t format;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	snd_pcm_uframes_t chunk_size;
	snd_pcm_uframes_t chunk_byte;
	int dir;                    //play/record
	int nonblock;               //nonblock
} s_alsainfo_t;

typedef union frame_head {
	u32_t value;
	struct {
		u32_t sync:        12;
		u32_t id:          1;
		u32_t layer:       2;
		u32_t protect:     1;
		u32_t bitrate:     4;
		u32_t samplerate:  2;
		u32_t padding:     1;
		u32_t private:     1;
		u32_t mode:        2;
		u32_t mode_ext:    2;
		u32_t copyright:   1;
		u32_t original:    1;
		u32_t emphasis:    2;
	} bits_mp3;
} u_framehead_t;

typedef struct {
	u_framehead_t head;
	int idx;
	int nframe;
} s_parserinfo_t;

typedef struct {
	int ibn;		            //is big endian
	double voldb;               //volume, dBFs
	double volgain;             //volume gain, db
	double length;		        //length in seconds
	int recsize;                //target record size
	int width;		            //width
	int rate;			        //sample rate
	int nchan;			        //number of channel
	int nsample;			    //number of sample
	int databytes;			    //data size in bytes
	int filebytes;              //file size in bytes
	int en_tone;                //generate sign of tone
	int freq;		            //frequency
	int rampup;                 //ramp up stop time in seconds
	int section_valid;          //section effect valid
	int section[SECTION_SIZE];  //sections with different gain
	int en_sweep;               //generate sign of sweep
	int sweep[SWEEP_SIZE];      //sweep start and end freq
	int en_whitenoise;          //generate sign of white noise
	int codec_type;             //enum CODEC_TYPE
	char fnamei[FILENAME_SIZE];
	char fnameo[FILENAME_SIZE];
	s_alsainfo_t alsa;
	s_waveinfo_t wave;
	s_parserinfo_t parser;
	char wavhead[WAVHEAD_SIZE];
	char *data;
	FILE *fpi;
	FILE *fpo;
} s_audinfo_t;

extern int debug_log;
extern int stopsign;

#define Log(...) \
    do {\
        if (debug_log) {\
            printf("%s %-10s [%d]:",__FILE__, __func__, __LINE__); \
            printf(__VA_ARGS__); \
            printf("\n"); \
        }\
    } while(0)

#define Loge(...) \
    do {\
        printf("%s %-10s [%d]:",__FILE__, __func__, __LINE__); \
        printf(__VA_ARGS__); \
        printf("\n"); \
    } while(0)

extern char *optarg;
extern int optind, opterr, optopt;

#define _GNU_SOURCE
#include <getopt.h>

int getopt_long(int argc, char * const argv[], const char *optstring,
		const struct option *longopts, int *longindex);

enum {
	MAIN_HELP, GEN_HELP, PLAY_HELP, RECORD_HELP, CONV_HELP, INFO_HELP, MAX_HELP
};

enum {
	COMMAND_NOP,
	COMMAND_GEN,
	COMMAND_PLAY,
	COMMAND_RECORD,
	COMMAND_CONV,
	COMMAND_INFO,
	COMMAND_MAX
};

enum {
	FILE_RAW,
	FILE_PCM,
	FILE_WAV,
	FILE_MP3,
	FILE_AAC,
	FILE_AMR,
	FILE_AWB,
	FILE_MAX
};

enum CODEC_TYPE {
	CODEC_PCM, CODEC_MP3, CODEC_AAC, CODEC_AMR, CODEC_AWB, CODEC_MAX
};

#define FILESTR_RAW ".raw"
#define FILESTR_PCM ".pcm"
#define FILESTR_WAV ".wav"
#define FILESTR_MP3 ".mp3"
#define FILESTR_AAC ".aac"
#define FILESTR_AMR ".amr"
#define FILESTR_AWB ".arb"

void init(s_audinfo_t *inf);
void uninit(s_audinfo_t *inf);
int argproc(s_audinfo_t *inf, int argc, char *argv[]);
int clear_struct(s_audinfo_t *inf);
void usage(int type);

int argproc_gen(s_audinfo_t *inf, int argc, char *argv[]);
int generate_process(s_audinfo_t *inf);
int parse_pair_done(char *args, int *pair);
int parse_pairs_done(char *args, int *pairs);
int save_output(s_audinfo_t *inf);

int argproc_play(s_audinfo_t *inf, int argc, char *argv[]);
int playwav(s_audinfo_t *inf);
int initplay(s_audinfo_t *inf);
int parsewav(s_audinfo_t *inf);

int argproc_record(s_audinfo_t *inf, int argc, char *argv[]);
void signal_handler(int sig);
int recwav(s_audinfo_t *inf);
int initrec(s_audinfo_t *inf);

int argproc_conv(s_audinfo_t *inf, int argc, char *argv[]);
int convert_process(s_audinfo_t *inf);
int file_is_compressed(int filetype);
int file_to_codec(int filetype);

int argproc_info(s_audinfo_t *inf, int argc, char *argv[]);
int info_process(s_audinfo_t *inf);
int search_tags(s_audinfo_t *inf);
int search_frames(s_audinfo_t *inf);
int is_frame_valid(s_audinfo_t *inf);
int frame_info(s_audinfo_t *inf);

int updatewavhead(s_audinfo_t *inf);
int parse_filename(char *fname, int length);
int audinfo(s_audinfo_t *inf);
int alsa_prepare(s_audinfo_t *inf);
int alsa_play(s_audinfo_t *inf);
int alsa_record(s_audinfo_t *inf);
u16_t str2int16(char *buf, int endian);
u32_t str2int32(char *buf, int endian);
void int2str16(char* buf, int val, int endian);
void int2str32(char* buf, int val, int endian);
int audio_file_decode(s_audinfo_t *inf);
int audio_file_encode(s_audinfo_t *inf);
#ifdef USE_LIBAVCODEC_FFMPEG
int audio_file_decode_ffmpeg(s_audinfo_t *inf);
int audio_file_encode_ffmpeg(s_audinfo_t *inf);
enum AVCodecID audio_codec_bba_to_ffmpeg(int codec);
#endif

#endif

