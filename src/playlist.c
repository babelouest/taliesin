/**
 *
 * Taliesin - Media server
 * 
 * Playlists functions definitions
 *
 * Copyright 2017-2021 Nicolas Mora <mail@babelouest.org>
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

json_t * playlist_get_stream_attached(struct config_elements * config, json_int_t tpl_id) {
  json_t * j_return = NULL, * j_stream_array;
  int i;
  
  if (tpl_id) {
    j_stream_array = json_array();
    if (j_stream_array != NULL) {
      for (i=0; i<config->nb_webradio; i++) {
        if (config->webradio_set[i]->tpl_id == tpl_id) {
          json_array_append_new(j_stream_array, json_pack("{ssssso}", "name", config->webradio_set[i]->name, "display_name", config->webradio_set[i]->display_name, "webradio", json_true()));
        }
      }
      for (i=0; i<config->nb_jukebox; i++) {
        if (config->jukebox_set[i]->tpl_id == tpl_id) {
          json_array_append_new(j_stream_array, json_pack("{ssssso}", "name", config->jukebox_set[i]->name, "display_name", config->jukebox_set[i]->display_name, "webradio", json_false()));
        }
      }
      j_return = json_pack("{siso}", "result", T_OK, "stream", j_stream_array);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "playlist_get_stream_attached - Error allocating resources for j_stream_array");
      j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_get_stream_attached - Error tpl_id is null");
    j_return = json_pack("{si}", "result", T_ERROR_PARAM);
  }
  return j_return;
}

json_t * playlist_get_media_list(struct config_elements * config, json_int_t tpl_id, int with_id, size_t offset, size_t limit) {
  json_t * j_result, * j_return;
  int res;
  char * query, * tmp;
  
  query = msprintf("SELECT %s`%s`.`tds_name` AS data_source, %s`%s`.`tm_path` AS path FROM `%s`, `%s`, `%s` "
                   "WHERE `%s`.`tpl_id`=%" JSON_INTEGER_FORMAT " AND `%s`.`tm_id`=`%s`.`tm_id` "
                   "AND `%s`.`tds_id`=`%s`.`tds_id` ORDER BY `%s`.`tds_name`%s, `%s`.`tm_path`%s, `%s`.`tm_name`%s",
                   with_id?"`"TALIESIN_TABLE_DATA_SOURCE"`.`tds_path` AS tds_path, ":"" , TALIESIN_TABLE_DATA_SOURCE,
                   (with_id?"`"TALIESIN_TABLE_MEDIA"`.`tm_id` AS tm_id, ":""), TALIESIN_TABLE_MEDIA, TALIESIN_TABLE_DATA_SOURCE, TALIESIN_TABLE_MEDIA,
                   TALIESIN_TABLE_PLAYLIST_ELEMENT, TALIESIN_TABLE_PLAYLIST_ELEMENT, tpl_id, TALIESIN_TABLE_PLAYLIST_ELEMENT, TALIESIN_TABLE_MEDIA,
                   TALIESIN_TABLE_MEDIA, TALIESIN_TABLE_DATA_SOURCE, TALIESIN_TABLE_DATA_SOURCE, config->conn->type==HOEL_DB_TYPE_MARIADB?"":" COLLATE NOCASE",
                   TALIESIN_TABLE_MEDIA, config->conn->type==HOEL_DB_TYPE_MARIADB?"":" COLLATE NOCASE",
                   TALIESIN_TABLE_MEDIA, config->conn->type==HOEL_DB_TYPE_MARIADB?"":" COLLATE NOCASE");
  if (limit) {
    tmp = msprintf("%s LIMIT %zu", query, limit);
    o_free(query);
    query = tmp;
  }
  if (offset) {
    tmp = msprintf("%s OFFSET %zu", query, limit);
    o_free(query);
    query = tmp;
  }
  res = h_execute_query_json(config->conn, query, &j_result);
  o_free(query);
  if (res == H_OK) {
    j_return = json_pack("{siso}", "result", T_OK, "media", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_get_media - Error executing j_query (1)");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * playlist_count_element(struct config_elements * config, json_int_t tpl_id) {
  json_t * j_query, * j_result, * j_return;
  int res;
  
  j_query = json_pack("{sss[s]s{sI}}",
                      "table",
                      TALIESIN_TABLE_PLAYLIST_ELEMENT,
                      "columns",
                        "COUNT(`tpe_id`) AS `count`",
                      "where",
                        "tpl_id",
                        tpl_id);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result)) {
      j_return = json_pack("{sisI}", "result", T_OK, "count", json_integer_value(json_object_get(json_array_get(j_result, 0), "count")));
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "playlist_count_element - Error no result");
      j_return = json_pack("{si}", "result", T_ERROR);
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_count_element - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * playlist_list(struct config_elements * config, const char * username) {
  json_t * j_query, * j_result, * j_element, * j_return, * j_count, * j_stream;
  int res;
  char * clause_username, * temp;
  size_t index;
  
  temp = h_escape_string(config->conn, username);
  clause_username = msprintf("`tpl_username` = '%s' OR `tpl_username` IS NULL", temp);
  o_free(temp);
  j_query = json_pack("{sss[ssss]s{s{ssss}}}",
                      "table",
                      TALIESIN_TABLE_PLAYLIST,
                      "columns",
                        "tpl_id",
                        "tpl_name AS name",
                        "tpl_description AS description",
                        "tpl_username",
                      "where",
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_username);
  o_free(clause_username);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    json_array_foreach(j_result, index, j_element) {
      j_count = playlist_count_element(config, json_integer_value(json_object_get(j_element, "tpl_id")));
      if (check_result_value(j_count, T_OK)) {
        json_object_set_new(j_element, "elements", json_integer(json_integer_value(json_object_get(j_count, "count"))));
      }
      json_decref(j_count);
      j_stream = playlist_get_stream_attached(config, json_integer_value(json_object_get(j_element, "tpl_id")));
      if (check_result_value(j_stream, T_OK)) {
        json_object_set(j_element, "stream", json_object_get(j_stream, "stream"));
      }
      json_decref(j_stream);
      if (json_object_get(j_element, "tpl_username")  != json_null()) {
        json_object_set_new(j_element, "scope", json_string(TALIESIN_SCOPE_ME));
      } else {
        json_object_set_new(j_element, "scope", json_string(TALIESIN_SCOPE_ALL));
      }
      json_object_del(j_element, "tpl_id");
      json_object_del(j_element, "tpl_username");
    }
    j_return = json_pack("{sisO}", "result", T_OK, "playlist", j_result);
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_list - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * playlist_get(struct config_elements * config, const char * username, const char * name, int with_id, size_t offset, size_t limit) {
  json_t * j_query, * j_result, * j_media, * j_return, * j_count, * j_stream;
  int res;
  char * clause_username, * temp;
  
  temp = h_escape_string(config->conn, username);
  clause_username = msprintf("(`tpl_username` = '%s' OR `tpl_username` IS NULL)", temp);
  o_free(temp);
  j_query = json_pack("{sss[ssss]s{s{ssss}ss}}",
                      "table",
                      TALIESIN_TABLE_PLAYLIST,
                      "columns",
                        "tpl_id",
                        "tpl_name AS name",
                        "tpl_description AS description",
                        "tpl_username",
                      "where",
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_username,
                        "tpl_name",
                        name);
  o_free(clause_username);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      j_count = playlist_count_element(config, json_integer_value(json_object_get(json_array_get(j_result, 0), "tpl_id")));
      if (check_result_value(j_count, T_OK)) {
        json_object_set_new(json_array_get(j_result, 0), "elements", json_integer(json_integer_value(json_object_get(j_count, "count"))));
      }
      json_decref(j_count);
      j_stream = playlist_get_stream_attached(config, json_integer_value(json_object_get(json_array_get(j_result, 0), "tpl_id")));
      if (check_result_value(j_stream, T_OK)) {
        json_object_set(json_array_get(j_result, 0), "stream", json_object_get(j_stream, "stream"));
      }
      json_decref(j_stream);
      j_media = playlist_get_media_list(config, json_integer_value(json_object_get(json_array_get(j_result, 0), "tpl_id")), with_id, offset, limit);
      if (check_result_value(j_media, T_OK)) {
        json_object_set(json_array_get(j_result, 0), "media", json_object_get(j_media, "media"));
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "playlist_get - Error check_result_value");
      }
      if (json_object_get(json_array_get(j_result, 0), "tpl_username")  != json_null()) {
        json_object_set_new(json_array_get(j_result, 0), "scope", json_string(TALIESIN_SCOPE_ME));
      } else {
        json_object_set_new(json_array_get(j_result, 0), "scope", json_string(TALIESIN_SCOPE_ALL));
      }
      json_decref(j_media);
      if (!with_id) {
        json_object_del(json_array_get(j_result, 0), "tpl_id");
      }
      json_object_del(json_array_get(j_result, 0), "tpl_username");
      j_return = json_pack("{sisO}", "result", T_OK, "playlist", json_array_get(j_result, 0));
    } else {
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_get - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * playlist_get_by_id(struct config_elements * config, json_int_t tpl_id) {
  json_t * j_query, * j_result, * j_media, * j_return;
  int res;
  
  j_query = json_pack("{sss[ssss]s{sI}}",
                      "table",
                      TALIESIN_TABLE_PLAYLIST,
                      "columns",
                        "tpl_id",
                        "tpl_name AS name",
                        "tpl_description AS description",
                        "tpl_username",
                      "where",
                        "tpl_id",
                        tpl_id);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      j_media = playlist_get_media_list(config, json_integer_value(json_object_get(json_array_get(j_result, 0), "tpl_id")), 1, 0, 0);
      if (check_result_value(j_media, T_OK)) {
        json_object_set(json_array_get(j_result, 0), "media", json_object_get(j_media, "media"));
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "playlist_get - Error check_result_value");
      }
      if (json_object_get(json_array_get(j_result, 0), "tpl_username")  != json_null()) {
        json_object_set_new(json_array_get(j_result, 0), "scope", json_string(TALIESIN_SCOPE_ME));
      } else {
        json_object_set_new(json_array_get(j_result, 0), "scope", json_string(TALIESIN_SCOPE_ALL));
      }
      json_decref(j_media);
      json_object_del(json_array_get(j_result, 0), "tpl_username");
      j_return = json_pack("{sisO}", "result", T_OK, "playlist", json_array_get(j_result, 0));
    } else {
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_get - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * is_playlist_element_list_valid(struct config_elements * config, int is_admin, const char * username, json_t * j_element_list) {
  json_t * j_return = json_array(), * j_element;
  size_t index;
  
  if (j_return != NULL) {
    if (!json_is_array(j_element_list)) {
      json_array_append_new(j_return, json_pack("{ss}", "parameters", "parameters must be a json array"));
    } else if (json_array_size(j_element_list) == 0) {
      json_array_append_new(j_return, json_pack("{ss}", "parameters", "parameters must be a json array of at least one element"));
    } else {
      json_array_foreach(j_element_list, index, j_element) {
        if (!is_valid_path_element_parameter(config, j_element, username, is_admin) && !is_valid_category_element_parameter(config, j_element, username, is_admin) && !is_valid_playlist_element_parameter(config, j_element, username)) {
          json_array_append_new(j_return, json_pack("{ss}", "parameter", "parameter is not a valid element"));
        }
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "is_playlist_element_list_valid - Error allocating resources for j_return");
  }
  return j_return;
}

json_t * is_playlist_valid(struct config_elements * config, const char * username, int is_admin, json_t * j_playlist, int update, int with_media) {
  json_t * j_return = json_array(), * j_element = NULL;
  size_t index;
  
  if (j_return != NULL) {
    if (j_playlist == NULL || !json_is_object(j_playlist)) {
      json_array_append_new(j_return, json_pack("{ss}", "playlist", "playlist must be a valid JSON object"));
    } else {
      if (!update) {
        if (json_object_get(j_playlist, "name") == NULL || !json_is_string(json_object_get(j_playlist, "name")) || json_string_length(json_object_get(j_playlist, "name")) == 0 || json_string_length(json_object_get(j_playlist, "name")) > 128) {
          json_array_append_new(j_return, json_pack("{ss}", "name", "name is mandatory and must be a non empty string of maximum 128 characters"));
        } else {
          j_element = playlist_get(config, username, json_string_value(json_object_get(j_playlist, "name")), 0, 0, 1);
          if (check_result_value(j_element, T_OK)) {
            json_array_append_new(j_return, json_pack("{ss}", "name", "playlist name already exist"));
          }
          json_decref(j_element);
        }
      }
      
      if (json_object_get(j_playlist, "description") != NULL && (!json_is_string(json_object_get(j_playlist, "description")) || json_string_length(json_object_get(j_playlist, "description")) > 512)) {
        json_array_append_new(j_return, json_pack("{ss}", "description", "description is an optional string of maximum 512 characters"));
      }
      
      if (json_object_get(j_playlist, "cover") != NULL && !json_is_string(json_object_get(j_playlist, "cover"))) {
        json_array_append_new(j_return, json_pack("{ss}", "cover", "cover is an optional string"));
      } else if (json_object_get(j_playlist, "cover") != NULL && is_valid_b64_image((unsigned char *)json_string_value(json_object_get(j_playlist, "cover"))) != T_OK) {
        json_array_append_new(j_return, json_pack("{ss}", "cover", "cover is not a valid image encoded in base64"));
      }
      
      if (with_media) {
        if (json_object_get(j_playlist, "media") != NULL && !json_is_array(json_object_get(j_playlist, "media"))) {
          json_array_append_new(j_return, json_pack("{ss}", "media", "media must be a JSON array"));
        } else if (json_object_get(j_playlist, "media") != NULL) {
          json_array_foreach(json_object_get(j_playlist, "media"), index, j_element) {
            if (!is_valid_path_element_parameter(config, j_element, username, is_admin) && !is_valid_category_element_parameter(config, j_element, username, is_admin) && !is_valid_playlist_element_parameter(config, j_element, username)) {
              json_array_append_new(j_return, json_pack("{ss}", "media", "media is not a valid object or does not exist"));
            }
          }
        }
      }

      if (json_object_get(j_playlist, "scope") != NULL &&
          (
            !json_is_string(json_object_get(j_playlist, "scope")) ||
            (
              o_strcmp(json_string_value(json_object_get(j_playlist, "scope")), TALIESIN_SCOPE_ALL) != 0 &&
              o_strcmp(json_string_value(json_object_get(j_playlist, "scope")), TALIESIN_SCOPE_ME) != 0
            )
            || (o_strcmp(json_string_value(json_object_get(j_playlist, "scope")), TALIESIN_SCOPE_ALL) == 0 && !is_admin)
          )
        ) {
        json_array_append_new(j_return, json_pack("{ss}", "scope", "scope value is an optional string and can be only " TALIESIN_SCOPE_ALL " or " TALIESIN_SCOPE_ME ", only administrator can add playlists for all users"));
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "is_playlist_valid - Error allocating resources for j_return");
  }
  return j_return;
}

int playlist_replace_element_list(struct config_elements * config, json_int_t tpl_id, json_t * j_playlist) {
  json_t * j_query, * j_element;
  int res, ret;
  size_t index;
  
  // Delete all existing elements
  j_query = json_pack("{sss{sI}}",
                      "table",
                      TALIESIN_TABLE_PLAYLIST_ELEMENT,
                      "where",
                        "tpl_id",
                        tpl_id);
  res = h_delete(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(json_object_get(j_playlist, "media"))) {
      // Insert new elements
      j_query = json_pack("{sss[]}",
                          "table",
                          TALIESIN_TABLE_PLAYLIST_ELEMENT,
                          "values");
      if (j_query != NULL) {
        json_array_foreach(json_object_get(j_playlist, "media"), index, j_element) {
          json_array_append_new(json_object_get(j_query, "values"), json_pack("{sIsO}", "tpl_id", tpl_id, "tm_id", json_object_get(j_element, "tm_id")));
        }
        res = h_insert(config->conn, j_query, NULL);
        json_decref(j_query);
        if (res != H_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "playlist_replace_element_list - Error executing j_query (2)");
          ret = T_ERROR_DB;
        } else {
          ret = T_OK;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "playlist_replace_element_list - Error allocating resources for j_query");
        ret = T_ERROR_MEMORY;
      }
    } else {
      ret = T_OK;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_replace_element_list - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

json_int_t playlist_add(struct config_elements * config, const char * username, json_t * j_playlist, struct _t_file_list * file_list) {
  json_t * j_query, * j_last_id;
  json_int_t tic_id = 0, tpl_id = -1;
  int res;
  
  j_query = json_pack("{sss{sossss}}",
                      "table",
                      TALIESIN_TABLE_PLAYLIST,
                      "values",
                        "tpl_username",
                        (0!=o_strcmp("all", json_string_value(json_object_get(j_playlist, "scope"))))?json_string(username):json_null(),
                        "tpl_name",
                        json_string_value(json_object_get(j_playlist, "name")),
                        "tpl_description",
                        json_object_get(j_playlist, "description")!=NULL?json_string_value(json_object_get(j_playlist, "description")):"");
  if (tic_id) {
    json_object_set_new(json_object_get(j_query, "values"), "tic_id", json_integer(tic_id));
  }
  res = h_insert(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    j_last_id = h_last_insert_id(config->conn);
    tpl_id = json_integer_value(j_last_id);
    if (!tpl_id) {
      y_log_message(Y_LOG_LEVEL_ERROR, "playlist_add - Error getting last id");
    }
    json_decref(j_last_id);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_add - Error executing j_query (1)");
  }
  return tpl_id;
}

int playlist_can_update(json_t * j_playlist, int is_admin) {
  return (is_admin || o_strcmp(TALIESIN_SCOPE_ME, json_string_value(json_object_get(j_playlist, "scope"))) == 0);
}

int playlist_set(struct config_elements * config, json_int_t tpl_id, json_t * j_playlist) {
  json_t * j_query;
  int res, ret;
  json_int_t tic_id = 0;
  
  j_query = json_pack("{sss{ss}s{sI}}",
                      "table",
                      TALIESIN_TABLE_PLAYLIST,
                      "set",
                        "tpl_description",
                        json_object_get(j_playlist, "description")!=NULL?json_string_value(json_object_get(j_playlist, "description")):"",
                      "where",
                        "tpl_id",
                        tpl_id);
  if (tic_id) {
    json_object_set_new(json_object_get(j_query, "set"), "tic_id", json_integer(tic_id));
  }
  res = h_update(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (playlist_replace_element_list(config, tpl_id, j_playlist) != T_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "playlist_set - Error replace playlist elements");
      ret = T_ERROR;
    } else {
      ret = T_OK;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_set - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

int playlist_delete(struct config_elements * config, json_int_t tpl_id) {
  json_t * j_query;
  int res;
  
  j_query = json_pack("{sss{sI}}",
                      "table",
                      TALIESIN_TABLE_PLAYLIST,
                      "where",
                        "tpl_id",
                        tpl_id);
  res = h_delete(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    return T_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_delete - Error executing j_query");
    return T_ERROR_DB;
  }
}

int playlist_add_media(struct config_elements * config, json_int_t tpl_id, json_t * media_list) {
  json_t * j_query, * j_element;
  int res, ret;
  size_t index;
  
  if (media_list != NULL && json_array_size(media_list) > 0) {
    // Insert new elements
    j_query = json_pack("{sss[]}",
                        "table",
                        TALIESIN_TABLE_PLAYLIST_ELEMENT,
                        "values");
    if (j_query != NULL) {
      json_array_foreach(media_list, index, j_element) {
        json_array_append_new(json_object_get(j_query, "values"), json_pack("{sIsO}", "tpl_id", tpl_id, "tm_id", json_object_get(j_element, "tm_id")));
      }
      res = h_insert(config->conn, j_query, NULL);
      json_decref(j_query);
      if (res != H_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "playlist_add_media - Error executing j_query");
        ret = T_ERROR_DB;
      } else {
        ret = T_OK;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "playlist_add_media - Error allocating resources for j_query");
      ret = T_ERROR_MEMORY;
    }
  } else{
    ret = T_OK;
  }
  return ret;
}

int playlist_delete_media(struct config_elements * config, json_int_t tpl_id, json_t * media_list) {
  json_t * j_query, * j_element, * j_tm_id_array = json_array();
  int res, ret;
  size_t index;
  
  if (j_tm_id_array != NULL) {
    json_array_foreach(media_list, index, j_element) {
      json_array_append(j_tm_id_array, json_object_get(j_element, "tm_id"));
    }
    j_query = json_pack("{sss{sIs{ssso}}}",
                        "table",
                        TALIESIN_TABLE_PLAYLIST_ELEMENT,
                        "where",
                          "tpl_id",
                          tpl_id,
                          "tm_id",
                            "operator",
                            "IN",
                            "value",
                            j_tm_id_array);
    res = h_delete(config->conn, j_query, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      ret = T_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "playlist_delete_media - Error executing j_query");
      ret = T_ERROR_DB;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_delete_media - Error allocating resources for j_tm_id_array");
    ret = T_ERROR_MEMORY;
  }
  return ret;
}

json_t * playlist_has_media(struct config_elements * config, json_int_t tpl_id, json_t * media_list, size_t offset, size_t limit) {
  json_t * j_query, * j_element, * j_result, * j_tm_id_array = json_array(), * j_return, * j_media;
  int res;
  size_t index;
  
  if (j_tm_id_array != NULL) {
    json_array_foreach(media_list, index, j_element) {
      json_array_append(j_tm_id_array, json_object_get(j_element, "tm_id"));
    }
    j_query = json_pack("{sss[s]s{sIs{ssso}}sisi}",
                        "table",
                        TALIESIN_TABLE_PLAYLIST_ELEMENT,
                        "columns",
                          "tm_id",
                        "where",
                          "tpl_id",
                          tpl_id,
                          "tm_id",
                            "operator",
                            "IN",
                            "value",
                            j_tm_id_array,
                        "limit",
                        limit,
                        "offset",
                        offset);
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      j_return = json_pack("{sis[]}", "result", T_OK, "media");
      if (j_return != NULL) {
        json_array_foreach(j_result, index, j_element) {
          j_media = media_get_by_id(config, json_integer_value(json_object_get(j_element, "tm_id")));
          if (check_result_value(j_media, T_OK)) {
            json_object_del(json_object_get(j_media, "media"), "tm_id");
            json_array_append(json_object_get(j_return, "media"), json_object_get(j_media, "media"));
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "playlist_delete_media - Error media_get_by_id");
          }
          json_decref(j_media);
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "playlist_delete_media - Error allocating resources for j_return");
        j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
      }
      json_decref(j_result);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "playlist_delete_media - Error executing j_query");
      j_return = json_pack("{si}", "result", T_ERROR_DB);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_delete_media - Error allocating resources for j_tm_id_array");
    j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
  }
  return j_return;
}

json_t * playlist_list_webradio_startup(struct config_elements * config) {
  json_t * j_query, * j_result, * j_media, * j_element, * j_return;
  int res;
  size_t index;
  
  j_query = json_pack("{sss[sssssssss]s{s{sssi}}}",
                      "table",
                      TALIESIN_TABLE_PLAYLIST,
                      "columns",
                        "tpl_id",
                        "tpl_name AS name",
                        "tpl_description AS description",
                        "tpl_username AS username",
                        "tpl_webradio_startup",
                        "tpl_webradio_startup_format",
                        "tpl_webradio_startup_channels",
                        "tpl_webradio_startup_sample_rate",
                        "tpl_webradio_startup_bit_rate",
                      "where",
                        "tpl_webradio_startup",
                          "operator",
                          "!=",
                          "value",
                          TALIESIN_STORED_PLAYLIST_WEBRADIO_STARTUP_OFF);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    json_array_foreach(j_result, index, j_element) {
      j_media = playlist_get_media_list(config, json_integer_value(json_object_get(j_element, "tpl_id")), 1, 0, 0);
      if (check_result_value(j_media, T_OK)) {
        json_object_set(j_element, "media", json_object_get(j_media, "media"));
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "playlist_list - Error check_result_value");
      }
      json_decref(j_media);
    }
    j_return = json_pack("{siso}", "result", T_OK, "playlist", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_list_webradio_startup - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}
