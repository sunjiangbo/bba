/* bba_misc.c */
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

int updatewavhead(s_audinfo_t *inf)
{
    int ret = 0;
    int idx = 0;
 
    inf->wave.riffid    = WAV_RIFF_ID;
    inf->wave.riffsz    = inf->filebytes - 8;
    inf->wave.waveid    = WAV_WAVE_ID;
    inf->wave.fmtid     = WAV_FMT_ID;
    inf->wave.fmtsz     = FORMAT_SIZE;
    inf->wave.ftag      = FORMATTAG_PCM;
    inf->wave.nchan     = inf->nchan;
    inf->wave.samplerate= inf->rate;
    inf->wave.byte_rate = inf->rate * inf->nchan * (inf->width >> 3);
    inf->wave.blockalign= inf->nchan * (inf->width >> 3);
    inf->wave.width     = inf->width;
    inf->wave.dataid    = WAV_DATA_ID;
    inf->wave.datasz    = inf->databytes;

    int2str32(inf->wavhead + idx, inf->wave.riffid, 1);
    idx += 4;
    int2str32(inf->wavhead + idx, inf->wave.riffsz, inf->ibn);
    idx += 4;
    int2str32(inf->wavhead + idx, inf->wave.waveid, 1);
    idx += 4;
    int2str32(inf->wavhead + idx, inf->wave.fmtid, 1);
    idx += 4;
    int2str32(inf->wavhead + idx, inf->wave.fmtsz, inf->ibn);
    idx += 4;
    int2str32(inf->wavhead + idx, inf->wave.ftag, inf->ibn);
    idx += 2;
    int2str32(inf->wavhead + idx, inf->wave.nchan, inf->ibn);
    idx += 2;
    int2str32(inf->wavhead + idx, inf->wave.samplerate, inf->ibn);
    idx += 4;
    int2str32(inf->wavhead + idx, inf->wave.byte_rate, inf->ibn);
    idx += 4;
    int2str32(inf->wavhead + idx, inf->wave.blockalign, inf->ibn);
    idx += 2;
    int2str32(inf->wavhead + idx, inf->wave.width, inf->ibn);
    idx += 2;
    int2str32(inf->wavhead + idx, inf->wave.dataid, 1);
    idx += 4;
    int2str32(inf->wavhead + idx, inf->wave.datasz, inf->ibn);
    idx += 4;

    return ret;
}

int parse_filename(char *fname, int length)
{
Log("%s %d", fname, length);
    int ret = 0;
    int idx = 0;
    char *ext = NULL;

    if (fname == NULL || length > (FILENAME_SIZE - 1))
        return -1;
    idx = length - 5;
    ext = fname;
    Log("%s %d", ext, idx);
    while (*(ext + idx) != '.') {
        Log(":%c", *(ext + idx));
        idx++;
        if (idx > length)
            return -1;
    }
    ext += idx;
    Log("%s %d", ext, idx);
    if (!strncmp(ext, FILESTR_RAW, strlen(FILESTR_RAW)))
        ret = FILE_RAW;
    else if (!strncmp(ext, FILESTR_PCM, strlen(FILESTR_PCM)))
        ret = FILE_PCM;
    else if (!strncmp(ext, FILESTR_WAV, strlen(FILESTR_WAV)))
        ret = FILE_WAV;
    else if (!strncmp(ext, FILESTR_MP3, strlen(FILESTR_MP3)))
        ret = FILE_MP3;
    else if (!strncmp(ext, FILESTR_AAC, strlen(FILESTR_AAC)))
        ret = FILE_AAC;
    else if (!strncmp(ext, FILESTR_AMR, strlen(FILESTR_AMR)))
        ret = FILE_AMR;
    else if (!strncmp(ext, FILESTR_AWB, strlen(FILESTR_AWB)))
        ret = FILE_AWB;
    return ret;
}

