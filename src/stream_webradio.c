/**
 *
 * Taliesin - Media server
 *
 * Stream functions definitions for webradio streaming
 *
 * Copyright 2017-2022 Nicolas Mora <mail@babelouest.org>
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

#include "taliesin.h"

void webradio_clean(struct _t_webradio * webradio) {
  if (webradio != NULL) {
    file_list_clean(webradio->file_list);
    audio_stream_clean(webradio->audio_stream);
    pthread_mutex_destroy(&(webradio->audio_stream->write_lock));
    pthread_mutex_destroy(&(webradio->audio_stream->transcode_lock));
    o_free(webradio->audio_stream);
    o_free(webradio->username);
    o_free(webradio->display_name);
    o_free(webradio->playlist_name);
    o_free(webradio);
  }
}

int webradio_init(struct _t_webradio * webradio, const char * stream_url, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate, short int icecast) {
  pthread_mutexattr_t mutexattr;
  int res = T_OK;

  if (webradio != NULL) {
    webradio->tpl_id = 0;
    if (o_strnullempty(stream_url)) {
      rand_string(webradio->name, TALIESIN_PLAYLIST_NAME_LENGTH);
    } else {
      memset(webradio->name, 0, TALIESIN_PLAYLIST_NAME_LENGTH+1);
      o_strncpy(webradio->name, stream_url, TALIESIN_PLAYLIST_NAME_LENGTH);
    }
    webradio->audio_stream = o_malloc(sizeof(struct _audio_stream));
    webradio->file_list = o_malloc(sizeof(struct _t_file_list));
    webradio->playlist_name = NULL;
    webradio->tpl_id = 0;
    webradio->busy = 0;
    if (webradio->audio_stream != NULL && webradio->file_list != NULL) {
      webradio->username = NULL;
      webradio->random = 0;
      webradio->icecast = icecast;
      webradio->display_name = NULL;
      webradio->current_index = 0;
      webradio->message_type = TALIESIN_PLAYLIST_MESSAGE_TYPE_NONE;
      webradio->nb_websocket = 0;
      if (file_list_init(webradio->file_list) == T_OK) {
        pthread_mutexattr_init ( &mutexattr );
        pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
        if (!pthread_mutex_init(&webradio->audio_stream->write_lock, &mutexattr) &&
            !pthread_mutex_init(&webradio->audio_stream->transcode_lock, &mutexattr) &&
            !pthread_mutex_init(&webradio->audio_stream->client_lock, NULL) &&
            !pthread_cond_init(&webradio->audio_stream->client_cond, NULL) &&
            !pthread_mutex_init(&webradio->audio_stream->stream_lock, NULL) &&
            !pthread_cond_init(&webradio->audio_stream->stream_cond, NULL) &&
            !pthread_mutex_init(&webradio->message_lock, NULL) &&
            !pthread_cond_init(&webradio->message_cond, NULL) &&
            !pthread_mutex_init(&webradio->websocket_lock, NULL) &&
            !pthread_cond_init(&webradio->websocket_cond, NULL)) {
          webradio->audio_stream->client_list = NULL;
          webradio->audio_stream->nb_client_connected = 0;
          webradio->audio_stream->first_buffer = NULL;
          webradio->audio_stream->last_buffer = NULL;
          webradio->audio_stream->transcode_status = TALIESIN_STREAM_TRANSCODE_STATUS_NOT_STARTED;
          webradio->audio_stream->nb_buffer = 0;
          webradio->audio_stream->status = TALIESIN_STREAM_STATUS_NOT_STARTED;
          webradio->audio_stream->stream_format = o_strdup(format);
          webradio->audio_stream->stream_channels = channels;
          webradio->audio_stream->stream_sample_rate = sample_rate;
          webradio->audio_stream->stream_bitrate = bit_rate;
          webradio->audio_stream->output_format_context = NULL;
          webradio->audio_stream->is_header = 0;
          webradio->audio_stream->header_buffer = o_malloc(sizeof(struct _audio_buffer));
          if (webradio->audio_stream->header_buffer != NULL && audio_buffer_init(webradio->audio_stream->header_buffer) == T_OK) {
            webradio->audio_stream->header_buffer->data = o_malloc(TALIESIN_STREAM_HEADER_SIZE);
            webradio->audio_stream->header_buffer->max_size = TALIESIN_STREAM_HEADER_SIZE;
            if (webradio->audio_stream->header_buffer->data == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "webradio_init - Error allocating resources for header_buffer->data");
              o_free(webradio->audio_stream);
              o_free(webradio->file_list);
              pthread_mutex_destroy(&(webradio->audio_stream->write_lock));
              pthread_mutex_destroy(&(webradio->audio_stream->transcode_lock));
              res = T_ERROR_MEMORY;
            } else {
              if (!webradio_open_output_buffer(webradio->audio_stream)) {
                webradio->audio_stream->pts = 0;
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "webradio_init - Error webradio_open_output_buffer");
                o_free(webradio->audio_stream);
                o_free(webradio->file_list);
                pthread_mutex_destroy(&(webradio->audio_stream->write_lock));
                pthread_mutex_destroy(&(webradio->audio_stream->transcode_lock));
                res = T_ERROR_MEMORY;
              }
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "webradio_init - Error allocating resources for header_buffer");
            o_free(webradio->audio_stream);
            o_free(webradio->file_list);
            res = T_ERROR_MEMORY;
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio_init - Error init pthread_mutex or client_lock or client_cond of stream_lock or stream_cond");
          o_free(webradio->audio_stream);
          o_free(webradio->file_list);
          res = T_ERROR_MEMORY;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_init - Error allocating resources for header_buffer");
        o_free(webradio->audio_stream);
        o_free(webradio->file_list);
        res = T_ERROR;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_init - Error allocating resources for audio_stream, file_list");
      o_free(webradio->audio_stream);
      o_free(webradio->file_list);
      res = T_ERROR_MEMORY;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_init - Error input parameters");
    res = T_ERROR_PARAM;
  }
  return res;
}

int client_data_webradio_init(struct _client_data_webradio * client_data) {
  if (client_data != NULL) {
    client_data->audio_stream = NULL;
    client_data->current_buffer = NULL;
    client_data->client_address = NULL;
    client_data->user_agent = NULL;
    client_data->metadata_send = 0;
    client_data->metadata_offset = 0;
    client_data->metadata_len = 0;
    client_data->metadata_current_offset = 0;
    client_data->metadata_buffer = NULL;
    client_data->global_offset = 0;
    client_data->start.tv_sec = 0;
    client_data->start.tv_nsec = 0;
    client_data->send_header = 1;
    client_data->header_offset = 0;
    client_data->command = TALIESIN_STREAM_COMMAND_NONE;
    return T_OK;
  } else {
    return T_ERROR_PARAM;
  }
}

void client_data_webradio_clean(struct _client_data_webradio * client_data) {
  if (client_data != NULL) {
    o_free(client_data->client_address);
    o_free(client_data->user_agent);
    client_data->client_address = NULL;
    client_data->user_agent = NULL;
    o_free(client_data);
  }
}

struct _t_file * webradio_get_next_file(struct _t_webradio * webradio, unsigned int * index) {
  struct _t_file * next_file = NULL;
  struct _audio_stream * stream = webradio->audio_stream;

  if (index != NULL) {
    if (stream->nb_buffer >= (TALIESIN_STREAM_BUFFER_MAX)) {
      while (!stream->first_buffer->read && stream->status == TALIESIN_STREAM_STATUS_STARTED) {
        stream->transcode_status = TALIESIN_STREAM_TRANSCODE_STATUS_PAUSED;
        pthread_mutex_lock(&stream->stream_lock);
        pthread_cond_wait(&stream->stream_cond, &stream->stream_lock);
        pthread_mutex_unlock(&stream->stream_lock);
        stream->transcode_status = TALIESIN_STREAM_TRANSCODE_STATUS_STARTED;
        y_log_message(Y_LOG_LEVEL_DEBUG, "signal received %d", stream->first_buffer->read);
      }
    }
    if (webradio->random) {
      if (webradio->file_list->nb_files > 0) {
        *index = (unsigned int)random_at_most(webradio->file_list->nb_files - 1);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_get_next_file - webradio %s (%s), error no file in list", webradio->name, webradio->display_name);
        return NULL;
      }
    } else {
      if (webradio->current_index >= webradio->file_list->nb_files) {
        webradio->current_index = 0;
      }
      *index = (unsigned int)webradio->current_index++;
    }
    next_file = file_list_get_file(webradio->file_list, *index);
  }
  return next_file;
}

struct _update_db_thread {
  struct config_elements * config;
  struct _t_webradio * webradio;
};

static void * webradio_update_db_stream_media_list_thread(void * args) {
  json_t * j_query, * j_result;
  json_int_t ts_id;
  int res;
  struct _t_file * file;

  if (args != NULL) {
    struct config_elements * config = ((struct _update_db_thread *)args)->config;
    struct _t_webradio * webradio = ((struct _update_db_thread *)args)->webradio;

    j_query = json_pack("{sss[s]s{ss}}",
                        "table",
                        TALIESIN_TABLE_STREAM,
                        "columns",
                          "ts_id",
                        "where",
                          "ts_name",
                          webradio->name);
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      if (json_array_size(j_result) > 0) {
        ts_id = json_integer_value(json_object_get(json_array_get(j_result, 0), "ts_id"));
        j_query = json_pack("{sss{sI}}",
                            "table",
                            TALIESIN_TABLE_STREAM_ELEMENT,
                            "where",
                              "ts_id",
                              ts_id);
        res = h_delete(config->conn, j_query, NULL);
        json_decref(j_query);
        if (res == H_OK) {
          j_query = json_pack("{sss[]}",
                              "table",
                              TALIESIN_TABLE_STREAM_ELEMENT,
                              "values");
          if (j_query != NULL) {
            file = webradio->file_list->start;
            while (file != NULL) {
              json_array_append_new(json_object_get(j_query, "values"), json_pack("{sIsI}", "ts_id", ts_id, "tm_id", file->tm_id));
              file = file->next;
            }
            res = h_insert(config->conn, j_query, NULL);
            json_decref(j_query);
            if (res != H_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "webradio_update_db_stream_media_list_thread - Error executing j_query (3)");
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "webradio_update_db_stream_media_list_thread - Error allocatinng resources for j_query");
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio_update_db_stream_media_list_thread - Error executing j_query (2)");
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_update_db_stream_media_list_thread - stream not found");
      }
      json_decref(j_result);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_update_db_stream_media_list_thread - Error executing j_query (1)");
    }
    webradio->busy = 0;
    o_free(args);
  }
  return NULL;
}

static int webradio_update_db_stream_media_list(struct config_elements * config, struct _t_webradio * webradio) {
  int ret;
  int ret_thread_update_db = 0, detach_thread_update_db = 0;
  pthread_t thread_update_db;
  struct _update_db_thread * args = o_malloc(sizeof(struct _update_db_thread));

  if (args != NULL) {
    args->config = config;
    args->webradio = webradio;
    ret_thread_update_db = pthread_create(&thread_update_db, NULL, webradio_update_db_stream_media_list_thread, (void *)args);
    detach_thread_update_db = pthread_detach(thread_update_db);
    if (ret_thread_update_db || detach_thread_update_db) {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_update_db_stream_media_list - Error running thread update_db");
      ret = T_ERROR;
    } else {
      ret = T_OK;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_update_db_stream_media_list - Error allocating resources for args");
    ret = T_ERROR_MEMORY;
  }
  return ret;
}

static int webradio_delete_db_stream_media(struct config_elements * config, struct _t_webradio * webradio, json_int_t tm_id) {
  json_t * j_query, * j_result;
  json_int_t ts_id;
  int res, ret;

  j_query = json_pack("{sss[s]s{ss}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "columns",
                        "ts_id",
                      "where",
                        "ts_name",
                        webradio->name);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      ts_id = json_integer_value(json_object_get(json_array_get(j_result, 0), "ts_id"));
      j_query = json_pack("{sss{sIsI}}", "table", TALIESIN_TABLE_STREAM_ELEMENT, "where", "ts_id", ts_id, "tm_id", tm_id);
      res = h_delete(config->conn, j_query, NULL);
      json_decref(j_query);
      if (res == H_OK) {
        ret = T_OK;
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_delete_db_stream_media - Error executing j_query (2)");
        ret = T_ERROR_DB;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_delete_db_stream_media - stream not found");
      ret = T_ERROR_NOT_FOUND;
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_delete_db_stream_media - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

static int webradio_delete_db_stream_media_list(struct config_elements * config, struct _t_webradio * webradio, json_t * tm_id_array) {
  json_t * j_query, * j_result;
  json_int_t ts_id;
  int res, ret;

  j_query = json_pack("{sss[s]s{ss}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "columns",
                        "ts_id",
                      "where",
                        "ts_name",
                        webradio->name);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      ts_id = json_integer_value(json_object_get(json_array_get(j_result, 0), "ts_id"));
      j_query = json_pack("{sss{sIs{sssO}}}", "table", TALIESIN_TABLE_STREAM_ELEMENT, "where", "ts_id", ts_id, "tm_id", "operator", "IN", "value", tm_id_array);
      res = h_delete(config->conn, j_query, NULL);
      json_decref(j_query);
      if (res == H_OK) {
        ret = T_OK;
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_delete_db_stream_media_list - Error executing j_query (2)");
        ret = T_ERROR_DB;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_delete_db_stream_media_list - stream not found");
      ret = T_ERROR_NOT_FOUND;
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_delete_db_stream_media_list - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

static int webradio_add_db_stream(struct config_elements * config, struct _t_webradio * webradio) {
  json_t * j_query, * j_last_id;
  int res, ret;
  json_int_t ts_id;
  struct _t_file * file;

  j_query = json_pack("{sss{sosssssosisisisssisisi}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "values",
                        "ts_username",
                        webradio->username!=NULL?json_string(webradio->username):json_null(),
                        "ts_name",
                        webradio->name,
                        "ts_display_name",
                        webradio->display_name,
                        "tpl_id",
                        webradio->tpl_id?json_integer(webradio->tpl_id):json_null(),
                        "ts_webradio",
                        webradio->icecast?2:1,
                        "ts_random",
                        webradio->random,
                        "ts_index",
                        webradio->current_index,
                        "ts_format",
                        webradio->audio_stream->stream_format,
                        "ts_channels",
                        webradio->audio_stream->stream_channels,
                        "ts_sample_rate",
                        webradio->audio_stream->stream_sample_rate,
                        "ts_bitrate",
                        webradio->audio_stream->stream_bitrate);
  res = h_insert(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    j_last_id = h_last_insert_id(config->conn);
    if (j_last_id != NULL) {
      ts_id = json_integer_value(j_last_id);
      json_decref(j_last_id);
      j_query = json_pack("{sss[]}",
                          "table",
                          TALIESIN_TABLE_STREAM_ELEMENT,
                          "values");
      if (j_query != NULL) {
        file = webradio->file_list->start;
        while (file != NULL) {
          json_array_append_new(json_object_get(j_query, "values"), json_pack("{sIsI}", "ts_id", ts_id, "tm_id", file->tm_id));
          file = file->next;
        }
        res = h_insert(config->conn, j_query, NULL);
        json_decref(j_query);
        if (res == H_OK) {
          ret = T_OK;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_db_stream - Error executing j_query (2)");
          ret = T_ERROR_DB;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_db_stream - Error allocating resources for j_query");
        ret = T_ERROR_MEMORY;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_db_stream - Error getting last id");
      ret = T_ERROR_DB;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_db_stream - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

static int webradio_set_display_name_db_stream(struct config_elements * config, const char * name, const char * display_name) {
  json_t * j_query;
  int res;

  j_query = json_pack("{sss{ss}s{ss}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "set",
                        "ts_display_name",
                        display_name,
                      "where",
                        "ts_name",
                        name);
  res = h_update(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    return T_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_set_display_name_db_stream - Error executing j_query");
    return T_ERROR_DB;
  }
}

static int webradio_set_name_db_stream(struct config_elements * config, const char * old_name, const char * new_name) {
  json_t * j_query;
  int res;

  j_query = json_pack("{sss{ss}s{ss}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "set",
                        "ts_name",
                        new_name,
                      "where",
                        "ts_name",
                        old_name);
  res = h_update(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    return T_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_set_name_db_stream - Error executing j_query");
    return T_ERROR_DB;
  }
}

static int webradio_set_playlist_db_stream(struct config_elements * config, const char * name, json_int_t tpl_id) {
  json_t * j_query;
  int res;

  j_query = json_pack("{sss{sI}s{ss}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "set",
                        "tpl_id",
                        tpl_id,
                      "where",
                        "ts_name",
                        name);
  res = h_update(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    return T_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_set_playlist_db_stream - Error executing j_query");
    return T_ERROR_DB;
  }
}

static int webradio_remove_db_stream(struct config_elements * config, const char * name) {
  json_t * j_query;
  int res;

  j_query = json_pack("{sss{ss}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "where",
                        "ts_name",
                        name);
  res = h_delete(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    return T_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_remove_db_stream - Error executing j_query");
    return T_ERROR_DB;
  }
}

json_t * add_webradio_from_path(struct config_elements * config,
                                const char * stream_url,
                                json_t * j_data_source,
                                const char * path,
                                const char * username,
                                const char * format,
                                unsigned short channels,
                                unsigned int sample_rate,
                                unsigned int bit_rate,
                                int recursive,
                                short int random,
                                short int icecast,
                                const char * name,
                                struct _t_webradio ** new_webradio) {
  json_t * j_result = NULL, * j_media_list, * j_element;
  size_t index;
  int webradio_index;
  const char * display_name;

  j_media_list = media_scan_path(config, j_data_source, path, recursive);
  if (j_media_list != NULL && json_array_size(json_object_get(j_media_list, "media_list")) > 0) {
    if (!pthread_mutex_lock(&config->playlist_lock)) {
      config->webradio_set = o_realloc(config->webradio_set, (config->nb_webradio + 1) * sizeof(struct _t_webradio *));
      if (config->webradio_set != NULL) {
        webradio_index = (int)config->nb_webradio;
        config->nb_webradio++;
        config->webradio_set[webradio_index] = o_malloc(sizeof(struct _t_webradio));
        if (config->webradio_set[webradio_index] != NULL) {
          if (name == NULL) {
            display_name = strrchr(path, '/')!=NULL?(strrchr(path, '/') + 1):path;
          } else {
            display_name = name;
          }
          if (o_strlen(display_name) <= 0) {
            display_name = json_string_value(json_object_get(j_data_source, "name"));
          }
          if (webradio_init(config->webradio_set[webradio_index], stream_url, format, channels, sample_rate, bit_rate, icecast) == T_OK) {
            config->webradio_set[webradio_index]->config = config;
            config->webradio_set[webradio_index]->username = o_strdup(username);
            config->webradio_set[webradio_index]->random = random;
            config->webradio_set[webradio_index]->icecast = icecast;
            config->webradio_set[webradio_index]->display_name = o_strdup(display_name);
            json_array_foreach(json_object_get(j_media_list, "media_list"), index, j_element) {
              if (file_list_enqueue_new_file(config->webradio_set[webradio_index]->file_list, json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_path - Error adding file %s", json_string_value(json_object_get(j_element, "full_path")));
              }
            }
            if (new_webradio != NULL) {
              *new_webradio = config->webradio_set[webradio_index];
            }
            if (webradio_add_db_stream(config, config->webradio_set[webradio_index]) == T_OK) {
              j_result = json_pack("{sis{sssssssosisisosisosos[]ss}}",
                                    "result", T_OK,
                                    "stream",
                                      "name", config->webradio_set[webradio_index]->name,
                                      "display_name", config->webradio_set[webradio_index]->display_name,
                                      "format", format,
                                      "stereo", channels==1?json_false():json_true(),
                                      "sample_rate", sample_rate,
                                      "bitrate", bit_rate,
                                      "webradio", json_true(),
                                      "elements", config->webradio_set[webradio_index]->file_list->nb_files,
                                      "random", random?json_true():json_false(),
                                      "icecast", icecast?json_true():json_false(),
                                      "clients",
                                      "scope",
                                        username!=NULL?"me":"all");
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_path - Error webradio_add_db_stream");
              j_result = json_pack("{si}", "result", T_ERROR);
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_path - Error webradio_init");
            j_result = json_pack("{si}", "result", T_ERROR);
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_path - Error malloc new playlist");
          j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_path - Error realloc webradio_set");
        j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
      }
      pthread_mutex_unlock(&config->playlist_lock);
    } else {
      j_result = json_pack("{si}", "result", T_ERROR);
    }
  } else if (j_media_list == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_path - Error getting j_media_list");
    j_result = json_pack("{si}", "result", T_ERROR);
  } else {
    j_result = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
  }
  json_decref(j_media_list);
  return j_result;
}

json_t * add_webradio_from_playlist(struct config_elements * config,
                                    const char * stream_url,
                                    json_t * j_playlist,
                                    const char * username,
                                    const char * format,
                                    unsigned short channels,
                                    unsigned int sample_rate,
                                    unsigned int bit_rate,
                                    short int random,
                                    short int icecast,
                                    const char * name,
                                    struct _t_webradio ** new_webradio) {
  json_t * j_result = NULL, * j_element;
  size_t index;
  int webradio_index;

  if (!pthread_mutex_lock(&config->playlist_lock)) {
    config->webradio_set = o_realloc(config->webradio_set, (config->nb_webradio + 1) * sizeof(struct _t_webradio *));
    if (config->webradio_set != NULL) {
      webradio_index = (int)config->nb_webradio;
      config->nb_webradio++;
      config->webradio_set[webradio_index] = o_malloc(sizeof(struct _t_webradio));
      if (config->webradio_set[webradio_index] != NULL) {
        if (webradio_init(config->webradio_set[webradio_index], stream_url, format, channels, sample_rate, bit_rate, icecast) == T_OK) {
          config->webradio_set[webradio_index]->config = config;
          config->webradio_set[webradio_index]->username = o_strdup(username);
          config->webradio_set[webradio_index]->random = random;
          config->webradio_set[webradio_index]->icecast = icecast;
          if (name == NULL) {
            config->webradio_set[webradio_index]->display_name = o_strdup(json_string_value(json_object_get(j_playlist, "name")));
          } else {
            config->webradio_set[webradio_index]->display_name = o_strdup(name);
          }
          config->webradio_set[webradio_index]->playlist_name = o_strdup(json_string_value(json_object_get(j_playlist, "name")));
          config->webradio_set[webradio_index]->tpl_id = json_integer_value(json_object_get(j_playlist, "tpl_id"));
          json_array_foreach(json_object_get(j_playlist, "media"), index, j_element) {
            if (file_list_enqueue_new_file(config->webradio_set[webradio_index]->file_list, json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error adding file %s", json_string_value(json_object_get(j_element, "full_path")));
            }
          }
          if (new_webradio != NULL) {
            *new_webradio = config->webradio_set[webradio_index];
          }
          if (webradio_add_db_stream(config, config->webradio_set[webradio_index]) == T_OK) {
            j_result = json_pack("{sis{sssssssosisisosisosos[]ss}}",
                                  "result", T_OK,
                                  "stream",
                                    "name", config->webradio_set[webradio_index]->name,
                                    "display_name", config->webradio_set[webradio_index]->display_name,
                                    "format", format,
                                    "stereo", channels==1?json_false():json_true(),
                                    "sample_rate", sample_rate,
                                    "bitrate", bit_rate,
                                    "webradio", json_true(),
                                    "elements", config->webradio_set[webradio_index]->file_list->nb_files,
                                    "random", random?json_true():json_false(),
                                    "icecast", icecast?json_true():json_false(),
                                    "clients",
                                    "scope",
                                      username!=NULL?"me":"all");
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error webradio_add_db_stream");
            j_result = json_pack("{si}", "result", T_ERROR);
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error webradio_init");
          j_result = json_pack("{si}", "result", T_ERROR);
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error malloc new playlist");
        j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error realloc webradio_set");
      j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
    }
    pthread_mutex_unlock(&config->playlist_lock);
  } else {
    j_result = json_pack("{si}", "result", T_ERROR);
  }
  return j_result;
}

int add_webradio_from_db_stream(struct config_elements * config, json_t * j_stream, struct _t_webradio ** new_webradio) {
  json_t * j_element, * j_playlist;
  size_t index;
  int webradio_index, ret;
  short int icecast = (short int)(json_integer_value(json_object_get(j_stream, "webradio"))==2);

  if (!pthread_mutex_lock(&config->playlist_lock)) {
    config->webradio_set = o_realloc(config->webradio_set, (config->nb_webradio + 1) * sizeof(struct _t_webradio *));
    if (config->webradio_set != NULL) {
      webradio_index = (int)config->nb_webradio;
      config->nb_webradio++;
      config->webradio_set[webradio_index] = o_malloc(sizeof(struct _t_webradio));
      if (config->webradio_set[webradio_index] != NULL) {
        if (webradio_init(config->webradio_set[webradio_index], json_string_value(json_object_get(j_stream, "name")), json_string_value(json_object_get(j_stream, "format")), (short unsigned int)json_integer_value(json_object_get(j_stream, "channels")), (unsigned int)json_integer_value(json_object_get(j_stream, "sample_rate")), (unsigned int)json_integer_value(json_object_get(j_stream, "bitrate")), icecast) == T_OK) {
          config->webradio_set[webradio_index]->config = config;
          config->webradio_set[webradio_index]->username = json_object_get(j_stream, "username")!=json_null()?o_strdup(json_string_value(json_object_get(j_stream, "username"))):NULL;
          config->webradio_set[webradio_index]->display_name = o_strdup(json_string_value(json_object_get(j_stream, "display_name")));
          config->webradio_set[webradio_index]->random = (short int)json_integer_value(json_object_get(j_stream, "random"));
          config->webradio_set[webradio_index]->current_index = (long unsigned int)json_integer_value(json_object_get(j_stream, "current_index"));
          config->webradio_set[webradio_index]->tpl_id = json_object_get(j_stream, "tpl_id")!=json_null()?json_integer_value(json_object_get(j_stream, "tpl_id")):0;
          if (config->webradio_set[webradio_index]->tpl_id) {
            j_playlist = playlist_get_by_id(config, config->webradio_set[webradio_index]->tpl_id);
            if (check_result_value(j_playlist, T_OK)) {
              config->webradio_set[webradio_index]->playlist_name = o_strdup(json_string_value(json_object_get(json_object_get(j_playlist, "playlist"), "name")));
            }
            json_decref(j_playlist);
          }
          json_array_foreach(json_object_get(j_stream, "media"), index, j_element) {
            if (file_list_enqueue_new_file(config->webradio_set[webradio_index]->file_list, json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error adding file %s", json_string_value(json_object_get(j_element, "full_path")));
            }
          }
          if (new_webradio != NULL) {
            *new_webradio = config->webradio_set[webradio_index];
          }
          ret = T_OK;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error webradio_init");
          ret = T_ERROR;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error malloc new playlist");
        ret = T_ERROR_MEMORY;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error realloc webradio_set");
      ret = T_ERROR_MEMORY;
    }
    pthread_mutex_unlock(&config->playlist_lock);
  } else {
    ret = T_ERROR;
  }
  return ret;
}

json_t * webradio_get_clients(struct _t_webradio * webradio) {
  json_t * j_clients, * j_return;
  size_t i;

  if (webradio != NULL && webradio->audio_stream != NULL) {
    j_clients = json_array();
    if (j_clients != NULL) {
      for (i=0; i<webradio->audio_stream->nb_client_connected; i++) {
        json_array_append_new(j_clients, json_pack("{ssss}",
          "ip_address",
          webradio->audio_stream->client_list[i]->client_address,
          "user_agent",
          webradio->audio_stream->client_list[i]->user_agent));
      }
      j_return = json_pack("{siso}", "result", T_OK, "clients", j_clients);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_get_clients - Error allocating resources for j_clients");
      j_return = json_pack("{si}", "result", T_ERROR_PARAM);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_get_clients - Error webradio is invalid");
    j_return = json_pack("{si}", "result", T_ERROR_PARAM);
  }
  return j_return;
}

json_t * webradio_get_info(struct _t_webradio * webradio) {
  json_t * j_stream = NULL, * j_client;

  if (webradio != NULL && webradio->audio_stream != NULL) {
    j_stream = json_pack("{sis{sssssbsosbsisssbsisiss}}",
                          "result", T_OK,
                          "webradio",
                            "name", webradio->name,
                            "display_name", webradio->display_name,
                            "random", webradio->random,
                            "webradio", json_true(),
                            "icecast", webradio->icecast,
                            "elements", webradio->file_list->nb_files,
                            "format", webradio->audio_stream->stream_format,
                            "stereo", webradio->audio_stream->stream_channels==2,
                            "sample_rate", webradio->audio_stream->stream_sample_rate,
                            "bitrate", webradio->audio_stream->stream_bitrate,
                            "scope", webradio->username!=NULL?"me":"all");
    j_client = webradio_get_clients(webradio);
    if (check_result_value(j_client, T_OK)) {
      json_object_set(json_object_get(j_stream, "webradio"), "clients", json_object_get(j_client, "clients"));
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_get_info - Error webradio_get_clients");
    }
    json_decref(j_client);
    if (webradio->playlist_name != NULL) {
      json_object_set_new(json_object_get(j_stream, "webradio"), "stored_playlist", json_string(webradio->playlist_name));
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_get_info - Error webradio is invalid");
    j_stream = json_pack("{si}", "result", T_ERROR_PARAM);
  }
  return j_stream;
}

json_t * webradio_get_file_list(struct config_elements * config, struct _t_webradio * webradio, json_int_t offset, json_int_t limit) {
  struct _t_file * file = NULL;
  json_t * j_media, * j_return;
  json_int_t cur_offset = 0, end_offset = limit?(offset+limit):webradio->file_list->nb_files;

  file = webradio->file_list->start;
  if (file != NULL) {
    if (pthread_mutex_lock(&webradio->file_list->file_lock)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error lock mutex file_list");
      j_return = json_pack("{si}", "result", T_ERROR);
    } else {
      j_return = json_pack("{sis[]}", "result", T_OK, "list");
      if (j_return != NULL) {
        while (file != NULL && (cur_offset < end_offset)) {
          if (cur_offset >= offset) {
            j_media = media_get_by_id(config, file->tm_id);
            if (check_result_value(j_media, T_OK)) {
              json_object_del(json_object_get(j_media, "media"), "tm_id");
              json_array_append(json_object_get(j_return, "list"), json_object_get(j_media, "media"));
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "webradio_get_file_list - Error media_get_by_id");
            }
            json_decref(j_media);
          }
          file = file->next;
          cur_offset++;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_get_file_list - Error allocating resourcs for j_return");
        j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
      }
      pthread_mutex_unlock(&webradio->file_list->file_lock);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_get_file_list - Error, getting first file");
    j_return = json_pack("{si}", "result", T_ERROR);
  }
  return j_return;
}

int webradio_remove_media_by_index(struct _t_webradio * webradio, unsigned long index, json_int_t * tm_id) {
  int ret;
  struct _t_file * file;
  struct _audio_buffer * audio_buffer;

  if (webradio->file_list->nb_files > 1) {
    file = file_list_dequeue_file(webradio->file_list, index);
    if (file != NULL) {
      if (tm_id != NULL) {
        *tm_id = file->tm_id;
      }
      if (webradio->audio_stream != NULL && webradio->audio_stream->first_buffer != NULL) {
        audio_buffer = webradio->audio_stream->first_buffer->next;
        while (audio_buffer != NULL) {
          if (audio_buffer->file->tm_id == file->tm_id) {
            audio_buffer->skip = 1;
          }
          audio_buffer = audio_buffer->next;
        }
        if (webradio->current_index >= index) {
          webradio->current_index--;
        }
      }
      file_list_clean_file(file);
      ret = T_OK;
    } else {
      ret = T_ERROR_NOT_FOUND;
    }
  } else {
    ret = T_ERROR_PARAM;
  }
  return ret;
}

int webradio_add_media(struct config_elements * config, struct _t_webradio * webradio, const char * username, const char * data_source, const char * path) {
  json_t * j_data_source, * j_media;
  int res = T_OK;
  char * save_path;

  j_data_source = data_source_get(config, username, data_source, 1);
  if (check_result_value(j_data_source, T_OK)) {
    save_path = (char *)path;
    while (save_path != NULL && save_path[0] != '\0' && save_path[0] == '/') {
      save_path++;
    }
    j_media = media_get_full(config, json_object_get(j_data_source, "data_source"), save_path);
    if (check_result_value(j_media, T_OK)) {
      if (file_list_enqueue_new_file(webradio->file_list, json_integer_value(json_object_get(json_object_get(j_media, "media"), "tm_id"))) != T_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_media - Error file_list_enqueue_new_file for %s", path);
        res = T_ERROR;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_media - Error media_get_full %s/%s", json_string_value(json_object_get(json_object_get(j_data_source, "data_source"), "path")), path);
      res = T_ERROR;
    }
    json_decref(j_media);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_media - Error data_source_get");
    res = T_ERROR;
  }
  json_decref(j_data_source);
  return res;
}

int audio_stream_add_data(struct _audio_stream * stream, uint8_t * buf, int buf_size) {
  struct _audio_buffer * buffer = NULL;
  int ret = -1;

  if (!stream->is_header && stream->nb_buffer) {
    buffer = stream->last_buffer;
  } else {
    buffer = stream->header_buffer;
  }
  if (buffer != NULL) {
    if (pthread_mutex_lock(&stream->write_lock)) {
      return -1;
    }
    while (buffer->size + (size_t)buf_size > buffer->max_size) {
      buffer->data = o_realloc(buffer->data, buffer->max_size + TALIESIN_STREAM_BUFFER_INC_SIZE);
      if (buffer->data == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error reallocating buffer->data");
        buffer->max_size = 0;
      } else {
        buffer->max_size += TALIESIN_STREAM_BUFFER_INC_SIZE;
      }
    }
    if (buffer->max_size) {
      memcpy(buffer->data + buffer->size, buf, (size_t)buf_size);
      buffer->size += (size_t)buf_size;
      ret = 0;
    }
    pthread_mutex_unlock(&stream->write_lock);
  }
  return ret;
}

int audio_stream_enqueue_buffer(struct _t_webradio * webradio, size_t max_size, char * title, struct _t_file * file, unsigned int index) {
  int ret = T_ERROR;
  struct _audio_buffer * new_buffer = NULL, * audio_buffer = NULL;
  struct _audio_stream * stream = webradio->audio_stream;

  if (stream->status == TALIESIN_STREAM_STATUS_STOPPED) {
    return T_OK;
  }
  if (pthread_mutex_lock(&stream->write_lock)) {
    return T_ERROR;
  }
  if (stream->nb_buffer >= TALIESIN_STREAM_BUFFER_MAX) {
    audio_buffer = stream->first_buffer;
    stream->first_buffer = audio_buffer->next;
    audio_buffer_clean(audio_buffer, 0);
    stream->nb_buffer--;
    if (stream->first_buffer != NULL) {
      webradio->message_type = TALIESIN_PLAYLIST_MESSAGE_TYPE_NEW_MEDIA;
      pthread_mutex_lock(&webradio->message_lock);
      pthread_cond_broadcast(&webradio->message_cond);
      pthread_mutex_unlock(&webradio->message_lock);
      if (media_add_history(webradio->config, webradio->name, webradio->tpl_id, stream->first_buffer->file->tm_id) != T_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "audio_stream_enqueue_buffer - Error media_add_history (1)");
      }
    }
  }
  new_buffer = o_malloc(sizeof(struct _audio_buffer));
  if (new_buffer != NULL && audio_buffer_init(new_buffer) == T_OK) {
    new_buffer->title    = o_strdup(title);
    new_buffer->max_size = max_size;
    new_buffer->file     = copy_file(file);
    new_buffer->index    = index;
    new_buffer->data     = o_malloc(max_size);
    if (new_buffer->data != NULL) {
      if (stream->last_buffer != NULL) {
        new_buffer->counter = stream->last_buffer->counter + 1;
      } else {
        new_buffer->counter = 0;
      }
        if (stream->last_buffer != NULL) {
          stream->last_buffer->next = new_buffer;
        }
        if (stream->first_buffer == NULL) {
          stream->first_buffer = new_buffer;
          webradio->message_type = TALIESIN_PLAYLIST_MESSAGE_TYPE_NEW_MEDIA;
          pthread_mutex_lock(&webradio->message_lock);
          pthread_cond_broadcast(&webradio->message_cond);
          pthread_mutex_unlock(&webradio->message_lock);
          if (media_add_history(webradio->config, webradio->name, webradio->tpl_id, stream->first_buffer->file->tm_id) != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "audio_stream_enqueue_buffer - Error media_add_history (1)");
          }
        }
        stream->last_buffer = new_buffer;
      stream->nb_buffer++;
      ret = T_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error initializing data");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error allocating resources for new_buffer");
  }
  pthread_mutex_unlock(&stream->write_lock);
  return ret;
}

int audio_buffer_init(struct _audio_buffer * audio_buffer) {
  pthread_mutexattr_t mutexattr;
  int ret;

  if (audio_buffer != NULL) {
    pthread_mutexattr_init ( &mutexattr );
    pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
    if (pthread_mutex_init(&(audio_buffer->buffer_lock), &mutexattr) != 0) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Impossible to initialize Mutex Lock for audio buffer");
      ret = T_ERROR;
    } else {
      audio_buffer->title         = NULL;
      audio_buffer->size          = 0;
      audio_buffer->nb_client     = 0;
      audio_buffer->max_size      = 0;
      audio_buffer->complete      = 0;
      audio_buffer->skip          = 0;
      audio_buffer->read          = 0;
      audio_buffer->next          = NULL;
      audio_buffer->file          = NULL;
      audio_buffer->data          = NULL;
      audio_buffer->offset_list   = NULL;
      audio_buffer->nb_offset     = 0;
      audio_buffer->index         = 0;
      audio_buffer->last_offset   = 0;
      audio_buffer->start.tv_sec  = 0;
      audio_buffer->start.tv_nsec = 0;
      ret = T_OK;
    }
    pthread_mutexattr_destroy( &mutexattr );
  } else {
    ret = T_ERROR_PARAM;
  }
  return ret;
}

void audio_buffer_clean(struct _audio_buffer * audio_buffer, int recursive) {
  if (audio_buffer != NULL) {
    if (recursive) {
      audio_buffer_clean(audio_buffer->next, recursive);
    }
    pthread_mutex_destroy(&(audio_buffer->buffer_lock));
    o_free(audio_buffer->data);
    o_free(audio_buffer->title);
    o_free(audio_buffer->offset_list);
    file_list_clean_file(audio_buffer->file);
    o_free(audio_buffer);
  }
}

void audio_stream_clean (struct _audio_stream * audio_stream) {
  audio_buffer_clean(audio_stream->first_buffer, 1);
  audio_buffer_clean(audio_stream->header_buffer, 0);
  if (audio_stream->fifo) {
    av_audio_fifo_free(audio_stream->fifo);
  }
  if (audio_stream->output_codec_context) {
    AVCodecContext * avctx = audio_stream->output_codec_context;
    avcodec_free_context(&avctx);
  }
  if (audio_stream->output_format_context) {
    /*
     * Because of a libav memory leak problem: https://bugzilla.libav.org/show_bug.cgi?id=273
     * I manually clean the context instead of using avio_close
     */
    avio_flush(audio_stream->output_format_context->pb);
    av_free(audio_stream->output_format_context->pb->buffer);
    av_free(audio_stream->output_format_context->pb);
    avformat_free_context(audio_stream->output_format_context);
  }
  o_free(audio_stream->stream_format);
}

