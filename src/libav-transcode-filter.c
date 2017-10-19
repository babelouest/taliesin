/**
 *
 * Taliesin - Media server
 * 
 * Stream functions definitions
 *
 * Copyright 2017 Nicolas Mora <mail@babelouest.org>
 * Copyright (c) 2013-2017 Andreas Unterweger
 *
 * This file contains parts of Libav.
 *
 * Taliesin is free software; you can redistribute it and/or
 * modify it under the terms of the GNU GENERAL PUBLIC LICENSE
 * License as published by the Free Software Foundation;
 * version 3 of the License.
 *
 * Taliesin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/avstring.h>
#include <libavutil/frame.h>
#include <libavutil/opt.h>
#include <libavresample/avresample.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include "taliesin.h"

static char * get_error_text(const int error) {
  static char error_buffer[255];
  av_strerror(error, error_buffer, sizeof(error_buffer));
  return error_buffer;
}

int open_input_file(const char *filename, AVFormatContext **input_format_context, AVCodecContext **input_codec_context, int type) {
  AVCodecContext *avctx;
  AVCodec *input_codec;
  int error, codec_index;
  
  /* Open the input file to read from it. */
  if ((error = avformat_open_input(input_format_context, filename, NULL, NULL)) < 0) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not open input file '%s' (error '%s')", filename, get_error_text(error));
    *input_format_context = NULL;
    return error;
  }
  /* Get information on the input file (number of streams etc.). */
  if ((error = avformat_find_stream_info(*input_format_context, NULL)) < 0) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not open find stream info (error '%s')", get_error_text(error));
    avformat_close_input(input_format_context);
    return error;
  }
  /* Make sure that there is only one stream in the input file. */
  if ((*input_format_context)->nb_streams != 1) {
    codec_index = av_find_best_stream((*input_format_context), type, -1, -1, &input_codec, 0);
  } else {
    codec_index = 0;
    input_codec = avcodec_find_decoder((*input_format_context)->streams[codec_index]->codecpar->codec_id);
  }
  /* Find a decoder for the audio stream. */
  if (input_codec == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not find input codec");
    avformat_close_input(input_format_context);
    return AVERROR_EXIT;
  }
  /* Allocate a new decoding context. */
  avctx = avcodec_alloc_context3(input_codec);
  if (!avctx) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate a decoding context");
    avformat_close_input(input_format_context);
    return AVERROR(ENOMEM);
  }
  /* Initialize the stream parameters with demuxer information. */
  error = avcodec_parameters_to_context(avctx, (*input_format_context)->streams[codec_index]->codecpar);
  if (error < 0) {
    avformat_close_input(input_format_context);
    avcodec_free_context(&avctx);
    return error;
  }
  /* Open the decoder for the audio stream to use it later. */
  if ((error = avcodec_open2(avctx, input_codec, NULL)) < 0) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not open input codec (error '%s')", get_error_text(error));
    avcodec_free_context(&avctx);
    avformat_close_input(input_format_context);
    return error;
  }
  /* Save the decoder context for easier access later. */
  *input_codec_context = avctx;
  return 0;
}

void init_packet(AVPacket *packet) {
  av_init_packet(packet);
  /* Set the packet data and size so that it is recognized as being empty. */
  packet->data = NULL;
  packet->size = 0;
}

int init_input_frame(AVFrame **frame) {
  if (!(*frame = av_frame_alloc())) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate input frame");
    return AVERROR(ENOMEM);
  }
  return 0;
}

int init_resampler(AVCodecContext *input_codec_context,
                   AVCodecContext *output_codec_context,
                   AVAudioResampleContext **resample_context) {
  int error;
  /* Create a resampler context for the conversion. */
  if (!(*resample_context = avresample_alloc_context())) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate resample context");
    return AVERROR(ENOMEM);
  }
  av_opt_set_int(*resample_context, "in_channel_layout",
                 av_get_default_channel_layout(input_codec_context->channels), 0);
  av_opt_set_int(*resample_context, "out_channel_layout",
                 av_get_default_channel_layout(output_codec_context->channels), 0);
  av_opt_set_int(*resample_context, "in_sample_rate",
                 input_codec_context->sample_rate, 0);
  av_opt_set_int(*resample_context, "out_sample_rate",
                 output_codec_context->sample_rate, 0);
  av_opt_set_int(*resample_context, "in_sample_fmt",
                 input_codec_context->sample_fmt, 0);
  av_opt_set_int(*resample_context, "out_sample_fmt",
                 output_codec_context->sample_fmt, 0);
  /* Open the resampler with the specified parameters. */
  if ((error = avresample_open(*resample_context)) < 0) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not open resample context");
    avresample_free(resample_context);
    return error;
  }
  return 0;
}

