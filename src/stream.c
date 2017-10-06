/**
 *
 * Taliesin - Media server
 * 
 * Stream functions definitions
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

/**
 * Return the list of available webradio and playlist streams for the specified user
 */
json_t * stream_list(struct config_elements * config, const char * username) {
  int i;
  json_t * j_result = json_pack("{sis[]}", "result", T_OK, "stream"), * j_stream_info, * j_clients, * j_stream;
  
  if (j_result != NULL) {
    for (i=0; i<config->nb_webradio; i++) {
      if (config->webradio_set[i]->username == NULL || 0 == o_strcmp(config->webradio_set[i]->username, username)) {
        j_stream_info = webradio_get_info(config->webradio_set[i]);
        if (check_result_value(j_stream_info, T_OK)) {
          j_stream = json_copy(json_object_get(j_stream_info, "webradio"));
          if (j_stream != NULL) {
            j_clients = webradio_get_clients(config->webradio_set[i]);
            if (check_result_value(j_clients, T_OK)) {
              json_object_set(j_stream, "clients", json_object_get(j_clients, "clients"));
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error webradio_get_clients");
            }
            json_decref(j_clients);
            json_array_append_new(json_object_get(j_result, "stream"), j_stream);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error allocating resources for j_stream");
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error j_stream_info");
        }
        json_decref(j_stream_info);
      }
    }
    for (i=0; i<config->nb_jukebox; i++) {
      if (config->jukebox_set[i]->username == NULL || 0 == o_strcmp(config->jukebox_set[i]->username, username)) {
        j_stream_info = jukebox_get_info(config->jukebox_set[i]);
        if (check_result_value(j_stream_info, T_OK)) {
          j_stream = json_copy(json_object_get(j_stream_info, "jukebox"));
          if (j_stream != NULL) {
            j_clients = jukebox_get_clients(config->jukebox_set[i]);
            if (check_result_value(j_clients, T_OK)) {
              json_object_set(j_stream, "clients", json_object_get(j_clients, "clients"));
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error jukebox_get_clients");
            }
            json_decref(j_clients);
            json_array_append_new(json_object_get(j_result, "stream"), j_stream);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error allocating resources for j_stream");
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error j_stream_info");
        }
        json_decref(j_stream_info);
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "stream_list - Error allocating resources for j_result");
  }
  return j_result;
}

json_t * is_stream_parameters_valid(const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate) {
  json_t * j_result = json_array();
  
  if (j_result != NULL) {
    if (0 != o_strcasecmp("mp3", format) &&
        0 != o_strcasecmp("vorbis", format) &&
        0 != o_strcasecmp("flac", format)) {
      json_array_append_new(j_result, json_pack("{ss}", "format", "formats available are 'mp3', 'vorbis' or 'flac'"));
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
    y_log_message(Y_LOG_LEVEL_ERROR, "is_stream_parameters_valid - Error allocating resources for j_result");
  }
  return j_result;
}

static json_t * db_stream_get_media_list(struct config_elements * config, json_int_t ts_id) {
  json_t * j_result, * j_return;
  int res;
  char * query;
  
  query = msprintf("SELECT `%s`.`tds_path` AS tds_path, `%s`.`tds_name` AS data_source, `%s`.`tm_id` AS tm_id, `%s`.`tm_path` AS path FROM `%s`, `%s`, `%s` "
                   "WHERE `%s`.`ts_id`=%" JSON_INTEGER_FORMAT " AND `%s`.`tm_id`=`%s`.`tm_id` "
                   "AND `%s`.`tds_id`=`%s`.`tds_id` ORDER BY `%s`.`tse_id`",
                   TALIESIN_TABLE_DATA_SOURCE, TALIESIN_TABLE_DATA_SOURCE, TALIESIN_TABLE_MEDIA, TALIESIN_TABLE_MEDIA, 
                   TALIESIN_TABLE_DATA_SOURCE, TALIESIN_TABLE_MEDIA, TALIESIN_TABLE_STREAM_ELEMENT, 
                   TALIESIN_TABLE_STREAM_ELEMENT, ts_id, TALIESIN_TABLE_STREAM_ELEMENT, TALIESIN_TABLE_MEDIA,
                   TALIESIN_TABLE_MEDIA, TALIESIN_TABLE_DATA_SOURCE, TALIESIN_TABLE_STREAM_ELEMENT);
  res = h_execute_query_json(config->conn, query, &j_result);
  o_free(query);
  if (res == H_OK) {
    j_return = json_pack("{siso}", "result", T_OK, "media", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "db_stream_get_media_list - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * db_stream_list(struct config_elements * config) {
  json_t * j_query, * j_result, * j_return, * j_element, * j_media;
  int res;
  size_t index;
  
  j_query = json_pack("{sss[ssssssssssss]}",
                      "table",
                      TALIESIN_TABLE_STREAM,
                      "columns",
                        "ts_id AS id",
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
      j_media = db_stream_get_media_list(config, json_integer_value(json_object_get(j_element, "id")));
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