int stream_get_next_buffer(struct _client_data_webradio * client_data) {
  struct _audio_buffer * previous_buffer;
  if (!client_data->send_header) {
    // Get next buffer in queue
    previous_buffer = client_data->current_buffer;
    client_data->current_buffer = previous_buffer->next;

    while (client_data->current_buffer == NULL && client_data->audio_stream->transcode_status != TALIESIN_STREAM_TRANSCODE_STATUS_COMPLETE) {
      usleep(50000);
      client_data->current_buffer = previous_buffer->next;
    }
    client_data->buffer_offset = 0;
  } else {
    previous_buffer = NULL;
    client_data->send_header = 0;
    while (!client_data->audio_stream->nb_buffer && client_data->audio_stream->transcode_status != TALIESIN_STREAM_TRANSCODE_STATUS_COMPLETE) {
      usleep(50000);
    }
    client_data->current_buffer = client_data->audio_stream->first_buffer;
    if (client_data->current_buffer != NULL && client_data->current_buffer->counter != client_data->first_buffer_counter) {
      client_data->buffer_offset = 0;
    }
  }
  if (client_data->current_buffer != NULL) {
    if (pthread_mutex_lock(&client_data->current_buffer->buffer_lock)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "stream_get_next_buffer - Error getting current_buffer->buffer_lock");
      return T_ERROR;
    } else {
      client_data->current_buffer->nb_client++;
      if (!client_data->current_buffer->start.tv_sec) {
        //client_data->current_buffer->read = 1;
        clock_gettime(CLOCK_MONOTONIC_RAW, &client_data->current_buffer->start);
      }
      pthread_mutex_unlock(&client_data->current_buffer->buffer_lock);
    }
  }
  if (previous_buffer != NULL) {
    if (pthread_mutex_lock(&previous_buffer->buffer_lock)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "stream_get_next_buffer - Error getting current_buffer->buffer_lock");
      return T_ERROR;
    } else {
      previous_buffer->nb_client--;
      previous_buffer->read = (!previous_buffer->nb_client);
      pthread_mutex_unlock(&previous_buffer->buffer_lock);
    }
    pthread_mutex_lock(&client_data->audio_stream->stream_lock);
    pthread_cond_signal(&client_data->audio_stream->stream_cond);
    pthread_mutex_unlock(&client_data->audio_stream->stream_lock);
  }
  if (client_data->current_buffer != NULL) {
    return T_OK;
  } else {
    return T_ERROR_NOT_FOUND;
  }
}