int init_fifo(AVAudioFifo **fifo, AVCodecContext *output_codec_context) {
  /* Create the FIFO buffer based on the specified output sample format. */
  if (!(*fifo = av_audio_fifo_alloc(output_codec_context->sample_fmt, output_codec_context->channels, 1))) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate FIFO");
    return AVERROR(ENOMEM);
  }
  return 0;
}

int my_decode(AVCodecContext *input_codec_context, AVFrame *frame, int *data_present, AVPacket *input_packet) {
  int ret = avcodec_send_packet(input_codec_context, input_packet);
  if (ret < 0) {
    return ret;
  }
  *data_present = 0;
  do {
    ret = avcodec_receive_frame(input_codec_context, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      break;
    } else if (ret < 0) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error during decode (%s)", get_error_text(ret));
      break;
    } else {
      *data_present += frame->nb_samples > 0;
    }
  } while (ret > 0);
  return 0;
}

int decode_audio_frame(AVFrame *frame,
                       AVFormatContext *input_format_context,
                       AVCodecContext *input_codec_context,
                       int *data_present, int *finished) {
  /* Packet used for temporary storage. */
  AVPacket input_packet;
  int error;
  init_packet(&input_packet);
  /* Read one audio frame from the input file into a temporary packet. */
  if ((error = av_read_frame(input_format_context, &input_packet)) < 0) {
    /* If we are the the end of the file, flush the decoder below. */
    if (error == AVERROR_EOF) {
      *finished = 1;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Could not read frame (error '%s')",
              get_error_text(error));
      *data_present = 0;
      return 0;
    }
  }
   /*if ((error = avcodec_decode_audio4(input_codec_context, frame, data_present, &input_packet)) < 0 && error != AVERROR_INVALIDDATA) {
     y_log_message(Y_LOG_LEVEL_ERROR, "Could not decode frame (error '%s')", get_error_text(error));
     av_packet_unref(&input_packet);
     return error;*/
  if ((error = my_decode(input_codec_context, frame, data_present, &input_packet)) < 0) {
    if (error == AVERROR_EOF) {
      *finished = 1;
      *data_present = 0;
      av_packet_unref(&input_packet);
      return error;
    } if (error == AVERROR_INVALIDDATA) {
      *data_present = 0;
      av_packet_unref(&input_packet);
      return error;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Could not decode frame (error '%s')", get_error_text(error));
      *data_present = 0;
      av_packet_unref(&input_packet);
      return error;
    }
  }
  /* If the decoder has not been flushed completely, we are not finished,
   * so that this function has to be called again. */
  if (*finished && *data_present) {
    *finished = 0;
  }
  av_packet_unref(&input_packet);
  return error;
}

int read_decode_convert_and_store(AVAudioFifo *fifo,
                                  AVFormatContext *input_format_context,
                                  AVCodecContext *input_codec_context,
                                  AVCodecContext *output_codec_context,
                                  AVAudioResampleContext *resample_context,
                                  int *finished) {
  AVFrame *input_frame = NULL;
  uint8_t ** converted_input_samples = NULL;
  int data_present, out_linesize, out_samples;
  int ret = AVERROR_EXIT;
  
  if (!init_input_frame(&input_frame)) {
    if (!decode_audio_frame(input_frame, input_format_context, input_codec_context, &data_present, finished)) {
      if (*finished && !data_present) {
        ret = 0;
      } else {
        /* If there is decoded data, convert and store it. */
        if (data_present) {
          out_samples = avresample_available(resample_context) + av_rescale_rnd(avresample_get_delay(resample_context) + input_frame->nb_samples, input_codec_context->sample_rate, output_codec_context->sample_rate, AV_ROUND_UP);
          if (!(converted_input_samples = calloc(output_codec_context->channels, sizeof(uint8_t *)))) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate converted input sample pointers");
            ret = AVERROR(ENOMEM);
          } else {
            av_samples_alloc(converted_input_samples, &out_linesize, 2, out_samples, output_codec_context->sample_fmt, 0);
            if (*converted_input_samples != NULL) {
              out_samples = avresample_convert(resample_context, converted_input_samples, out_linesize, out_samples, input_frame->extended_data, *input_frame->linesize, input_frame->nb_samples);
              if (out_samples) {
                if ((ret = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + out_samples)) < 0) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Could not reallocate FIFO");
                } else  if (av_audio_fifo_write(fifo, (void **)converted_input_samples, out_samples) < out_samples) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Could not write data to FIFO");
                } else {
                  ret = 0;
                }
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate converted_input_samples");
            }
            av_freep(converted_input_samples);
            free(converted_input_samples);
          }
        }
      }
    }
  }
  av_frame_free(&input_frame);
  return ret;
}