int audinfo(s_audinfo_t *inf)
{
    int ret = 0;
    Log("is big endian:         %8d", inf->ibn);
    Log("Attenuate in dBFS:     %8.4f", inf->voldb);
    Log("volume gain in dB:     %8.4f", inf->volgain);
    Log("clip length in seconds:%8.4f", inf->length);
    Log("clip size in bytes:    %8d", inf->recsize);
    Log("sample width in bits:  %8d", inf->width);
    Log("sample rate per second:%8d", inf->rate);
    Log("channel number:        %8d", inf->nchan);    
    Log("number of sample:      %8d", inf->nsample);
    Log("clip size in Bytes:    %8d", inf->databytes);
    Log("file size in Bytes:    %8d", inf->filebytes);
    Log("tone frequency in HZ:  %8d", inf->freq);
    Log("alsa stream type:      %8d", inf->alsa.stream);
    Log("alsa access type:      %8d", inf->alsa.access);
    Log("alsa format type:      %8d", inf->alsa.format);
    Log("alsa direction:        %8d", inf->alsa.dir);
    Log("alsa non-block:        %8d", inf->alsa.nonblock);
    Log("wav riffid:          0x%8x", inf->wave.riffid);
    Log("wav riff size:         %8d", inf->wave.riffsz);
    Log("wav waveid:          0x%8x", inf->wave.waveid);
    Log("wav fmt id:          0x%8x", inf->wave.fmtid);
    Log("wav fmt size in Bytes: %8d", inf->wave.fmtsz);
    Log("wav ftag:            0x%8x", inf->wave.ftag);
    Log("wav channel number:    %8d", inf->wave.nchan);
    Log("wav sample per seconds:%8d", inf->wave.samplerate);
    Log("wav bytes per seconds: %8d", inf->wave.byte_rate);
    Log("wav bytes align:       %8d", inf->wave.blockalign);
    Log("wav sample width:      %8d", inf->wave.width);
    Log("wav dataid:          0x%8x", inf->wave.dataid);
    Log("wav data size in Bytes:%8d", inf->wave.datasz);
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

int parsemp3(s_audinfo_t *inf)
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

    /* search for frame header */

    /* search for ID3 tag */

    fclose(inf->fpi);
    return ret;
}

int parseaac(s_audinfo_t *inf)
{
	int ret = 0;
	return ret;
}

int parseamr(s_audinfo_t *inf)
{
	int ret = 0;
	return ret;
}

int parseawb(s_audinfo_t *inf)
{
	int ret = 0;
	return ret;
}

