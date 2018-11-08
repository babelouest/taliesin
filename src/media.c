/**
 *
 * Taliesin - Media server
 * 
 * Media file and folder functions definitions
 *
 * Copyright 2017-2018 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU GENERAL PUBLIC LICENSE
 * License as published by the Free Software Foundation;
 * version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <jansson.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>
#include <zlib.h>

#include "taliesin.h"
#include "static_file_callback.h"

json_t * media_get_tags(AVFormatContext * fmt_ctx) {
  AVDictionaryEntry * tag = NULL;
  json_t * to_return = json_object();
  char * key, * value;

  if (to_return != NULL) {
    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
      key = o_strndup(tag->key, MIN(TALIESIN_TAG_KEY_LENGTH, strlen(tag->key)));
      value = o_strndup(tag->value, MIN(TALIESIN_TAG_VALUE_LENGTH, strlen(tag->value)));
      int i;
      for(i = 0; key[i]; i++){
        key[i] = tolower(key[i]);
      }
      json_object_set_new(to_return, key, json_string(value));
      o_free(key);
      o_free(value);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_get_tags - Error allocating resources for to_return");
  }
  return to_return;
}

json_t * media_folder_detect_cover_by_id(struct config_elements * config, json_t * j_data_source, json_int_t tf_id, const char * path) {
  json_t * j_query, * j_result, * j_return, * j_folder_list, * j_media_cover, * j_element;
  int res, cover_found = 0, p_index;
  char * escape, * pattern, * where_clause, * media_path;
  size_t index;

  // Look for cover file as cover file pattern
  for (p_index=0; config->cover_file_pattern[p_index]!=NULL; p_index++) {
    if (!cover_found) {
      escape = h_escape_string(config->conn, config->cover_file_pattern[p_index]);
      pattern = str_replace(escape, "*", "%");
      o_free(escape);
      if (pattern != NULL) {
        where_clause = msprintf("= (SELECT `tic_id` FROM `%s` WHERE `tf_id`=%" JSON_INTEGER_FORMAT " AND `tm_name` LIKE '%s' AND `tm_refresh_status`='%d' ORDER BY `tm_name` LIMIT 1)", TALIESIN_TABLE_MEDIA, tf_id, pattern, DATA_SOURCE_REFRESH_MODE_PROCESSED);
        j_query = json_pack("{sss[s]s{s{ssss}}}",
                            "table",
                            TALIESIN_TABLE_IMAGE_COVER,
                            "columns",
                              "tic_id",
                            "where",
                              "tic_id",
                                "operator",
                                "raw",
                                "value",
                                where_clause);
        o_free(where_clause);
        if (j_query != NULL) {
          res = h_select(config->conn, j_query, &j_result, NULL);
          json_decref(j_query);
          if (res == H_OK) {
            if (json_array_size(j_result) > 0) {
              j_return = json_pack("{sisO}", "result", T_OK, "cover", json_array_get(j_result, 0));
              cover_found = 1;
            }
            json_decref(j_result);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_folder_detect_cover_by_id - Error executing j_query");
            j_return = json_pack("{si}", "result", T_ERROR_DB);
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_folder_detect_cover_by_id - Error allocating resources for j_query");
          j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
        }
        o_free(pattern);
      }
    }
  }
  
  if (!cover_found) {
    // No cover found as files in the folder, look for cover in the media files
    j_folder_list = media_list_folder(config, j_data_source, tf_id, 1);
    if (check_result_value(j_folder_list, T_OK)) {
      json_array_foreach(json_object_get(j_folder_list, "media"), index, j_element) {
        if (!cover_found && 0 == o_strcmp(json_string_value(json_object_get(j_element, "type")), "audio")) {
          media_path = msprintf("%s/%s", path, json_string_value(json_object_get(j_element, "name")));
          j_media_cover = media_cover_get_all(config, j_data_source, media_path, 0);
          o_free(media_path);
          if (check_result_value(j_media_cover, T_OK)) {
            j_return = json_pack("{sisO}", "result", T_OK, "cover", json_object_get(j_media_cover, "cover"));
            cover_found = 1;
          } else if (!check_result_value(j_media_cover, T_ERROR_NOT_FOUND)) {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_folder_detect_cover_by_id - Error getting media cover");
            j_return = json_pack("{si}", "result", T_ERROR);
          }
          json_decref(j_media_cover);
        }
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_folder_detect_cover_by_id - Error getting folder list");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_folder_list);
  }
  if (!cover_found) {
    j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
  }
  return j_return;
}

json_t * media_folder_get_cover(struct config_elements * config, json_t * j_data_source, const char * path) {
  json_int_t tf_id;
  json_t * j_return;
  
  tf_id = folder_get_id(config, j_data_source, 0, path);
  if (tf_id > 0) {
    j_return = media_folder_detect_cover_by_id(config, j_data_source, tf_id, path);
  } else {
    j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
  }
  return j_return;
}

int get_media_cover(AVFormatContext * full_size_cover_format_context, AVCodecContext ** full_size_cover_codec_context, AVPacket * full_size_cover_packet) {
  int i, ret = T_ERROR;
  AVCodec * cover_codec = NULL;
  int video_stream;
  
  video_stream = -1;
  *full_size_cover_codec_context = NULL;
  for (i=0; i<full_size_cover_format_context->nb_streams; i++) {
    cover_codec = avcodec_find_decoder(full_size_cover_format_context->streams[i]->codecpar->codec_id);
    if (cover_codec->type == AVMEDIA_TYPE_VIDEO) {
      if (cover_codec->id == AV_CODEC_ID_MJPEG) { // TODO See if we can't convert gif or png to jpeg
        video_stream = i;
        *full_size_cover_codec_context = avcodec_alloc_context3(cover_codec);
        avcodec_parameters_to_context(*full_size_cover_codec_context, full_size_cover_format_context->streams[i]->codecpar);
        avcodec_open2(*full_size_cover_codec_context, cover_codec, NULL);
        break;
      }
    }
  }
  
  if (video_stream != -1) {
    while (av_read_frame(full_size_cover_format_context, full_size_cover_packet) >= 0) {
      if (full_size_cover_packet->stream_index == video_stream) {
        ret = T_OK;
        break;
      }
      av_packet_unref(full_size_cover_packet);
    }
  } else {
    ret = T_ERROR_NOT_FOUND;
  }
  
  return ret;
}

json_t * get_format(struct config_elements * config, AVFormatContext *fmt_ctx, const char * path) {
  json_t * j_format = json_object();
  AVCodec * input_codec = NULL;
  int codec_index, i, file_type = config_get_type_from_path(config, path);
  
  if (j_format != NULL) {
    if (file_type == TALIESIN_FILE_TYPE_AUDIO || file_type == TALIESIN_FILE_TYPE_VIDEO) {
      if (fmt_ctx->nb_streams != 1) {
        if (file_type == TALIESIN_FILE_TYPE_AUDIO) {
          codec_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &input_codec, 0);
        } else {
          codec_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &input_codec, 0);
        }
      } else {
        codec_index = 0;
      }
      if (codec_index >= 0) {
        input_codec = avcodec_find_decoder(fmt_ctx->streams[codec_index]->codecpar->codec_id);
      } else {
        // Try manually
        for (i=0; i<fmt_ctx->nb_streams; i++) {
          input_codec = avcodec_find_decoder(fmt_ctx->streams[i]->codecpar->codec_id);
          if (input_codec->type == AVMEDIA_TYPE_AUDIO || input_codec->type == AVMEDIA_TYPE_VIDEO) {
            break;
          }
        }
      }
      if (input_codec != NULL) {
        if (input_codec->type == AVMEDIA_TYPE_VIDEO) {
          json_object_set_new(j_format, "media", json_string("video"));
        } else if (input_codec->type == AVMEDIA_TYPE_AUDIO) {
          json_object_set_new(j_format, "media", json_string("audio"));
        } else {
          json_object_set_new(j_format, "media", json_string("other"));
        }
      } else {
        json_object_set_new(j_format, "media", json_string("unknown"));
      }
      json_object_set_new(j_format, "name", json_string(fmt_ctx->iformat->name));
      json_object_set_new(j_format, "longname", json_string(fmt_ctx->iformat->long_name));
    } else {
      switch (file_type) {
        case TALIESIN_FILE_TYPE_SUBTITLE:
          json_object_set_new(j_format, "media", json_string("subtitle"));
          break;
        case TALIESIN_FILE_TYPE_IMAGE:
          json_object_set_new(j_format, "media", json_string("image"));
          break;
        case TALIESIN_FILE_TYPE_UNKNOWN:
        default:
          json_object_set_new(j_format, "media", json_string("unknown"));
          break;
      }
    }
    
  }
  return j_format;
}

unsigned char * media_get_cover_from_path(const char * path, size_t * size) {
  AVFormatContext * full_size_cover_format_context = NULL;
  AVCodecContext  * full_size_cover_codec_context  = NULL;
  AVPacket          full_size_cover_packet;
  unsigned char * cover = NULL;
  int ret;
  
  if (path != NULL && size != NULL) {
    if (!avformat_open_input(&full_size_cover_format_context, path, NULL, NULL)) {
      av_init_packet(&full_size_cover_packet);
      if ((ret = get_media_cover(full_size_cover_format_context, &full_size_cover_codec_context, &full_size_cover_packet)) == T_OK) {
        cover = o_malloc(full_size_cover_packet.size);
        if (cover != NULL) {
          memcpy(cover, full_size_cover_packet.data, full_size_cover_packet.size);
          *size = full_size_cover_packet.size;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_get_cover_from_path - Error allocating resources for cover");
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_get_cover_from_path - Error get_media_cover for path %s", path);
      }
      av_packet_unref(&full_size_cover_packet);
      avcodec_free_context(&full_size_cover_codec_context);
      avformat_close_input(&full_size_cover_format_context);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_get_cover_from_path - Error avformat_open_input for path %s", path);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_get_cover_from_path - Error, path is NULL");
  }
  return cover;
}

json_t * media_get_metadata(struct config_elements * config, AVCodecContext * thumbnail_cover_codec_context, const char * path) {
  AVFormatContext * full_size_cover_format_context = NULL;
  json_t * j_metadata = json_object(), * j_format;
  AVCodecContext  * full_size_cover_codec_context  = NULL;
  AVPacket          full_size_cover_packet, thumbnail_cover_packet;
  int ret;
  unsigned char * cover_b64 = NULL;
  size_t cover_b64_len;
  int file_type = config_get_type_from_path(config, path);
  
  if (j_metadata != NULL) {
    if (file_type == TALIESIN_FILE_TYPE_AUDIO || file_type == TALIESIN_FILE_TYPE_VIDEO || file_type == TALIESIN_FILE_TYPE_IMAGE) {
      if (!avformat_open_input(&full_size_cover_format_context, path, NULL, NULL)) {
        /* Get information on the input file (number of streams etc.). */
        if (!avformat_find_stream_info(full_size_cover_format_context, NULL)) {
          json_object_set_new(j_metadata, "tags", media_get_tags(full_size_cover_format_context));
          json_object_set_new(j_metadata, "duration", json_integer(((full_size_cover_format_context->duration*1000)/AV_TIME_BASE)));
          j_format = get_format(config, full_size_cover_format_context, path);
          if (j_format != NULL) {
            if (0 == o_strcmp("audio", json_string_value(json_object_get(j_format, "media"))) || 0 == o_strcmp("image", json_string_value(json_object_get(j_format, "media")))) {
              av_init_packet(&full_size_cover_packet);
              av_init_packet(&thumbnail_cover_packet);
              if ((ret = get_media_cover(full_size_cover_format_context, &full_size_cover_codec_context, &full_size_cover_packet)) == T_OK) {
                if (resize_image(full_size_cover_codec_context, thumbnail_cover_codec_context, &full_size_cover_packet, &thumbnail_cover_packet, TALIESIN_COVER_THUMB_WIDTH, TALIESIN_COVER_THUMB_HEIGHT) >= 0) {
                  cover_b64 = o_malloc(2 * full_size_cover_packet.size * sizeof(char));
                  if (cover_b64 != NULL) {
                    if (o_base64_encode(full_size_cover_packet.data, full_size_cover_packet.size, cover_b64, &cover_b64_len)) {
                      json_object_set_new(j_metadata, "cover", json_stringn((const char *)cover_b64, cover_b64_len));
                    }
                    o_free(cover_b64);
                  }
                  cover_b64 = o_malloc(2 * thumbnail_cover_packet.size * sizeof(char));
                  if (cover_b64 != NULL) {
                    if (o_base64_encode(thumbnail_cover_packet.data, thumbnail_cover_packet.size, cover_b64, &cover_b64_len)) {
                      json_object_set_new(j_metadata, "cover_thumbnail", json_stringn((const char *)cover_b64, cover_b64_len));
                    }
                    o_free(cover_b64);
                  }
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "media_get_metadata - Error resize_image for %s", path);
                }
              } else if (ret != T_ERROR_NOT_FOUND) {
                y_log_message(Y_LOG_LEVEL_ERROR, "media_get_metadata - Error get_media_cover for %s", path);
              }
              av_packet_unref(&full_size_cover_packet);
              av_packet_unref(&thumbnail_cover_packet);
              avcodec_free_context(&full_size_cover_codec_context);
            }
            json_object_set_new(j_metadata, "format", j_format);
          }
          avformat_close_input(&full_size_cover_format_context);
          full_size_cover_format_context = NULL;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_get_metadata - Error avformat_open_input for %s", path);
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_get_metadata - Error avformat_find_stream_info");
      }
    }
  }
  return j_metadata;
}