int init_output_frame(AVFrame **frame,
                      AVCodecContext *output_codec_context,
                      int frame_size) {
    int error;
    /* Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc())) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate output frame");
      return AVERROR_EXIT;
    }
    /* Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity. */
    (*frame)->nb_samples     = frame_size;
    (*frame)->channel_layout = output_codec_context->channel_layout;
    (*frame)->format         = output_codec_context->sample_fmt;
    (*frame)->sample_rate    = output_codec_context->sample_rate;
    /* Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified. */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate output frame samples (error '%s')", get_error_text(error));
      av_frame_free(frame);
      return error;
    }
    return 0;
}

int my_encode(AVCodecContext *output_codec_context, AVPacket *output_packet, AVFrame *frame, int * data_present) {
  int ret;

  //y_log_message(Y_LOG_LEVEL_DEBUG, "avcodec_send_frame for %p %p", output_codec_context, frame->data);
  ret = avcodec_send_frame(output_codec_context, frame);
  if (ret < 0) {
    return ret;
  }
  *data_present = 0;
  do {
    ret = avcodec_receive_packet(output_codec_context, output_packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      break;
    } else if (ret < 0) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error during encode: '%s'", get_error_text(ret));
      return ret;
    } else {
      *data_present = 1;
    }
  } while (ret > 0);
  return 0;
}

int encode_audio_frame_and_return(AVFrame * frame,
                                  AVCodecContext * output_codec_context,
                                  AVFormatContext * output_format_context,
                                  int64_t * pts,
                                  int * data_present) {
  int error;
  AVPacket output_packet;
  init_packet(&output_packet);
  
  //y_log_message(Y_LOG_LEVEL_DEBUG, "inside encode_audio_frame_and_return");
  if (frame) {
    frame->pts = *pts;
    *pts += frame->nb_samples;
  }
  /*if ((error = avcodec_encode_audio2(output_codec_context, &output_packet, frame, data_present)) < 0) {
   y_log_message(Y_LOG_LEVEL_ERROR, "Could not encode frame (error '%s')", get_error_text(error));
   av_packet_unref(&output_packet);
   return error;*/
  if ((error = my_encode(output_codec_context, &output_packet, frame, data_present)) < 0) {
    if (error == AVERROR_EOF) {
      *data_present = 0;
      av_packet_unref(&output_packet);
      return error;
    } if (error == AVERROR_INVALIDDATA) {
      *data_present = 0;
      av_packet_unref(&output_packet);
      return error;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Could not encode frame (error '%s')", get_error_text(error));
      *data_present = 0;
      av_packet_unref(&output_packet);
      return error;
    }
  }
  /* Write one audio frame from the temporary packet to the output file. */
  if (*data_present) {
    if ((error = av_write_frame(output_format_context, &output_packet)) < 0) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Could not write frame (error '%s')", get_error_text(error));
    }
  }
  av_packet_unref(&output_packet);
  return error;
}

int load_encode_and_return(AVAudioFifo * fifo,
                           AVCodecContext * output_codec_context,
                           AVFormatContext * output_format_context,
                           int64_t * pts,
                           int * data_present) {
  AVFrame *output_frame;
  const int frame_size = FFMIN(av_audio_fifo_size(fifo), output_codec_context->frame_size);
  if (init_output_frame(&output_frame, output_codec_context, frame_size)) {
    return AVERROR_EXIT;
  }
  if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not read data from FIFO");
    av_frame_free(&output_frame);
    return AVERROR_EXIT;
  }
  if (encode_audio_frame_and_return(output_frame, output_codec_context, output_format_context, pts, data_present) < 0) {
    av_frame_free(&output_frame);
    return AVERROR_EXIT;
  }
  av_frame_free(&output_frame);
  return 0;
}