json_t * is_webradio_command_valid(struct config_elements * config, struct _t_webradio * webradio, json_t * j_command, const char * username, int is_admin) {
  json_t * j_result = json_array(), * j_element;
  const char * str_command;
  size_t index;

  if (j_result != NULL) {
    if (j_command == NULL || !json_is_object(j_command)) {
      json_array_append_new(j_result, json_pack("{ss}", "json", "command must be a valid JSON object"));
    } else {
      str_command = json_string_value(json_object_get(j_command, "command"));
      if (str_command == NULL) {
        json_array_append_new(j_result, json_pack("{ss}", "command", "command key must be a valid string"));
      } else if (0 != o_strcmp(str_command, "stop") &&
                 0 != o_strcmp(str_command, "replay") &&
                 0 != o_strcmp(str_command, "skip") &&
                 0 != o_strcmp(str_command, "history") &&
                 0 != o_strcmp(str_command, "info") &&
                 0 != o_strcmp(str_command, "now") &&
                 0 != o_strcmp(str_command, "next") &&
                 0 != o_strcmp(str_command, "list") &&
                 0 != o_strcmp(str_command, "append_list") &&
                 0 != o_strcmp(str_command, "remove_list") &&
                 0 != o_strcmp(str_command, "has_list") &&
                 0 != o_strcmp(str_command, "move") &&
                 0 != o_strcmp(str_command, "attach_playlist") &&
                 0 != o_strcmp(str_command, "reload") &&
                 0 != o_strcmp(str_command, "rename") &&
                 0 != o_strcmp(str_command, "save") &&
                 0 != o_strcmp(str_command, "reset_url")) {
        json_array_append_new(j_result, json_pack("{ss}", "command", "invalid command"));
      } else if (!is_admin && 0 != o_strcmp(webradio->username, username) &&
                              (0 == o_strcmp(str_command, "stop") ||
                               0 == o_strcmp(str_command, "append_list") ||
                               0 == o_strcmp(str_command, "remove_list") ||
                               0 == o_strcmp(str_command, "has_list") ||
                               0 == o_strcmp(str_command, "move") ||
                               0 == o_strcmp(str_command, "attach_playlist") ||
                               0 == o_strcmp(str_command, "reload") ||
                               0 == o_strcmp(str_command, "rename") ||
                               0 == o_strcmp(str_command, "save") ||
                               0 == o_strcmp(str_command, "reset_url"))) {
        json_array_append_new(j_result, json_pack("{ss}", "parameters", "User is not allowed to run this command"));
      } else if (o_strcmp(str_command, "history") == 0 ||
          o_strcmp(str_command, "append_list") == 0 ||
          o_strcmp(str_command, "has_list") == 0 ||
          o_strcmp(str_command, "remove_list") == 0 ||
          o_strcmp(str_command, "attach_playlist") == 0) {
        if (o_strcmp(str_command, "history") == 0 || o_strcmp(str_command, "list") == 0) {
          if (json_object_get(j_command, "parameters") != NULL) {
            if (!json_is_object(json_object_get(j_command, "parameters"))) {
              json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json object"));
            } else {
              if (json_object_get(json_object_get(j_command, "parameters"), "offset") &&
                        (!json_is_integer(json_object_get(json_object_get(j_command, "parameters"), "offset")) ||
                        json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "offset")) <= 0)) {
                json_array_append_new(j_result, json_pack("{ss}", "parameters", "offset must ba a positive non zero integer"));
              }
              if (json_object_get(json_object_get(j_command, "parameters"), "limit") &&
                        (!json_is_integer(json_object_get(json_object_get(j_command, "parameters"), "limit")) ||
                        json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "limit")) <= 0)) {
                json_array_append_new(j_result, json_pack("{ss}", "parameters", "limit must ba a positive non zero integer"));
              }
            }
          }
        } else if (o_strcmp(str_command, "append_list") == 0) {
          if (!json_is_array(json_object_get(j_command, "parameters"))) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json object"));
          } else if (!json_array_size(json_object_get(j_command, "parameters"))) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json array of at least one element"));
          } else {
            json_array_foreach(json_object_get(j_command, "parameters"), index, j_element) {
              if (!is_valid_path_element_parameter(config, j_element, username, is_admin) && !is_valid_category_element_parameter(config, j_element, username, is_admin) && !is_valid_playlist_element_parameter(config, j_element, username)) {
                json_array_append_new(j_result, json_pack("{ss}", "parameter", "parameter is not a valid playlist element"));
              }
            }
          }
        } else if (o_strcmp(str_command, "has_list") == 0) {
          if (json_object_get(j_command, "parameters") != NULL) {
            if (!json_is_object(json_object_get(j_command, "parameters"))) {
              json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json object"));
            } else {
              if (json_object_get(json_object_get(j_command, "parameters"), "offset") &&
                        (!json_is_integer(json_object_get(json_object_get(j_command, "parameters"), "offset")) ||
                        json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "offset")) <= 0)) {
                json_array_append_new(j_result, json_pack("{ss}", "parameters", "offset must ba a positive non zero integer"));
              }
              if (json_object_get(json_object_get(j_command, "parameters"), "limit") &&
                        (!json_is_integer(json_object_get(json_object_get(j_command, "parameters"), "limit")) ||
                        json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "limit")) <= 0)) {
                json_array_append_new(j_result, json_pack("{ss}", "parameters", "limit must ba a positive non zero integer"));
              }
            }
            if (!json_is_array(json_object_get(json_object_get(j_command, "parameters"), "media"))) {
              json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json object"));
            } else if (!json_array_size(json_object_get(json_object_get(j_command, "parameters"), "media"))) {
              json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json array of at least one element"));
            } else {
              json_array_foreach(json_object_get(json_object_get(j_command, "parameters"), "media"), index, j_element) {
                if (!is_valid_path_element_parameter(config, j_element, username, is_admin) && !is_valid_category_element_parameter(config, j_element, username, is_admin) && !is_valid_playlist_element_parameter(config, j_element, username)) {
                  json_array_append_new(j_result, json_pack("{ss}", "parameter", "parameter is not a valid playlist element"));
                }
              }
            }
          }
        } else if (o_strcmp(str_command, "remove_list") == 0) {
          if (!json_is_object(json_object_get(j_command, "parameters"))) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json object"));
          } else if (json_object_get(json_object_get(j_command, "parameters"), "index") == NULL && json_object_get(json_object_get(j_command, "parameters"), "media") == NULL) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must have an index value or a media array"));
          } else if (json_object_get(json_object_get(j_command, "parameters"), "index") != NULL &&
                    (!json_is_integer(json_object_get(json_object_get(j_command, "parameters"), "index")) ||
                      json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "index")) < 0)) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "index must ba a positive integer"));
          } else if (json_object_get(json_object_get(j_command, "parameters"), "media") != NULL) {
            if (!json_is_array(json_object_get(json_object_get(j_command, "parameters"), "media"))) {
              json_array_append_new(j_result, json_pack("{ss}", "parameter", "media must be a JSON array"));
            } else {
              json_array_foreach(json_object_get(j_command, "parameters"), index, j_element) {
                if (!is_valid_path_element_parameter(config, j_element, username, is_admin) && !is_valid_category_element_parameter(config, j_element, username, is_admin) && !is_valid_playlist_element_parameter(config, j_element, username)) {
                  json_array_append_new(j_result, json_pack("{ss}", "parameter", "parameter is not a valid playlist element"));
                }
              }
            }
          }
        } else if (o_strcmp(str_command, "attach_playlist") == 0) {
          if (!json_is_object(json_object_get(j_command, "parameters"))) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json object"));
          } else if (json_object_get(json_object_get(j_command, "parameters"), "name") == NULL ||
                              !json_is_string(json_object_get(json_object_get(j_command, "parameters"), "name")) ||
                              json_string_length(json_object_get(json_object_get(j_command, "parameters"), "name")) == 0 ||
                              json_string_length(json_object_get(json_object_get(j_command, "parameters"), "name")) > 128) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "name must ba a non empty string of at least 128 characters"));
          }
        } else if (o_strcmp(str_command, "move") == 0) {
          if (!json_is_object(json_object_get(j_command, "parameters"))) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json object"));
          } else if (json_object_get(json_object_get(j_command, "parameters"), "index") == NULL ||
                              !json_is_integer(json_object_get(json_object_get(j_command, "parameters"), "index")) ||
                              json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "index")) < 0) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "index must ba a positive integer"));
          } else if (json_object_get(json_object_get(j_command, "parameters"), "target") == NULL ||
                              !json_is_integer(json_object_get(json_object_get(j_command, "parameters"), "target")) ||
                              json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "target")) < 0) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "target must ba a positive integer"));
          }
        } else if (o_strcmp(str_command, "save") == 0) {
          j_element = is_playlist_valid(config, username, is_admin, json_object_get(j_command, "parameters"), 0, 0);
          json_array_extend(j_result, j_element);
          json_decref(j_element);
        } else if (o_strcmp(str_command, "rename") == 0) {
          if (!json_is_object(json_object_get(j_command, "parameters"))) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameter must be a JSON object"));
          } else if (json_object_get(json_object_get(j_command, "parameters"), "name") == NULL || !json_is_string(json_object_get(json_object_get(j_command, "parameters"), "name")) || json_string_length(json_object_get(json_object_get(j_command, "parameters"), "name")) == 0 || json_string_length(json_object_get(json_object_get(j_command, "parameters"), "name")) > 512) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameter name must be a non empty JSON string of maximum 512 characters"));
          }
        }
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error allocating resources for j_result");
  }
  return j_result;
}