int alsa_prepare(s_audinfo_t *inf)
{
    int ret;
    snd_pcm_t *handle = inf->alsa.handle;

    //hw sw parameters
    snd_pcm_hw_params_t *hw_params = inf->alsa.hw_params;
    snd_pcm_sw_params_t *sw_params = inf->alsa.sw_params;

    ret = snd_pcm_hw_params_malloc (&hw_params);
    if (ret < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    ret = snd_pcm_sw_params_malloc (&sw_params);
    if (ret < 0) {
        fprintf (stderr, "cannot allocate sw parameter structure (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    ret = snd_pcm_hw_params_any (handle, hw_params);
    if (ret < 0) {
        fprintf (stderr, "cannot initialize sw parameter structure (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    ret = snd_pcm_hw_params_set_access (handle, hw_params, inf->alsa.access);
    if (ret < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    ret = snd_pcm_hw_params_set_format (handle, hw_params, inf->alsa.format);
    if (ret < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    ret = snd_pcm_hw_params_set_channels (handle, hw_params, inf->nchan);
    if (ret < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    ret = snd_pcm_hw_params_set_rate_near (handle,
                                           hw_params,
                                           (unsigned int *)&(inf->rate),
                                           &(inf->alsa.dir));
    if (ret < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    unsigned int buffer_time = 0;
    ret = snd_pcm_hw_params_get_buffer_time_max(hw_params,
                                                &buffer_time,
                                                &(inf->alsa.dir));
    if (ret < 0) {
        fprintf (stderr, "cannot get buffer time max (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    Log("Before: buffer_time %d us", buffer_time);
    if (buffer_time > 500000)
            buffer_time = 500000;
    Log("After: buffer_time %d us", buffer_time);
    unsigned int period_time = 0;
    period_time = buffer_time / 4;
    ret = snd_pcm_hw_params_set_period_time_near(handle,
                                                 hw_params,
                                                 &period_time,
                                                 &(inf->alsa.dir));
    if (ret < 0) {
        fprintf (stderr, "cannot set period time (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    ret = snd_pcm_hw_params_set_buffer_time_near(handle,
                                                 hw_params,
                                                 &buffer_time,
                                                 &(inf->alsa.dir));
    if (ret < 0) {
        fprintf (stderr, "cannot set buffer time (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    ret = snd_pcm_hw_params (handle, hw_params);
    if (ret < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    snd_pcm_uframes_t chunk_size = inf->alsa.chunk_size;
    snd_pcm_uframes_t buffer_size = 0;
    ret = snd_pcm_hw_params_get_period_size(hw_params,
                                            &chunk_size,
                                            &(inf->alsa.dir));
    if (ret < 0) {
        fprintf (stderr, "cannot get period size (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    inf->alsa.chunk_size = chunk_size;
    inf->alsa.chunk_byte = chunk_size * (inf->width >> 3) * inf->nchan;
    ret = snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
    if (ret < 0) {
        fprintf (stderr, "cannot get buffer size (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    Log("chunk_size %ld buffer_size %ld", chunk_size, buffer_size);
    ret = snd_pcm_sw_params_current(handle, sw_params);
    if (ret < 0) {
        fprintf (stderr, "cannot get current sw params (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    ret = snd_pcm_sw_params_set_avail_min(handle, sw_params, chunk_size);
    if (ret < 0) {
        fprintf (stderr, "cannot set available min (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    snd_pcm_uframes_t start_threshold, stop_threshold;
    if (inf->alsa.stream == SND_PCM_STREAM_PLAYBACK)
        start_threshold = buffer_size;
    else
        start_threshold = 1;
    ret = snd_pcm_sw_params_set_start_threshold(handle,
                                                sw_params,
                                                start_threshold);
    if (ret < 0) {
        fprintf (stderr, "cannot set start threshold (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    stop_threshold = buffer_size;
    ret = snd_pcm_sw_params_set_stop_threshold(handle,
                                               sw_params,
                                               stop_threshold);
    if (ret < 0) {
        fprintf (stderr, "cannot set stop threshold (%s)\n",
             snd_strerror (ret));
        goto end;
    }
    ret = snd_pcm_sw_params (handle, sw_params);
    if (ret < 0) {
        fprintf (stderr, "cannot set sw parameters (%s)\n",
             snd_strerror (ret));
        goto end;
    }    
    snd_pcm_hw_params_free (hw_params);
    snd_pcm_sw_params_free (sw_params);
    ret = snd_pcm_nonblock (handle, 0);
    if (ret < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (ret));
        goto end;
    }
end:
    return ret;
}

int alsa_play(s_audinfo_t *inf)
{
    int ret = 0;
    int idx = 0;
    int samples_to_play = 0;
    int samples_remain = inf->nsample;
    int bytes_per_sample = inf->nchan * (inf->width >> 3);
    snd_pcm_t *handle;
    snd_pcm_info_t *info;

    ret = snd_pcm_open (&(inf->alsa.handle),
                        inf->alsa.dev,
                        inf->alsa.stream,
                        0);
    if (ret < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
             inf->alsa.dev,
             snd_strerror (ret));
        return ret;
    }
    handle = inf->alsa.handle;

    snd_pcm_info_alloca(&info);

    ret = snd_pcm_info(handle, info);
    if (ret < 0) {
        fprintf (stderr, "snd_pcm_info error(%s)\n",
             snd_strerror (ret));
        goto end;
    }

    ret = alsa_prepare(inf);
    if (ret < 0) {
        fprintf (stderr, "alsa_prepare error(%s)\n",
             snd_strerror (ret));
        goto end;
    }

    /*
     * Notice: snd_pcm_writei() write in _(frame) while idx 
     * increase in _(byte)
     */
    do {
        if (samples_remain < inf->alsa.chunk_size)
            samples_to_play = samples_remain;
        else
            samples_to_play = inf->alsa.chunk_size;
        #if 0
        //review available/delay frame numbers
        snd_pcm_sframes_t avail, delay;
        avail = delay = 0;
        ret = snd_pcm_avail_delay(handle, &avail, &delay);
        if (ret < 0) {
            Log("snd_pcm_avail_delay error: %d(%s)", \
                ret, snd_strerror (ret));
            goto end;
        }
        Log("idx %x %d sz %d buf %ld avail %ld delay %ld", \
             idx, idx, i->sz, i->a.chunk_byte, avail, delay);
        #endif
        ret = snd_pcm_writei (handle,
                              inf->data + idx,
                              samples_to_play);
        if (ret < 0) {
            Loge("ret < 0: %d(%s)", ret, snd_strerror (ret));
            ret = snd_pcm_recover(handle, ret, 0);
            Loge("ret recover result: %d(%s)", ret, snd_strerror (ret));
        } else if (ret != samples_to_play) {
            fprintf (stderr, "write to audio interface failed: %d of %d\n",
                     ret, samples_to_play);
            ret = -1;
            goto end;
        } else {
            idx += samples_to_play * bytes_per_sample;
            samples_remain -= samples_to_play;
            ret = 0;
        }
    }while (samples_remain > 0);
end:
    snd_pcm_close (handle);
    return ret;
}

int alsa_record(s_audinfo_t *inf)
{
    int ret = 0;
    int samples_to_record = 0;
    int bytes_to_record = 0;
    int bytes_per_sample = inf->nchan * (inf->width >> 3);
    int length_to_bytes = inf->length * inf->rate * bytes_per_sample;
    int limit_bytes = (length_to_bytes < inf->recsize) ?
                      inf->recsize : length_to_bytes;
    Log("%d %d %d", limit_bytes, inf->recsize, length_to_bytes);
    snd_pcm_t *handle;
    snd_pcm_info_t *info;

    ret = snd_pcm_open (&(inf->alsa.handle),
                        inf->alsa.dev,
                        inf->alsa.stream,
                        0);
    if (ret < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
             inf->alsa.dev,
             snd_strerror (ret));
        return ret;
    }
    handle = inf->alsa.handle;

    snd_pcm_info_alloca(&info);

    ret = snd_pcm_info(handle, info);
    if (ret < 0) {
        fprintf (stderr, "snd_pcm_info error(%s)\n",
             snd_strerror (ret));
        goto end;
    }

    ret = alsa_prepare(inf);
    if (ret < 0) {
        fprintf (stderr, "alsa_prepare error(%s)\n",
             snd_strerror (ret));
        goto end;
    }

    /* allocate buffer for record */
    samples_to_record = inf->alsa.chunk_size;
    bytes_to_record = samples_to_record * bytes_per_sample;
    if ((inf->data = calloc(sizeof(char), bytes_to_record)) == NULL) {
        Loge("allocate data buffer of %d bytes failed, exit.", bytes_to_record);
        return -1;
    }

    inf->databytes = 0;
    while (!stopsign) {
        if (inf->databytes > limit_bytes) {
            Loge("Recorded size up to limit(%d), stop.", limit_bytes);
            break;
        }
        ret = snd_pcm_readi (handle, inf->data, samples_to_record);
        if (ret < 0) {
            Log("ret err: %s(%d)", snd_strerror (ret), ret);
            ret = snd_pcm_recover(handle, ret, 0);
            Log("ret recover: %s(%d)", snd_strerror (ret), ret);
        } else {
            inf->databytes += bytes_to_record;
            fseek(inf->fpo, 0L, SEEK_END);
            fwrite(inf->data, sizeof(char), bytes_to_record, inf->fpo);
            ret = 0;
        }
    }
    /* update structure and wav head per inf->databytes */
    Log("%d bytes written", inf->databytes);
    inf->nsample = inf->databytes / bytes_per_sample;
    inf->length = (double)inf->nsample / inf->rate;
    inf->filebytes = inf->databytes + WAVHEAD_SIZE;
    updatewavhead(inf);
end:
    free(inf->data);
    inf->data = NULL;
    snd_pcm_close (handle);
    return ret;
}

u16_t str2int16(char* buf, int endian)
{
    if (endian)
        return ((((u16_t)buf[0] & 0xFF) << 8) + buf[1]);
    else
        return ((((u16_t)buf[1] & 0xFF) << 8) + buf[0]);
}

u32_t str2int32(char* buf, int endian)
{
    if (endian)
        return ((((((((u32_t)buf[0] & 0xFF)  << 8) +
                     ((u32_t)buf[1] & 0xFF)) << 8) +
                     ((u32_t)buf[2] & 0xFF)) << 8) +
                     ((u32_t)buf[3] & 0xFF));
    else
        return ((((((((u32_t)buf[3] & 0xFF)  << 8) +
                     ((u32_t)buf[2] & 0xFF)) << 8) +
                     ((u32_t)buf[1] & 0xFF)) << 8) +
                     ((u32_t)buf[0] & 0xFF));
}

void int2str16(char* buf, int val, int endian)
{
    if (endian) {
        buf[1] = val & 0xFF;
        buf[0] = (val >> 8) & 0xFF;
     } else {
        buf[0] = val & 0xFF;
        buf[1] = (val >> 8) & 0xFF;
    }
}

void int2str32(char* buf, int val, int endian)
{
    if (endian) {
        buf[3] = val & 0xFF;
        buf[2] = (val >> 8)  & 0xFF;
        buf[1] = (val >> 16) & 0xFF;
        buf[0] = (val >> 24) & 0xFF;
     } else {
        buf[0] = val & 0xFF;
        buf[1] = (val >> 8)  & 0xFF;
        buf[2] = (val >> 16) & 0xFF;
        buf[3] = (val >> 24) & 0xFF;
    }
}

int audio_file_decode(s_audinfo_t *inf)
{
    int ret = 0;

#ifdef USE_LIBAVCODEC_FFMPEG
    ret = audio_file_decode_ffmpeg(inf);
#endif

    return ret;
}

int audio_file_encode(s_audinfo_t *inf)
{
    int ret = 0;

#ifdef USE_LIBAVCODEC_FFMPEG
    ret = audio_file_encode_ffmpeg(inf);
#endif

    return ret;
}

#ifdef USE_LIBAVCODEC_FFMPEG
int audio_file_decode_ffmpeg(s_audinfo_t *inf)
{
    int ret = 0;
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int len;
    FILE *f, *outfile;
    u8_t inbuf[AUDIO_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
    AVPacket avpkt;
    AVFrame *decoded_frame = NULL;
    enum AVCodecID codec_id = audio_codec_bba_to_ffmpeg(inf->codec_type);
Loge("codec_id=%d, register codec...", codec_id);
    avcodec_register_all();
Loge("av init packet...");
    av_init_packet(&avpkt);

    printf("Decode audio file %s to %s\n", inf->fnamei, inf->fnameo);

    /* find the mpeg audio decoder */
    codec = avcodec_find_decoder(codec_id);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
Loge("codec found, allocating...");
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }
Loge("allocate done, codec opening...");
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }
Loge("codec open; open input file...");
    f = fopen(inf->fnamei, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", inf->fnamei);
        exit(1);
    }
Loge("open output file...");
    outfile = fopen(inf->fnameo, "wb");
    if (!outfile) {
        av_free(c);
        exit(1);
    }
Loge("read head...");
    int frame_begin = 0;
    if (inf->codec_type == CODEC_MP3)
        frame_begin = find_valid_mp3_head(f);
    fseek(f, frame_begin, SEEK_SET);
Loge("frame begin position: 0x%x", frame_begin);
    /* decode until eof */
    avpkt.data = inbuf;
    avpkt.size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);
int len_done = 0;
    while (avpkt.size > 0) {
        int got_frame = 0;

        if (!decoded_frame) {
            if (!(decoded_frame = av_frame_alloc())) {
                fprintf(stderr, "Could not allocate audio frame\n");
                exit(1);
            }
        }

        len = avcodec_decode_audio4(c, decoded_frame, &got_frame, &avpkt);
        if (len < 0) {
            fprintf(stderr, "Error while decoding, len=%d %x\n", len, len);
            Loge("avpkt.size=%d, len_done=%d", avpkt.size, len_done);
            exit(1);
        }
        if (got_frame) {
            /* if a frame has been decoded, output it */
            int data_size = av_samples_get_buffer_size(NULL, c->channels,
                                                       decoded_frame->nb_samples,
                                                       c->sample_fmt, 1);
            if (data_size < 0) {
                /* This should not occur, checking just for paranoia */
                fprintf(stderr, "Failed to calculate data size\n");
                exit(1);
            }
            fwrite(decoded_frame->data[0], 1, data_size, outfile);
        }
        len_done += len;
        avpkt.size -= len;
        avpkt.data += len;
        avpkt.dts =
        avpkt.pts = AV_NOPTS_VALUE;
        if (avpkt.size < AUDIO_REFILL_THRESH) {
            /* Refill the input buffer, to avoid trying to decode
             * incomplete frames. Instead of this, one could also use
             * a parser, or use a proper container format through
             * libavformat. */
            memmove(inbuf, avpkt.data, avpkt.size);
            avpkt.data = inbuf;
            len = fread(avpkt.data + avpkt.size, 1,
                        AUDIO_INBUF_SIZE - avpkt.size, f);
            if (len > 0)
                avpkt.size += len;
        }
    }
    Loge("Done. avpkt.size=%d len_done=%d", avpkt.size, len_done);

    fclose(outfile);
    fclose(f);

    avcodec_close(c);
    av_free(c);
    av_frame_free(&decoded_frame);

    return ret;
}

int audio_file_encode_ffmpeg(s_audinfo_t *inf)
{
    int ret = 0;
#if 0
    AVCodec *codec;
    AVCodecContext *c= NULL;
    AVFrame *frame;
    AVPacket pkt;
    int i, j, k, ret, got_output;
    int buffer_size;
    FILE *f;
    uint16_t *samples;
    float t, tincr;
    enum AVCodecID codec_id = audio_codec_bba_to_ffmpeg(inf->codec_type);

    printf("Encode audio file %s\n", inf->fnamei);
    avcodec_register_all();

    /* find the encoder */
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    /* put sample parameters */
    c->bit_rate = 64000;

    /* check that the encoder supports s16 pcm input */
    c->sample_fmt = AV_SAMPLE_FMT_S16;
    if (!check_sample_fmt(codec, c->sample_fmt)) {
        fprintf(stderr, "Encoder does not support sample format %s",
                av_get_sample_fmt_name(c->sample_fmt));
        exit(1);
    }

    /* select other audio parameters supported by the encoder */
    c->sample_rate    = select_sample_rate(codec);
    c->channel_layout = select_channel_layout(codec);
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(inf->fnamei, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", inf->fnamei);
        exit(1);
    }

    /* frame containing input raw audio */
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate audio frame\n");
        exit(1);
    }

    frame->nb_samples     = c->frame_size;
    frame->format         = c->sample_fmt;
    frame->channel_layout = c->channel_layout;

    /* the codec gives us the frame size, in samples,
     * we calculate the size of the samples buffer in bytes */
    buffer_size = av_samples_get_buffer_size(NULL, c->channels, c->frame_size,
                                             c->sample_fmt, 0);
    if (buffer_size < 0) {
        fprintf(stderr, "Could not get sample buffer size\n");
        exit(1);
    }
    samples = av_malloc(buffer_size);
    if (!samples) {
        fprintf(stderr, "Could not allocate %d bytes for samples buffer\n",
                buffer_size);
        exit(1);
    }
    /* setup the data pointers in the AVFrame */
    ret = avcodec_fill_audio_frame(frame, c->channels, c->sample_fmt,
                                   (const uint8_t*)samples, buffer_size, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not setup audio frame\n");
        exit(1);
    }

    /* encode a single tone sound */
    t = 0;
    tincr = 2 * M_PI * 440.0 / c->sample_rate;
    for (i = 0; i < 200; i++) {
        av_init_packet(&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;

        for (j = 0; j < c->frame_size; j++) {
            samples[2*j] = (int)(sin(t) * 10000);

            for (k = 1; k < c->channels; k++)
                samples[2*j + k] = samples[2*j];
            t += tincr;
        }
        /* encode the samples */
        ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding audio frame\n");
            exit(1);
        }
        if (got_output) {
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }

    /* get the delayed frames */
    for (got_output = 1; got_output; i++) {
        ret = avcodec_encode_audio2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if (got_output) {
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }
    fclose(f);

    av_freep(&samples);
    av_frame_free(&frame);
    avcodec_close(c);
    av_free(c);

#endif
    return ret;
}

enum AVCodecID audio_codec_bba_to_ffmpeg(int codec)
{
    enum AVCodecID ret = AV_CODEC_ID_FIRST_AUDIO;
    switch (codec) {
    case CODEC_PCM:
        ret = AV_CODEC_ID_PCM_S16LE;
        break;
    case CODEC_MP3:
        ret = AV_CODEC_ID_MP3;
        break;
    case CODEC_AAC:
        ret = AV_CODEC_ID_AAC;
        break;
    case CODEC_AMR:
        ret = AV_CODEC_ID_AMR_NB;
        break;
    case CODEC_AWB:
        ret = AV_CODEC_ID_AMR_WB;
        break;
    }
    return ret;
}
#endif

int find_valid_mp3_head(FILE *fp){
    int ret = -1;
    if (fp == NULL) {
        Loge("empty file! return");
        return ret;
    }
    int orig_posi = ftell(fp);
    int flen = 0;

    fseek(fp, 0, SEEK_END);
    flen = ftell(fp);
    rewind(fp);
    Loge("original position 0x%x, file length: 0x%x", orig_posi, flen);
    
    int fposi = 0;
    char buf[10] = {0};
    /*
    while (fposi < flen - 3) {
        fread(&value, sizeof(value), 1, fp);
        if (id3_is_valid(value))
            
        if (head_is_valid(value))
            return fposi;
        else
            fseek(fp, -3L, SEEK_CUR);
        fposi++;
    }*/
    fread(buf, sizeof(buf), 1, fp);
    fposi += sizeof(buf);
    if ((buf[0] == 0x49) && (buf[1] == 0x44) && (buf[2] == 0x33)) {   //"ID3"
        fposi += (buf[7] << 14) + (buf[8] << 7) + buf[9];
        Loge("position is %x", fposi);
        return fposi;
    }

    fseek(fp, orig_posi, SEEK_SET);
    return ret;
}

int head_is_valid(int value) {
    int ret = 0;
    if ((((value >> 20) & 0xFFF) == 0xFFF) &&
        (((value >> 17) & 0x7) == 0x7)) {
        Loge("value is 0x%x", value);
        ret = 1;
    }
    return ret;
}