static int write_packet_webradio(void *opaque, uint8_t *buf, int buf_size) {
  struct _audio_stream * audio_stream = (struct _audio_stream *)opaque;

  if (audio_stream_add_data(audio_stream, buf, buf_size)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error adding data to audio_stream");
  }
  return buf_size;
}

int webradio_open_output_buffer(struct _audio_stream * audio_stream) {
  AVCodecContext * avctx          = NULL;
  AVCodec * output_codec          = NULL;
  AVAudioFifo * fifo              = NULL;
  AVIOContext * output_io_context = NULL;
  AVStream * stream               = NULL;
  int error = 0;
  int codec_id;
  uint8_t * avio_ctx_buffer       = NULL;
  size_t avio_ctx_buffer_size     = 4096;
  char format[8]                  = {0};
  
  if (0 == o_strcasecmp("vorbis", audio_stream->stream_format)) {
    codec_id = AV_CODEC_ID_VORBIS;
    o_strcpy(format, "ogg");
  } else if (0 == o_strcasecmp("flac", audio_stream->stream_format)) {
    codec_id = AV_CODEC_ID_FLAC;
    o_strcpy(format, "flac");
  } else {
    codec_id = AV_CODEC_ID_MP3;
    o_strcpy(format, "mp3");
  }
  
  if ((audio_stream->output_format_context = avformat_alloc_context()) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate output format context");
    error = AVERROR(ENOMEM);
  } else if ((avio_ctx_buffer = av_malloc(avio_ctx_buffer_size)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error malloc avio_ctx_buffer");
    error = AVERROR(ENOMEM);
  } else if ((output_codec = avcodec_find_encoder(codec_id)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not find encoder.");
  } else if ((avctx = avcodec_alloc_context3(output_codec)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate an encoding context");
    error = AVERROR(ENOMEM);
  } else if ((output_io_context = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 1, audio_stream, NULL, &write_packet_webradio, NULL)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error avio_alloc_context");
    error = AVERROR(ENOMEM);
  } else {
    if (!(stream = avformat_new_stream(audio_stream->output_format_context, NULL))) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Could not create new stream");
      error = AVERROR(ENOMEM);
    } else {
      audio_stream->output_format_context->pb = output_io_context;
      if ((audio_stream->output_format_context->oformat = av_guess_format(format, NULL, NULL)) == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Could not find output format '%s'", format);
        error = AVERROR(ENOMEM);
      } else {
        avctx->channels       = audio_stream->stream_channels;
        avctx->channel_layout = av_get_default_channel_layout(audio_stream->stream_channels);
        avctx->sample_rate    = audio_stream->stream_sample_rate;
        avctx->sample_fmt     = output_codec->sample_fmts[0];
        if (0 != o_strcasecmp("flac", audio_stream->stream_format)) {
          avctx->bit_rate     = audio_stream->stream_bitrate;
        }
        avctx->strict_std_compliance = FF_COMPLIANCE_NORMAL;
        stream->time_base.den = audio_stream->stream_sample_rate;
        stream->time_base.num = 1;

        if ((error = avcodec_open2(avctx, output_codec, NULL)) < 0) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Could not open output codec (error '%s')", get_error_text(error));
        } else if ((audio_stream->fifo = av_audio_fifo_alloc(avctx->sample_fmt, avctx->channels, 1)) == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Could not open fifo (error '%s')", get_error_text(error));
        } else {
          if ((avcodec_parameters_from_context(stream->codecpar, avctx)) < 0) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Could not initialize stream parameters");
          } else {
            audio_stream->is_header = 1;
            if ((error = avformat_write_header(audio_stream->output_format_context, NULL)) < 0) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Error write header frame (error '%s')", get_error_text(error));
            } else {
              error = 0;
              audio_stream->header_buffer->max_size = audio_stream->header_buffer->size;
            }
            audio_stream->header_buffer->complete = 1;
            audio_stream->is_header = 0;
          }
        }
      }
    }
  }
  
  if (error) {
    av_audio_fifo_free(fifo);
    if (audio_stream->output_format_context != NULL) {
      avio_close(audio_stream->output_format_context->pb);
    }
    avformat_free_context(audio_stream->output_format_context);
    avcodec_free_context(&avctx);
  }
  audio_stream->output_codec_context = avctx;
  return error;
}