int webradio_close(struct config_elements * config, struct _t_webradio * webradio) {
  if (webradio_remove_db_stream(config, webradio->name) != T_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error webradio_remove_db_stream");
    return T_ERROR;
  }
  webradio->audio_stream->status = TALIESIN_STREAM_STATUS_STOPPED;
  if (webradio->audio_stream != NULL) {
    pthread_mutex_lock(&webradio->audio_stream->stream_lock);
    pthread_cond_signal(&webradio->audio_stream->stream_cond);
    pthread_mutex_unlock(&webradio->audio_stream->stream_lock);
  }
  return T_OK;
}

json_t * webradio_command(struct config_elements * config, struct _t_webradio * webradio, const char * username, json_t * j_command) {
  const char * str_command = json_string_value(json_object_get(j_command, "command"));
  int ret;
  json_t * j_return = NULL, * j_result, * j_element, * j_playlist, * j_media_list;
  json_int_t offset, limit, move_index, move_target, tm_id, tpl_id;
  size_t index, i;
  struct _audio_buffer * audio_buffer;
  struct _t_file * file;
  char old_name[TALIESIN_PLAYLIST_NAME_LENGTH + 1] = {0};

  if (0 == o_strcmp(str_command, "stop")) {
    webradio_close(config, webradio);
    j_return = json_pack("{si}", "result", T_OK);
  } else if (0 == o_strcmp(str_command, "reset_url")) {
    o_strcpy(old_name, webradio->name);
    if (json_is_string(json_object_get(json_object_get(j_command, "parameters"), "streamUrl"))) {
      j_result = json_array();
      is_stream_url_valid(config, json_string_value(json_object_get(json_object_get(j_command, "parameters"), "streamUrl")), j_result);
      if (!json_array_size(j_result)) {
        bzero(webradio->name, TALIESIN_PLAYLIST_NAME_LENGTH);
        o_strncpy(webradio->name, json_string_value(json_object_get(json_object_get(j_command, "parameters"), "streamUrl")), TALIESIN_PLAYLIST_NAME_LENGTH);
      } else {
        j_return = json_pack("{si}", "result", T_ERROR);
      }
      json_decref(j_result);
    } else {
      rand_string(webradio->name, TALIESIN_PLAYLIST_NAME_LENGTH);
    }
    if (j_return == NULL) {
      if (webradio_set_name_db_stream(config, old_name, webradio->name) == T_OK) {
        j_return = json_pack("{sis{ss}}", "result", T_OK, "command", "name", webradio->name);
      } else {
        j_return = json_pack("{si}", "result", T_ERROR);
      }
    }
  } else if (0 == o_strcmp(str_command, "replay")) {
    for (i=0; i<webradio->audio_stream->nb_client_connected; i++) {
      webradio->audio_stream->client_list[i]->command = TALIESIN_STREAM_COMMAND_PREVIOUS;
    }
    j_return = json_pack("{si}", "result", T_OK);
  } else if (0 == o_strcmp(str_command, "skip")) {
    if (webradio->audio_stream->nb_client_connected > 0) {
      webradio->audio_stream->first_buffer->skip = 1;
      for (i=0; i<webradio->audio_stream->nb_client_connected; i++) {
        webradio->audio_stream->client_list[i]->command = TALIESIN_STREAM_COMMAND_NEXT;
      }
      j_return = json_pack("{si}", "result", T_OK);
    } else {
      if (pthread_mutex_lock(&webradio->audio_stream->write_lock)) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - skip - Error locking webradio->audio_stream->write_lock");
        j_return = json_pack("{si}", "result", T_ERROR);
      } else {
        audio_buffer = webradio->audio_stream->first_buffer;
        if (audio_buffer != NULL) {
          webradio->audio_stream->first_buffer = audio_buffer->next;
          audio_buffer_clean(audio_buffer, 0);
          webradio->audio_stream->nb_buffer--;
        }
        pthread_mutex_unlock(&webradio->audio_stream->write_lock);
        j_return = json_pack("{si}", "result", T_OK);
      }
    }
  } else if (0 == o_strcmp(str_command, "history")) {
    if (json_object_get(json_object_get(j_command, "parameters"), "offset") != NULL) {
      offset = json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "offset"));
    } else {
      offset = 0;
    }
    if (json_object_get(json_object_get(j_command, "parameters"), "limit") != NULL) {
      limit = json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "limit"));
    } else {
      limit = TALIESIN_MEDIA_LIMIT_DEFAULT;
    }
    j_result = media_get_history(config, webradio->name, offset, limit);
    if (check_result_value(j_result, T_OK)) {
      j_return = json_pack("{sisO}", "result", T_OK, "command", json_object_get(j_result, "history"));
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error executing command history");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_result);
  } else if (0 == o_strcmp(str_command, "info")) {
    j_result = webradio_get_info(webradio);
    if (check_result_value(j_result, T_OK)) {
      j_return = json_pack("{sisO}", "result", T_OK, "command", json_object_get(j_result, "webradio"));
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error executing command info");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_result);
  } else if (0 == o_strcmp(str_command, "now")) {
    if (webradio->audio_stream->first_buffer != NULL && webradio->audio_stream->first_buffer->file != NULL) {
      audio_buffer = webradio->audio_stream->first_buffer;
      if (audio_buffer != NULL && audio_buffer->file != NULL) {
        j_result = media_get_by_id(config, audio_buffer->file->tm_id);
        if (check_result_value(j_result, T_OK)) {
          json_object_del(json_object_get(j_result, "media"), "tm_id");
          json_object_set_new(json_object_get(j_result, "media"), "index", json_integer(audio_buffer->index));
          j_return = json_pack("{sisO}", "result", T_OK, "command", json_object_get(j_result, "media"));
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error executing command now");
          j_return = json_pack("{si}", "result", T_ERROR);
        }
        json_decref(j_result);
      } else {
        j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
      }
    } else {
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    }
  } else if (0 == o_strcmp(str_command, "next")) {
    if (webradio->audio_stream->first_buffer != NULL && webradio->audio_stream->first_buffer->next != NULL) {
      audio_buffer = webradio->audio_stream->first_buffer->next;
      if (audio_buffer != NULL && audio_buffer->file != NULL) {
        j_result = media_get_by_id(config, audio_buffer->file->tm_id);
        if (check_result_value(j_result, T_OK)) {
          json_object_del(json_object_get(j_result, "media"), "tm_id");
          json_object_set_new(json_object_get(j_result, "media"), "index", json_integer(audio_buffer->index));
          j_return = json_pack("{sisO}", "result", T_OK, "command", json_object_get(j_result, "media"));
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error executing command next");
          j_return = json_pack("{si}", "result", T_ERROR);
        }
        json_decref(j_result);
      } else {
        j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
      }
    } else {
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    }
  } else if (0 == o_strcmp(str_command, "list")) {
    if (json_object_get(json_object_get(j_command, "parameters"), "offset") != NULL) {
      offset = json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "offset"));
    } else {
      offset = 0;
    }
    if (json_object_get(json_object_get(j_command, "parameters"), "limit") != NULL) {
      limit = json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "limit"));
    } else {
      limit = TALIESIN_MEDIA_LIMIT_DEFAULT;
    }
    j_result = webradio_get_file_list(config, webradio, offset, limit);
    if (check_result_value(j_result, T_OK)) {
      j_return = json_pack("{sisO}", "result", T_OK, "command", json_object_get(j_result, "list"));
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error executing command list");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_result);
  } else if (0 == o_strcmp(str_command, "append_list")) {
    ret = T_OK;
    if (!webradio->busy) {
      webradio->busy = 1;
      j_result = media_append_list_to_media_list(config, json_object_get(j_command, "parameters"), username);
      if (check_result_value(j_result, T_OK)) {
        if (json_array_size(json_object_get(j_result, "media")) > 0) {
          if (file_list_add_media_list(webradio->file_list, json_object_get(j_result, "media")) != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error appending to webradio");
            ret = T_ERROR;
          } else if (webradio_update_db_stream_media_list(config, webradio) != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error webradio_update_db_stream_media_list");
            ret = T_ERROR;
          }
        } else {
          ret = T_ERROR_NOT_FOUND;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error media_append_list_to_media_list");
        ret = T_ERROR;
      }
      json_decref(j_result);
    } else {
      ret = T_ERROR_PARAM;
    }
    j_return = json_pack("{si}", "result", ret);
  } else if (0 == o_strcmp(str_command, "remove_list")) {
    if (json_object_get(json_object_get(j_command, "parameters"), "index") != NULL) {
      if (webradio_remove_media_by_index(webradio, (long unsigned int)json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "index")), &tm_id) == T_OK) {
        if (webradio_delete_db_stream_media(config, webradio, tm_id) == T_OK) {
          j_return = json_pack("{si}", "result", T_OK);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error webradio_delete_db_stream_media");
          j_return = json_pack("{si}", "result", T_ERROR);
        }
      } else {
        j_return = json_pack("{si}", "result", T_ERROR);
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error webradio_remove_media_by_index");
      }
    } else {
      j_result = media_append_list_to_media_list(config, json_object_get(json_object_get(j_command, "parameters"), "media"), username);
      if (check_result_value(j_result, T_OK)) {
        if (json_array_size(json_object_get(j_result, "media")) > 0) {
          if (file_list_remove_media_list(webradio->file_list, json_object_get(j_result, "media")) == T_OK) {
            j_media_list = json_array();
            if (j_media_list != NULL) {
              json_array_foreach(json_object_get(j_result, "media"), index, j_element) {
                json_array_append(j_media_list, json_object_get(j_element, "tm_id"));
              }
              if (webradio_delete_db_stream_media_list(config, webradio, j_media_list) == T_OK) {
                j_return = json_pack("{si}", "result", T_OK);
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error webradio_delete_db_stream_media_list");
                j_return = json_pack("{si}", "result", T_ERROR);
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error allocating resources for j_media_list");
              j_return = json_pack("{si}", "result", T_ERROR);
            }
            json_decref(j_media_list);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error deleting from webradio");
            j_return = json_pack("{si}", "result", T_ERROR);
          }
        } else {
          j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error media_append_list_to_media_list");
        j_return = json_pack("{si}", "result", T_ERROR);
      }
      json_decref(j_result);
    }
  } else if (0 == o_strcmp(str_command, "has_list")) {
    if (json_object_get(json_object_get(j_command, "parameters"), "offset") != NULL) {
      offset = json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "offset"));
    } else {
      offset = 0;
    }
    if (json_object_get(json_object_get(j_command, "parameters"), "limit") != NULL) {
      limit = json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "limit"));
    } else {
      limit = TALIESIN_MEDIA_LIMIT_DEFAULT;
    }
    j_result = media_append_list_to_media_list(config, json_object_get(json_object_get(j_command, "parameters"), "media"), username);
    if (check_result_value(j_result, T_OK)) {
      if (json_array_size(json_object_get(j_result, "media")) > 0) {
        j_media_list = file_list_has_media_list(config, webradio->file_list, json_object_get(j_result, "media"), offset, limit);
        if (check_result_value(j_media_list, T_OK)) {
          if (json_array_size(json_object_get(j_media_list, "media")) > 0) {
            j_return = json_pack("{sisO}", "result", T_OK, "command", json_object_get(j_media_list, "media"));
          } else {
            j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error appending to webradio");
          j_return = json_pack("{si}", "result", T_ERROR);
        }
        json_decref(j_media_list);
      } else {
        j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error media_append_list_to_media_list");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_result);
  } else if (0 == o_strcmp(str_command, "reload")) {
    if (webradio->tpl_id) {
      if (!webradio->busy) {
        j_playlist = playlist_get_by_id(config, webradio->tpl_id);
        if (check_result_value(j_playlist, T_OK)) {
          if (pthread_mutex_lock(&webradio->file_list->file_lock)) {
            j_return = json_pack("{si}", "result", T_ERROR);
            y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error lock mutex file_list");
          } else {
            webradio->busy = 1;
            if (webradio->file_list->start != NULL) {
              file_list_clean_file(webradio->file_list->start);
              webradio->file_list->start = NULL;
              webradio->file_list->nb_files = 0;
            }
            json_array_foreach(json_object_get(json_object_get(j_playlist, "playlist"), "media"), index, j_element) {
              if (file_list_enqueue_new_file(webradio->file_list, json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error adding file %s", json_string_value(json_object_get(j_element, "full_path")));
              }
            }
            pthread_mutex_unlock(&webradio->file_list->file_lock);
            if (webradio_update_db_stream_media_list(config, webradio) == T_OK) {
              j_return = json_pack("{si}", "result", T_OK);
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error webradio_update_db_stream_media_list");
              j_return = json_pack("{si}", "result", T_ERROR);
            }
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error playlist_get_by_id");
          j_return = json_pack("{si}", "result", T_ERROR);
        }
        json_decref(j_playlist);
      } else {
        j_return = json_pack("{si}", "result", T_ERROR_PARAM);
      }
    } else {
      j_return = json_pack("{si}", "result", T_ERROR_PARAM);
    }
  } else if (0 == o_strcmp(str_command, "attach_playlist")) {
    j_playlist = playlist_get(config, username, json_string_value(json_object_get(json_object_get(j_command, "parameters"), "name")), 1, 0, 1);
    if (check_result_value(j_playlist, T_OK)) {
      webradio->tpl_id = json_integer_value(json_object_get(json_object_get(j_playlist, "playlist"), "tpl_id"));
      webradio->playlist_name = o_strdup(json_string_value(json_object_get(json_object_get(j_playlist, "playlist"), "name")));
      j_return = json_pack("{si}", "result", T_OK);
    } else if (check_result_value(j_playlist, T_ERROR_NOT_FOUND)) {
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error playlist_get");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_playlist);
  } else if (0 == o_strcmp(str_command, "move")) {
    if (!webradio->busy) {
      webradio->busy = 1;
      move_index = json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "index"));
      move_target = json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "target"));
      if (move_index < move_target) {
        move_target--;
      }
      file = file_list_dequeue_file(webradio->file_list, (long unsigned int)move_index);
      if (file != NULL) {
        if ((ret = file_list_insert_file_at(webradio->file_list, file, (unsigned long)move_target)) == T_OK) {
          if (webradio_update_db_stream_media_list(config, webradio) == T_OK) {
            j_return = json_pack("{si}", "result", T_OK);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "webradio_command - Error webradio_update_db_stream_media_list");
            j_return = json_pack("{si}", "result", T_ERROR);
          }
        } else {
          file_list_clean_file(file);
          j_return = json_pack("{si}", "result", ret);
        }
      } else {
        j_return = json_pack("{si}", "result", T_ERROR);
      }
    } else {
      j_return = json_pack("{si}", "result", T_ERROR_PARAM);
    }
  } else if (0 == o_strcmp(str_command, "save")) {
    if ((tpl_id = playlist_add(config, username, json_object_get(j_command, "parameters"))) != -1) {
      j_media_list = db_stream_get_media_list_from_name(config, webradio->name);
      if (check_result_value(j_media_list, T_OK)) {
        if (playlist_add_media(config, tpl_id, json_object_get(j_media_list, "media")) == T_OK) {
          if (webradio_set_playlist_db_stream(config, webradio->name, tpl_id) == T_OK) {
            j_return = json_pack("{sis{ss}}", "result", T_OK, "command", "name", json_string_value(json_object_get(json_object_get(j_command, "parameters"), "name")));
          } else {
            j_return = json_pack("{si}", "result", T_ERROR);
          }
        } else {
          j_return = json_pack("{si}", "result", T_ERROR);
        }
      } else {
        j_return = json_pack("{si}", "result", T_ERROR);
      }
      json_decref(j_media_list);
    } else {
      j_return = json_pack("{si}", "result", T_ERROR);
    }
  } else if (0 == o_strcmp(str_command, "rename")) {
    o_free(webradio->display_name);
    webradio->display_name = o_strdup(json_string_value(json_object_get(json_object_get(j_command, "parameters"), "name")));
    if (webradio_set_display_name_db_stream(config, webradio->name, webradio->display_name) == T_OK) {
      j_return = json_pack("{si}", "result", T_OK);
    } else {
      j_return = json_pack("{si}", "result", T_ERROR);
    }
  } else {
    j_return = json_pack("{si}", "result", T_ERROR_PARAM);
  }
  return j_return;
}

