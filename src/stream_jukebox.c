/**
 *
 * Taliesin - Media server
 * 
 * Stream functions definitions for jukebox streaming
 *
 * Copyright 2017 Nicolas Mora <mail@babelouest.org>
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

int jukebox_init(struct _t_jukebox * jukebox, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate) {
  pthread_mutexattr_t mutexattr;
  int res;
  
  if (jukebox != NULL) {
    jukebox->tpl_id = 0;
    rand_string(jukebox->name, TALIESIN_PLAYLIST_NAME_LENGTH);
    jukebox->nb_jukebox_audio_buffer = 0;
    jukebox->jukebox_audio_buffer = NULL;
    jukebox->stream_format = o_strdup(format);
    jukebox->stream_channels = channels;
    jukebox->stream_sample_rate = sample_rate;
    jukebox->stream_bitrate = bit_rate;
    jukebox->playlist_name = NULL;
    jukebox->tpl_id = 0;
    jukebox->nb_client = 0;
    jukebox->last_seen = 0;
    jukebox->file_list = o_malloc(sizeof(struct _t_file_list));
    if (jukebox->file_list != NULL) {
      jukebox->username = NULL;
      jukebox->display_name = NULL;
      jukebox->nb_websocket = 0;
      if (file_list_init(jukebox->file_list) == T_OK) {
        pthread_mutexattr_init ( &mutexattr );
        pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE_NP );
        if (!pthread_mutex_init(&jukebox->websocket_lock, NULL) &&
            !pthread_cond_init(&jukebox->websocket_cond, NULL) &&
            !pthread_mutex_init(&jukebox->message_lock, NULL) &&
            !pthread_cond_init(&jukebox->message_cond, NULL)) {
          res = T_OK;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_init - Error init pthread_mutex or client_lock or client_cond of buffer_lock or buffer_cond");
          o_free(jukebox->file_list);
          res = T_ERROR_MEMORY;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_init - Error allocating resources for header_buffer");
        o_free(jukebox->file_list);
        res = T_ERROR;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_init - Error allocating resources for file_list");
      o_free(jukebox->file_list);
      res = T_ERROR_MEMORY;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_init - Error input parameters");
    res = T_ERROR_PARAM;
  }
  return res;
}

void jukebox_clean(struct _t_jukebox * jukebox) {
  int i;
  if (jukebox != NULL) {
    file_list_clean(jukebox->file_list);
    for (i=0; i<jukebox->nb_jukebox_audio_buffer; i++) {
      jukebox_audio_buffer_clean(jukebox->jukebox_audio_buffer[i]);
      o_free(jukebox->jukebox_audio_buffer[i]);
    }
    o_free(jukebox->username);
    o_free(jukebox->display_name);
    o_free(jukebox->stream_format);
    o_free(jukebox->playlist_name);
    o_free(jukebox);
  }
}

int jukebox_audio_buffer_init (struct _jukebox_audio_buffer * jukebox_audio_buffer) {
  if (jukebox_audio_buffer != NULL) {
    jukebox_audio_buffer->size = 0;
    jukebox_audio_buffer->max_size = 0;
    jukebox_audio_buffer->complete = 0;
    jukebox_audio_buffer->data = NULL;
    jukebox_audio_buffer->file = NULL;
    jukebox_audio_buffer->jukebox = NULL;
    jukebox_audio_buffer->client_address = NULL;
    jukebox_audio_buffer->user_agent = NULL;
    if (!pthread_mutex_init(&jukebox_audio_buffer->buffer_lock, NULL) &&
        !pthread_cond_init(&jukebox_audio_buffer->buffer_cond, NULL)) {
      return T_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_audio_buffer_init - Error init mutex");
      return T_ERROR;
    }
  } else {
    return T_ERROR_PARAM;
  }
}

void jukebox_audio_buffer_clean (struct _jukebox_audio_buffer * jukebox_audio_buffer) {
  if (jukebox_audio_buffer != NULL) {
    o_free(jukebox_audio_buffer->data);
    o_free(jukebox_audio_buffer->client_address);
    o_free(jukebox_audio_buffer->user_agent);
    o_free(jukebox_audio_buffer);
  }
}

static int jukebox_update_db_stream_media_list(struct config_elements * config, struct _t_jukebox * jukebox) {
  json_t * j_query, * j_result;
  json_int_t ts_id;
  int res, ret;
  struct _t_file * file;
  
  j_query = json_pack("{sss[s]s{ss}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "columns",
                        "ts_id",
                      "where",
                        "ts_name",
                        jukebox->name);
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
          file = jukebox->file_list->start;
          while (file != NULL) {
            json_array_append_new(json_object_get(j_query, "values"), json_pack("{sIsI}", "ts_id", ts_id, "tm_id", file->tm_id));
            file = file->next;
          }
          res = h_insert(config->conn, j_query, NULL);
          json_decref(j_query);
          if (res == H_OK) {
            ret = T_OK;
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_update_db_stream_media_list - Error executing j_query (3)");
            ret = T_ERROR_DB;
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_update_db_stream_media_list - Error allocatinng resources for j_query");
          ret = T_ERROR_MEMORY;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_update_db_stream_media_list - Error executing j_query (2)");
        ret = T_ERROR_DB;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_update_db_stream_media_list - stream not found");
      ret = T_ERROR_NOT_FOUND;
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_update_db_stream_media_list - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

static int jukebox_add_db_stream_media_list(struct config_elements * config, struct _t_jukebox * jukebox, json_t * j_media) {
  json_t * j_query, * j_result, * j_element;
  json_int_t ts_id;
  int res, ret;
  size_t index;
  
  j_query = json_pack("{sss[s]s{ss}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "columns",
                        "ts_id",
                      "where",
                        "ts_name",
                        jukebox->name);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      ts_id = json_integer_value(json_object_get(json_array_get(j_result, 0), "ts_id"));
      j_query = json_pack("{sss[]}", "table", TALIESIN_TABLE_STREAM_ELEMENT, "values");
      if (j_query != NULL) {
        json_array_foreach(j_media, index, j_element) {
          json_array_append_new(json_object_get(j_query, "values"), json_pack("{sIsI}", "ts_id", ts_id, "tm_id", json_integer_value(json_object_get(j_element, "tm_id"))));
        }
        res = h_insert(config->conn, j_query, NULL);
        json_decref(j_query);
        if (res == H_OK) {
          ret = T_OK;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_add_db_stream_media_list - Error executing j_query (2)");
          ret = T_ERROR_DB;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_add_db_stream_media_list - Error allocating resources for j_query");
        ret = T_ERROR_MEMORY;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_add_db_stream_media_list - stream not found");
      ret = T_ERROR_NOT_FOUND;
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_add_db_stream_media_list - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

static int jukebox_delete_db_stream_media(struct config_elements * config, struct _t_jukebox * jukebox, json_int_t tm_id) {
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
                        jukebox->name);
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
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_delete_db_stream_media - Error executing j_query (2)");
        ret = T_ERROR_DB;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_update_db_stream_media_list - stream not found");
      ret = T_ERROR_NOT_FOUND;
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_delete_db_stream_media - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

static int jukebox_add_db_stream(struct config_elements * config, struct _t_jukebox * jukebox) {
  json_t * j_query, * j_last_id;
  int res, ret;
  json_int_t ts_id;
  struct _t_file * file;
  
  j_query = json_pack("{sss{sosssssososssisisi}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "values",
                        "ts_username",
                        jukebox->username!=NULL?json_string(jukebox->username):json_null(),
                        "ts_name",
                        jukebox->name,
                        "ts_display_name",
                        jukebox->display_name,
                        "tpl_id",
                        jukebox->tpl_id?json_integer(jukebox->tpl_id):json_null(),
                        "ts_webradio",
                        json_false(),
                        "ts_format",
                        jukebox->stream_format,
                        "ts_channels",
                        jukebox->stream_channels,
                        "ts_sample_rate",
                        jukebox->stream_sample_rate,
                        "ts_bitrate",
                        jukebox->stream_bitrate);
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
        file = jukebox->file_list->start;
        while (file != NULL) {
          json_array_append_new(json_object_get(j_query, "values"), json_pack("{sIsI}", "ts_id", ts_id, "tm_id", file->tm_id));
          file = file->next;
        }
        res = h_insert(config->conn, j_query, NULL);
        json_decref(j_query);
        if (res == H_OK) {
          ret = T_OK;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_add_db_stream - Error executing j_query (2)");
          ret = T_ERROR_DB;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_add_db_stream - Error allocating resources for j_query");
        ret = T_ERROR_MEMORY;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_add_db_stream - Error getting last id");
      ret = T_ERROR_DB;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_add_db_stream - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

static int jukebox_set_name_db_stream(struct config_elements * config, const char * old_name, const char * new_name) {
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
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_set_name_db_stream - Error executing j_query");
    return T_ERROR_DB;
  }
}

static int jukebox_set_display_name_db_stream(struct config_elements * config, const char * name, const char * display_name) {
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
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_set_display_name_db_stream - Error executing j_query");
    return T_ERROR_DB;
  }
}

static int jukebox_set_playlist_db_stream(struct config_elements * config, const char * name, json_int_t tpl_id) {
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
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_set_playlist_db_stream - Error executing j_query");
    return T_ERROR_DB;
  }
}

static int jukebox_remove_db_stream(struct config_elements * config, const char * name) {
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
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_remove_db_stream - Error executing j_query");
    return T_ERROR_DB;
  }
}

json_t * add_jukebox_from_path(struct config_elements * config, json_t * j_data_source, const char * path, const char * username, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate, int recursive, const char * name) {
  json_t * j_result = NULL, * j_media_list, * j_element;
  size_t index;
  int jukebox_index;
  const char * display_name;
  
  j_media_list = media_scan_path(config, j_data_source, path, recursive);
  if (j_media_list != NULL && json_array_size(json_object_get(j_media_list, "media_list")) > 0) {
    if (!pthread_mutex_lock(&config->playlist_lock)) {
      //y_log_message(Y_LOG_LEVEL_DEBUG, "realloc playlist_set at %d", (config->nb_jukebox + 1));
      config->jukebox_set = o_realloc(config->jukebox_set, (config->nb_jukebox + 1) * sizeof(struct _t_jukebox *));
      if (config->jukebox_set != NULL) {
        jukebox_index = config->nb_jukebox;
        config->nb_jukebox++;
        config->jukebox_set[jukebox_index] = o_malloc(sizeof(struct _t_jukebox));
        if (config->jukebox_set[jukebox_index] != NULL) {
          if (jukebox_init(config->jukebox_set[jukebox_index], format, channels, sample_rate, bit_rate) == T_OK) {
						if (name == NULL) {
							display_name = strrchr(path, '/')!=NULL?(strrchr(path, '/') + 1):path;
						} else {
							display_name = name;
						}
            if (o_strlen(display_name) <= 0) {
              display_name = json_string_value(json_object_get(j_data_source, "name"));
            }
            config->jukebox_set[jukebox_index]->config = config;
            config->jukebox_set[jukebox_index]->username = o_strdup(username);
            config->jukebox_set[jukebox_index]->display_name = o_strdup(display_name);
            json_array_foreach(json_object_get(j_media_list, "media_list"), index, j_element) {
              if (file_list_enqueue_new_file(config->jukebox_set[jukebox_index]->file_list, json_string_value(json_object_get(j_element, "full_path")), json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_path - Error adding file %s", json_string_value(json_object_get(j_element, "full_path")));
              }
            }
            if (jukebox_add_db_stream(config, config->jukebox_set[jukebox_index]) == T_OK) {
              j_result = json_pack("{sis{sssssssssisisosis[]}}",
                                    "result",
                                    T_OK,
                                    "stream",
                                      "name",
                                      config->jukebox_set[jukebox_index]->name,
                                      "display_name",
                                      config->jukebox_set[jukebox_index]->display_name,
                                      "format",
                                      format,
                                      "channels",
                                      channels==1?"mono":"stereo",
                                      "sample_rate",
                                      sample_rate,
                                      "bitrate",
                                      bit_rate,
                                      "webradio",
                                      json_false(),
                                      "elements",
                                      config->jukebox_set[jukebox_index]->file_list->nb_files,
                                      "clients");
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_path - Error jukebox_add_db_stream");
              j_result = json_pack("{si}", "result", T_ERROR);
            }
          } else {
            jukebox_clean(config->jukebox_set[jukebox_index]);
            o_free(config->jukebox_set[jukebox_index]);
            y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_path - Error jukebox_init");
            j_result = json_pack("{si}", "result", T_ERROR);
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_path - Error malloc new jukebox");
          j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_path - Error realloc playlist_set");
        j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
      }
      pthread_mutex_unlock(&config->playlist_lock);
    } else {
      j_result = json_pack("{si}", "result", T_ERROR);
    }
  } else if (j_media_list == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_path - Error getting j_media_list");
    j_result = json_pack("{si}", "result", T_ERROR);
  } else {
    j_result = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
  }
  json_decref(j_media_list);
  return j_result;
}

json_t * add_jukebox_from_playlist(struct config_elements * config, json_t * j_playlist, const char * username, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate, const char * name) {
  json_t * j_result = NULL, * j_element;
  size_t index;
  char * full_path;
  int jukebox_index;
  
  if (!pthread_mutex_lock(&config->playlist_lock)) {
    //y_log_message(Y_LOG_LEVEL_DEBUG, "realloc playlist_set at %d", (config->nb_jukebox + 1));
    config->jukebox_set = o_realloc(config->jukebox_set, (config->nb_jukebox + 1) * sizeof(struct _t_jukebox *));
    if (config->jukebox_set != NULL) {
      jukebox_index = config->nb_jukebox;
      config->nb_jukebox++;
      config->jukebox_set[jukebox_index] = o_malloc(sizeof(struct _t_jukebox));
      if (config->jukebox_set[jukebox_index] != NULL) {
        if (jukebox_init(config->jukebox_set[jukebox_index], format, channels, sample_rate, bit_rate) == T_OK) {
          config->jukebox_set[jukebox_index]->config = config;
          config->jukebox_set[jukebox_index]->username = o_strdup(username);
					if (name == NULL) {
						config->jukebox_set[jukebox_index]->display_name = o_strdup(json_string_value(json_object_get(j_playlist, "description")));
					} else {
						config->jukebox_set[jukebox_index]->display_name = o_strdup(name);
					}
          config->jukebox_set[jukebox_index]->playlist_name = o_strdup(json_string_value(json_object_get(j_playlist, "name")));
          config->jukebox_set[jukebox_index]->tpl_id = json_integer_value(json_object_get(j_playlist, "tpl_id"));
          json_array_foreach(json_object_get(j_playlist, "media"), index, j_element) {
            full_path = msprintf("%s/%s", json_string_value(json_object_get(j_element, "tds_path")), json_string_value(json_object_get(j_element, "path")));
            if (file_list_enqueue_new_file(config->jukebox_set[jukebox_index]->file_list, full_path, json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_playlist - Error adding file %s", json_string_value(json_object_get(j_element, "full_path")));
            }
            o_free(full_path);
          }
          if (jukebox_add_db_stream(config, config->jukebox_set[jukebox_index]) == T_OK) {
            j_result = json_pack("{sis{sssssssssisisosis[]}}",
                                  "result",
                                  T_OK,
                                  "stream",
                                    "name",
                                    config->jukebox_set[jukebox_index]->name,
                                    "display_name",
                                    config->jukebox_set[jukebox_index]->playlist_name,
                                    "format",
                                    format,
                                    "channels",
                                    channels==1?"mono":"stereo",
                                    "sample_rate",
                                    sample_rate,
                                    "bitrate",
                                    bit_rate,
                                    "webradio",
                                    json_false(),
                                    "elements",
                                    config->jukebox_set[jukebox_index]->file_list->nb_files,
                                    "clients");
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_playlist - Error jukebox_add_db_stream");
            j_result = json_pack("{si}", "result", T_ERROR);
          }
        } else {
          jukebox_clean(config->jukebox_set[jukebox_index]);
          o_free(config->jukebox_set[jukebox_index]);
          y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_playlist - Error jukebox_init");
          j_result = json_pack("{si}", "result", T_ERROR);
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_playlist - Error malloc new jukebox");
        j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_playlist - Error realloc playlist_set");
      j_result = json_pack("{si}", "result", T_ERROR_MEMORY);
    }
    pthread_mutex_unlock(&config->playlist_lock);
  } else {
    j_result = json_pack("{si}", "result", T_ERROR);
  }
  return j_result;
}

int add_jukebox_from_db_stream(struct config_elements * config, json_t * j_stream) {
  json_t * j_element, * j_playlist;
  size_t index;
  char * full_path;
  int jukebox_index, ret;
  
  if (!pthread_mutex_lock(&config->playlist_lock)) {
    //y_log_message(Y_LOG_LEVEL_DEBUG, "realloc playlist_set at %d", (config->nb_jukebox + 1));
    config->jukebox_set = o_realloc(config->jukebox_set, (config->nb_jukebox + 1) * sizeof(struct _t_jukebox *));
    if (config->jukebox_set != NULL) {
      jukebox_index = config->nb_jukebox;
      config->nb_jukebox++;
      config->jukebox_set[jukebox_index] = o_malloc(sizeof(struct _t_jukebox));
      if (config->jukebox_set[jukebox_index] != NULL) {
        if (jukebox_init(config->jukebox_set[jukebox_index], json_string_value(json_object_get(j_stream, "format")), json_integer_value(json_object_get(j_stream, "channels")), json_integer_value(json_object_get(j_stream, "sample_rate")), json_integer_value(json_object_get(j_stream, "bitrate"))) == T_OK) {
          config->jukebox_set[jukebox_index]->config = config;
          config->jukebox_set[jukebox_index]->username = json_object_get(j_stream, "username")!=json_null()?o_strdup(json_string_value(json_object_get(j_stream, "username"))):NULL;
          o_strcpy(config->jukebox_set[jukebox_index]->name, json_string_value(json_object_get(j_stream, "name")));
          config->jukebox_set[jukebox_index]->display_name = o_strdup(json_string_value(json_object_get(j_stream, "display_name")));
          config->jukebox_set[jukebox_index]->tpl_id = json_object_get(j_stream, "tpl_id")!=json_null()?json_integer_value(json_object_get(j_stream, "tpl_id")):0;
          if (config->jukebox_set[jukebox_index]->tpl_id) {
            j_playlist = playlist_get_by_id(config, config->jukebox_set[jukebox_index]->tpl_id);
            if (check_result_value(j_playlist, T_OK)) {
              config->jukebox_set[jukebox_index]->playlist_name = o_strdup(json_string_value(json_object_get(json_object_get(j_playlist, "playlist"), "name")));
            }
            json_decref(j_playlist);
          }
          json_array_foreach(json_object_get(j_stream, "media"), index, j_element) {
            full_path = msprintf("%s/%s", json_string_value(json_object_get(j_element, "tds_path")), json_string_value(json_object_get(j_element, "path")));
            if (file_list_enqueue_new_file(config->jukebox_set[jukebox_index]->file_list, full_path, json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_db_stream - Error adding file %s", json_string_value(json_object_get(j_element, "full_path")));
            }
            o_free(full_path);
          }
          ret = T_OK;
        } else {
          jukebox_clean(config->jukebox_set[jukebox_index]);
          o_free(config->jukebox_set[jukebox_index]);
          y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_db_stream - Error jukebox_init");
          ret = T_ERROR;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_db_stream - Error malloc new jukebox");
        ret = T_ERROR_MEMORY;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "add_jukebox_from_db_stream - Error realloc playlist_set");
      ret = T_ERROR_MEMORY;
    }
    pthread_mutex_unlock(&config->playlist_lock);
  } else {
    ret = T_ERROR;
  }
  return ret;
}

int jukebox_build_m3u(struct config_elements * config, struct _t_jukebox * jukebox, char ** m3u_data) {
  json_t * j_media;
  char * icy_title, * tmp, * m3u_song;
  struct _t_file * file;
  int i = 0, res = T_OK, counter = 0;
  
  *m3u_data = msprintf("#EXTM3U\n\n");
  if (*m3u_data != NULL) {
    while ((*m3u_data != NULL) && (file = file_list_get_file(jukebox->file_list, i)) != NULL) {
      j_media = media_get_by_id(config, file->tm_id);
      if (check_result_value(j_media, T_OK)) {
        icy_title = build_icy_title(json_object_get(j_media, "media"));
        if (icy_title != NULL) {
          m3u_song = msprintf("#EXTINF:%"JSON_INTEGER_FORMAT",%s\n%s/%s/stream/%s?index=%d\n",
                              (json_integer_value(json_object_get(json_object_get(j_media, "media"), "duration"))/1000),
                              icy_title,
                              config->server_remote_address,
                              config->api_prefix,
                              jukebox->name,
                              i);
        } else {
          m3u_song = msprintf("%s/%s/stream/%s?index=%d\n",
                              config->server_remote_address,
                              config->api_prefix,
                              jukebox->name,
                              i);
        }
        tmp = msprintf("%s%s", (*m3u_data), m3u_song);
        o_free(m3u_song);
        o_free(icy_title);
        o_free(*m3u_data);
        (*m3u_data) = tmp;
        if (tmp != NULL) {
          counter++;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_build_m3u - Error getting media %"JSON_INTEGER_FORMAT, file->tm_id);
      }
      i++;
      json_decref(j_media);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_build_m3u - Error allocating resources for m3u_data");
    res = T_ERROR;
  }
  if (!counter) {
    res = T_ERROR_NOT_FOUND;
    if (*m3u_data != NULL) {
      o_free(*m3u_data);
      *m3u_data = NULL;
    }
  }
  return res;
}

json_t * jukebox_get_clients(struct _t_jukebox * jukebox) {
  json_t * j_clients, * j_return;
  int i;
  
  if (jukebox != NULL) {
    j_clients = json_array();
    if (j_clients != NULL) {
      for (i=0; i<jukebox->nb_jukebox_audio_buffer; i++) {
        json_array_append_new(j_clients, json_pack("{ssss}",
          "ip_address",
          jukebox->jukebox_audio_buffer[i]->client_address,
          "user_agent",
          jukebox->jukebox_audio_buffer[i]->user_agent));
      }
      j_return = json_pack("{siso}", "result", T_OK, "clients", j_clients);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_get_clients - Error allocating resources for j_clients");
      j_return = json_pack("{si}", "result", T_ERROR_PARAM);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_get_clients - Error jukebox is invalid");
    j_return = json_pack("{si}", "result", T_ERROR_PARAM);
  }
  return j_return;
}

json_t * jukebox_get_info(struct _t_jukebox * jukebox) {
  json_t * j_stream = NULL;
  
  if (jukebox != NULL) {
    j_stream = json_pack("{sis{sssssosisssosisisisi}}",
                          "result",
                          T_OK,
                          "jukebox",
                            "name",
                            jukebox->name,
                            "display_name",
                            jukebox->display_name,
                            "webradio",
                            json_false(),
                            "elements",
                            jukebox->file_list->nb_files,
                            "format",
                            jukebox->stream_format,
                            "stereo",
                            jukebox->stream_channels==2?json_true():json_false(),
                            "sample_rate",
                            jukebox->stream_sample_rate,
                            "bitrate",
                            jukebox->stream_bitrate,
                            "nb_client",
                            jukebox->nb_client,
                            "last_seen",
                            jukebox->last_seen);
    if (jukebox->playlist_name != NULL) {
      json_object_set_new(json_object_get(j_stream, "jukebox"), "stored_playlist", json_string(jukebox->playlist_name));
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_get_info - Error jukebox is invalid");
    j_stream = json_pack("{si}", "result", T_ERROR_PARAM);
  }
  return j_stream;
}

json_t * jukebox_get_file_list(struct config_elements * config, struct _t_jukebox * jukebox, json_int_t offset, json_int_t limit) {
  struct _t_file * file = NULL;
  json_t * j_media, * j_return = NULL;
  json_int_t cur_offset = 0, end_offset = limit?(offset+limit):jukebox->file_list->nb_files;
  
  if (jukebox != NULL && jukebox->file_list != NULL) {
    file = jukebox->file_list->start;
  }
  if (file != NULL) {
    if (pthread_mutex_lock(&jukebox->file_list->file_lock)) {
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
              y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_get_file_list - Error media_get_by_id: %"JSON_INTEGER_FORMAT, file->tm_id);
              break;
            }
            json_decref(j_media);
          }
          file = file->next;
          cur_offset++;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_get_file_list - Error allocating resourcs for j_return");
        j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
      }
      pthread_mutex_unlock(&jukebox->file_list->file_lock);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_get_file_list - Error, getting first file");
    j_return = json_pack("{si}", "result", T_ERROR);
  }
  return j_return;
}

int jukebox_remove_media_by_index(struct _t_jukebox * jukebox, int index, json_int_t * tm_id) {
  int ret;
  struct _t_file * file;
  
  if (jukebox->file_list->nb_files > 1) {
    file = file_list_dequeue_file(jukebox->file_list, index);
    if (file != NULL) {
      if (tm_id != NULL) {
        *tm_id = file->tm_id;
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

int jukebox_close(struct config_elements * config, struct _t_jukebox * jukebox) {
  int i;

  if (jukebox != NULL) {
    for (i=0; i<jukebox->nb_jukebox_audio_buffer; i++) {
      jukebox->jukebox_audio_buffer[i]->status = TALIESIN_STREAM_STATUS_STOPPED;
      pthread_mutex_lock(&jukebox->jukebox_audio_buffer[i]->buffer_lock);
      pthread_cond_signal(&jukebox->jukebox_audio_buffer[i]->buffer_cond);
      pthread_mutex_unlock(&jukebox->jukebox_audio_buffer[i]->buffer_lock);
    }
    while (jukebox->nb_websocket) {
      jukebox->message_type = TALIESIN_PLAYLIST_MESSAGE_TYPE_CLOSING;
      pthread_mutex_lock(&jukebox->message_lock);
      pthread_cond_signal(&jukebox->message_cond);
      pthread_mutex_unlock(&jukebox->message_lock);
    }
    
    for (i=0; i<config->nb_jukebox && config->jukebox_set[i] != jukebox; i++);
    for (; i<config->nb_jukebox-1; i++) {
      config->jukebox_set[i] = config->jukebox_set[i+1];
    }
    if (config->nb_jukebox > 1) {
      config->jukebox_set = o_realloc(config->jukebox_set, (config->nb_jukebox - 1)*sizeof(struct _t_jukebox *));
    } else {
      o_free(config->jukebox_set);
      config->jukebox_set = NULL;
    }
    config->nb_jukebox--;
    jukebox_clean(jukebox);
    return T_OK;
  } else {
    return T_ERROR_PARAM;
  }
}

int jukebox_audio_buffer_add_data(struct _jukebox_audio_buffer * jukebox_audio_buffer, uint8_t * buf, int buf_size) {
  int ret = -1;
  
  //y_log_message(Y_LOG_LEVEL_DEBUG, "stream->nb_buffer is %d", stream->nb_buffer);
  if (jukebox_audio_buffer != NULL) {
    while (jukebox_audio_buffer->size + buf_size > jukebox_audio_buffer->max_size) {
      jukebox_audio_buffer->data = o_realloc(jukebox_audio_buffer->data, jukebox_audio_buffer->max_size + TALIESIN_STREAM_BUFFER_INC_SIZE);
      if (jukebox_audio_buffer->data == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error reallocating jukebox_audio_buffer->data");
        jukebox_audio_buffer->max_size = 0;
      } else {
        //y_log_message(Y_LOG_LEVEL_ERROR, "reallocating jukebox_audio_buffer->data");
        jukebox_audio_buffer->max_size += TALIESIN_STREAM_BUFFER_INC_SIZE;
      }
    }
    if (jukebox_audio_buffer->max_size) {
      memcpy(jukebox_audio_buffer->data + jukebox_audio_buffer->size, buf, buf_size);
      jukebox_audio_buffer->size += buf_size;
      ret = 0;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error jukebox_audio_buffer is NULL");
  }
  return ret;
}

struct _t_jukebox * jukebox_get_stream(struct config_elements * config, const char * stream_name, const char * username, int is_admin) {
  int i;
  struct _t_jukebox * jukebox = NULL;
  
  if (pthread_mutex_lock(&config->playlist_lock)) {
    return NULL;
  }
  for (i=0; i<config->nb_jukebox; i++) {
    if (0 == o_strcmp(stream_name, config->jukebox_set[i]->name) && (is_admin || 0 == o_strcmp(config->jukebox_set[i]->username, username))) {
      jukebox = config->jukebox_set[i];
      break;
    }
  }
  pthread_mutex_unlock(&config->playlist_lock);
  return jukebox;
}

int is_valid_jukebox_element_parameter(struct config_elements * config, json_t * jukebox_element, const char * username, int is_admin) {
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
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error get data source %s %s", username, json_string_value(json_object_get(jukebox_element, "data_source")));
      res = 0;
    } else {
      y_log_message(Y_LOG_LEVEL_DEBUG, "Error data_source_get");
      res = 0;
    }
    json_decref(j_data_source);
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error input params");
    res = 0;
  }
  return res;
}

void * run_thread_close_playlist(void * args) {
  struct _close_jukebox * jukebox = (struct _close_jukebox *)args;
  
  if (jukebox_remove_db_stream(jukebox->config, jukebox->jukebox->name) != T_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "run_thread_close_playlist - Error jukebox_remove_db_stream");
  }

  if (jukebox_close(jukebox->config, jukebox->jukebox) != T_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error running jukebox_close");
  }
  o_free(jukebox);
  return NULL;
}

json_t * is_jukebox_command_valid(struct config_elements * config, json_t * j_command, const char * username, int is_admin) {
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
                 0 != o_strcmp(str_command, "history") &&
                 0 != o_strcmp(str_command, "info") &&
                 0 != o_strcmp(str_command, "list") &&
                 0 != o_strcmp(str_command, "append_list") &&
                 0 != o_strcmp(str_command, "remove_list") &&
                 0 != o_strcmp(str_command, "move") &&
                 0 != o_strcmp(str_command, "attach_playlist") &&
                 0 != o_strcmp(str_command, "reload") &&
                 0 != o_strcmp(str_command, "rename") &&
                 0 != o_strcmp(str_command, "save") &&
                 0 != o_strcmp(str_command, "reset_url")) {
        json_array_append_new(j_result, json_pack("{ss}", "command", "invalid command"));
      }
      if (o_strcmp(str_command, "history") == 0 ||
          o_strcmp(str_command, "append_list") == 0 ||
          o_strcmp(str_command, "remove_list") == 0 ||
          o_strcmp(str_command, "attach_playlist") == 0) {
        if (o_strcmp(str_command, "history") == 0) {
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
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json array"));
          } else if (json_array_size(json_object_get(j_command, "parameters")) == 0) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json array of at least one element"));
          } else {
            json_array_foreach(json_object_get(j_command, "parameters"), index, j_element) {
              if (!is_valid_jukebox_element_parameter(config, j_element, username, is_admin)) {
                json_array_append_new(j_result, json_pack("{ss}", "parameter", "parameter is not a valid jukebox element"));
              }
            }
          }
        } else if (o_strcmp(str_command, "remove_list") == 0) {
          if (!json_is_object(json_object_get(j_command, "parameters"))) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameters must be a json object"));
          } else if (json_object_get(json_object_get(j_command, "parameters"), "index") == NULL || 
                              !json_is_integer(json_object_get(json_object_get(j_command, "parameters"), "index")) ||
                              json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "index")) < 0) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "index must ba a positive integer"));
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
        } else if (o_strcmp(str_command, "rename") == 0) {
          if (!json_is_object(json_object_get(j_command, "parameters"))) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameter must be a JSON object"));
          } else if (json_object_get(json_object_get(j_command, "parameters"), "name") == NULL || !json_is_string(json_object_get(json_object_get(j_command, "parameters"), "name")) || json_string_length(json_object_get(json_object_get(j_command, "parameters"), "name")) == 0 || json_string_length(json_object_get(json_object_get(j_command, "parameters"), "name")) > 512) {
            json_array_append_new(j_result, json_pack("{ss}", "parameters", "parameter name must be a non empty JSON string of maximum 512 characters"));
          }
        } else if (o_strcmp(str_command, "save") == 0) {
          j_element = is_playlist_valid(config, username, is_admin, json_object_get(j_command, "parameters"), 0, 0);
          json_array_extend(j_result, j_element);
          json_decref(j_element);
        }
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error allocating resources for j_result");
  }
  return j_result;
}

json_t * jukebox_command(struct config_elements * config, struct _t_jukebox * jukebox, const char * username, json_t * j_command) {
  const char * str_command = json_string_value(json_object_get(j_command, "command"));
  int ret;
  json_t * j_return = NULL, * j_result, * j_element, * j_playlist, * j_data_source;
  json_int_t offset, limit, move_index, move_target, tm_id, tpl_id;
  size_t index;
  struct _t_file * file;
  int ret_thread_close_playlist = 0, detach_thread_close_playlist = 0;
  pthread_t thread_close_playlist;
  struct _close_jukebox * close_jukebox;
  char * full_path, old_name[TALIESIN_PLAYLIST_NAME_LENGTH + 1] = {0};
  
  if (0 == o_strcmp(str_command, "stop")) {
    close_jukebox = o_malloc(sizeof(struct _close_jukebox));
    if (close_jukebox != NULL) {
      close_jukebox->config = config;
      close_jukebox->jukebox = jukebox;
      ret_thread_close_playlist = pthread_create(&thread_close_playlist, NULL, run_thread_close_playlist, (void *)close_jukebox);
      detach_thread_close_playlist = pthread_detach(thread_close_playlist);
      if (ret_thread_close_playlist || detach_thread_close_playlist) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error running thread run_thread_close_playlist");
        j_return = json_pack("{si}", "result", T_ERROR);
      } else {
        j_return = json_pack("{si}", "result", T_OK);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error allocating resources for close_jukebox");
      j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
    }
  } else if (0 == o_strcmp(str_command, "reset_url")) {
    strcpy(old_name, jukebox->name);
    rand_string(jukebox->name, TALIESIN_PLAYLIST_NAME_LENGTH);
    if (jukebox_set_name_db_stream(config, old_name, jukebox->name) == T_OK) {
      j_return = json_pack("{sis{ss}}", "result", T_OK, "command", "name", jukebox->name);
    } else {
      j_return = json_pack("{si}", "result", T_ERROR);
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
      limit = 10;
    }
    j_result = media_get_history(config, jukebox->name, offset, limit);
    if (check_result_value(j_result, T_OK)) {
      j_return = json_pack("{sisO}", "result", T_OK, "command", json_object_get(j_result, "history"));
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error executing command info");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_result);
  } else if (0 == o_strcmp(str_command, "info")) {
    j_result = jukebox_get_info(jukebox);
    if (check_result_value(j_result, T_OK)) {
      j_return = json_pack("{sisO}", "result", T_OK, "command", json_object_get(j_result, "jukebox"));
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error executing command info");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_result);
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
    j_result = jukebox_get_file_list(config, jukebox, offset, limit);
    if (check_result_value(j_result, T_OK)) {
      j_return = json_pack("{sisO}", "result", T_OK, "command", json_object_get(j_result, "list"));
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error executing command list");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_result);
  } else if (0 == o_strcmp(str_command, "append_list")) {
    ret = T_OK;
    json_array_foreach(json_object_get(j_command, "parameters"), index, j_element) {
      j_data_source = data_source_get(config, username, json_string_value(json_object_get(j_element, "data_source")), 1);
      if (check_result_value(j_data_source, T_OK)) {
        j_result = media_get_file_list_from_path(config, json_object_get(j_data_source, "data_source"), json_string_value(json_object_get(j_element, "path")), (json_object_get(j_element, "recursive") == json_true()));
        if (check_result_value(j_result, T_OK)) {
          if (0 == o_strcmp("audio", json_string_value(json_object_get(json_object_get(j_result, "media"), "type")))) {
            if (file_list_add_media_list(config, jukebox->file_list, json_object_get(j_result, "media")) != T_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error appending %s/%s to jukebox", json_string_value(json_object_get(j_element, "data_source")), json_string_value(json_object_get(j_element, "path")));
              ret = T_ERROR;
            }
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error media_get_file_list_from_path");
          ret = T_ERROR;
        }
        if (jukebox_add_db_stream_media_list(config, jukebox, json_object_get(j_result, "media")) != T_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error jukebox_add_db_stream_media_list");
          ret = T_ERROR;
        }
        json_decref(j_result);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error data_source_get");
        ret = T_ERROR;
      }
      json_decref(j_data_source);
    }
    j_return = json_pack("{si}", "result", ret);
  } else if (0 == o_strcmp(str_command, "remove_list")) {
    if (jukebox_remove_media_by_index(jukebox, json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "index")), &tm_id) == T_OK) {
      if (jukebox_delete_db_stream_media(config, jukebox, tm_id) == T_OK) {
        j_return = json_pack("{si}", "result", T_OK);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error jukebox_delete_db_stream_media");
        j_return = json_pack("{si}", "result", T_ERROR);
      }
    } else {
      j_return = json_pack("{si}", "result", T_ERROR);
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error jukebox_remove_media_by_index");
    }
  } else if (0 == o_strcmp(str_command, "reload")) {
    if (jukebox->tpl_id) {
      j_playlist = playlist_get_by_id(config, jukebox->tpl_id);
      if (check_result_value(j_playlist, T_OK)) {
        if (pthread_mutex_lock(&jukebox->file_list->file_lock)) {
          j_return = json_pack("{si}", "result", T_ERROR);
          y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error lock mutex file_list");
        } else {
          if (jukebox->file_list->start != NULL) {
            file_list_clean_file(jukebox->file_list->start);
            jukebox->file_list->start = NULL;
            jukebox->file_list->nb_files = 0;
          }
          pthread_mutex_unlock(&jukebox->file_list->file_lock);
          json_array_foreach(json_object_get(json_object_get(j_playlist, "playlist"), "media"), index, j_element) {
            full_path = msprintf("%s/%s", json_string_value(json_object_get(j_element, "tds_path")), json_string_value(json_object_get(j_element, "path")));
            if (file_list_enqueue_new_file(jukebox->file_list, full_path, json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error adding file %s", json_string_value(json_object_get(j_element, "full_path")));
            }
            o_free(full_path);
          }
          if (jukebox_update_db_stream_media_list(config, jukebox) == T_OK) {
            j_return = json_pack("{si}", "result", T_OK);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error jukebox_update_db_stream_media_list");
            j_return = json_pack("{si}", "result", T_ERROR);
          }
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error playlist_get_by_id");
        j_return = json_pack("{si}", "result", T_ERROR);
      }
      json_decref(j_playlist);
    } else {
      j_return = json_pack("{si}", "result", T_ERROR_PARAM);
    }
  } else if (0 == o_strcmp(str_command, "attach_playlist")) {
    j_playlist = playlist_get(config, username, json_string_value(json_object_get(json_object_get(j_command, "parameters"), "name")), 1, 0, 1);
    if (check_result_value(j_playlist, T_OK)) {
      jukebox->tpl_id = json_integer_value(json_object_get(json_object_get(j_playlist, "playlist"), "tpl_id"));
      jukebox->playlist_name = o_strdup(json_string_value(json_object_get(json_object_get(j_playlist, "playlist"), "name")));
      j_return = json_pack("{si}", "result", T_OK);
    } else if (check_result_value(j_playlist, T_ERROR_NOT_FOUND)) {
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error playlist_get");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_playlist);
  } else if (0 == o_strcmp(str_command, "move")) {
    move_index = json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "index"));
    move_target = json_integer_value(json_object_get(json_object_get(j_command, "parameters"), "target"));
    if (move_index < move_target) {
      move_target--;
    }
    file = file_list_dequeue_file(jukebox->file_list, move_index);
    if (file != NULL) {
      if ((ret = file_list_insert_file_at(jukebox->file_list, file, move_target)) == T_OK) {
        if (jukebox_update_db_stream_media_list(config, jukebox) == T_OK) {
          j_return = json_pack("{si}", "result", T_OK);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_command - Error jukebox_update_db_stream_media_list");
          j_return = json_pack("{si}", "result", T_ERROR);
        }
      } else {
        file_list_clean_file(file);
        j_return = json_pack("{si}", "result", ret);
      }
    } else {
      j_return = json_pack("{si}", "result", T_ERROR);
    }
  } else if (0 == o_strcmp(str_command, "rename")) {
    o_free(jukebox->display_name);
    jukebox->display_name = o_strdup(json_string_value(json_object_get(json_object_get(j_command, "parameters"), "name")));
    if (jukebox_set_display_name_db_stream(config, jukebox->name, jukebox->display_name) == T_OK) {
      j_return = json_pack("{si}", "result", T_OK);
    } else {
      j_return = json_pack("{si}", "result", T_ERROR);
    }
  } else if (0 == o_strcmp(str_command, "save")) {
    if ((tpl_id = playlist_add(config, username, json_object_get(j_command, "parameters"), jukebox->file_list)) != -1) {
			if (jukebox_set_playlist_db_stream(config, jukebox->name, tpl_id) == T_OK) {
				j_return = json_pack("{sis{ss}}", "result", T_OK, "command", "name", json_string_value(json_object_get(json_object_get(j_command, "parameters"), "name")));
			} else {
				j_return = json_pack("{si}", "result", T_ERROR);
			}
    } else {
      j_return = json_pack("{si}", "result", T_ERROR);
    }
  } else {
    j_return = json_pack("{si}", "result", T_ERROR_PARAM);
  }
  return j_return;
}

int init_client_data_jukebox(struct _client_data_jukebox * client_data_jukebox) {
  if (client_data_jukebox != NULL) {
    client_data_jukebox->audio_buffer = NULL;
    client_data_jukebox->buffer_offset = 0;
    client_data_jukebox->server_remote_address = NULL;
    client_data_jukebox->api_prefix = NULL;
    client_data_jukebox->command = TALIESIN_PLAYLIST_MESSAGE_TYPE_NONE;
    client_data_jukebox->client_present = 0;
    client_data_jukebox->stream_name[0] = '\0';
    return T_OK;
  } else {
    return T_ERROR_PARAM;
  }
}

void clean_client_data_jukebox(struct _client_data_jukebox * client_data_jukebox) {
  if (client_data_jukebox != NULL) {
    o_free(client_data_jukebox);
  }
}

void * jukebox_run_thread(void * args) {
  struct _client_data_jukebox * client_data_jukebox = (struct _client_data_jukebox *)args;
  struct config_elements * config               = client_data_jukebox->jukebox->config;
  AVFormatContext        * input_format_context = NULL, * output_format_context = NULL;
  AVCodecContext         * input_codec_context  = NULL, * output_codec_context  = NULL;
  AVAudioResampleContext * resample_context     = NULL;
  AVAudioFifo            * fifo                 = NULL;
  int64_t pts = 0;
  int output_frame_size, finished = 0, error, data_present = 0, data_written = 0;
  
  if (!open_output_buffer_playlist(client_data_jukebox->audio_buffer, &output_format_context, &output_codec_context, &fifo)) {
    if (!open_input_file(client_data_jukebox->audio_buffer->file->path, &input_format_context, &input_codec_context, AVMEDIA_TYPE_AUDIO) && !init_resampler(input_codec_context, output_codec_context, &resample_context)) {
      y_log_message(Y_LOG_LEVEL_INFO, "Processing file '%s', duration: %" PRId64, client_data_jukebox->audio_buffer->file->path, (input_format_context->duration/AV_TIME_BASE));
      if (media_add_history(config, client_data_jukebox->jukebox->name, client_data_jukebox->jukebox->tpl_id, client_data_jukebox->audio_buffer->file->tm_id) != T_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_run_thread - Error media_add_history");
      }
      while (client_data_jukebox->audio_buffer->status != TALIESIN_STREAM_STATUS_STOPPED) {
        output_frame_size = output_codec_context->frame_size;
        finished          = 0;
        error             = 0;
        while (av_audio_fifo_size(fifo) < output_frame_size && !error) {
          if (read_decode_convert_and_store(fifo, input_format_context, input_codec_context, output_codec_context, resample_context, &finished)) {
            error = 1;
          }
          if (finished) {
            break;
          }
        }
        while ((av_audio_fifo_size(fifo) >= output_frame_size || (finished && av_audio_fifo_size(fifo) > 0)) && !error) {
          if (load_encode_and_return(fifo, output_codec_context, output_format_context, &pts, &data_present)) {
            error = 1;
          }
        }
        if (finished && !error) {
          do {
            if (encode_audio_frame_and_return(NULL, output_codec_context, output_format_context, &pts, &data_written)) {
              error = 1;
            }
          } while (data_written && !error);
          break;
        }
      }
      y_log_message(Y_LOG_LEVEL_INFO, "End processing file");
      if (client_data_jukebox->audio_buffer->status != TALIESIN_STREAM_STATUS_STOPPED) {
        if (av_write_trailer(output_format_context)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error av_write_trailer");
        }
      }
      client_data_jukebox->audio_buffer->complete = 1;
      avcodec_flush_buffers(output_codec_context);
      client_data_jukebox->audio_buffer->status = TALIESIN_STREAM_STATUS_COMPLETED;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_run_thread - Error opening input file or resample context");
    }
    if (resample_context) {
      avresample_close(resample_context);
      avresample_free(&resample_context);
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
      //avio_close(audio_stream->output_format_context->pb);
      avformat_free_context(output_format_context);
    }
    av_audio_fifo_free(fifo);
    pthread_mutex_lock(&client_data_jukebox->audio_buffer->buffer_lock);
    pthread_cond_wait(&client_data_jukebox->audio_buffer->buffer_cond, &client_data_jukebox->audio_buffer->buffer_lock);
    pthread_mutex_unlock(&client_data_jukebox->audio_buffer->buffer_lock);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "jukebox_run_thread - Error opening output buffer");
  }
  jukebox_audio_buffer_clean(client_data_jukebox->audio_buffer);
  clean_client_data_jukebox(client_data_jukebox);
  return NULL;
}

ssize_t u_jukebox_stream (void * cls, uint64_t pos, char * buf, size_t max) {
  struct _client_data_jukebox * client_data_jukebox = (struct _client_data_jukebox *)cls;
  size_t len;
  
  //y_log_message(Y_LOG_LEVEL_DEBUG, "u_jukebox_stream - start");
  if (client_data_jukebox->audio_buffer->status != TALIESIN_STREAM_STATUS_STOPPED || (client_data_jukebox->buffer_offset >= client_data_jukebox->audio_buffer->size && client_data_jukebox->audio_buffer->status == TALIESIN_STREAM_STATUS_COMPLETED)) {
    while (client_data_jukebox->buffer_offset + max > client_data_jukebox->audio_buffer->size && client_data_jukebox->audio_buffer->status == TALIESIN_STREAM_STATUS_STARTED) {
      usleep(50000);
    }
    if (client_data_jukebox->buffer_offset + max > client_data_jukebox->audio_buffer->size) {
      len = (client_data_jukebox->audio_buffer->size - client_data_jukebox->buffer_offset);
    } else {
      len = max;
    }
    
    memcpy(buf, client_data_jukebox->audio_buffer->data + client_data_jukebox->buffer_offset, len);
    client_data_jukebox->buffer_offset += len;
    return len;
  } else {
    return U_STREAM_END;
  }
}

void u_jukebox_stream_free(void * cls) {
  struct _client_data_jukebox * client_data_jukebox = (struct _client_data_jukebox *)cls;
  
  client_data_jukebox->client_present = 0;
  client_data_jukebox->jukebox->nb_client--;
  time(&client_data_jukebox->jukebox->last_seen);
  
  if (client_data_jukebox->audio_buffer->status == TALIESIN_STREAM_STATUS_STARTED) {
    client_data_jukebox->audio_buffer->status = TALIESIN_STREAM_STATUS_STOPPED;
  }
  pthread_mutex_lock(&client_data_jukebox->audio_buffer->buffer_lock);
  pthread_cond_signal(&client_data_jukebox->audio_buffer->buffer_cond);
  pthread_mutex_unlock(&client_data_jukebox->audio_buffer->buffer_lock);
}