struct _decoded_image {
  size_t context_size;
  size_t size;
  unsigned char * buffer;
};

static int read_image_packet(void * opaque, uint8_t * buf, int buf_size) {
  struct _decoded_image * image = (struct _decoded_image *)opaque;
  
  buf_size = FFMIN(buf_size, (image->size-image->context_size));
  if (buf_size)
  memcpy(buf, (image->buffer + image->context_size), buf_size);
  image->context_size += buf_size;
  
  return buf_size;
}

int open_input_buffer(const unsigned char * base64_buffer, AVFormatContext **image_format_context, AVCodecContext **image_codec_context, int * codec_index, int type) {
  AVIOContext    *          input_io_context = NULL;
  AVCodecContext *                     avctx = NULL;
  AVCodec        *               input_codec = NULL;
  uint8_t        *           avio_ctx_buffer = NULL;
  size_t                avio_ctx_buffer_size = 4096;
  struct _decoded_image image;
  int                   error, my_codec_index;
  int ret = T_OK;

  image.context_size = 0;
  if ((image.buffer = o_malloc(strlen((const char *)base64_buffer))) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "get_image_from_buffer - Error malloc image.buffer");
    ret = T_ERROR_MEMORY;
  } else if (!o_base64_decode(base64_buffer, strlen((const char *)base64_buffer), image.buffer, &image.size)) {
    ret = T_ERROR;
  } else if ((avio_ctx_buffer = av_malloc(avio_ctx_buffer_size)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "get_image_from_buffer - Error malloc avio_ctx_buffer");
    ret = T_ERROR_MEMORY;
  } else if ((input_io_context = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &image, &read_image_packet, NULL, NULL)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "get_image_from_buffer - Error avio_alloc_context");
    ret = T_ERROR_MEMORY;
  } else if (((*image_format_context) = avformat_alloc_context()) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "get_image_from_buffer - Error avio_alloc_context");
    ret = T_ERROR_MEMORY;
  } else {
    // Set pb to the input_io_context that will read the buffer from memory instead of a file
    (*image_format_context)->pb = input_io_context;
    
    /* Open the input file to read from it. */
    if ((error = avformat_open_input(image_format_context, NULL, NULL, NULL)) < 0) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Could not open format (error '%s')", get_error_text(error));
      ret = T_ERROR;
    } else if ((error = avformat_find_stream_info(*image_format_context, NULL)) < 0) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Could not open find stream info (error '%s')", get_error_text(error));
      ret = T_ERROR;
    } else {
      if ((*image_format_context)->nb_streams != 1) {
        my_codec_index = av_find_best_stream((*image_format_context), type, -1, -1, &input_codec, 0);
      } else {
        my_codec_index = 0;
        input_codec = avcodec_find_decoder((*image_format_context)->streams[my_codec_index]->codecpar->codec_id);
      }
      
      /* Find a decoder for the audio stream. */
      if (input_codec == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Could not find input codec");
        ret = T_ERROR;
      } else if ((avctx = avcodec_alloc_context3(input_codec)) == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate a decoding context");
        ret = T_ERROR;
      } else if ((avcodec_parameters_to_context(avctx, (*image_format_context)->streams[my_codec_index]->codecpar)) < 0) {
        avcodec_free_context(&avctx);
        ret = T_ERROR;
      } else if ((error = avcodec_open2(avctx, input_codec, NULL)) < 0) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Could not open input codec (error '%s')", get_error_text(error));
        avcodec_free_context(&avctx);
        ret = T_ERROR;
      }
      if (codec_index != NULL) {
        *codec_index = my_codec_index;
      }
      /* Save the decoder context for easier access later. */
      *image_codec_context = avctx;
      
    }
  }
  o_free(image.buffer);
  if (ret != T_OK) {
    if (*image_codec_context) {
      avcodec_flush_buffers(*image_codec_context);
      avcodec_close(*image_codec_context);
      avcodec_free_context(image_codec_context);
      *image_codec_context = NULL;
    }
    if (*image_format_context) {
      avio_flush((*image_format_context)->pb);
      av_free((*image_format_context)->pb->buffer);
      (*image_format_context)->pb->buffer = NULL;
      av_free((*image_format_context)->pb);
      (*image_format_context)->pb = NULL;
      avformat_close_input(image_format_context);
      *image_format_context = NULL;
    }
  }
  return ret;
}

