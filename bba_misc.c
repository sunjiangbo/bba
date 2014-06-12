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
    int ret = 0;
    int idx = 0;
    char *ext = NULL;

    if (fname == NULL || length > FILENAME_SIZE - 1)
        return -1;
    idx = length - 5;
    ext = fname + idx;
    do {
        idx++;
        if (idx > length)
            return -1;
    } while (*(ext + idx) != '.');
    ext = ext + idx + 1;
    if (!strcmp(ext, FILESTR_RAW))
        ret = FILE_RAW;
    else if (!strcmp(ext, FILESTR_PCM))
        ret = FILE_PCM;
    else if (!strcmp(ext, FILESTR_WAV))
        ret = FILE_WAV;
    else if (!strcmp(ext, FILESTR_MP3))
        ret = FILE_MP3;
    else if (!strcmp(ext, FILESTR_AAC))
        ret = FILE_AAC;
    else if (!strcmp(ext, FILESTR_AMR))
        ret = FILE_AMR;
    else if (!strcmp(ext, FILESTR_AWB))
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
    ret = audio_file_decode_ffmpeg(inf)
#endif

    return ret;
}

int audio_file_encode(s_audinfo_t *inf)
{
    int ret = 0;

#ifdef USE_LIBAVCODEC_FFMPEG
    ret = audio_file_encode_ffmpeg(inf)
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

    av_init_packet(&avpkt);

    printf("Decode audio file %s to %s\n", inf->fnamei, inf->fnameo);

    /* find the mpeg audio decoder */
    codec = avcodec_find_decoder(codec_id);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(inf->fnamei, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", inf->fnamei);
        exit(1);
    }
    outfile = fopen(inf->fnameo, "wb");
    if (!outfile) {
        av_free(c);
        exit(1);
    }

    /* decode until eof */
    avpkt.data = inbuf;
    avpkt.size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);

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
            fprintf(stderr, "Error while decoding\n");
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

    return ret;
}

enum AVCodecID audio_codec_bba_to_ffmpeg(int codec)
{
    enum AVCodecID ret = AV_CODEC_ID_FIRST_AUDIO;
    switch (codec) {
    case CODEC_PCM:
        ret = AV_CODEC_ID_PCM_S16LE;
    case CODEC_MP3:
        ret = AV_CODEC_ID_MP3;
    case CODEC_AAC:
        ret = AV_CODEC_ID_AAC;
    case CODEC_AMR:
        ret = AV_CODEC_ID_AMR_NB;
    case CODEC_AWB:
        ret = AV_CODEC_ID_AMR_WB;
    }
    return ret;
}
#endif