void * webradio_run_thread(void * args) {
  struct _t_webradio     * webradio             = (struct _t_webradio *)args;
  struct config_elements * config               = webradio->config;
  struct _audio_buffer   * current_buffer       = NULL;
  AVFormatContext        * input_format_context = NULL;
  AVCodecContext         * input_codec_context  = NULL;
  SwrContext             * resample_context     = NULL;
  struct _t_file         * current_file         = NULL;
  int data_present, send_signal = 0;
  size_t i;
  int64_t duration;
  int output_frame_size;
  int finished;
  int error;
  int data_written = 0;
  char * title = NULL, * path = NULL;
  json_t * j_media;
  unsigned int current_index;

  webradio->audio_stream->status           = TALIESIN_STREAM_STATUS_STARTED;
  webradio->audio_stream->transcode_status = TALIESIN_STREAM_TRANSCODE_STATUS_STARTED;
  webradio->audio_stream->pts              = 0;
  while ((current_file = webradio_get_next_file(webradio, &current_index)) != NULL &&
          webradio->audio_stream->status != TALIESIN_STREAM_STATUS_STOPPED) {
    j_media = media_get_by_id_for_stream(config, current_file->tm_id);
    if (check_result_value(j_media, T_OK)) {
      title = build_icy_title(json_object_get(j_media, "media"));
    } else {
      title = o_strdup(webradio->display_name);
    }
    // Open file
    path = msprintf("%s/%s", json_string_value(json_object_get(json_object_get(j_media, "media"), "path_ds")), json_string_value(json_object_get(json_object_get(j_media, "media"), "path_media")));
    if (!open_input_file(path, &input_format_context, &input_codec_context, AVMEDIA_TYPE_AUDIO)) {
      y_log_message(Y_LOG_LEVEL_INFO, "Transcode for stream '%s' file '%s'", webradio->display_name, path);
      duration = (input_format_context->duration/AV_TIME_BASE);

      // Open new buffer
      if (audio_stream_enqueue_buffer(webradio, (size_t)((duration + 1) * webradio->audio_stream->stream_bitrate / 8), title, current_file, current_index) == T_OK) {
        current_buffer = webradio->audio_stream->last_buffer;
        if (webradio->audio_stream->status != TALIESIN_STREAM_STATUS_STOPPED &&
            !init_resampler(input_codec_context, webradio->audio_stream->output_codec_context, &resample_context)) {
          finished = 0;
          while (webradio->audio_stream->status != TALIESIN_STREAM_STATUS_STOPPED && !finished) {
            output_frame_size = webradio->audio_stream->output_codec_context->frame_size;
            error             = 0;

            // Transcode file

            // Decode frames
            while (av_audio_fifo_size(webradio->audio_stream->fifo) < output_frame_size && !error) {
              if (read_decode_convert_and_store(webradio->audio_stream->fifo, input_format_context, input_codec_context, webradio->audio_stream->output_codec_context, resample_context, &finished)) {
                error = 1;
              }
              if (finished) {
                // End of file
                break;
              }
            }

            // Re-encode frames
            while ((av_audio_fifo_size(webradio->audio_stream->fifo) >= output_frame_size || (finished && av_audio_fifo_size(webradio->audio_stream->fifo) > 0)) && !error) {
              if (load_encode_and_return(webradio->audio_stream->fifo, webradio->audio_stream->output_codec_context, webradio->audio_stream->output_format_context, &webradio->audio_stream->pts, &data_present)) {
                error = 1;
              } else {
                if (pthread_mutex_lock(&current_buffer->buffer_lock)) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Error pthread_mutex_lock for buffer_lock");
                } else {
                  current_buffer->offset_list = o_realloc(current_buffer->offset_list, (current_buffer->nb_offset+1)*sizeof(size_t));
                  if (current_buffer->offset_list != NULL) {
                    current_buffer->offset_list[current_buffer->nb_offset] = current_buffer->size;
                    current_buffer->nb_offset++;
                  } else {
                    y_log_message(Y_LOG_LEVEL_ERROR, "Error realloc for offset_list");
                  }
                  pthread_mutex_unlock(&current_buffer->buffer_lock);
                }
              }
            }

            // Write encoded frames into buffer
            if (finished && !error) {
              data_written = 0;
              do {
                if (encode_audio_frame_and_return(NULL,
                                                  webradio->audio_stream->output_codec_context,
                                                  webradio->audio_stream->output_format_context,
                                                  &webradio->audio_stream->pts,
                                                  &data_written)) {
                  error = 1;
                } else {
                  if (pthread_mutex_lock(&current_buffer->buffer_lock)) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "Error pthread_mutex_lock for buffer_lock");
                  } else {
                    current_buffer->offset_list = o_realloc(current_buffer->offset_list, (current_buffer->nb_offset+1)*sizeof(size_t));
                    if (current_buffer->offset_list != NULL) {
                      current_buffer->offset_list[current_buffer->nb_offset] = current_buffer->size;
                      current_buffer->nb_offset++;
                    } else {
                      y_log_message(Y_LOG_LEVEL_ERROR, "Error realloc for offset_list");
                    }
                    pthread_mutex_unlock(&current_buffer->buffer_lock);
                  }
                }
              } while (data_written && !error);
            }
          }
        }
        current_buffer->complete = 1;
        avcodec_flush_buffers(webradio->audio_stream->output_codec_context);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error enqueing buffer_stream of size %u", ((duration + 1) * webradio->audio_stream->stream_bitrate / 8));
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error opening file %s", path);
    }
    o_free(path);
    path = NULL;
    o_free(title);
    title = NULL;
    if (resample_context) {
      swr_close(resample_context);
      swr_free(&resample_context);
      resample_context = NULL;
    }
    if (input_codec_context) {
      avcodec_free_context(&input_codec_context);
      input_codec_context = NULL;
    }
    if (input_format_context) {
      avformat_close_input(&input_format_context);
      input_format_context = NULL;
    }
    json_decref(j_media);
  }
  webradio->audio_stream->transcode_status = TALIESIN_STREAM_TRANSCODE_STATUS_COMPLETE;
  webradio->message_type = TALIESIN_PLAYLIST_MESSAGE_TYPE_CLOSE;
  pthread_mutex_lock(&webradio->message_lock);
  pthread_cond_broadcast(&webradio->message_cond);
  pthread_mutex_unlock(&webradio->message_lock);
  while (webradio->audio_stream->nb_client_connected) {
    pthread_mutex_lock(&webradio->audio_stream->client_lock);
    pthread_cond_wait(&webradio->audio_stream->client_cond, &webradio->audio_stream->client_lock);
    pthread_mutex_unlock(&webradio->audio_stream->client_lock);
  }
  while (webradio->nb_websocket) {
    pthread_mutex_lock(&webradio->websocket_lock);
    pthread_cond_wait(&webradio->websocket_cond, &webradio->websocket_lock);
    pthread_mutex_unlock(&webradio->websocket_lock);
  }
  send_signal = (webradio->audio_stream->status == TALIESIN_STREAM_STATUS_STOPPED);
  for (i=0; i<config->nb_webradio; i++) {
    if (config->webradio_set[i] == webradio) {
      break;
    }
  }
  for (;i<webradio->config->nb_webradio-1; i++) {
    config->webradio_set[i] = config->webradio_set[i+1];
  }
  config->nb_webradio--;
  webradio_clean(webradio);
  if (send_signal) {
    pthread_mutex_lock(&config->stream_stop_lock);
    pthread_cond_signal(&config->stream_stop_cond);
    pthread_mutex_unlock(&config->stream_stop_lock);
  }
  return NULL;
}