static int write_packet_playlist(void * opaque, uint8_t * buf, int buf_size) {
  struct _jukebox_audio_buffer * jukebox_audio_buffer = (struct _jukebox_audio_buffer *)opaque;
  
  if (jukebox_audio_buffer_add_data(jukebox_audio_buffer, buf, buf_size)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error adding data to jukebox_audio_buffer");
  }
  return buf_size;
}

int open_output_buffer_playlist(struct _jukebox_audio_buffer * jukebox_audio_buffer, AVFormatContext ** output_format_context, AVCodecContext ** output_codec_context, AVAudioFifo ** fifo) {
  AVCodecContext * avctx          = NULL;
  AVCodec * output_codec          = NULL;
  AVIOContext * output_io_context = NULL;
  AVStream * stream               = NULL;
  int error = 0;
  int codec_id;
  uint8_t * avio_ctx_buffer       = NULL;
  size_t avio_ctx_buffer_size     = 4096;
  char format[8]                  = {0};
  
  if (0 == o_strcasecmp("vorbis", jukebox_audio_buffer->jukebox->stream_format)) {
    codec_id = AV_CODEC_ID_VORBIS;
    o_strcpy(format, "ogg");
  } else if (0 == o_strcasecmp("flac", jukebox_audio_buffer->jukebox->stream_format)) {
    codec_id = AV_CODEC_ID_FLAC;
    o_strcpy(format, "flac");
  } else {
    codec_id = AV_CODEC_ID_MP3;
    o_strcpy(format, "mp3");
  }
  
  if (((*output_format_context) = avformat_alloc_context()) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate output format context");
    error = AVERROR(ENOMEM);
  } else if ((avio_ctx_buffer = av_malloc(avio_ctx_buffer_size)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error malloc avio_ctx_buffer");
    error = AVERROR(ENOMEM);
  } else if ((output_codec = avcodec_find_encoder(codec_id)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not find encoder.");
  } else if ((avctx = avcodec_alloc_context3(output_codec)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Could not allocate an encoding context");
    error = AVERROR(ENOMEM);
  } else if ((output_io_context = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 1, jukebox_audio_buffer, NULL, &write_packet_playlist, NULL)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error avio_alloc_context");
    error = AVERROR(ENOMEM);
  } else {
    if (!(stream = avformat_new_stream((*output_format_context), NULL))) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Could not create new stream");
      error = AVERROR(ENOMEM);
    } else {
      (*output_format_context)->pb = output_io_context;
      if (((*output_format_context)->oformat = av_guess_format(format, NULL, NULL)) == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Could not find output format '%s'", format);
        error = AVERROR(ENOMEM);
      } else {
        //y_log_message(Y_LOG_LEVEL_DEBUG, "stream parameters are format: %s channels: %d sample_rate: %d bitrate: %d", jukebox_audio_buffer->jukebox->stream_format, jukebox_audio_buffer->jukebox->stream_channels, jukebox_audio_buffer->jukebox->stream_sample_rate, jukebox_audio_buffer->jukebox->stream_bitrate);
        avctx->channels       = jukebox_audio_buffer->jukebox->stream_channels;
        avctx->channel_layout = av_get_default_channel_layout(jukebox_audio_buffer->jukebox->stream_channels);
        avctx->sample_rate    = jukebox_audio_buffer->jukebox->stream_sample_rate;
        avctx->sample_fmt     = output_codec->sample_fmts[0];
        if (0 != o_strcasecmp("flac", jukebox_audio_buffer->jukebox->stream_format)) {
          avctx->bit_rate     = jukebox_audio_buffer->jukebox->stream_bitrate;
        }
        avctx->strict_std_compliance = FF_COMPLIANCE_NORMAL;
        stream->time_base.den = jukebox_audio_buffer->jukebox->stream_sample_rate;
        stream->time_base.num = 1;

        if ((error = avcodec_open2(avctx, output_codec, NULL)) < 0) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Could not open output codec (error '%s')", get_error_text(error));
        } else if ((*fifo = av_audio_fifo_alloc(avctx->sample_fmt, avctx->channels, 1)) == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Could not open fifo (error '%s')", get_error_text(error));
        } else {
          if ((avcodec_parameters_from_context(stream->codecpar, avctx)) < 0) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Could not initialize stream parameters");
          } else {
            if ((error = avformat_write_header((*output_format_context), NULL)) < 0) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Error avformat_write_header %s", get_error_text(error));
            } else {
              error = 0;
            }
          }
        }
      }
    }
  }
  
  if (error) {
    av_audio_fifo_free(*fifo);
    if (*output_format_context != NULL) {
      avio_close((*output_format_context)->pb);
    }
    avformat_free_context(*output_format_context);
    avcodec_free_context(&avctx);
  }
  *output_codec_context = avctx;
  return error;
}