json_t * media_get_tags_from_id(struct config_elements * config, json_int_t tm_id) {
  json_t * j_query, * j_result, * j_tag, * j_return;
  size_t index;
  int res;
  
  j_query = json_pack("{sss[ss]s{sI}}",
                      "table",
                      TALIESIN_TABLE_META_DATA,
                      "columns",
                        "tmd_key",
                        "tmd_value",
                      "where",
                        "tm_id",
                        tm_id);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    j_return = json_pack("{sis{}}", "result", T_OK, "tags");
    if (j_return != NULL) {
      json_array_foreach(j_result, index, j_tag) {
        json_object_set(json_object_get(j_return, "tags"), json_string_value(json_object_get(j_tag, "tmd_key")), json_object_get(j_tag, "tmd_value"));
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_get_tags_from_id - Error allocating resources for j_return");
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_get_tags_from_id - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_int_t folder_get_id(struct config_elements * config, json_t * j_data_source, json_int_t tf_parent_id, const char * path) {
  char * sub_path;
  json_t * j_query, * j_result = NULL;
  int res;
  json_int_t ret = 0;
  
  if (o_strlen(path) > 0) {
    if (strchrnul(path, '/') != NULL) {
      sub_path = o_strndup(path, strchrnul(path, '/') - path);
    } else {
      sub_path = o_strdup(path);
    }
    if (0 != o_strcmp("..", sub_path)) {
      j_query = json_pack("{sss[s]s{sssI}}",
                          "table",
                          TALIESIN_TABLE_FOLDER,
                          "columns",
                            "tf_id",
                          "where",
                            "tf_name",
                            sub_path,
                            "tds_id",
                            json_integer_value(json_object_get(j_data_source, "tds_id")));
      if (j_query != NULL) {
        if (tf_parent_id > 0) {
          json_object_set_new(json_object_get(j_query, "where"), "tf_parent_id", json_integer(tf_parent_id));
        } else {
          json_object_set_new(json_object_get(j_query, "where"), "tf_parent_id", json_null());
        }
        res = h_select(config->conn, j_query, &j_result, NULL);
        json_decref(j_query);
        if (res == H_OK) {
          if (json_array_size(j_result) > 0) {
            ret = json_integer_value(json_object_get(json_array_get(j_result, 0), "tf_id"));
            path += o_strlen(sub_path);
            if (path[0] == '/') {
              path++;
            }
            if (path[0] != '\0') {
              ret = folder_get_id(config, j_data_source, ret, path);
            }
          } else {
            ret = -1; // Not found
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "folder_get_id - Error executing j_query");
        }
        json_decref(j_result);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "folder_get_id - Error allocating resources for j_query");
      }
    } else {
      ret = -1; // One can't have '..' as a folder in the path
    }
    o_free(sub_path);
  }
  return ret;
}

json_t * media_get_by_id(struct config_elements * config, json_int_t tm_id) {
  json_t * j_query, * j_result, * j_result_tag, * j_result_data_source, * j_result_history, * j_return, * j_tag, * j_element;
  int res;
  size_t index_tag;
  
  j_query = json_pack("{sss[sssssss]s{sI}}",
                      "table",
                      TALIESIN_TABLE_MEDIA,
                      "columns",
                        "tm_id",
                        "tds_id",
                        "tm_path AS path",
                        "tm_name AS name",
                        "tm_type AS type",
                        "tm_duration AS duration",
                        config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(tm_last_updated) AS last_updated":"tm_last_updated AS last_updated",
                      "where",
                        "tm_id",
                        tm_id);
  if (j_query != NULL) {
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      if (json_array_size(j_result) >0) {
        j_element = json_array_get(j_result, 0);
        j_query = json_pack("{sss[ss]s{sI}}",
                            "table",
                            TALIESIN_TABLE_META_DATA,
                            "columns",
                              "tmd_key",
                              "tmd_value",
                            "where",
                              "tm_id",
                              tm_id);
        json_object_set_new(j_element, "tags", json_object());
        if (j_query != NULL) {
          res = h_select(config->conn, j_query, &j_result_tag, NULL);
          json_decref(j_query);
          if (res == H_OK) {
            json_array_foreach(j_result_tag, index_tag, j_tag) {
              json_object_set(json_object_get(j_element, "tags"), json_string_value(json_object_get(j_tag, "tmd_key")), json_object_get(j_tag, "tmd_value"));
            }
            json_decref(j_result_tag);
            j_query = json_pack("{sss[s]s{sI}}",
                                "table",
                                TALIESIN_TABLE_DATA_SOURCE,
                                "columns",
                                  "tds_name",
                                "where",
                                  "tds_id",
                                  json_integer_value(json_object_get(j_element, "tds_id")));
            res = h_select(config->conn, j_query, &j_result_data_source, NULL);
            json_decref(j_query);
            json_object_del(j_element, "tds_id");
            if (res == H_OK) {
              json_object_set(j_element, "data_source", json_object_get(json_array_get(j_result_data_source, 0), "tds_name"));
              
              json_decref(j_result_data_source);
              j_query = json_pack("{sss[s]s{sI}}",
                                  "table",
                                  TALIESIN_TABLE_MEDIA_HISTORY,
                                  "columns",
                                    "COUNT(`tmh_id`) AS nb_play",
                                   "where",
                                     "tm_id",
                                     tm_id);
              res = h_select(config->conn, j_query, &j_result_history, NULL);
              json_decref(j_query);
              if (res == H_OK) {
                json_object_set(j_element, "nb_play", json_object_get(json_array_get(j_result_history, 0), "nb_play"));
                json_decref(j_result_history);
                j_return = json_pack("{sisO}", "result", T_OK, "media", j_element);
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "media_get_by_id - Error executing j_query for history");
                j_return = json_pack("{si}", "result", T_ERROR_DB);
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_get_by_id - Error executing j_query for data_source");
              j_return = json_pack("{si}", "result", T_ERROR_DB);
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_get_by_id - Error executing j_query (tag)");
            j_return = json_pack("{si}", "result", T_ERROR_DB);
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_get_by_id - Error allocating resources for j_query (tag)");
          j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
        }
      } else {
        j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
      }
      json_decref(j_result);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_get_by_id - Error executing j_query");
      j_return = json_pack("{si}", "result", T_ERROR_DB);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_get_by_id - Error allocating resources for j_query");
    j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
  }
  return j_return;
}

json_t * media_get_file(struct config_elements * config, json_t * j_data_source, json_int_t tf_id, const char * file, int get_id) {
  json_t * j_query, * j_result, * j_result_tag, * j_result_history, * j_return, * j_tag, * j_element;
  int res;
  size_t index_tag;
  
  j_query = json_pack("{sss[ssssss]s{sIsoss}ss}",
                      "table",
                      TALIESIN_TABLE_MEDIA,
                      "columns",
                        "tm_id",
                        "tm_name AS name",
                        "tm_type AS type",
                        "tm_path AS path",
                        "tm_duration AS duration",
                        config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(tm_last_updated) AS last_updated":"tm_last_updated AS last_updated",
                      "where",
                        "tds_id",
                        json_integer_value(json_object_get(j_data_source, "tds_id")),
                        "tf_id",
                        tf_id==0?json_null():json_integer(tf_id),
                        "tm_name",
                        file,
                      "order_by",
                      "path");
  if (j_query != NULL) {
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      if (json_array_size(j_result) >0) {
        j_element = json_array_get(j_result, 0);
        j_query = json_pack("{sss[ss]s{sI}}",
                            "table",
                            TALIESIN_TABLE_META_DATA,
                            "columns",
                              "tmd_key",
                              "tmd_value",
                            "where",
                              "tm_id",
                              json_integer_value(json_object_get(j_element, "tm_id")));
        json_object_set_new(j_element, "tags", json_object());
        if (j_query != NULL) {
          res = h_select(config->conn, j_query, &j_result_tag, NULL);
          json_decref(j_query);
          if (res == H_OK) {
            json_array_foreach(j_result_tag, index_tag, j_tag) {
              json_object_set(json_object_get(j_element, "tags"), json_string_value(json_object_get(j_tag, "tmd_key")), json_object_get(j_tag, "tmd_value"));
            }
            json_decref(j_result_tag);
            j_query = json_pack("{sss[s]s{sI}}",
                                "table",
                                TALIESIN_TABLE_MEDIA_HISTORY,
                                "columns",
                                  "COUNT(`tmh_id`) AS nb_play",
                                 "where",
                                   "tm_id",
                                   json_integer_value(json_object_get(j_element, "tm_id")));
            res = h_select(config->conn, j_query, &j_result_history, NULL);
            json_decref(j_query);
            if (res == H_OK) {
              json_object_set(j_element, "nb_play", json_object_get(json_array_get(j_result_history, 0), "nb_play"));
              json_decref(j_result_history);
              j_return = json_pack("{sisO}", "result", T_OK, "media", j_element);
              if (!get_id) {
                json_object_del(j_element, "tm_id");
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_get_file - Error executing j_query for history");
              j_return = json_pack("{si}", "result", T_ERROR_DB);
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_get_file - Error executing j_query (tag)");
            j_return = json_pack("{si}", "result", T_ERROR_DB);
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_get_file - Error allocating resources for j_query (tag)");
          j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
        }
      } else {
        j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
      }
      json_decref(j_result);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_get_file - Error executing j_query");
      j_return = json_pack("{si}", "result", T_ERROR_DB);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_get_file - Error allocating resources for j_query");
    j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
  }
  return j_return;
}

json_t * media_list_folder(struct config_elements * config, json_t * j_data_source, json_int_t tf_id, int get_id) {
  json_t * j_query, * j_result, * j_result_tag, * j_result_history, * j_return, * j_element, * j_tag;
  int res;
  size_t index, index_tag;
  
  j_query = json_pack("{sss[ssssss]s{sIso}ss}",
                      "table",
                      TALIESIN_TABLE_MEDIA,
                      "columns",
                        "tm_id",
                        "tm_name AS name",
                        "tm_type AS type",
                        "tm_path AS path",
                        "tm_duration AS duration",
                        config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(tm_last_updated) AS last_updated":"tm_last_updated AS last_updated",
                      "where",
                        "tds_id",
                        json_integer_value(json_object_get(j_data_source, "tds_id")),
                        "tf_id",
                        tf_id==0?json_null():json_integer(tf_id),
                      "order_by",
                      "tm_type, tm_name");
  if (j_query != NULL) {
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      json_array_foreach(j_result, index, j_element) {
        j_query = json_pack("{sss[ss]s{sI}}",
                            "table",
                            TALIESIN_TABLE_META_DATA,
                            "columns",
                              "tmd_key",
                              "tmd_value",
                            "where",
                              "tm_id",
                              json_integer_value(json_object_get(j_element, "tm_id")));
        json_object_set_new(j_element, "tags", json_object());
        if (j_query != NULL) {
          res = h_select(config->conn, j_query, &j_result_tag, NULL);
          json_decref(j_query);
          if (res == H_OK) {
            json_array_foreach(j_result_tag, index_tag, j_tag) {
              json_object_set(json_object_get(j_element, "tags"), json_string_value(json_object_get(j_tag, "tmd_key")), json_object_get(j_tag, "tmd_value"));
            }
            json_decref(j_result_tag);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_list_folder - Error executing j_query (tag)");
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_list_folder - Error allocating resources for j_query (tag)");
        }
        j_query = json_pack("{sss[s]s{sI}}",
                            "table",
                            TALIESIN_TABLE_MEDIA_HISTORY,
                            "columns",
                              "COUNT(`tmh_id`) AS nb_play",
                             "where",
                               "tm_id",
                               json_integer_value(json_object_get(j_element, "tm_id")));
        res = h_select(config->conn, j_query, &j_result_history, NULL);
        json_decref(j_query);
        if (res == H_OK) {
          json_object_set(j_element, "nb_play", json_object_get(json_array_get(j_result_history, 0), "nb_play"));
          json_decref(j_result_history);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_list_folder - Error executing j_query for history");
          j_return = json_pack("{si}", "result", T_ERROR_DB);
        }
        if (!get_id) {
          json_object_del(j_element, "tm_id");
        }
      }
      j_return = json_pack("{siso}", "result", T_OK, "media", j_result);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_list_folder - Error executing j_query");
      j_return = json_pack("{si}", "result", T_ERROR_DB);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_list_folder - Error allocating resources for j_query");
    j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
  }
  return j_return;
}

json_t * media_get(struct config_elements * config, json_t * j_data_source, const char * path) {
  json_t * j_query, * j_result_folders, * j_result_files, * j_result = NULL, * j_element;
  json_int_t tf_id = 0;
  size_t index;
  int res;
  char * file = NULL, * dup_path = o_strdup(path);
  
  if (dup_path != NULL) {
    if (o_strlen(dup_path) > 0) {
      tf_id = folder_get_id(config, j_data_source, 0, dup_path);
      if (tf_id < 0) {
        // Retry with the last file removed
        char * last = strrchr(dup_path, '/');
        if (last != NULL) {
          dup_path[last - dup_path] = '\0';
          file = last + 1;
          tf_id = folder_get_id(config, j_data_source, 0, dup_path);
        } else {
          tf_id = 0;
          file = dup_path;
        }
      }
    }
  }
  
  if (tf_id != -1) {
    if (file == NULL) {
      j_query = json_pack("{sss[ss]s{sosI}ss}",
                          "table",
                          TALIESIN_TABLE_FOLDER,
                            "columns",
                              "tf_name AS name",
                              "tf_path AS path",
                          "where",
                            "tf_parent_id",
                            tf_id==0?json_null():json_integer(tf_id),
                            "tds_id",
                            json_integer_value(json_object_get(j_data_source, "tds_id")),
                          "order_by",
                          "path");
      if (j_query != NULL) {
        res = h_select(config->conn, j_query, &j_result_folders, NULL);
        json_decref(j_query);
        if (res == H_OK) {
          json_array_foreach(j_result_folders, index, j_element) {
            json_object_set_new(j_element, "type", json_string("folder"));
          }
          j_result_files = media_list_folder(config, j_data_source, tf_id, 0);
          if (check_result_value(j_result_files, T_OK)) {
            j_result = json_pack("{sis[]}", "result", T_OK, "media");
            if (j_result != NULL) {
              json_array_extend(json_object_get(j_result, "media"), j_result_folders);
              json_array_extend(json_object_get(j_result, "media"), json_object_get(j_result_files, "media"));
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_get - Error allocating resources for j_result");
              j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_get - Error getting j_result_files (1)");
            j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
          }
          json_decref(j_result_folders);
          json_decref(j_result_files);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_get - Error executing j_query");
          j_result = json_pack("{si}", "result", T_ERROR_DB);
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_get - Error allocating resources for j_query");
        j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
      }
    } else {
      j_result_files = media_get_file(config, j_data_source, tf_id, file, 0);
      if (check_result_value(j_result_files, T_OK)) {
        j_result = json_pack("{sisO}", "result", T_OK, "media", json_object_get(j_result_files, "media"));
      } else if (check_result_value(j_result_files, T_ERROR_NOT_FOUND)) {
        j_result = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_get - Error getting j_result_files (2)");
        j_result = json_pack("{si}", "result", T_ERROR);
      }
      json_decref(j_result_files);
    }
  } else {
    j_result = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
  }
  o_free(dup_path);
  return j_result;
}

json_int_t cover_save_new_or_get_id(struct config_elements * config, json_int_t tds_id, json_t * j_media) {
  json_int_t tic_id = 0;
  const char * cover, * cover_thumbnail;
  unsigned long cover_crc = crc32(0L, Z_NULL, 0);
  char cover_crc_str[16] = {0};
  json_t * j_query, * j_result, * j_last_id;
  int res;
  
  cover = json_string_value(json_object_get(json_object_get(j_media, "metadata"), "cover"));
  cover_thumbnail = json_string_value(json_object_get(json_object_get(j_media, "metadata"), "cover_thumbnail"));
  if (strlen(cover) < TALIESIN_MEDIUMBLOB_MAX_SIZE) {
    cover_crc = crc32(cover_crc, (const unsigned char *)cover, strlen(cover));
    sprintf(cover_crc_str, "0x%08lx", cover_crc);
    j_query = json_pack("{sss[s]s{ssso}}",
                        "table",
                        TALIESIN_TABLE_IMAGE_COVER,
                        "columns",
                          "tic_id",
                        "where",
                          "tic_fingerprint",
                          cover_crc_str,
                          "tds_id",
                          tds_id?json_integer(tds_id):json_null());
    if (j_query != NULL) {
      res = h_select(config->conn, j_query, &j_result, NULL);
      json_decref(j_query);
      if (res == H_OK) {
        if (json_array_size(j_result) > 0) {
          tic_id = json_integer_value(json_object_get(json_array_get(j_result, 0), "tic_id"));
        } else {
          // There's no cover with this crc, so we add it
          j_query = json_pack("{sss{sossssss}}",
                              "table",
                              TALIESIN_TABLE_IMAGE_COVER,
                              "values",
                                "tds_id",
                                tds_id?json_integer(tds_id):json_null(),
                                "tic_path",
                                json_string_value(json_object_get(j_media, "path")),
                                "tic_cover_thumbnail",
                                cover_thumbnail,
                                "tic_fingerprint",
                                cover_crc_str);
          res = h_insert(config->conn, j_query, NULL);
          json_decref(j_query);
          if (res == H_OK) {
            j_last_id = h_last_insert_id(config->conn);
            if (j_last_id != NULL) {
              // Insert media metadata
              tic_id = json_integer_value(j_last_id);
              json_decref(j_last_id);
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "cover_save_new_or_get_id - error executing j_query (2)");
          }
        }
        json_decref(j_result);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "cover_save_new_or_get_id - error executing j_query");
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "cover_save_new_or_get_id - error allocating resources for j_query");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "cover_save_new_or_get_id - cover too large, over %d bytes", TALIESIN_MEDIUMBLOB_MAX_SIZE);
  }
  
  return tic_id;
}

int media_add(struct config_elements * config, json_int_t tds_id, json_int_t tf_id, const char * path, json_t * j_media) {
  json_t * j_query, * j_last_id, * j_tag;
  int res, ret = T_OK;
  char * clause_last_updated;
  json_int_t tm_id, tic_id = 0;
  const char * key, * media_type;
  
  // Insert media cover and thumbnail
  if (json_object_get(json_object_get(j_media, "metadata"), "cover")) {
    json_object_set_new(j_media, "path", json_string(path));
    tic_id = cover_save_new_or_get_id(config, tds_id, j_media);
  }
  
  if (config->conn->type == HOEL_DB_TYPE_MARIADB) {
    clause_last_updated = msprintf("FROM_UNIXTIME(%" JSON_INTEGER_FORMAT ")", json_integer_value(json_object_get(j_media, "last_modified")));
  } else {
    clause_last_updated = msprintf("%" JSON_INTEGER_FORMAT, json_integer_value(json_object_get(j_media, "last_modified")));
  }
  
  media_type = json_string_value(json_object_get(json_object_get(json_object_get(j_media, "metadata"), "format"), "media"));
  if (media_type == NULL) {
    media_type = "unknown";
  }
  j_query = json_pack("{sss{sssss{ss}sIsosssI}}",
                      "table",
                      TALIESIN_TABLE_MEDIA,
                      "values",
                        "tm_name",
                        json_string_value(json_object_get(j_media, "name")),
                        "tm_type",
                        media_type,
                        "tm_last_updated",
                          "raw",
                          clause_last_updated,
                        "tds_id",
                        tds_id,
                        "tic_id",
                        tic_id?json_integer(tic_id):json_null(),
                        "tm_path",
                        path,
                        "tm_duration",
                        json_object_get(json_object_get(j_media, "metadata"), "duration")!=NULL?json_integer_value(json_object_get(json_object_get(j_media, "metadata"), "duration")):0);
  o_free(clause_last_updated);
  if (j_query != NULL) {
    if (tf_id > 0) {
      json_object_set_new(json_object_get(j_query, "values"), "tf_id", json_integer(tf_id));
    }
    res = h_insert(config->conn, j_query, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      j_last_id = h_last_insert_id(config->conn);
      if (j_last_id != NULL) {
        if (json_object_size(json_object_get(json_object_get(j_media, "metadata"), "tags"))) {
          // Insert media metadata
          tm_id = json_integer_value(j_last_id);
          j_query = json_pack("{sss[]}",
                              "table",
                              TALIESIN_TABLE_META_DATA,
                              "values");
          if (j_query != NULL) {
            json_object_foreach(json_object_get(json_object_get(j_media, "metadata"), "tags"), key, j_tag) {
              json_array_append_new(json_object_get(j_query, "values"), json_pack("{sIssss}", "tm_id", tm_id, "tmd_key", key, "tmd_value", json_string_value(j_tag)));
            }
            res = h_insert(config->conn, j_query, NULL);
            json_decref(j_query);
            if (res != H_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_add - error insert media metadata in database");
              ret = T_ERROR_DB;
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_add - error allocating resources for j_query (2)");
            ret = T_ERROR_MEMORY;
          }
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_add - error getting last tm_id");
        ret = T_ERROR_DB;
      }
      json_decref(j_last_id);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_add - error insert media in database");
      ret = T_ERROR_DB;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_add - error allocating resources for j_query");
    ret = T_ERROR_MEMORY;
  }
  return ret;
}

int media_update(struct config_elements * config, json_int_t tm_id, json_t * j_media) {
  json_t * j_query, * j_last_id, * j_tag, * j_result;
  int res, ret = T_OK;
  char * clause_last_updated;
  json_int_t tic_id = 0;
  const char * key, * media_type, * cover, * cover_thumbnail;
  unsigned long cover_crc = crc32(0L, Z_NULL, 0);
  char cover_crc_str[16] = {0};
  
  // Insert media cover and thumbnail
  if (json_object_get(json_object_get(j_media, "metadata"), "cover")) {
    cover = json_string_value(json_object_get(json_object_get(j_media, "metadata"), "cover"));
    cover_thumbnail = json_string_value(json_object_get(json_object_get(j_media, "metadata"), "cover_thumbnail"));
    cover_crc = crc32(cover_crc, (const unsigned char *)cover, strlen(cover));
    sprintf(cover_crc_str, "0x%08lx", cover_crc);
    j_query = json_pack("{sss[s]s{ss}}",
                        "table",
                        TALIESIN_TABLE_IMAGE_COVER,
                        "columns",
                          "tic_id",
                        "where",
                          "tic_fingerprint",
                          cover_crc_str);
    if (j_query != NULL) {
      res = h_select(config->conn, j_query, &j_result, NULL);
      json_decref(j_query);
      if (res == H_OK) {
        if (json_array_size(j_result) > 0) {
          tic_id = json_integer_value(json_object_get(json_array_get(j_result, 0), "tic_id"));
        } else {
          // There's no cover with this crc, so we add it
          j_query = json_pack("{sss{ssssss}}",
                              "table",
                              TALIESIN_TABLE_IMAGE_COVER,
                              "values",
                                "tic_path",
                                json_string_value(json_object_get(j_media, "path")),
                                "tic_cover_thumbnail",
                                cover_thumbnail,
                                "tic_fingerprint",
                                cover_crc_str);
          res = h_insert(config->conn, j_query, NULL);
          json_decref(j_query);
          if (res == H_OK) {
            j_last_id = h_last_insert_id(config->conn);
            if (j_last_id != NULL) {
              // Insert media metadata
              tic_id = json_integer_value(j_last_id);
              json_decref(j_last_id);
            }
          }
        }
        json_decref(j_result);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_update - error executing j_query (3)");
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_update - error allocating resources for j_query (3)");
    }
  }
  
  if (config->conn->type == HOEL_DB_TYPE_MARIADB) {
    clause_last_updated = msprintf("FROM_UNIXTIME(%" JSON_INTEGER_FORMAT ")", json_integer_value(json_object_get(j_media, "last_modified")));
  } else {
    clause_last_updated = msprintf("%" JSON_INTEGER_FORMAT, json_integer_value(json_object_get(j_media, "last_modified")));
  }
  
  media_type = json_string_value(json_object_get(json_object_get(json_object_get(j_media, "metadata"), "format"), "media"));
  if (media_type == NULL) {
    media_type = "unknown";
  }
  j_query = json_pack("{sss{sis{ss}sosI}s{sI}}",
                      "table",
                      TALIESIN_TABLE_MEDIA,
                      "set",
                        "tm_refresh_status",
                        DATA_SOURCE_REFRESH_MODE_PROCESSED,
                        "tm_last_updated",
                          "raw",
                          clause_last_updated,
                        "tic_id",
                        tic_id?json_integer(tic_id):json_null(),
                        "tm_duration",
                        json_object_get(json_object_get(j_media, "metadata"), "duration")!=NULL?json_integer_value(json_object_get(json_object_get(j_media, "metadata"), "duration")):0,
                      "where",
                        "tm_id",
                        tm_id);
  o_free(clause_last_updated);
  if (j_query != NULL) {
    res = h_update(config->conn, j_query, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      if (json_object_get(json_object_get(j_media, "metadata"), "tags") != NULL) {
        // delete media metadata
        j_query = json_pack("{sss{sI}}",
                            "table",
                            TALIESIN_TABLE_META_DATA,
                            "where",
                              "tm_id",
                              tm_id);
        res = h_delete(config->conn, j_query, NULL);
        json_decref(j_query);
        if (res == H_OK) {
          // Insert media metadata (if any)
          if (json_array_size(json_object_get(json_object_get(j_media, "metadata"), "tags")) > 0) {
            j_query = json_pack("{sss[]}",
                                "table",
                                TALIESIN_TABLE_META_DATA,
                                "values");
            if (j_query != NULL) {
              json_object_foreach(json_object_get(json_object_get(j_media, "metadata"), "tags"), key, j_tag) {
                json_array_append_new(json_object_get(j_query, "values"), json_pack("{sIssss}", "tm_id", tm_id, "tmd_key", key, "tmd_value", json_string_value(j_tag)));
              }
              res = h_insert(config->conn, j_query, NULL);
              json_decref(j_query);
              if (res != H_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "media_update - error insert media metadata in database");
                ret = T_ERROR_DB;
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_update - error allocating resources for j_query (2)");
              ret = T_ERROR_MEMORY;
            }
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_update - error delete media metadata in database");
          ret = T_ERROR_DB;
        }
      } else {
        ret = T_OK;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_update - error insert media in database");
      ret = T_ERROR_DB;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_update - error allocating resources for j_query");
    ret = T_ERROR_MEMORY;
  }
  return ret;
}

char * build_icy_title(json_t * media) {
  char * title = NULL, year[5] = {0};
  const char * artist = NULL, * album = NULL, * song = NULL;
  
  if (media != NULL) {
    song = json_string_value(json_object_get(json_object_get(media, "tags"), "title"));
    artist = json_string_value(json_object_get(json_object_get(media, "tags"), "artist"));
    album = json_string_value(json_object_get(json_object_get(media, "tags"), "album"));
    o_strncpy(year, json_string_value(json_object_get(json_object_get(media, "tags"), "date")), 4);
    title = msprintf("%s%s%s%s%s%s%s%s%s", 
                      artist!=NULL?artist:"",
                      (artist!=NULL && album!=NULL)?" - ":"",
                      album!=NULL?album:"",
                      album!=NULL?" ":"",
                      year[0]!='\0'?"(":"",
                      year[0]!='\0'?year:"",
                      year[0]!='\0'?")":"",
                      ((artist!=NULL || album!=NULL || year[0]!='\0') && song!=NULL)?" - ":"",
                      song!=NULL?song:json_string_value(json_object_get(media, "name")));
  }
  return title;
}

char * build_m3u_title(json_t * media) {
  char * title = NULL;
  const char * artist = NULL, * song = NULL;
  
  if (media != NULL) {
    song = json_string_value(json_object_get(json_object_get(media, "tags"), "title"));
    artist = json_string_value(json_object_get(json_object_get(media, "tags"), "artist"));
    title = msprintf("%s%s%s", 
                      artist!=NULL?artist:"",
                      artist!=NULL?" - ":"",
                      song!=NULL?song:json_string_value(json_object_get(media, "name")));
  }
  return title;
}

int scan_path_to_webradio(struct config_elements * config, json_t * j_data_source, const char * path, int recursive, struct _t_webradio * webradio) {
  json_t * j_media = media_get_full(config, j_data_source, path), * j_element;
  int res;
  char * new_path, * full_path;
  size_t index;
  
  if (check_result_value(j_media, T_OK)) {
    if (json_is_array(json_object_get(j_media, "media"))) {
      json_array_foreach(json_object_get(j_media, "media"), index, j_element) {
        new_path = msprintf("%s/%s", path, json_string_value(json_object_get(j_element, "name")));
        if (0 == o_strcmp(json_string_value(json_object_get(j_element, "type")), "folder") && recursive) {
          if (scan_path_to_webradio(config, j_data_source, new_path, recursive, webradio) != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "scan_path_to_webradio - Error scanning folder '%s'", new_path);
          }
        } else if (0 == o_strcmp(json_string_value(json_object_get(j_element, "type")), "audio")) {
          full_path = msprintf("%s/%s", json_string_value(json_object_get(j_data_source, "path")), new_path);
          if (file_list_enqueue_new_file(webradio->file_list, full_path, json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "scan_path_to_webradio - Error adding file %s", new_path);
          }
          o_free(full_path);
        }
        o_free(new_path);
      }
      res = T_OK;
    } else {
      full_path = msprintf("%s/%s", json_string_value(json_object_get(j_data_source, "path")), path);
      res = file_list_enqueue_new_file(webradio->file_list, full_path, json_integer_value(json_object_get(json_object_get(j_media, "media"), "tm_id")));
      o_free(full_path);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "scan_path_to_webradio - Error scanning path '%s' in data_source %s", path, json_string_value(json_object_get(j_data_source, "name")));
    res = T_ERROR;
  }
  json_decref(j_media);
  return res;
}

json_t * media_scan_path(struct config_elements * config, json_t * j_data_source, const char * path, int recursive) {
  json_t * j_media = media_get_full(config, j_data_source, path), * j_element, * j_result = NULL, * j_temp_result;
  char * new_path, * full_path;
  size_t index;
  
  if (check_result_value(j_media, T_OK)) {
    j_result = json_pack("{sOs[]}", "data_source", j_data_source, "media_list");
    if (j_result != NULL) {
      if (json_is_array(json_object_get(j_media, "media"))) {
        json_array_foreach(json_object_get(j_media, "media"), index, j_element) {
          if (o_strlen(path) > 0) {
            new_path = msprintf("%s/%s", path, json_string_value(json_object_get(j_element, "name")));
          } else {
            new_path = o_strdup(json_string_value(json_object_get(j_element, "name")));
          }
          if (0 == o_strcmp(json_string_value(json_object_get(j_element, "type")), "folder") && recursive) {
            j_temp_result = media_scan_path(config, j_data_source, new_path, recursive);
            if (j_temp_result != NULL) {
              json_array_extend(json_object_get(j_result, "media_list"), json_object_get(j_temp_result, "media_list"));
            }
            json_decref(j_temp_result);
          } else if (0 == o_strcmp(json_string_value(json_object_get(j_element, "type")), "audio")) {
            full_path = msprintf("%s/%s", json_string_value(json_object_get(j_data_source, "path")), new_path);
            json_array_append_new(json_object_get(j_result, "media_list"), json_pack("{sIss}", "tm_id", json_integer_value(json_object_get(j_element, "tm_id")), "full_path", full_path));
            o_free(full_path);
          }
          o_free(new_path);
        }
      } else {
        full_path = msprintf("%s/%s", json_string_value(json_object_get(j_data_source, "path")), path);
        json_array_append_new(json_object_get(j_result, "media_list"), json_pack("{sIssss}", "tm_id", json_integer_value(json_object_get(json_object_get(j_media, "media"), "tm_id")), "path", path, "full_path", full_path));
        o_free(full_path);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_scan_path - Error allocating resources for j_result");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_scan_path - Error scanning path '%s' in data_source %s", path, json_string_value(json_object_get(j_data_source, "name")));
  }
  json_decref(j_media);
  return j_result;
}

json_t * media_get_file_list_from_path(struct config_elements * config, json_t * j_data_source, const char * path, int recursive) {
  json_t * j_result = media_get_full(config, j_data_source, path), * j_sub_result, * j_return, * j_element;
  size_t index;
  char * sub_path;
  
  if (check_result_value(j_result, T_OK)) {
    if (json_is_array(json_object_get(j_result, "media"))) {
      j_return = json_pack("{sis[]}", "result", T_OK, "media");
      if (j_return != NULL) {
        json_array_foreach(json_object_get(j_result, "media"), index, j_element) {
          if (recursive && 0 == o_strcmp("folder", json_string_value(json_object_get(j_element, "type")))) {
            sub_path = msprintf("%s/%s", path, json_string_value(json_object_get(j_element, "name")));;
            j_sub_result = media_get_file_list_from_path(config, j_data_source, sub_path, recursive);
            if (check_result_value(j_sub_result, T_OK)) {
              json_array_extend(json_object_get(j_return, "media"), json_object_get(j_sub_result, "media"));
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_get_file_list_from_path - Error media_get_file_list_from_path for sub_path %s", sub_path);
            }
            json_decref(j_sub_result);
            o_free(sub_path);
          } else {
            json_object_set(j_element, "data_source_path", json_object_get(j_data_source, "path"));
            json_array_append(json_object_get(j_return, "media"), j_element);
          }
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_get_file_list_from_path - Error allocating resources for j_return");
      }
    } else {
      j_return = json_pack("{sis[O]}", "result", T_OK, "media", json_object_get(j_result, "media"));
      json_object_set(json_array_get(json_object_get(j_return, "media"), 0), "data_source_path", json_object_get(j_data_source, "path"));
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_get_file_list_from_path - Error media_get_full");
    j_return = json_pack("{si}", "result", T_ERROR);
  }
  json_decref(j_result);
  return j_return;
}

json_t * media_get_audio_list_from_path(struct config_elements * config, json_t * j_data_source, const char * path, int recursive) {
  json_t * j_result = media_get_full(config, j_data_source, path), * j_sub_result, * j_return, * j_element;
  size_t index;
  char * sub_path;
  
  if (check_result_value(j_result, T_OK)) {
    if (json_is_array(json_object_get(j_result, "media"))) {
      j_return = json_pack("{sis[]}", "result", T_OK, "media");
      if (j_return != NULL) {
        json_array_foreach(json_object_get(j_result, "media"), index, j_element) {
          if (recursive && 0 == o_strcmp("folder", json_string_value(json_object_get(j_element, "type")))) {
            sub_path = msprintf("%s/%s", path, json_string_value(json_object_get(j_element, "name")));;
            j_sub_result = media_get_audio_list_from_path(config, j_data_source, sub_path, recursive);
            if (check_result_value(j_sub_result, T_OK)) {
              json_array_extend(json_object_get(j_return, "media"), json_object_get(j_sub_result, "media"));
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_get_audio_list_from_path - Error media_get_audio_list_from_path for sub_path %s", sub_path);
            }
            json_decref(j_sub_result);
            o_free(sub_path);
          } else if (0 == o_strcmp("audio", json_string_value(json_object_get(j_element, "type")))) {
            json_object_set(j_element, "data_source_path", json_object_get(j_data_source, "path"));
            json_array_append(json_object_get(j_return, "media"), j_element);
          }
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_get_audio_list_from_path - Error allocating resources for j_return");
      }
    } else {
      j_return = json_pack("{sis[O]}", "result", T_OK, "media", json_object_get(j_result, "media"));
      json_object_set(json_array_get(json_object_get(j_return, "media"), 0), "data_source_path", json_object_get(j_data_source, "path"));
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_get_audio_list_from_path - Error media_get_full");
    j_return = json_pack("{si}", "result", T_ERROR);
  }
  json_decref(j_result);
  return j_return;
}

json_t * media_get_full(struct config_elements * config, json_t * j_data_source, const char * path) {
  json_t * j_query, * j_result_folders, * j_result = NULL, * j_result_files, * j_element;
  json_int_t tf_id = 0;
  int res;
  char * file = NULL, * dup_path, * dup_path_save;
  size_t index;
  
  dup_path = dup_path_save = o_strdup(path);
  if (o_strlen(dup_path) > 0) {
    while (dup_path[0] == '/') {
      dup_path++;
    }
    while (o_strlen(dup_path) > 0 && dup_path[o_strlen(dup_path) - 1] == '/') {
      dup_path[o_strlen(dup_path) - 1] = '\0';
    }
    tf_id = folder_get_id(config, j_data_source, 0, dup_path);
    if (tf_id < 0) {
      // Retry with the last file removed
      char * last = strrchr(dup_path, '/');
      if (last != NULL) {
        dup_path[last - dup_path] = '\0';
        file = last + 1;
        tf_id = folder_get_id(config, j_data_source, 0, dup_path);
      } else {
        tf_id = 0;
        file = dup_path;
      }
    }
  }

  if (tf_id != -1) {
    if (file == NULL) {
      j_query = json_pack("{sss[ss]s{sosI}}",
                          "table",
                          TALIESIN_TABLE_FOLDER,
                            "columns",
                              "tf_name AS name",
                              "tf_id",
                          "where",
                            "tf_parent_id",
                            tf_id==0?json_null():json_integer(tf_id),
                            "tds_id",
                            json_integer_value(json_object_get(j_data_source, "tds_id")));
      if (j_query != NULL) {
        res = h_select(config->conn, j_query, &j_result_folders, NULL);
        json_decref(j_query);
        if (res == H_OK) {
          j_result = json_pack("{sis[]}", "result", T_OK, "media");
          if (j_result != NULL) {
            json_array_foreach(j_result_folders, index, j_element) {
              json_object_set_new(j_element, "type", json_string("folder"));
              json_array_append(json_object_get(j_result, "media"), j_element);
            }
            json_decref(j_result_folders);
            j_result_files = media_list_folder(config, j_data_source, tf_id, 1);
            if (check_result_value(j_result_files, T_OK)) {
              json_array_extend(json_object_get(j_result, "media"), json_object_get(j_result_files, "media"));
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_get - Error getting j_result_files (1)");
              j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
            }
            json_decref(j_result_files);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_get - Error allocating resources for j_result");
            j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_get - Error executing j_query");
          j_result = json_pack("{si}", "result", T_ERROR_DB);
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_get - Error allocating resources for j_query");
        j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
      }
    } else {
      j_result = media_get_file(config, j_data_source, tf_id, file, 1);
    }
  } else {
    j_result = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
  }
  o_free(dup_path_save);
  return j_result;
}

json_t * media_cover_get_all(struct config_elements * config, json_t * j_data_source, const char * path, int thumbnail) {
  json_t * j_query, * j_result, * j_return, * j_media;
  int res;
  char * clause_where, * cover_path;
  unsigned char * cover_full, * cover_full_b64;
  size_t cover_size, cover_size_b64;
  json_int_t tf_id;
  
  j_media = media_get_full(config, j_data_source, path);
  if (check_result_value(j_media, T_OK)) {
    if (json_is_array(json_object_get(j_media, "media"))) {
      tf_id = folder_get_id(config, j_data_source, 0, path);
      clause_where = msprintf("= (SELECT `tic_id` FROM `%s` WHERE `tf_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_FOLDER, tf_id);
    } else {
      clause_where = msprintf("= (SELECT `tic_id` FROM `%s` WHERE `tm_id`=%" JSON_INTEGER_FORMAT ") OR `tic_id` = (SELECT `tic_id` FROM `%s` WHERE `tf_id`=(SELECT `tf_id` FROM `%s` WHERE `tm_id`=%" JSON_INTEGER_FORMAT "))", TALIESIN_TABLE_MEDIA, json_integer_value(json_object_get(json_object_get(j_media, "media"), "tm_id")), TALIESIN_TABLE_FOLDER, TALIESIN_TABLE_MEDIA, json_integer_value(json_object_get(json_object_get(j_media, "media"), "tm_id")));
    }
    j_query = json_pack("{sss[ss]s{s{ssss}}}",
                        "table",
                        TALIESIN_TABLE_IMAGE_COVER,
                        "columns",
                          thumbnail?"tic_cover_thumbnail AS thumbnail":"tic_path AS path",
                          "tic_id",
                        "where",
                          "tic_id",
                            "operator",
                            "raw",
                            "value",
                            clause_where);
    o_free(clause_where);
    if (j_query != NULL) {
      res = h_select(config->conn, j_query, &j_result, NULL);
      json_decref(j_query);
      if (res == H_OK) {
        if (json_array_size(j_result) > 0) {
          if (!thumbnail) {
            cover_path = msprintf("%s/%s", json_string_value(json_object_get(j_data_source, "path")), json_string_value(json_object_get(json_array_get(j_result, 0), "path")));
            cover_full = media_get_cover_from_path(cover_path, &cover_size);
            if (cover_full != NULL) {
              cover_full_b64 = o_malloc(2 * cover_size * sizeof(char));
              if (cover_full_b64 != NULL) {
                if (o_base64_encode(cover_full, cover_size, cover_full_b64, &cover_size_b64)) {
                  json_object_set_new(json_array_get(j_result, 0), "full", json_stringn((const char *)cover_full_b64, cover_size_b64));
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_all - Error o_base64_encode");
                }
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_all - Error allocating resources for cover_full_b64");
              }
              o_free(cover_full_b64);
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_all - Error media_get_cover_from_path for path %s", cover_path);
            }
            o_free(cover_path);
            o_free(cover_full);
          }
          j_return = json_pack("{sisO}", "result", T_OK, "cover", json_array_get(j_result, 0));
        } else {
          j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
        }
        json_decref(j_result);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get - Error executing j_query");
        j_return = json_pack("{si}", "result", T_ERROR_DB);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get - Error allocating resources for j_query");
      j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
    }
  } else if (check_result_value(j_media, T_ERROR_NOT_FOUND)) {
    j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get - Error media_get_full");
    j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
  }
  json_decref(j_media);
  return j_return;
}

json_t * media_cover_get(struct config_elements * config, json_t * j_data_source, const char * path, int thumbnail) {
  json_t * j_media_cover = media_cover_get_all(config, j_data_source, path, thumbnail), * j_return;
  if (check_result_value(j_media_cover, T_OK)) {
    if (thumbnail) {
      j_return = json_pack("{sisO}", "result", T_OK, "cover", json_object_get(json_object_get(j_media_cover, "cover"), "thumbnail"));
    } else {
      j_return = json_pack("{sisO}", "result", T_OK, "cover", json_object_get(json_object_get(j_media_cover, "cover"), "full"));
    }
    json_decref(j_media_cover);
    return j_return;
  } else {
    return j_media_cover;
  }
}

json_t * media_cover_get_by_id(struct config_elements * config, json_int_t tm_id, int thumbnail) {
  json_t * j_query, * j_result_media, * j_result_folder, * j_return, * j_data_source;
  int res;
  char * clause_where, * cover_path;
  unsigned char * cover_full, * cover_full_b64;
  size_t cover_size, cover_size_b64;
  
  clause_where = msprintf("= (SELECT `tic_id` FROM `%s` WHERE `tm_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_MEDIA, tm_id);
  j_query = json_pack("{sss[sss]s{s{ssss}}}",
                      "table",
                      TALIESIN_TABLE_IMAGE_COVER,
                      "columns",
                        "tds_id",
                        thumbnail?"tic_cover_thumbnail AS thumbnail":"tic_path AS path",
                        "tic_id",
                      "where",
                        "tic_id",
                          "operator",
                          "raw",
                          "value",
                          clause_where);
  o_free(clause_where);
  res = h_select(config->conn, j_query, &j_result_media, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result_media) > 0) {
      if (!thumbnail) {
        j_data_source = data_source_get_by_id(config, json_integer_value(json_object_get(json_array_get(j_result_media, 0), "tds_id")));
        if (check_result_value(j_data_source, T_OK)) {
          cover_path = msprintf("%s/%s", json_string_value(json_object_get(json_object_get(j_data_source, "data_source"), "path")), json_string_value(json_object_get(json_array_get(j_result_media, 0), "path")));
          cover_full = media_get_cover_from_path(cover_path, &cover_size);
          if (cover_full != NULL) {
            cover_full_b64 = o_malloc(2 * cover_size * sizeof(char));
            if (cover_full_b64 != NULL) {
              if (o_base64_encode(cover_full, cover_size, cover_full_b64, &cover_size_b64)) {
                json_object_set_new(json_array_get(j_result_media, 0), "full", json_stringn((const char *)cover_full_b64, cover_size_b64));
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_by_id - Error o_base64_encode");
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_by_id - Error allocating resources for cover_full_b64");
            }
            o_free(cover_full_b64);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_by_id - Error media_get_cover_from_path for path %s", cover_path);
          }
          o_free(cover_path);
          o_free(cover_full);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_by_id - Error data_source_get_by_id");
          j_return = json_pack("{si}", "result", T_ERROR);
        }
        json_decref(j_data_source);
      }
      j_return = json_pack("{sisO}", "result", T_OK, "cover", json_array_get(j_result_media, 0));
    } else {
      // If no cover in the media file, get cover in the folder
      clause_where = msprintf("= (SELECT `tic_id` FROM `%s` WHERE `tf_id`= (SELECT `tf_id` FROM %s WHERE `tm_id`=%" JSON_INTEGER_FORMAT "))", TALIESIN_TABLE_FOLDER, TALIESIN_TABLE_MEDIA, tm_id);
      j_query = json_pack("{sss[sss]s{s{ssss}}}",
                          "table",
                          TALIESIN_TABLE_IMAGE_COVER,
                          "columns",
                            "tds_id",
                            thumbnail?"tic_cover_thumbnail AS thumbnail":"tic_path AS path",
                            "tic_id",
                          "where",
                            "tic_id",
                              "operator",
                              "raw",
                              "value",
                              clause_where);
      o_free(clause_where);
      res = h_select(config->conn, j_query, &j_result_folder, NULL);
      json_decref(j_query);
      if (res == H_OK) {
        if (json_array_size(j_result_folder) > 0) {
          if (!thumbnail) {
            j_data_source = data_source_get_by_id(config, json_integer_value(json_object_get(json_array_get(j_result_folder, 0), "tds_id")));
            if (check_result_value(j_data_source, T_OK)) {
              cover_path = msprintf("%s/%s", json_string_value(json_object_get(json_object_get(j_data_source, "data_source"), "path")), json_string_value(json_object_get(json_array_get(j_result_folder, 0), "path")));
              cover_full = media_get_cover_from_path(cover_path, &cover_size);
              if (cover_full != NULL) {
                cover_full_b64 = o_malloc(2 * cover_size * sizeof(char));
                if (cover_full_b64 != NULL) {
                  if (o_base64_encode(cover_full, cover_size, cover_full_b64, &cover_size_b64)) {
                    json_object_set_new(json_array_get(j_result_folder, 0), "full", json_stringn((const char *)cover_full_b64, cover_size_b64));
                  } else {
                    y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_by_id - Error o_base64_encode (2)");
                  }
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_by_id - Error allocating resources for cover_full_b64");
                }
                o_free(cover_full_b64);
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_by_id - Error media_get_cover_from_path for path %s (2)", cover_path);
              }
              o_free(cover_path);
              o_free(cover_full);
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_by_id - Error data_source_get_by_id (2)");
              j_return = json_pack("{si}", "result", T_ERROR);
            }
            json_decref(j_data_source);
          }
          j_return = json_pack("{sisO}", "result", T_OK, "cover", json_array_get(j_result_folder, 0));
        } else {
          j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
        }
        json_decref(j_result_folder);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_by_id - Error executing j_query (folder)");
        j_return = json_pack("{si}", "result", T_ERROR_DB);
      }
    }
    json_decref(j_result_media);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_cover_get_by_id - Error executing j_query (media)");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

int media_add_history(struct config_elements * config, const char * stream_name, json_int_t tpl_id, json_int_t tm_id) {
  json_t * j_query;
  int res;
  
  j_query = json_pack("{sss{sssI}}",
                      "table",
                      TALIESIN_TABLE_MEDIA_HISTORY,
                      "values",
                        "tmh_stream_name",
                        stream_name,
                        "tm_id",
                        tm_id);
  if (j_query != NULL) {
    if (tpl_id) {
      json_object_set_new(json_object_get(j_query, "values"), "tpl_id", json_integer(tpl_id));
    }
    res = h_insert(config->conn, j_query, NULL);
    json_decref(j_query);
    
    if (res == H_OK) {
      return T_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_add_history - Error executing j_query");
      return T_ERROR_DB;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_add_history - Error allocating resources for j_query");
    return T_ERROR_MEMORY;
  }
}

json_t * media_get_history(struct config_elements * config, const char * stream_name, json_int_t offset, json_int_t limit) {
  json_t * j_query, * j_result, * j_element, * j_media, * j_media_list, * j_return;
  int res;
  size_t index;
  
  j_query = json_pack("{sss[ss]s{ss}sIsIss}",
                      "table",
                      TALIESIN_TABLE_MEDIA_HISTORY,
                      "columns",
                        "tm_id",
                        config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(tmh_datestamp) AS tmh_datestamp":"tmh_datestamp",
                      "where",
                        "tmh_stream_name",
                        stream_name,
                      "offset",
                      offset,
                      "limit",
                      limit,
                      "order_by",
                      "tmh_datestamp DESC");
  if (j_query != NULL) {
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      j_media_list = json_array();
      if (j_media_list != NULL) {
        json_array_foreach(j_result, index, j_element) {
          j_media = media_get_by_id(config, json_integer_value(json_object_get(j_element, "tm_id")));
          if (check_result_value(j_media, T_OK)) {
            json_array_append_new(j_media_list, json_pack("{sIsO}", "datestamp", json_integer_value(json_object_get(j_element, "tmh_datestamp")), "media", json_object_get(j_media, "media")));
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_get_history - Error getting media %"JSON_INTEGER_FORMAT, json_integer_value(json_object_get(j_element, "tm_id")));
          }
          json_decref(j_media);
        }
        j_return = json_pack("{siso}", "result", T_OK, "history", j_media_list);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_get_history - Error allocating resources for j_media_list");
        j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
      }
      json_decref(j_result);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_get_history - Error executing j_query");
      j_return = json_pack("{si}", "result", T_ERROR_DB);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_get_history - Error allocating resources for j_query");
    j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
  }
  return j_return;
}

/**
 * Browse by category (artist, album, year, genre)
 */
json_t * media_category_get(struct config_elements * config, json_t * j_data_source, const char * level) {
  json_t * j_result, * j_return, * j_query;
  int res = H_ERROR;
  char * query, * clause_data_source;
  json_int_t tds_id = json_integer_value(json_object_get(j_data_source, "tds_id"));
  
  if (o_strcmp(level, "artist") == 0) {
    query = msprintf("SELECT DISTINCT(`name`) AS `name`, 'artist' AS `type` \
FROM (\
  SELECT (TRIM(`tmd_value`)) AS `name` FROM `" TALIESIN_TABLE_META_DATA "` WHERE LOWER(TRIM(`tmd_key`))='album_artist' AND `tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_MEDIA "` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")\
  UNION ALL\
  SELECT (TRIM(`tmd_value`)) AS `name` FROM `" TALIESIN_TABLE_META_DATA "` WHERE LOWER(TRIM(`tmd_key`))='artist' AND `tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_MEDIA "` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")\
) \
AS `t_union` \
ORDER BY `name`;", tds_id, tds_id);
    res = h_execute_query_json(config->conn, query, &j_result);
    o_free(query);
  } else if (o_strcmp(level, "album") == 0) {
    clause_data_source = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_MEDIA, tds_id);
    j_query = json_pack("{sss[ss]s{sss{ssss}}ss}",
                        "table",
                        TALIESIN_TABLE_META_DATA,
                        "columns",
                          "DISTINCT TRIM(`tmd_value`) AS name",
                          "'album' AS type",
                        "where",
                          "LOWER(TRIM(`tmd_key`))",
                          "album",
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_data_source,
                        "order_by",
                        "name");
    o_free(clause_data_source);
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
  } else if (o_strcmp(level, "year") == 0) {
    clause_data_source = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_MEDIA, tds_id);
    j_query = json_pack("{sss[ss]s{sss{ssss}}ss}",
                        "table",
                        TALIESIN_TABLE_META_DATA,
                        "columns",
                          config->conn->type==HOEL_DB_TYPE_MARIADB?"DISTINCT LEFT(TRIM(`tmd_value`), 4) AS `name`":"DISTINCT SUBSTR(TRIM(`tmd_value`), 0, 5) AS `name`",
                          "'year' AS type",
                        "where",
                          "LOWER(TRIM(`tmd_key`))",
                          "date",
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_data_source,
                        "order_by",
                        "name");
    o_free(clause_data_source);
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
  } else if (o_strcmp(level, "genre") == 0) {
    clause_data_source = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_MEDIA, tds_id);
    j_query = json_pack("{sss[ss]s{sss{ssss}}ss}",
                        "table",
                        TALIESIN_TABLE_META_DATA,
                        "columns",
                          "DISTINCT TRIM(`tmd_value`) AS name",
                          "'genre' AS type",
                        "where",
                          "LOWER(TRIM(`tmd_key`))",
                          "genre",
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_data_source,
                        "order_by",
                        "name");
    o_free(clause_data_source);
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
  }
  if (res == H_OK) {
    j_return = json_pack("{siso}", "result", T_OK, "list", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_category_get - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * media_category_list(struct config_elements * config, json_t * j_data_source, const char * level, const char * category, int with_id) {
  json_t * j_result, * j_return, * j_query, * j_element, * j_tags;
  int res = H_ERROR;
  char * clause_data_source, * escape_category = NULL, * clause_category = NULL;
  json_int_t tds_id = json_integer_value(json_object_get(j_data_source, "tds_id"));
  size_t index;
  
  clause_data_source = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_MEDIA, tds_id);
  escape_category = h_escape_string(config->conn, category);
  if (o_strcmp(level, "artist") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM " TALIESIN_TABLE_META_DATA " WHERE (`tmd_key`='artist' OR `tmd_key`='album_artist') AND TRIM(`tmd_value`)='%s')", escape_category);
  } else if (o_strcmp(level, "album") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM " TALIESIN_TABLE_META_DATA " WHERE `tmd_key`='album' AND TRIM(`tmd_value`)='%s')", escape_category);
  } else if (o_strcmp(level, "year") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM " TALIESIN_TABLE_META_DATA " WHERE `tmd_key`='date' AND %s(TRIM(`tmd_value`),%s)='%s')", config->conn->type==HOEL_DB_TYPE_MARIADB?"LEFT":"SUBSTR", config->conn->type==HOEL_DB_TYPE_MARIADB?"4":"0,5", escape_category);
  } else if (o_strcmp(level, "genre") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM " TALIESIN_TABLE_META_DATA " WHERE `tmd_key`='genre' AND TRIM(`tmd_value`)='%s')", escape_category);
  }
  j_query = json_pack("{sss[sssss]s{s{ssss}s{ssss}}ss}",
                      "table",
                      TALIESIN_TABLE_MEDIA,
                      "columns",
                        "`tm_id`",
                        "`tm_name` AS name",
                        "`tm_path` AS path",
                        "'media' AS type",
                        "`tm_duration` AS duration",
                      "where",
                        "  ",
                          "operator",
                          "raw",
                          "value",
                          clause_category,
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_data_source,
                      "order_by",
                      "path");
  o_free(clause_category);
  o_free(escape_category);
  o_free(clause_data_source);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      if (!with_id) {
        json_array_foreach(j_result, index, j_element) {
          json_object_set(j_element, "data_source", json_object_get(j_data_source, "name"));
          j_tags = media_get_tags_from_id(config, json_integer_value(json_object_get(j_element, "tm_id")));
          if (check_result_value(j_tags, T_OK)) {
            json_object_set(j_element, "tags", json_object_get(j_tags, "tags"));
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_category_list - Error media_get_tags_from_id");
          }
          json_decref(j_tags);
          json_object_del(j_element, "tm_id");
        }
      }
      j_return = json_pack("{siso}", "result", T_OK, "media", j_result);
    } else {
      json_decref(j_result);
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_category_list - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * media_subcategory_get(struct config_elements * config, json_t * j_data_source, const char * level, const char * category, const char * sublevel) {
  json_t * j_result, * j_return, * j_query;
  int res = H_ERROR;
  char * query, * clause_data_source, * escape_category, * clause_category = NULL;
  json_int_t tds_id = json_integer_value(json_object_get(j_data_source, "tds_id"));
  
  escape_category = h_escape_string(config->conn, category);
  if (o_strcmp(level, "artist") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_META_DATA "` WHERE (`tmd_key`='artist' OR `tmd_key`='album_artist') AND TRIM(`tmd_value`)='%s')", escape_category);
  } else if (o_strcmp(level, "album") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_META_DATA "` WHERE `tmd_key`='album' AND TRIM(`tmd_value`)='%s')", escape_category);
  } else if (o_strcmp(level, "year") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_META_DATA "` WHERE `tmd_key`='date' AND %s(TRIM(`tmd_value`),%s)='%s')", config->conn->type==HOEL_DB_TYPE_MARIADB?"LEFT":"SUBSTR", config->conn->type==HOEL_DB_TYPE_MARIADB?"4":"0,5", escape_category);
  } else if (o_strcmp(level, "genre") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_META_DATA "` WHERE `tmd_key`='genre' AND TRIM(`tmd_value`)='%s')", escape_category);
  }
  o_free(escape_category);
  
  if (o_strcmp(sublevel, "artist") == 0) {
    query = msprintf("SELECT DISTINCT(`name`) AS `name`, 'artist' AS `type` \
FROM (\
  SELECT (TRIM(`tmd_value`)) AS `name` FROM `" TALIESIN_TABLE_META_DATA "` WHERE LOWER(TRIM(`tmd_key`))='album_artist' AND `tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_MEDIA "` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ") AND %s\
  UNION ALL\
  SELECT (TRIM(`tmd_value`)) AS `name` FROM `" TALIESIN_TABLE_META_DATA "` WHERE LOWER(TRIM(`tmd_key`))='artist' AND `tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_MEDIA "` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ") AND %s\
) \
AS `t_union` \
ORDER BY `name`;", tds_id, clause_category, tds_id, clause_category);
    res = h_execute_query_json(config->conn, query, &j_result);
    o_free(query);
  } else if (o_strcmp(sublevel, "album") == 0) {
    clause_data_source = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_MEDIA, tds_id);
    j_query = json_pack("{sss[ss]s{sss{ssss}s{ssss}}ss}",
                        "table",
                        TALIESIN_TABLE_META_DATA,
                        "columns",
                          "DISTINCT TRIM(`tmd_value`) AS name",
                          "'album' AS type",
                        "where",
                          "LOWER(TRIM(`tmd_key`))",
                          "album",
                        "  ",
                          "operator",
                          "raw",
                          "value",
                          clause_category,
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_data_source,
                        "order_by",
                        "name");
    o_free(clause_data_source);
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
  } else if (o_strcmp(sublevel, "year") == 0) {
    clause_data_source = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_MEDIA, tds_id);
    j_query = json_pack("{sss[ss]s{sss{ssss}s{ssss}}ss}",
                        "table",
                        TALIESIN_TABLE_META_DATA,
                        "columns",
                          config->conn->type==HOEL_DB_TYPE_MARIADB?"DISTINCT LEFT(TRIM(`tmd_value`), 4) AS `name`":"DISTINCT SUBSTR(TRIM(`tmd_value`), 0, 5) AS `name`",
                          "'year' AS type",
                        "where",
                          "LOWER(TRIM(`tmd_key`))",
                          "date",
                        "  ",
                          "operator",
                          "raw",
                          "value",
                          clause_category,
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_data_source,
                        "order_by",
                        "name");
    o_free(clause_data_source);
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
  } else if (o_strcmp(sublevel, "genre") == 0) {
    clause_data_source = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_MEDIA, tds_id);
    j_query = json_pack("{sss[ss]s{sss{ssss}s{ssss}}ss}",
                        "table",
                        TALIESIN_TABLE_META_DATA,
                        "columns",
                          "DISTINCT TRIM(`tmd_value`) AS name",
                          "'genre' AS type",
                        "where",
                          "LOWER(TRIM(`tmd_key`))",
                          "genre",
                        "  ",
                          "operator",
                          "raw",
                          "value",
                          clause_category,
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_data_source,
                        "order_by",
                        "name");
    o_free(clause_data_source);
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
  }
  o_free(clause_category);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      j_return = json_pack("{siso}", "result", T_OK, "list", j_result);
    } else {
      json_decref(j_result);
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_subcategory_get - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * media_subcategory_list(struct config_elements * config, json_t * j_data_source, const char * level, const char * category, const char * sublevel, const char * subcategory, int with_id) {
  json_t * j_result, * j_return, * j_query, * j_element, * j_tags;
  int res = H_ERROR;
  char * clause_data_source, * escape_category = NULL, * clause_category = NULL, * escape_subcategory = NULL, * clause_subcategory = NULL;
  json_int_t tds_id = json_integer_value(json_object_get(j_data_source, "tds_id"));
  size_t index;
  
  escape_category = h_escape_string(config->conn, category);
  if (o_strcmp(level, "artist") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_META_DATA "` WHERE (`tmd_key`='artist' OR `tmd_key`='album_artist') AND TRIM(`tmd_value`)='%s')", escape_category);
  } else if (o_strcmp(level, "album") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_META_DATA "` WHERE `tmd_key`='album' AND TRIM(`tmd_value`)='%s')", escape_category);
  } else if (o_strcmp(level, "year") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_META_DATA "` WHERE `tmd_key`='date' AND %s(TRIM(`tmd_value`),%s)='%s')", config->conn->type==HOEL_DB_TYPE_MARIADB?"LEFT":"SUBSTR", config->conn->type==HOEL_DB_TYPE_MARIADB?"4":"0,5", escape_category);
  } else if (o_strcmp(level, "genre") == 0) {
    clause_category = msprintf("`tm_id` IN (SELECT `tm_id` FROM `" TALIESIN_TABLE_META_DATA "` WHERE `tmd_key`='genre' AND TRIM(`tmd_value`)='%s')", escape_category);
  }
  o_free(escape_category);

  escape_subcategory = h_escape_string(config->conn, subcategory);
  if (o_strcmp(sublevel, "artist") == 0) {
    clause_subcategory = msprintf("`tm_id` IN (SELECT `tm_id` FROM " TALIESIN_TABLE_META_DATA " WHERE (`tmd_key`='artist' OR `tmd_key`='album_artist') AND TRIM(`tmd_value`)='%s')", escape_subcategory);
  } else if (o_strcmp(sublevel, "album") == 0) {
    clause_subcategory = msprintf("`tm_id` IN (SELECT `tm_id` FROM " TALIESIN_TABLE_META_DATA " WHERE `tmd_key`='album' AND TRIM(`tmd_value`)='%s')", escape_subcategory);
  } else if (o_strcmp(sublevel, "year") == 0) {
    clause_subcategory = msprintf("`tm_id` IN (SELECT `tm_id` FROM " TALIESIN_TABLE_META_DATA " WHERE `tmd_key`='date' AND %s(TRIM(`tmd_value`),%s)='%s')", config->conn->type==HOEL_DB_TYPE_MARIADB?"LEFT":"SUBSTR", config->conn->type==HOEL_DB_TYPE_MARIADB?"4":"0,5", escape_subcategory);
  } else if (o_strcmp(sublevel, "genre") == 0) {
    clause_subcategory = msprintf("`tm_id` IN (SELECT `tm_id` FROM " TALIESIN_TABLE_META_DATA " WHERE `tmd_key`='genre' AND TRIM(`tmd_value`)='%s')", escape_subcategory);
  }
  clause_data_source = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_MEDIA, tds_id);
  j_query = json_pack("{sss[sssss]s{s{ssss}s{ssss}s{ssss}}ss}",
                      "table",
                      TALIESIN_TABLE_MEDIA,
                      "columns",
                        "`tm_id`",
                        "`tm_name` AS name",
                        "`tm_path` AS path",
                        "'media' AS type",
                        "`tm_duration` AS duration",
                      "where",
                        "   ",
                          "operator",
                          "raw",
                          "value",
                          clause_subcategory,
                        "  ",
                          "operator",
                          "raw",
                          "value",
                          clause_category,
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_data_source,
                      "order_by",
                      "path");
  o_free(clause_subcategory);
  o_free(escape_subcategory);
  o_free(clause_category);
  o_free(clause_data_source);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      if (!with_id) {
        json_array_foreach(j_result, index, j_element) {
          json_object_set(j_element, "data_source", json_object_get(j_data_source, "name"));
          j_tags = media_get_tags_from_id(config, json_integer_value(json_object_get(j_element, "tm_id")));
          if (check_result_value(j_tags, T_OK)) {
            json_object_set(j_element, "tags", json_object_get(j_tags, "tags"));
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_subcategory_list - Error media_get_tags_from_id");
          }
          json_decref(j_tags);
          json_object_del(j_element, "tm_id");
        }
      }
      j_return = json_pack("{siso}", "result", T_OK, "media", j_result);
    } else {
      json_decref(j_result);
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_subcategory_list - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * media_category_get_info(struct config_elements * config, json_t * j_data_source, const char * level, const char * category) {
  json_t * j_query, * j_result, * j_return;
  int res;
  
  j_query = json_pack("{sss[s]s{sssssI}}",
                      "table",
                      TALIESIN_TABLE_CATEGORY_INFO,
                      "columns",
                        "tci_content AS info",
                      "where",
                        "tci_level",
                        level,
                        "tci_category",
                        category,
                        "tds_id",
                        json_integer_value(json_object_get(j_data_source, "tds_id")));
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      j_return = json_pack("{sis{ss}}", "result", T_OK, "info", "info", json_string_value(json_object_get(json_array_get(j_result, 0), "info")));
    } else {
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_category_get_info - Error executing j_query");
    j_return = json_pack("result", T_ERROR_DB);
  }
  return j_return;
}

int media_image_cover_clean_orphan(struct config_elements * config, json_int_t tds_id, json_int_t tic_id) {
  int res;
  json_t * j_query;
  char * clause_where;
  
  if (tic_id != 0) {
    clause_where = msprintf("`tic_id`= %"JSON_INTEGER_FORMAT" AND NOT EXISTS (SELECT 1 FROM `"TALIESIN_TABLE_MEDIA"` WHERE `"TALIESIN_TABLE_MEDIA"`.`tic_id`=`"TALIESIN_TABLE_IMAGE_COVER"`.`tic_id`) AND NOT EXISTS (SELECT 1 FROM `"TALIESIN_TABLE_FOLDER"` WHERE `"TALIESIN_TABLE_FOLDER"`.`tic_id`=`"TALIESIN_TABLE_IMAGE_COVER"`.`tic_id`) AND NOT EXISTS (SELECT 1 FROM `"TALIESIN_TABLE_PLAYLIST"` WHERE `"TALIESIN_TABLE_PLAYLIST"`.`tic_id`=`"TALIESIN_TABLE_IMAGE_COVER"`.`tic_id`) AND NOT EXISTS (SELECT 1 FROM `"TALIESIN_TABLE_CATEGORY_INFO"` WHERE `"TALIESIN_TABLE_CATEGORY_INFO"`.`tic_id`=`"TALIESIN_TABLE_IMAGE_COVER"`.`tic_id`)", tic_id);
  } else {
    clause_where = o_strdup("NOT EXISTS (SELECT 1 FROM `"TALIESIN_TABLE_MEDIA"` WHERE `"TALIESIN_TABLE_MEDIA"`.`tic_id`=`"TALIESIN_TABLE_IMAGE_COVER"`.`tic_id`) AND NOT EXISTS (SELECT 1 FROM `"TALIESIN_TABLE_FOLDER"` WHERE `"TALIESIN_TABLE_FOLDER"`.`tic_id`=`"TALIESIN_TABLE_IMAGE_COVER"`.`tic_id`) AND NOT EXISTS (SELECT 1 FROM `"TALIESIN_TABLE_PLAYLIST"` WHERE `"TALIESIN_TABLE_PLAYLIST"`.`tic_id`=`"TALIESIN_TABLE_IMAGE_COVER"`.`tic_id`) AND NOT EXISTS (SELECT 1 FROM `"TALIESIN_TABLE_CATEGORY_INFO"` WHERE `"TALIESIN_TABLE_CATEGORY_INFO"`.`tic_id`=`"TALIESIN_TABLE_IMAGE_COVER"`.`tic_id`)");
  }
  
  j_query = json_pack("{sss{s{ssss}}}",
                      "table",
                      TALIESIN_TABLE_IMAGE_COVER,
                      "where",
                      " ",
                        "operator",
                        "raw",
                        "value",
                        clause_where);
  o_free(clause_where);
  if (tds_id) {
    json_object_set_new(json_object_get(j_query, "where"), "tds_id", json_integer(tds_id));
  } else {
    json_object_set_new(json_object_get(j_query, "where"), "tds_id", json_null());
  }
  
  res = h_delete(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    return T_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_image_cover_clean_orphan - Error executing query");
    return T_ERROR_DB;
  }
}

int media_category_delete_info(struct config_elements * config, json_t * j_data_source, const char * level, const char * category) {
  json_t * j_query, * j_result = NULL;
  int res, ret;
  
  j_query = json_pack("{sss[ss]s{sssssI}}",
                      "table",
                      TALIESIN_TABLE_CATEGORY_INFO,
                      "columns",
                        "tci_id",
                        "tic_id",
                      "where",
                        "tci_level",
                        level,
                        "tci_category",
                        category,
                        "tds_id",
                        json_integer_value(json_object_get(j_data_source, "tds_id")));
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      j_query = json_pack("{sss[s]s{sI}}",
                          "table",
                          TALIESIN_TABLE_CATEGORY_INFO,
                          "columns",
                            "tci_content AS info",
                          "where",
                            "tci_id",
                            json_integer_value(json_object_get(json_array_get(j_result, 0), "tci_id")));
      res = h_delete(config->conn, j_query, NULL);
      json_decref(j_query);
      if (res == H_OK) {
        if (json_object_get(json_array_get(j_result, 0), "tic_id") != json_null()) {
          ret = media_image_cover_clean_orphan(config, json_integer_value(json_object_get(j_data_source, "tds_id")), json_integer_value(json_object_get(json_array_get(j_result, 0), "tic_id")));
        } else {
          ret = T_OK;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_category_delete_info - Error executing j_query (2)");
        ret = T_ERROR_DB;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_category_delete_info - Error category not found");
      ret = T_ERROR_NOT_FOUND;
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_category_delete_info - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

int media_category_set_info(struct config_elements * config, json_t * j_data_source, const char * level, const char * category, json_t * j_info) {
  json_t * j_query, * j_result;
  int res, ret;
  
  j_query = json_pack("{sss[ss]s{sIssss}}",
                      "table",
                      TALIESIN_TABLE_CATEGORY_INFO,
                      "columns",
                        "tci_id",
                        "tic_id",
                      "where",
                        "tds_id",
                        json_integer_value(json_object_get(j_data_source, "tds_id")),
                        "tci_level",
                        level,
                        "tci_category",
                        category);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      j_query = json_pack("{sss{ss}s{sI}}",
                          "table",
                          TALIESIN_TABLE_CATEGORY_INFO,
                          "set",
                            "tci_content",
                            json_string_value(json_object_get(j_info, "content")),
                          "where",
                            "tci_id",
                            json_integer_value(json_object_get(json_array_get(j_result, 0), "tci_id")));
      res = h_update(config->conn, j_query, NULL);
      json_decref(j_query);
      if (res == H_OK) {
        if (json_object_get(json_array_get(j_result, 0), "tic_id") != json_null()) {
          ret = media_image_cover_clean_orphan(config, json_integer_value(json_object_get(j_data_source, "tds_id")), json_integer_value(json_object_get(json_array_get(j_result, 0), "tic_id")));
        } else {
          ret = T_OK;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_category_set_info - Error executing j_query (2)");
        ret = T_ERROR_DB;
      }
    } else {
      j_query = json_pack("{sss{sssIssss}}",
                          "table",
                          TALIESIN_TABLE_CATEGORY_INFO,
                          "values",
                            "tci_content",
                            json_string_value(json_object_get(j_info, "content")),
                            "tds_id",
                            json_integer_value(json_object_get(j_data_source, "tds_id")),
                            "tci_level",
                            level,
                            "tci_category",
                            category);
      res = h_insert(config->conn, j_query, NULL);
      json_decref(j_query);
      if (res == H_OK) {
        ret = T_OK;
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_category_set_info - Error executing j_query (3)");
        ret = T_ERROR_DB;
      }
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_category_set_info - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

int is_valid_b64_image(const unsigned char * base64_image) {
  AVFormatContext * image_format_context = NULL;
  AVCodecContext *  image_codec_context = NULL;
  int res;
  
  res = open_input_buffer(base64_image, &image_format_context, &image_codec_context, NULL, AVMEDIA_TYPE_VIDEO);
  
  if (image_codec_context) {
    avcodec_flush_buffers(image_codec_context);
    avcodec_free_context(&image_codec_context);
    image_codec_context = NULL;
  }
  if (image_format_context) {
    av_free(image_format_context->pb->buffer);
    avio_flush(image_format_context->pb);
    av_free(image_format_context->pb);
    avformat_close_input(&image_format_context);
  }
  
  return res;
}

json_t * is_media_category_info_valid(struct config_elements * config, json_t * j_info) {
  json_t * j_result = json_array();
  
  if (j_result != NULL) {
    if (j_info == NULL || !json_is_object(j_info)) {
      json_array_append_new(j_result, json_pack("{ss}", "body", "info body must be a valid JSON object"));
    } else {
      if (json_object_get(j_info, "content") == NULL && json_object_get(j_info, "cover") == NULL) {
        json_array_append_new(j_result, json_pack("{ss}", "data", "You must set a content or a cover or both"));
      }
        
      if (json_object_get(j_info, "content") != NULL && (!json_is_string(json_object_get(j_info, "content")) || json_string_length(json_object_get(j_info, "content")) > TALIESIN_MEDIUMBLOB_MAX_SIZE)) {
        json_array_append_new(j_result, json_pack("{ss}", "content", "content is mandatory and must be a valid string of maximum 16MB"));
      }
      
      if (json_object_get(j_info, "cover") != NULL && (!json_is_string(json_object_get(j_info, "cover")) || is_valid_b64_image((const unsigned char *)json_string_value(json_object_get(j_info, "cover"))) != T_OK)) {
        json_array_append_new(j_result, json_pack("{ss}", "cover", "cover is optional and must be a valid image encoded in Base64 format"));
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "is_media_category_info_valid - Error allocating resources for j_result");
  }
  return j_result;
}

int is_valid_path_element_parameter(struct config_elements * config, json_t * jukebox_element, const char * username, int is_admin) {
  json_t * j_data_source, * j_media;
  int res;
  char * path;
  
  if (jukebox_element != NULL &&
      json_is_object(jukebox_element) &&
      json_object_get(jukebox_element, "data_source") != NULL &&
      json_is_string(json_object_get(jukebox_element, "data_source")) &&
      json_object_get(jukebox_element, "path") != NULL &&
      json_is_string(json_object_get(jukebox_element, "path")) &&
      (json_object_get(jukebox_element, "recursive") == NULL || json_is_boolean(json_object_get(jukebox_element, "recursive")))) {
    j_data_source = data_source_get(config, is_admin?NULL:username, json_string_value(json_object_get(jukebox_element, "data_source")), 1);
    if (check_result_value(j_data_source, T_OK)) {
      path = (char *)json_string_value(json_object_get(jukebox_element, "path"));
      while (path != NULL && path[0] != '\0' && path[0] == '/') {
        path++;
      }
      j_media = media_get_full(config, json_object_get(j_data_source, "data_source"), path);
      res = check_result_value(j_media, T_OK);
      if (!res && !check_result_value(j_media, T_ERROR_NOT_FOUND)) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error get media %s/%s: %d", json_string_value(json_object_get(jukebox_element, "data_source")), json_string_value(json_object_get(jukebox_element, "path")), res);
      } else {
        json_object_set(jukebox_element, "tm_id", json_object_get(json_object_get(j_media, "media"), "tm_id"));
      }
      json_decref(j_media);
    } else if (!check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error get data source %s %s", username, json_string_value(json_object_get(jukebox_element, "data_source")));
      res = 0;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error data_source_get");
      res = 0;
    }
    json_decref(j_data_source);
  } else {
    res = 0;
  }
  return res;
}

static int is_valid_category_name(const char * category) {
  return category != NULL && (0 == o_strcmp("artist", category) || 0 == o_strcmp("album", category) || 0 == o_strcmp("year", category) || 0 == o_strcmp("genre", category));
}

int is_valid_category_element_parameter(struct config_elements * config, json_t * category_element, const char * username, int is_admin) {
  json_t * j_data_source;
  int res;
  
  if (
        category_element != NULL &&
        json_is_object(category_element) &&
        json_object_get(category_element, "data_source") != NULL &&
        json_is_string(json_object_get(category_element, "data_source")) &&
        json_object_get(category_element, "category") != NULL &&
        json_is_string(json_object_get(category_element, "category")) &&
        is_valid_category_name(json_string_value(json_object_get(category_element, "category"))) &&
        json_object_get(category_element, "category_value") != NULL &&
        json_is_string(json_object_get(category_element, "category_value")) &&
        (
          json_object_get(category_element, "sub_category") == NULL ||
          (
            json_is_string(json_object_get(category_element, "sub_category")) &&
            is_valid_category_name(json_string_value(json_object_get(category_element, "sub_category"))) &&
            json_object_get(category_element, "sub_category_value") != NULL &&
            json_is_string(json_object_get(category_element, "sub_category_value"))
          )
        )
      ) {
    j_data_source = data_source_get(config, is_admin?NULL:username, json_string_value(json_object_get(category_element, "data_source")), 1);
    if (check_result_value(j_data_source, T_OK)) {
      res = 1;
    } else if (!check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error get data source %s %s", username, json_string_value(json_object_get(category_element, "data_source")));
      res = 0;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error data_source_get");
      res = 0;
    }
    json_decref(j_data_source);
  } else {
    res = 0;
  }
  return res;
}

int is_valid_playlist_element_parameter(struct config_elements * config, json_t * j_playlist, const char * username) {
  json_t * j_db_playlist;
  int res;
  
  if (j_playlist != NULL && json_is_object(j_playlist) && json_object_get(j_playlist, "playlist") != NULL && json_is_string(json_object_get(j_playlist, "playlist")) && json_string_length(json_object_get(j_playlist, "playlist")) > 0) {
    j_db_playlist = playlist_get(config, username, json_string_value(json_object_get(j_playlist, "playlist")), 0, 0, 1);
    if (check_result_value(j_db_playlist, T_OK)) {
      res = 1;
    } else if (check_result_value(j_db_playlist, T_ERROR_NOT_FOUND)) {
      res = 0;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "is_valid_playlist_element_parameter - Error playlist_get");
      res = 0;
    }
    json_decref(j_db_playlist);
  } else {
    res = 0;
  }
  return res;
}

json_t * media_append_list_to_media_list(struct config_elements * config, json_t * append_list, const char * username) {
  json_t * j_element, * j_return, * j_media_list, * j_result, * j_data_source;
  size_t index;
  
  j_media_list = json_array();
  if (j_media_list != NULL) {
    json_array_foreach(append_list, index, j_element) {
      if (json_object_get(j_element, "path") != NULL) {
        j_data_source = data_source_get(config, username, json_string_value(json_object_get(j_element, "data_source")), 1);
        if (check_result_value(j_data_source, T_OK)) {
          j_result = media_get_audio_list_from_path(config, json_object_get(j_data_source, "data_source"), json_string_value(json_object_get(j_element, "path")), (json_object_get(j_element, "recursive") == json_true()));
          if (check_result_value(j_result, T_OK)) {
            json_array_extend(j_media_list, json_object_get(j_result, "media"));
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_append_list_to_media_list - Error media_get_audio_list_from_path");
          }
          json_decref(j_result);
        } else if (!check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_append_list_to_media_list - Error data_source_get (1)");
        }
        json_decref(j_data_source);
      } else if (json_object_get(j_element, "sub_category") != NULL) {
        j_data_source = data_source_get(config, username, json_string_value(json_object_get(j_element, "data_source")), 1);
        if (check_result_value(j_data_source, T_OK)) {
          j_result = media_subcategory_list(config, json_object_get(j_data_source, "data_source"), json_string_value(json_object_get(j_element, "category")), json_string_value(json_object_get(j_element, "category_value")), json_string_value(json_object_get(j_element, "sub_category")), json_string_value(json_object_get(j_element, "sub_category_value")), 1);
          if (check_result_value(j_result, T_OK)) {
            json_array_extend(j_media_list, json_object_get(j_result, "media"));
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_append_list_to_media_list - Error media_subcategory_list");
          }
          json_decref(j_result);
        } else if (!check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_append_list_to_media_list - Error data_source_get (3)");
        }
        json_decref(j_data_source);
      } else if (json_object_get(j_element, "category") != NULL) {
        j_data_source = data_source_get(config, username, json_string_value(json_object_get(j_element, "data_source")), 1);
        if (check_result_value(j_data_source, T_OK)) {
          j_result = media_category_list(config, json_object_get(j_data_source, "data_source"), json_string_value(json_object_get(j_element, "category")), json_string_value(json_object_get(j_element, "category_value")), 1);
          if (check_result_value(j_result, T_OK)) {
            json_array_extend(j_media_list, json_object_get(j_result, "media"));
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "media_append_list_to_media_list - Error media_category_list");
          }
          json_decref(j_result);
        } else if (!check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_append_list_to_media_list - Error data_source_get (2)");
        }
        json_decref(j_data_source);
      } else { // playlist
        j_result = playlist_get(config, username, json_string_value(json_object_get(j_element, "playlist")), 1, 0, 0);
        if (check_result_value(j_result, T_OK)) {
          json_array_extend(j_media_list, json_object_get(json_object_get(j_result, "playlist"), "media"));
        } else if (!check_result_value(j_result, T_ERROR_NOT_FOUND)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_append_list_to_media_list - Error playlist_get");
        }
        json_decref(j_result);
      }
    }
    j_return = json_pack("{sisO}", "result", T_OK, "media", j_media_list);
    json_decref(j_media_list);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_append_list_to_media_list - Error allocating resources for j_media_list");
    j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
  }
  return j_return;
}
