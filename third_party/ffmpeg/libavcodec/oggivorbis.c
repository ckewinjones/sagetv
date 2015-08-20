/*
 * copyright (c) 2002 Mark Hills <mark@pogo.org.uk>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file oggvorbis.c
 * Ogg Vorbis codec support via libvorbisenc.
 * @author Mark Hills <mark@pogo.org.uk>
 */

#include <ivorbiscodec.h>

#include "avcodec.h"

#undef NDEBUG
#include <assert.h>

#define OGGVORBIS_FRAME_SIZE 64

#define BUFFER_SIZE (1024*64)

typedef struct OggVorbisContext {
    vorbis_info vi ;
    vorbis_dsp_state vd ;
    vorbis_block vb ;
    uint8_t buffer[BUFFER_SIZE];
    int buffer_index;

    /* decoder */
    vorbis_comment vc ;
    ogg_packet op;
} OggVorbisContext ;


static int oggvorbis_decode_init(AVCodecContext *avccontext) {
    OggVorbisContext *context = avccontext->priv_data ;
    uint8_t *p= avccontext->extradata;
    int i, hsizes[3];
    unsigned char *headers[3], *extradata = avccontext->extradata;

    vorbis_info_init(&context->vi) ;
    vorbis_comment_init(&context->vc) ;

    if(! avccontext->extradata_size || ! p) {
        av_log(avccontext, AV_LOG_ERROR, "vorbis extradata absent\n");
        return -1;
    }

    if(p[0] == 0 && p[1] == 30) {
        for(i = 0; i < 3; i++){
            hsizes[i] = *p++ << 8;
            hsizes[i] += *p++;
            headers[i] = p;
            p += hsizes[i];
        }
    } else if(*p == 2) {
        unsigned int offset = 1;
        p++;
        for(i=0; i<2; i++) {
            hsizes[i] = 0;
            while((*p == 0xFF) && (offset < avccontext->extradata_size)) {
                hsizes[i] += 0xFF;
                offset++;
                p++;
            }
            if(offset >= avccontext->extradata_size - 1) {
                av_log(avccontext, AV_LOG_ERROR,
                       "vorbis header sizes damaged\n");
                return -1;
            }
            hsizes[i] += *p;
            offset++;
            p++;
        }
        hsizes[2] = avccontext->extradata_size - hsizes[0]-hsizes[1]-offset;
#if 0
        av_log(avccontext, AV_LOG_DEBUG,
               "vorbis header sizes: %d, %d, %d, / extradata_len is %d \n",
               hsizes[0], hsizes[1], hsizes[2], avccontext->extradata_size);
#endif
        headers[0] = extradata + offset;
        headers[1] = extradata + offset + hsizes[0];
        headers[2] = extradata + offset + hsizes[0] + hsizes[1];
    } else {
        av_log(avccontext, AV_LOG_ERROR,
               "vorbis initial header len is wrong: %d\n", *p);
        return -1;
    }

    for(i=0; i<3; i++){
        context->op.b_o_s= i==0;
        context->op.bytes = hsizes[i];
        context->op.packet = headers[i];
        if(vorbis_synthesis_headerin(&context->vi, &context->vc, &context->op)<0){
            av_log(avccontext, AV_LOG_ERROR, "%d. vorbis header damaged\n", i+1);
            return -1;
        }
    }

    avccontext->channels = context->vi.channels;
    avccontext->sample_rate = context->vi.rate;
    avccontext->time_base= (AVRational){1, avccontext->sample_rate};

    vorbis_synthesis_init(&context->vd, &context->vi);
    vorbis_block_init(&context->vd, &context->vb);

    return 0 ;
}


static inline int conv(int samples, ogg_int32_t **pcm, char *buf, int channels) {
    int i, j, val ;
    ogg_int16_t *ptr, *data = (ogg_int16_t*)buf ;
    ogg_int32_t *mono ;

    for(i = 0 ; i < channels ; i++){
        ptr = &data[i];
        mono = pcm[i] ;

        for(j = 0 ; j < samples ; j++) {

            val = mono[j]>>9; // * 32767.f;

            if(val > 32767) val = 32767 ;
            if(val < -32768) val = -32768 ;

            *ptr = val ;
            ptr += channels;
        }
    }

    return 0 ;
}


static int oggvorbis_decode_frame(AVCodecContext *avccontext,
                        void *data, int *data_size,
                        uint8_t *buf, int buf_size)
{
    OggVorbisContext *context = avccontext->priv_data ;
    ogg_int32_t **pcm ;
    ogg_packet *op= &context->op;
    int samples, total_samples, total_bytes;

    if(!buf_size){
    //FIXME flush
        return 0;
    }

    op->packet = buf;
    op->bytes  = buf_size;

//    av_log(avccontext, AV_LOG_DEBUG, "%d %d %d %"PRId64" %"PRId64" %d %d\n", op->bytes, op->b_o_s, op->e_o_s, op->granulepos, op->packetno, buf_size, context->vi.rate);

/*    for(i=0; i<op->bytes; i++)
      av_log(avccontext, AV_LOG_DEBUG, "%02X ", op->packet[i]);
    av_log(avccontext, AV_LOG_DEBUG, "\n");*/

    if(vorbis_synthesis(&context->vb, op) == 0)
        vorbis_synthesis_blockin(&context->vd, &context->vb) ;

    total_samples = 0 ;
    total_bytes = 0 ;

    while((samples = vorbis_synthesis_pcmout(&context->vd, &pcm)) > 0) {
        conv(samples, pcm, (char*)data + total_bytes, context->vi.channels) ;
        total_bytes += samples * 2 * context->vi.channels ;
        total_samples += samples ;
        vorbis_synthesis_read(&context->vd, samples) ;
    }

    *data_size = total_bytes ;
    return buf_size ;
}


static int oggvorbis_decode_close(AVCodecContext *avccontext) {
    OggVorbisContext *context = avccontext->priv_data ;

    vorbis_info_clear(&context->vi) ;
    vorbis_comment_clear(&context->vc) ;

    return 0 ;
}


AVCodec oggivorbis_decoder = {
    "vorbis",
    CODEC_TYPE_AUDIO,
    CODEC_ID_VORBIS,
    sizeof(OggVorbisContext),
    oggvorbis_decode_init,
    NULL,
    oggvorbis_decode_close,
    oggvorbis_decode_frame,
    .capabilities= CODEC_CAP_DELAY,
} ;