int init_output_jpeg_image(AVCodecContext ** image_codec_context, int dst_width, int dst_height) {
  AVCodec * output_codec = NULL;
  int ret = T_ERROR;
  char * err;
  
  if ((output_codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "init_output_jpeg_image - error avcodec_find_encoder");
  } else if (((*image_codec_context) = avcodec_alloc_context3(output_codec)) == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "init_output_jpeg_image - error avcodec_alloc_context3");
  } else {
    (*image_codec_context)->bit_rate = 0;
    (*image_codec_context)->width = dst_width;
    (*image_codec_context)->height = dst_height;
    //(*image_codec_context)->pkt_timebase.num = 0;
    //(*image_codec_context)->pkt_timebase.den = 0;
    (*image_codec_context)->time_base.num = 1;
    (*image_codec_context)->time_base.den = 30;
    (*image_codec_context)->pix_fmt = AV_PIX_FMT_YUVJ420P;
    if ((ret = avcodec_open2((*image_codec_context), output_codec, NULL)) < 0) {
      err = get_error_text(ret);
      y_log_message(Y_LOG_LEVEL_ERROR, "init_output_jpeg_image - error avcodec_open2 (%s)", err);
      ret = T_ERROR;
    } else {
      ret = T_OK;
    }
  }
  return ret;
}

int resize_image(AVCodecContext * original_image_codec_context, AVCodecContext * resized_image_codec_context, AVPacket * original_image_cover_packet, AVPacket * resized_image_cover_packet, int resized_width, int resized_height) {
  int ret = -1, data_present, err;
  AVFrame * input_frame = av_frame_alloc(), * output_frame = av_frame_alloc();
  struct SwsContext * sws_c;
  
  if (input_frame != NULL && output_frame != NULL) {
    if (!my_decode(original_image_codec_context, input_frame, &data_present, original_image_cover_packet)) {
      if (!resized_width) {
        resized_width = input_frame->width;
      }
      if (!resized_height) {
        resized_height = input_frame->height;
      }
      sws_c = sws_getContext(input_frame->width, input_frame->height, original_image_codec_context->pix_fmt, resized_width, resized_height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
      if (sws_c != NULL) {
        av_image_alloc(output_frame->data, output_frame->linesize, resized_width, resized_height, original_image_codec_context->pix_fmt, 32);
        output_frame->width = resized_width;
        output_frame->height = resized_height;
        output_frame->format = AV_PIX_FMT_YUV420P;
        if ((err = sws_scale(sws_c, (const uint8_t * const*)input_frame->data, input_frame->linesize, 0, input_frame->height, output_frame->data, output_frame->linesize)) < 0) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error sws_scale: %d", err);
          ret = -1;
        } else if (err > 0) {
          ret = my_encode(resized_image_codec_context, resized_image_cover_packet, output_frame, &data_present);
          //y_log_message(Y_LOG_LEVEL_DEBUG, "my_encode for %d %p %p %d %p => %d", ret, resized_image_codec_context, resized_image_cover_packet->size, output_frame->data, ret);
        }
        sws_freeContext(sws_c);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error sws_getContext");
      }
    }
    av_freep(&output_frame->data[0]);
    av_frame_free(&output_frame);
    av_frame_free(&input_frame);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error allocating resources for input_frame or output_frame");
    ret = -1;
  }
  return ret;
}