ssize_t u_webradio_stream (void * cls, uint64_t pos, char * buf, size_t max) {
  UNUSED(pos);
  struct _client_data_webradio * client_data_webradio = (struct _client_data_webradio *)cls;
  struct _audio_stream * stream = client_data_webradio->audio_stream;
  ssize_t len, tmp_len;
  size_t current_offset, i;
  struct timespec now, nsleep;
  uint64_t time_delta, time_should, diff;
  unsigned int stream_bitrate = stream->stream_bitrate;
  short int next_buffer;
  int res;

  // If no more buffer or stream is stopped, close stream
  if (client_data_webradio->current_buffer == NULL || stream->status == TALIESIN_STREAM_STATUS_STOPPED) {
    return (ssize_t)U_STREAM_END;
  } else if (client_data_webradio->command != TALIESIN_STREAM_COMMAND_NONE && !client_data_webradio->send_header) {
    // Handle command replay or next
    switch (client_data_webradio->command) {
      case TALIESIN_STREAM_COMMAND_PREVIOUS:
        client_data_webradio->buffer_offset = 0;
        if (pthread_mutex_lock(&client_data_webradio->current_buffer->buffer_lock)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Impossible to initialize Mutex Lock for current_buffer");
        } else {
          client_data_webradio->current_buffer->last_offset = 0;
          pthread_mutex_unlock(&client_data_webradio->current_buffer->buffer_lock);
        }
        client_data_webradio->command = TALIESIN_STREAM_COMMAND_NONE;
        break;
      case TALIESIN_STREAM_COMMAND_NEXT:
        if (client_data_webradio->buffer_offset == client_data_webradio->current_buffer->size || client_data_webradio->current_buffer->offset_list[client_data_webradio->current_buffer->last_offset] == client_data_webradio->buffer_offset) {
          if ((res = stream_get_next_buffer(client_data_webradio)) == T_ERROR_NOT_FOUND) {
            return (ssize_t)U_STREAM_END;
          } else if (res != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Error getting next buffer");
            return (ssize_t)U_STREAM_END;
          }
          client_data_webradio->buffer_offset = 0;
          client_data_webradio->command = TALIESIN_STREAM_COMMAND_NONE;
        }
        break;
    }
  }

  if (client_data_webradio->metadata_send != -1 && client_data_webradio->metadata_send) {
    // Send metadata if flag is set
    if ((len = webradio_buffer_metadata(buf, max, client_data_webradio)) <= 0) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error sending metadata");
    }
  } else {
    // Calculate sleep time
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    time_delta = (((uint64_t)now.tv_sec * 1000000000 + (uint64_t)now.tv_nsec) - ((uint64_t)client_data_webradio->start.tv_sec * 1000000000 + (uint64_t)client_data_webradio->start.tv_nsec)) + ((stream_bitrate / 8) * TALIESIN_STREAM_INITIAL_CLIENT_BUFFER_LENGTH);
    time_should = ((uint64_t)client_data_webradio->global_offset * 1000000000) / (stream_bitrate / 8);
    if (time_should > time_delta) {
      diff = time_should - time_delta;
      nsleep.tv_sec = (__time_t)(diff / 1000000000);
      nsleep.tv_nsec = (__syscall_slong_t)(diff % 1000000000);
      nanosleep(&nsleep, NULL);
    }

    if (stream->status == TALIESIN_STREAM_STATUS_STOPPED) {
      return (ssize_t)U_STREAM_END;
    }

    // Calculate buffer len to send
    if (client_data_webradio->send_header) {
      current_offset = client_data_webradio->header_offset;
    } else {
      current_offset = client_data_webradio->buffer_offset;
    }
    len = (ssize_t)(max<(client_data_webradio->current_buffer->size - current_offset) ? max : (client_data_webradio->current_buffer->size - current_offset));
    if (client_data_webradio->command == TALIESIN_STREAM_COMMAND_NEXT &&
        !client_data_webradio->send_header &&
        client_data_webradio->current_buffer->last_offset < client_data_webradio->current_buffer->nb_offset &&
        (ssize_t)client_data_webradio->buffer_offset + (ssize_t)len > (ssize_t)client_data_webradio->current_buffer->offset_list[client_data_webradio->current_buffer->last_offset + 1]) {
      tmp_len = (ssize_t)(client_data_webradio->current_buffer->offset_list[client_data_webradio->current_buffer->last_offset + 1] - client_data_webradio->buffer_offset);
      len = (ssize_t)max < tmp_len ? (ssize_t)max : tmp_len;
    }
    while (len < (ssize_t)max && !client_data_webradio->current_buffer->complete) {
      // Wait until current_buffer is full or len is equal to max
      usleep(50000);
      len = max<(client_data_webradio->current_buffer->size - current_offset) ? (ssize_t)max : (ssize_t)(client_data_webradio->current_buffer->size - current_offset);
    }

    // Calculate if we're supposed to send metadata
    if (client_data_webradio->metadata_send == 0 && client_data_webradio->metadata_offset + (long unsigned int)len >= (stream_bitrate / 8 * TALIESIN_STREAM_METADATA_INTERVAL)) {
      // Send metadata in next round
      len = (ssize_t)((stream_bitrate / 8 * TALIESIN_STREAM_METADATA_INTERVAL) - client_data_webradio->metadata_offset);
      client_data_webradio->metadata_send = 1;
    } else {
      client_data_webradio->metadata_offset += (long unsigned int)len;
    }

    memcpy(buf, client_data_webradio->current_buffer->data + current_offset, (size_t)len);
    client_data_webradio->global_offset += (long unsigned int)len;
    next_buffer = (current_offset + (size_t)len >= client_data_webradio->current_buffer->size);
    if (next_buffer) {
      if ((res = stream_get_next_buffer(client_data_webradio)) == T_ERROR_NOT_FOUND) {
        return (ssize_t)U_STREAM_END;
      } else if (res != T_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error getting next buffer");
        return (ssize_t)U_STREAM_END;
      }
    } else {
      if (client_data_webradio->send_header) {
        client_data_webradio->header_offset += (long unsigned int)len;
      } else {
        client_data_webradio->buffer_offset += (long unsigned int)len;
        if (pthread_mutex_lock(&client_data_webradio->current_buffer->buffer_lock)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Impossible to initialize Mutex Lock for current_buffer");
        } else {
          for (i = client_data_webradio->current_buffer->last_offset + 1; i < client_data_webradio->current_buffer->nb_offset && client_data_webradio->current_buffer->offset_list[i] <= client_data_webradio->buffer_offset; i++) {
            client_data_webradio->current_buffer->last_offset = (unsigned int)i;
          }
          pthread_mutex_unlock(&client_data_webradio->current_buffer->buffer_lock);
        }
      }
    }
  }
  return len;
}

