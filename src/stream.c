/**
 *
 * Taliesin - Media server
 * 
 * Stream functions definitions
 *
 * Copyright 2017-2023 Nicolas Mora <mail@babelouest.org>
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

static json_t * _db_stream_get_media_list_from_id(struct config_elements * config, json_int_t ts_id) {
  json_t * j_query, * j_result, * j_return;
  int res;
  
  j_query = json_pack("{sss[s]s{sI}}",
                      "table", TALIESIN_TABLE_STREAM_ELEMENT,
                      "columns",
                        "tm_id",
                      "where",
                        "ts_id", ts_id);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    j_return = json_pack("{siso}", "result", T_OK, "media", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "_db_stream_get_media_list_from_id - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

/**
 * Return the list of available webradio and playlist streams for the specified user
 */
json_t * stream_list(struct config_elements * config, const char * username) {
  size_t i;
  json_t * j_result = json_pack("{sis[]}", "result", T_OK, "stream"), * j_stream_info;
  
  if (j_result != NULL) {
    for (i=0; i<config->nb_webradio; i++) {
      if (config->webradio_set[i]->username == NULL || 0 == o_strcmp(config->webradio_set[i]->username, username)) {
        j_stream_info = webradio_get_info(config->webradio_set[i]);
        if (check_result_value(j_stream_info, T_OK)) {
          json_array_append(json_object_get(j_result, "stream"), json_object_get(j_stream_info, "webradio"));
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error webradio_get_info");
        }
        json_decref(j_stream_info);
      }
    }
    for (i=0; i<config->nb_jukebox; i++) {
      if (config->jukebox_set[i]->username == NULL || 0 == o_strcmp(config->jukebox_set[i]->username, username)) {
        j_stream_info = jukebox_get_info(config->jukebox_set[i]);
        if (check_result_value(j_stream_info, T_OK)) {
          json_array_append(json_object_get(j_result, "stream"), json_object_get(j_stream_info, "jukebox"));
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error jukebox_get_info");
        }
        json_decref(j_stream_info);
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error allocating resources for j_result");
  }
  return j_result;
}

int is_stream_name_taken(struct config_elements * config, const char * stream_url) {
  unsigned int i;

  for (i=0; i<config->nb_webradio; i++) {
    if (0 == o_strcmp(stream_url, config->webradio_set[i]->name)) {
      return 1;
    }
  }
  for (i=0; i<config->nb_jukebox; i++) {
    if (0 == o_strcmp(stream_url, config->jukebox_set[i]->name)) {
      return 1;
    }
  }
  return 0;
}

void is_stream_url_valid(struct config_elements * config, const char * stream_url, json_t * j_result) {
  unsigned int i, j, found;
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_";
  if (o_strlen(stream_url) > TALIESIN_PLAYLIST_NAME_LENGTH || o_strlen(stream_url) < 3) {
    json_array_append_new(j_result, json_pack("{ss}", "stream_url", "stream_url must be a string of minimum 3 and maximum "TALIESIN_PLAYLIST_NAME_LENGTH_STR" characters"));
  } else {
    for (i=0; i<o_strlen(stream_url); i++) {
      found = 0;
      for (j=0; j<sizeof(charset); j++) {
        if (stream_url[i] == charset[j]) {
          found = 1;
          break;
        }
      }
      if (!found) {
        json_array_append_new(j_result, json_pack("{ss}", "stream_url", "invalid stream_url charset"));
        break;
      }
    }
    if (!pthread_mutex_lock(&config->playlist_lock)) {
      if (is_stream_name_taken(config, stream_url)) {
        json_array_append_new(j_result, json_pack("{ss}", "stream_url", "invalid stream_url name"));
      }
      pthread_mutex_unlock(&config->playlist_lock);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "is_stream_url_valid - Error locking playlist_lock");
    }
  }
}

json_t * is_stream_libav_parameters_valid(struct config_elements * config, int webradio, int icecast, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate) {
  json_t * j_result = json_array();

  if (j_result != NULL) {
    if (0 != o_strcasecmp("mp3", format) &&
        0 != o_strcasecmp("vorbis", format) &&
        0 != o_strcasecmp("flac", format)) {
      json_array_append_new(j_result, json_pack("{ss}", "format", "formats available are 'mp3', 'vorbis' or 'flac'"));
    }
    
    if (icecast && o_strnullempty(config->icecast_host)) {
      json_array_append_new(j_result, json_pack("{ss}", "icecast", "icecast streaming not allowed"));
    }
    
    if (webradio) {
      if (!icecast) {
        if (0 != o_strcasecmp("mp3", format)) {
          json_array_append_new(j_result, json_pack("{ss}", "format", "webradio allows only 'mp3' format"));
        }
      } else {
        if (0 != o_strcasecmp("mp3", format) && 0 != o_strcasecmp("vorbis", format)) {
          json_array_append_new(j_result, json_pack("{ss}", "format", "icecast allows only 'mp3' of 'vorbis' formats"));
        }
      }
    }

    if (channels > 2) {
      json_array_append_new(j_result, json_pack("{ss}", "channels", "channels can be 1 (mono) or 2 (stereo)"));
    }
    
    if (sample_rate != 8000 && sample_rate != 11025 && sample_rate != 22050 && sample_rate != 32000 && sample_rate != 44100 && sample_rate != 48000) {
      json_array_append_new(j_result, json_pack("{ss}", "sample_rate", "sample_rate can be 8000, 11025, 22050, 32000, 44100 or 48000"));
    }
    
    if (0 != o_strcasecmp("flac", format) && bit_rate != 32000 && bit_rate != 96000 && bit_rate != 128000 && bit_rate != 192000 && bit_rate != 256000 && bit_rate != 320000) {
      json_array_append_new(j_result, json_pack("{ss}", "bit_rate", "bit_rate can be 32000, 96000, 128000, 192000, 256000 or 320000"));
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "is_stream_libav_parameters_valid - Error allocating resources for j_result");
  }
  return j_result;
}

json_t * is_stream_parameters_valid(struct config_elements * config, int webradio, int icecast, const char * stream_url, const char * scope, int is_admin, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate) {
  json_t * j_result;

  j_result = is_stream_libav_parameters_valid(config, webradio, icecast, format, channels, sample_rate, bit_rate);
  if (j_result != NULL) {
    if ((!is_admin && 0 == o_strcmp(TALIESIN_SCOPE_ALL, scope)) || (scope != NULL && 0 != o_strcmp(TALIESIN_SCOPE_ME, scope) && 0 != o_strcmp(TALIESIN_SCOPE_ALL, scope))) {
      json_array_append_new(j_result, json_pack("{ss}", "scope", "scope value is an optional string and can be only " TALIESIN_SCOPE_ALL " or " TALIESIN_SCOPE_ME ", only administrator can add playlists for all users"));
    }
    
    if (!o_strnullempty(stream_url)) {
      is_stream_url_valid(config, stream_url, j_result);
    }
  }
  return j_result;
}

json_t * db_stream_list(struct config_elements * config) {
  json_t * j_query, * j_result, * j_return, * j_element, * j_media;
  int res;
  size_t index;
  
  j_query = json_pack("{sss[ssssssssssss]}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "columns",
                        "ts_id",
                        "ts_username AS username",
                        "ts_name AS name",
                        "ts_display_name AS display_name",
                        "tpl_id",
                        "ts_random AS random",
                        "ts_index AS current_index",
                        "ts_webradio AS webradio",
                        "ts_format AS format",
                        "ts_channels AS channels",
                        "ts_sample_rate AS sample_rate",
                        "ts_bitrate AS bitrate");
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    json_array_foreach(j_result, index, j_element) {
      j_media = _db_stream_get_media_list_from_id(config, json_integer_value(json_object_get(j_element, "ts_id")));
      if (check_result_value(j_media, T_OK)) {
        json_object_set(j_element, "media", json_object_get(j_media, "media"));
      }
      json_decref(j_media);
    }
    j_return = json_pack("{siso}", "result", T_OK, "stream", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "db_stream_list - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

int db_stream_reload_file_lists(struct config_elements * config) {
  int ret;
  size_t index, index2, i;
  json_t * j_stream, * j_element, * j_stream_list = db_stream_list(config);
  
  if (check_result_value(j_stream_list, T_OK)) {
    json_array_foreach(json_object_get(j_stream_list, "stream"), index, j_stream) {
      if (json_integer_value(json_object_get(j_stream, "webradio"))) {
        for (i=0; i<config->nb_webradio; i++) {
          if (0 == o_strcmp(config->webradio_set[i]->name, json_string_value(json_object_get(j_stream, "name")))) {
            if (pthread_mutex_lock(&config->webradio_set[i]->file_list->file_lock)) {
              y_log_message(Y_LOG_LEVEL_ERROR, "db_stream_reload_file_lists - Error lock stream %s", config->webradio_set[i]->display_name);
            } else {
              if (file_list_empty_nolock(config->webradio_set[i]->file_list) == T_OK) {
                json_array_foreach(json_object_get(j_stream, "media"), index2, j_element) {
                  if (file_list_enqueue_new_file_nolock(config->webradio_set[i]->file_list, json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error adding file %s", json_string_value(json_object_get(j_element, "full_path")));
                  }
                }
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "db_stream_reload_file_lists - Error file_list_empty_nolock stream %s", config->webradio_set[i]->display_name);
              }
              pthread_mutex_unlock(&config->webradio_set[i]->file_list->file_lock);
            }
            break;
          }
        }
      } else {
        for (i=0; i<config->nb_jukebox; i++) {
          if (0 == o_strcmp(config->jukebox_set[i]->name, json_string_value(json_object_get(j_stream, "name")))) {
            if (pthread_mutex_lock(&config->jukebox_set[i]->file_list->file_lock)) {
              y_log_message(Y_LOG_LEVEL_ERROR, "db_stream_reload_file_lists - Error lock stream %s", config->jukebox_set[i]->display_name);
            } else {
              if (file_list_empty_nolock(config->jukebox_set[i]->file_list) == T_OK) {
                json_array_foreach(json_object_get(j_stream, "media"), index2, j_element) {
                  if (file_list_enqueue_new_file_nolock(config->jukebox_set[i]->file_list, json_integer_value(json_object_get(j_element, "tm_id"))) != T_OK) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "add_webradio_from_playlist - Error adding file %s", json_string_value(json_object_get(j_element, "full_path")));
                  }
                }
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "db_stream_reload_file_lists - Error file_list_empty_nolock stream %s", config->jukebox_set[i]->display_name);
              }
              pthread_mutex_unlock(&config->jukebox_set[i]->file_list->file_lock);
            }
            break;
          }
        }
      }
    }
    ret = T_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "db_stream_reload_file_lists - Error db_stream_list");
    ret = T_ERROR_DB;
  }
  json_decref(j_stream_list);
  return ret;
}