void u_webradio_stream_free(void * cls) {
  struct _client_data_webradio * client_data_webradio = (struct _client_data_webradio *)cls;
  size_t i;

  if (client_data_webradio->audio_stream->nb_client_connected > 1) {
    for (i = 0; i < client_data_webradio->audio_stream->nb_client_connected; i++) {
      if (client_data_webradio->audio_stream->client_list[i] == cls) {
        break;
      }
    }
    for (; i < client_data_webradio->audio_stream->nb_client_connected - 1; i++) {
      client_data_webradio->audio_stream->client_list[i] = client_data_webradio->audio_stream->client_list[i + 1];
    }
    client_data_webradio->audio_stream->client_list = o_realloc(client_data_webradio->audio_stream->client_list, (client_data_webradio->audio_stream->nb_client_connected - 1) * sizeof(struct _client_data_webradio *));
  } else {
    o_free(client_data_webradio->audio_stream->client_list);
    client_data_webradio->audio_stream->client_list = NULL;
  }
  client_data_webradio->audio_stream->nb_client_connected--;
  if (client_data_webradio->audio_stream->status != TALIESIN_STREAM_STATUS_STOPPED && client_data_webradio->current_buffer != NULL) {
    if (pthread_mutex_lock(&client_data_webradio->current_buffer->buffer_lock)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "stream_get_next_buffer - Error getting current_buffer->buffer_lock");
    } else {
      client_data_webradio->current_buffer->nb_client--;
      pthread_mutex_unlock(&client_data_webradio->current_buffer->buffer_lock);
    }
  }
  pthread_mutex_lock(&client_data_webradio->audio_stream->client_lock);
  pthread_cond_signal(&client_data_webradio->audio_stream->client_cond);
  pthread_mutex_unlock(&client_data_webradio->audio_stream->client_lock);
  pthread_mutex_lock(&client_data_webradio->audio_stream->stream_lock);
  pthread_cond_signal(&client_data_webradio->audio_stream->stream_cond);
  pthread_mutex_unlock(&client_data_webradio->audio_stream->stream_lock);
  client_data_webradio_clean(client_data_webradio);
}

ssize_t webradio_buffer_metadata(char * buf, size_t max, struct _client_data_webradio * client_data) {
  char * str_metadata = NULL;
  size_t block_len;
  ssize_t len = -1;

  if (buf != NULL && client_data != NULL) {
    if (client_data->metadata_buffer == NULL) {
      str_metadata = msprintf("StreamTitle='%s';", client_data->current_buffer->title);
      block_len = o_strlen(str_metadata);
      if (block_len % 16) {
        block_len += (16 - (block_len % 16));
      }
      client_data->metadata_len = block_len + 1;
      client_data->metadata_buffer = o_malloc(client_data->metadata_len);
      if (client_data->metadata_buffer != NULL) {
        block_len = block_len / 16;
        memcpy(client_data->metadata_buffer, &block_len, 1);
        memcpy(client_data->metadata_buffer + 1, str_metadata, o_strlen(str_metadata));
        memset(client_data->metadata_buffer + 1 + o_strlen(str_metadata) + 1, 0, (block_len * 16) - o_strlen(str_metadata));
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_buffer_metadata - Error allocating resources for client_data->metadata_buffer");
        o_free(str_metadata);
        return -1;
      }
      client_data->metadata_current_offset = 0;
      o_free(str_metadata);
    }
    if (client_data->metadata_len - client_data->metadata_current_offset <= max) {
      memcpy(buf, client_data->metadata_buffer + client_data->metadata_current_offset, (client_data->metadata_len - client_data->metadata_current_offset));
      client_data->metadata_send = 0;
      client_data->metadata_offset = 0;
      o_free(client_data->metadata_buffer);
      client_data->metadata_buffer = NULL;
      len = (ssize_t)(client_data->metadata_len - client_data->metadata_current_offset);
      client_data->metadata_current_offset = client_data->metadata_len;
    } else {
      memcpy(buf, client_data->metadata_buffer + client_data->metadata_current_offset, max);
      client_data->metadata_current_offset += max;
      len = (ssize_t)max;
    }
  }
  return len;
}

void * webradio_icecast_run_thread(void * args) {
  struct _t_webradio     * webradio              = (struct _t_webradio *)args;
  struct config_elements * config                = webradio->config;
  struct _audio_buffer   * current_buffer        = NULL;
  AVFormatContext        * input_format_context  = NULL,
                         * output_format_context = NULL;
  AVCodecContext         * input_codec_context   = NULL,
                         * output_codec_context  = NULL;
  SwrContext             * resample_context      = NULL;
  AVAudioFifo            * fifo                  = NULL;
  struct _t_file         * current_file         = NULL;
  char * title = NULL,   * path = NULL;
  int output_frame_size, finished = 0, error, data_present = 0, data_written = 0, send_signal = 0;
  size_t i;
  int64_t duration;
  json_t * j_media;
  unsigned int current_index;

  webradio->audio_stream->status           = TALIESIN_STREAM_STATUS_STARTED;
  webradio->audio_stream->transcode_status = TALIESIN_STREAM_TRANSCODE_STATUS_STARTED;
  while ((current_file = webradio_get_next_file(webradio, &current_index)) != NULL &&
          webradio->audio_stream->status != TALIESIN_STREAM_STATUS_STOPPED) {
    webradio->audio_stream->pts = 0;
    j_media = media_get_by_id_for_stream(config, current_file->tm_id);
    if (check_result_value(j_media, T_OK)) {
      title = build_icy_title(json_object_get(j_media, "media"));
    } else {
      title = o_strdup(webradio->display_name);
    }
    // Open file
    path = msprintf("%s/%s", json_string_value(json_object_get(json_object_get(j_media, "media"), "path_ds")), json_string_value(json_object_get(json_object_get(j_media, "media"), "path_media")));
    if (!open_input_file(path, &input_format_context, &input_codec_context, AVMEDIA_TYPE_AUDIO)) {
      y_log_message(Y_LOG_LEVEL_INFO, "Icecast transcode for stream '%s' file '%s'", webradio->display_name, path);
      duration = (input_format_context->duration/AV_TIME_BASE);
      // Open new buffer
      if (audio_stream_enqueue_buffer(webradio, (size_t)((duration + 1) * webradio->audio_stream->stream_bitrate / 8), title, current_file, current_index) == T_OK) {
        if (!open_output_buffer_icecast(webradio, &output_format_context, &output_codec_context, &fifo)) {
          current_buffer = webradio->audio_stream->last_buffer;
          if (!init_resampler(input_codec_context, output_codec_context, &resample_context)) {
            while (webradio->audio_stream->status != TALIESIN_STREAM_STATUS_STOPPED) {
              output_frame_size = output_codec_context->frame_size;
              finished          = 0;
              error             = 0;
              // Transcode file

              // Decode frames
              while (av_audio_fifo_size(fifo) < output_frame_size && !error) {
                if (read_decode_convert_and_store(fifo, input_format_context, input_codec_context, output_codec_context, resample_context, &finished)) {
                  error = 1;
                }
                if (finished) {
                  break;
                }
              }

              // Re-encode frames
              while ((av_audio_fifo_size(fifo) >= output_frame_size || (finished && av_audio_fifo_size(fifo) > 0)) && !error) {
                if (load_encode_and_return(fifo, output_codec_context, output_format_context, &webradio->audio_stream->pts, &data_present)) {
                  error = 1;
                }
              }

              // Write encoded frames into buffer
              if (finished && !error) {
                y_log_message(Y_LOG_LEVEL_DEBUG, "write encoded frames");
                do {
                  if (encode_audio_frame_and_return(NULL, output_codec_context, output_format_context, &webradio->audio_stream->pts, &data_written)) {
                    error = 1;
                  }
                  y_log_message(Y_LOG_LEVEL_DEBUG, "encode_audio_frame_and_return %d %d", data_written, error);
                } while (data_written && !error);
                break;
              }
            }
            if (webradio->audio_stream->status != TALIESIN_STREAM_STATUS_STOPPED) {
              if (av_write_trailer(output_format_context)) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Error av_write_trailer");
              }
            }
            current_buffer->complete = 1;
            avcodec_flush_buffers(output_codec_context);
            avcodec_flush_buffers(input_codec_context);
          }
          if (resample_context) {
            swr_close(resample_context);
            swr_free(&resample_context);
            resample_context = NULL;
          }
          if (input_codec_context) {
            avcodec_free_context(&input_codec_context);
            input_codec_context = NULL;
          }
          if (output_codec_context) {
            avcodec_free_context(&output_codec_context);
            output_codec_context = NULL;
          }
          if (input_format_context) {
            avformat_close_input(&input_format_context);
            input_format_context = NULL;
          }

          if (output_format_context != NULL) {
            avio_flush(output_format_context->pb);
            av_free(output_format_context->pb->buffer);
            av_free(output_format_context->pb);
            avformat_free_context(output_format_context);
          }
        }
        av_audio_fifo_free(fifo);
        fifo = NULL;
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error enqueing buffer_stream of size %u", ((duration + 1) * webradio->audio_stream->stream_bitrate / 8));
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error opening file %s", path);
    }
    o_free(path);
    path = NULL;
    o_free(title);
    title = NULL;
    if (resample_context) {
      swr_close(resample_context);
      swr_free(&resample_context);
      resample_context = NULL;
    }
    if (input_codec_context) {
      avcodec_free_context(&input_codec_context);
      input_codec_context = NULL;
    }
    if (input_format_context) {
      avformat_close_input(&input_format_context);
      input_format_context = NULL;
    }
    json_decref(j_media);
  }
  webradio->audio_stream->transcode_status = TALIESIN_STREAM_TRANSCODE_STATUS_COMPLETE;
  webradio->message_type = TALIESIN_PLAYLIST_MESSAGE_TYPE_CLOSE;
  pthread_mutex_lock(&webradio->message_lock);
  pthread_cond_broadcast(&webradio->message_cond);
  pthread_mutex_unlock(&webradio->message_lock);
  while (webradio->audio_stream->nb_client_connected) {
    pthread_mutex_lock(&webradio->audio_stream->client_lock);
    pthread_cond_wait(&webradio->audio_stream->client_cond, &webradio->audio_stream->client_lock);
    pthread_mutex_unlock(&webradio->audio_stream->client_lock);
  }
  while (webradio->nb_websocket) {
    pthread_mutex_lock(&webradio->websocket_lock);
    pthread_cond_wait(&webradio->websocket_cond, &webradio->websocket_lock);
    pthread_mutex_unlock(&webradio->websocket_lock);
  }
  send_signal = (webradio->audio_stream->status == TALIESIN_STREAM_STATUS_STOPPED);
  for (i=0; i<config->nb_webradio; i++) {
    if (config->webradio_set[i] == webradio) {
      break;
    }
  }
  for (;i<webradio->config->nb_webradio-1; i++) {
    config->webradio_set[i] = config->webradio_set[i+1];
  }
  config->nb_webradio--;
  webradio_clean(webradio);
  if (send_signal) {
    pthread_mutex_lock(&config->stream_stop_lock);
    pthread_cond_signal(&config->stream_stop_cond);
    pthread_mutex_unlock(&config->stream_stop_lock);
  }
  return NULL;
}

int icecast_audio_buffer_add_data(struct _audio_buffer * buffer, uint8_t * buf, int buf_size) {
  int ret = -1;

  if (buffer != NULL) {
    if (pthread_mutex_lock(&buffer->buffer_lock)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error pthread_mutex_lock");
    } else {
      while (buffer->size + (size_t)buf_size > buffer->max_size) {
        buffer->data = o_realloc(buffer->data, buffer->max_size + TALIESIN_STREAM_BUFFER_INC_SIZE);
        if (buffer->data == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error reallocating buffer->data");
          buffer->max_size = 0;
        } else {
          //y_log_message(Y_LOG_LEVEL_ERROR, "reallocating buffer->data");
          buffer->max_size += TALIESIN_STREAM_BUFFER_INC_SIZE;
        }
      }
      if (buffer->max_size) {
        memcpy(buffer->data + buffer->size, buf, (size_t)buf_size);
        buffer->size += (size_t)buf_size;
        ret = 0;
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error buffer max_size");
      }
      pthread_mutex_unlock(&buffer->buffer_lock);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error buffer NULL");
  }
  return ret;
}

void * icecast_push_run_thread(void * args) {
  struct _t_webradio * webradio = (struct _t_webradio *)args;
  shout_t * shout;
  shout_metadata_t * meta = NULL;
  unsigned int shout_format = SHOUT_FORMAT_MP3;
  char * icecast_mount = NULL, * title = NULL;
  size_t offset = 0, send_size = 0;
  struct _audio_buffer * buffer;
  int ret = 0;
  json_t * j_media;

  shout_init();
  if (webradio != NULL) {
    do {
      ret = 1;
      if ((shout = shout_new()) == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_new");
        ret = 0;
        break;
      }
      if (shout_set_host(shout, webradio->config->icecast_host) != SHOUTERR_SUCCESS) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_set_host");
        ret = 0;
        break;
      }

      if (shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_set_protocol");
        ret = 0;
        break;
      }

      if (shout_set_port(shout, (short unsigned int)webradio->config->icecast_port) != SHOUTERR_SUCCESS) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_set_port");
        ret = 0;
        break;
      }

      if (shout_set_user(shout, webradio->config->icecast_user) != SHOUTERR_SUCCESS) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_set_user");
        ret = 0;
        break;
      }

      if (shout_set_password(shout, webradio->config->icecast_password) != SHOUTERR_SUCCESS) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_set_password");
        ret = 0;
        break;
      }
      
      if (shout_set_name(shout, webradio->display_name) != SHOUTERR_SUCCESS) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_set_name");
        ret = 0;
        break;
      }

      if (shout_set_description(shout, webradio->display_name) != SHOUTERR_SUCCESS) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_set_description");
        ret = 0;
        break;
      }

      icecast_mount = msprintf("%s/%s", webradio->config->icecast_mount_prefix, webradio->name);
      if (shout_set_mount(shout, icecast_mount) != SHOUTERR_SUCCESS) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_set_mount %s", icecast_mount);
        ret = 0;
        break;
      }

      if (0 == o_strcasecmp("vorbis", webradio->audio_stream->stream_format)) {
        shout_format = SHOUT_FORMAT_OGG;
      } else if (0 == o_strcasecmp("mp3", webradio->audio_stream->stream_format)) {
        shout_format = SHOUT_FORMAT_MP3;
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error invalid format %s", icecast_mount);
        ret = 0;
        break;
      }
      if (shout_set_format(shout, shout_format) != SHOUTERR_SUCCESS) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_set_format");
        ret = 0;
        break;
      }

      if (shout_open(shout) != SHOUTERR_SUCCESS) {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_open");
        ret = 0;
        break;
      }

    } while (0);
    o_free(icecast_mount);
    
    if (ret) {
      sleep(2);
      while (webradio->audio_stream == NULL || webradio->audio_stream->first_buffer == NULL || (webradio->audio_stream->first_buffer->size < TALIESIN_STREAM_ICECAST_MAX_BUFFER && !webradio->audio_stream->first_buffer->complete)) {
        usleep(50000);
      }
      buffer = webradio->audio_stream->first_buffer;
      offset = 0;
      //y_log_message(Y_LOG_LEVEL_DEBUG, "send buffer %p to icecast", buffer);
      while (buffer != NULL) {
        if (pthread_mutex_lock(&buffer->buffer_lock)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error pthread_mutex_lock");
          webradio->audio_stream->status = TALIESIN_STREAM_STATUS_STOPPED;

          pthread_mutex_lock(&webradio->audio_stream->stream_lock);
          pthread_cond_signal(&webradio->audio_stream->stream_cond);
          pthread_mutex_unlock(&webradio->audio_stream->stream_lock);

          pthread_mutex_lock(&webradio->config->stream_stop_lock);
          pthread_cond_wait(&webradio->config->stream_stop_cond, &webradio->config->stream_stop_lock);
          pthread_mutex_unlock(&webradio->config->stream_stop_lock);
          
          buffer = NULL;
          break;
        }
        j_media = media_get_by_id_for_stream(webradio->config, buffer->file->tm_id);
        if (check_result_value(j_media, T_OK)) {
          title = build_icy_title(json_object_get(j_media, "media"));
        } else {
          title = o_strdup(webradio->display_name);
        }
        json_decref(j_media);
        if ((meta = shout_metadata_new()) != NULL) {
          shout_metadata_add(meta, "song", title);
          shout_set_metadata(shout, meta);
          shout_metadata_free(meta);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_metadata_new");
        }
        o_free(title);
        if ((buffer->size - offset) > TALIESIN_STREAM_ICECAST_MAX_BUFFER) {
          send_size = TALIESIN_STREAM_ICECAST_MAX_BUFFER;
        } else {
          send_size = buffer->size - offset;
        }
        if (send_size > 0) {
          //y_log_message(Y_LOG_LEVEL_DEBUG, "send %zu data at offset %zu (max %zu, complete %d) from buffer %p", send_size, offset, buffer->size, buffer->complete, buffer);
          if (shout_send(shout, buffer->data + offset, send_size) != SHOUTERR_SUCCESS) {
            y_log_message(Y_LOG_LEVEL_ERROR, "webradio icecast - Error shout_send");
            pthread_mutex_unlock(&buffer->buffer_lock);
            buffer = NULL;
            break;
          }
          offset += send_size;
          //y_log_message(Y_LOG_LEVEL_DEBUG, "grut 0 %zu", send_size);
          shout_sync(shout);
          //y_log_message(Y_LOG_LEVEL_DEBUG, "grut 1");
        }
        pthread_mutex_unlock(&buffer->buffer_lock);
        if (buffer->complete && offset >= buffer->size) {
          //y_log_message(Y_LOG_LEVEL_DEBUG, "Next buffer");
          //y_log_message(Y_LOG_LEVEL_DEBUG, "send buffer %p to icecast", buffer);
          buffer->read = 1;
          buffer = buffer->next;
          offset = 0;
          
          pthread_mutex_lock(&webradio->audio_stream->stream_lock);
          pthread_cond_signal(&webradio->audio_stream->stream_cond);
          pthread_mutex_unlock(&webradio->audio_stream->stream_lock);
        }
      }
      shout_close(shout);
    } else {
      webradio->audio_stream->status = TALIESIN_STREAM_STATUS_STOPPED;
    }
  }

  shout_shutdown();
  return NULL;
}