json_t * db_stream_get_media_list_from_name(struct config_elements * config, const char * name) {
  json_t * j_query, * j_result, * j_return;
  int res;
  char * name_clause = h_build_where_clause(config->conn, "IN (SELECT ts_id FROM "TALIESIN_TABLE_STREAM" WHERE ts_name=%s)", name);
  
  j_query = json_pack("{sss[s]s{s{ssss}}}",
                      "table", TALIESIN_TABLE_STREAM_ELEMENT,
                      "columns",
                        "tm_id",
                      "where",
                        "ts_id",
                          "operator", "raw",
                          "value", name_clause);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  h_free(name_clause);
  if (res == H_OK) {
    j_return = json_pack("{siso}", "result", T_OK, "media", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "_db_stream_get_media_list_from_id - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

int stream_db_media_add_list(struct config_elements * config, const char * name, json_t * j_array_media) {
  json_t * j_query, * j_result, * j_element = NULL;
  json_int_t ts_id;
  int res, ret;
  size_t index = 0;

  j_query = json_pack("{sss[s]s{ss}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "columns",
                        "ts_id",
                      "where",
                        "ts_name",
                        name);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      ts_id = json_integer_value(json_object_get(json_array_get(j_result, 0), "ts_id"));
      j_query = json_pack("{sss[]}",
                          "table",
                          TALIESIN_TABLE_STREAM_ELEMENT,
                          "values");
      if (j_query != NULL) {
        json_array_foreach(j_array_media, index, j_element) {
          json_array_append_new(json_object_get(j_query, "values"), json_pack("{sIsO}", "ts_id", ts_id, "tm_id", j_element));
        }
        res = h_insert(config->conn, j_query, NULL);
        json_decref(j_query);
        if (res != H_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_db_stream_media_list - Error executing j_query (2)");
          ret = T_ERROR_DB;
        } else {
          ret = T_OK;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_db_stream_media_list - Error allocatinng resources for j_query");
        ret = T_ERROR_MEMORY;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_db_stream_media_list - stream not found");
      ret = T_ERROR_MEMORY;
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "webradio_add_db_stream_media_list - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

int stream_db_media_remove_list(struct config_elements * config, const char * name, json_t * j_array_media) {
  json_t * j_query, * j_result;
  json_int_t ts_id;
  int res, ret;

  j_query = json_pack("{sss[s]s{ss}}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "columns",
                        "ts_id",
                      "where",
                        "ts_name", name);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      ts_id = json_integer_value(json_object_get(json_array_get(j_result, 0), "ts_id"));
      j_query = json_pack("{sss{sIs{sssO}}}",
                          "table",
                          TALIESIN_TABLE_STREAM_ELEMENT,
                          "where",
                            "ts_id", ts_id,
                            "tm_id",
                              "operator", "IN",
                              "value", j_array_media);
      if (j_query != NULL) {
        res = h_delete(config->conn, j_query, NULL);
        json_decref(j_query);
        if (res != H_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "stream_db_media_remove_list - Error executing j_query (2)");
          ret = T_ERROR_DB;
        } else {
          ret = T_OK;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "stream_db_media_remove_list - Error allocatinng resources for j_query");
        ret = T_ERROR_MEMORY;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "stream_db_media_remove_list - stream not found");
      ret = T_ERROR_MEMORY;
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "stream_db_media_remove_list - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}
