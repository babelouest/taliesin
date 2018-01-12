/**
 *
 * Taliesin - Media server
 * 
 * Search functions definitions
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

#include "taliesin.h"

json_t * playlist_search(struct config_elements * config, const char * username_escape, const char * search_pattern_escape) {
  int res;
  char * clause_where;
  json_t * j_result, * j_query, * j_return;
  
  clause_where = msprintf("(`tpl_name` LIKE '%%%s%%' OR `tpl_description` LIKE '%%%s%%') AND (`tpl_username` = '%s' OR `tpl_username` IS NULL)", search_pattern_escape, search_pattern_escape, username_escape);
  j_query = json_pack("{sss[s]s{s{ssss}}si}",
                      "table",
                      TALIESIN_TABLE_PLAYLIST,
                      "columns",
                        "tpl_name AS name",
                      "where",
                        " ",
                          "operator",
                          "raw",
                          "value",
                          clause_where,
                      "limit",
                      TALIESIN_MEDIA_LIMIT_DEFAULT);
  o_free(clause_where);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    j_return = json_pack("{sisO}", "result", T_OK, "playlist", j_result);
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "playlist_search - Error executing j_query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * stream_search(struct config_elements * config, const char * username, const char * search_pattern) {
  json_t * j_return = json_pack("{sis[]}", "result", T_OK, "stream");
  int i;
  
  if (j_return != NULL) {
    for (i=0; i<config->nb_webradio && i<TALIESIN_MEDIA_LIMIT_DEFAULT; i++) {
      if (o_strcasestr(config->webradio_set[i]->display_name, search_pattern) != NULL && (config->webradio_set[i]->username == NULL || o_strcmp(config->webradio_set[i]->username, username) == 0)) {
        json_array_append_new(json_object_get(j_return, "stream"), json_pack("{ssssso}", "name", config->webradio_set[i]->name, "display_name", config->webradio_set[i]->display_name, "webradio", json_true()));
      }
    }
    for (i=0; i<config->nb_jukebox; i++) {
      if (o_strcasestr(config->jukebox_set[i]->display_name, search_pattern) != NULL && (config->jukebox_set[i]->username == NULL || o_strcmp(config->jukebox_set[i]->username, username) == 0)) {
        json_array_append_new(json_object_get(j_return, "stream"), json_pack("{ssssso}", "name", config->jukebox_set[i]->name, "display_name", config->jukebox_set[i]->display_name, "webradio", json_false()));
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "stream_search - Error allocating resources for j_return");
  }
  return j_return;
}

json_t * folder_search(struct config_elements * config, const char * username_escape, const char * search_pattern_escape) {
  json_t * j_result, * j_return;
  int res;
  char * query;
  
  query = msprintf("SELECT " TALIESIN_TABLE_DATA_SOURCE ".tds_name AS data_source, " TALIESIN_TABLE_FOLDER ".tf_name AS name, " TALIESIN_TABLE_FOLDER 
                   ".tf_path AS path FROM `" TALIESIN_TABLE_FOLDER "`, `" TALIESIN_TABLE_DATA_SOURCE "` WHERE `" TALIESIN_TABLE_DATA_SOURCE 
                   "`.`tds_id` IN (SELECT `tds_id` FROM `%s` WHERE `tds_username`='%s' OR `tds_username` IS NULL) AND tf_name LIKE '%%%s%%' AND `" 
                   TALIESIN_TABLE_DATA_SOURCE "`.`tds_id` = `" TALIESIN_TABLE_FOLDER "`.`tds_id` LIMIT %d", TALIESIN_TABLE_DATA_SOURCE, username_escape, search_pattern_escape, TALIESIN_MEDIA_LIMIT_DEFAULT);
  res = h_execute_query_json(config->conn, query, &j_result);
  o_free(query);
  if (res == H_OK) {
    j_return = json_pack("{sisO}", "result", T_OK, "folder", j_result);
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_search - Error executing query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

json_t * media_search(struct config_elements * config, const char * username_escape, const char * search_pattern_escape, unsigned short int search_category) {
  json_t * j_result, * j_return, * j_element, * j_tags;
  int res;
  char * clause_search = NULL, * query;
  size_t index;
  
  if (search_category == TALIESIN_SEARCH_CATEGORY_NONE) {
    clause_search = msprintf("`tm_name` LIKE '%%%s%%' OR `tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tmd_value` LIKE '%%%s%%')", search_pattern_escape, TALIESIN_TABLE_META_DATA, search_pattern_escape);
  } else if (search_category == TALIESIN_SEARCH_CATEGORY_FILE) {
    clause_search = msprintf("`tm_name` LIKE '%%%s%%'", search_pattern_escape);
  } else if (search_category == TALIESIN_SEARCH_CATEGORY_TITLE) {
    clause_search = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tmd_value` LIKE '%%%s%%' AND `tmd_key`='title')", TALIESIN_TABLE_META_DATA, search_pattern_escape);
  } else if (search_category == TALIESIN_SEARCH_CATEGORY_ARTIST) {
    clause_search = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tmd_value` LIKE '%%%s%%' AND (`tmd_key`='artist' OR `tmd_key`='album_artist'))", TALIESIN_TABLE_META_DATA, search_pattern_escape);
  } else if (search_category == TALIESIN_SEARCH_CATEGORY_ALBUM) {
    clause_search = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tmd_value` LIKE '%%%s%%' AND `tmd_key`='album')", TALIESIN_TABLE_META_DATA, search_pattern_escape);
  } else if (search_category == TALIESIN_SEARCH_CATEGORY_YEAR) {
    clause_search = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tmd_value` LIKE '%s%%' AND `tmd_key`='date')", TALIESIN_TABLE_META_DATA, search_pattern_escape);
  } else if (search_category == TALIESIN_SEARCH_CATEGORY_GENRE) {
    clause_search = msprintf("`tm_id` IN (SELECT `tm_id` FROM `%s` WHERE `tmd_value` LIKE '%%%s%%' AND `tmd_key`='genre')", TALIESIN_TABLE_META_DATA, search_pattern_escape);
  }
  query = msprintf("SELECT " TALIESIN_TABLE_MEDIA ".tm_id, " TALIESIN_TABLE_DATA_SOURCE ".tds_name AS data_source, " TALIESIN_TABLE_MEDIA ".tm_name AS name, " TALIESIN_TABLE_MEDIA ".tm_path AS path FROM `" TALIESIN_TABLE_MEDIA "`, `" TALIESIN_TABLE_DATA_SOURCE "` WHERE `" TALIESIN_TABLE_DATA_SOURCE "`.`tds_id` in (SELECT `tds_id` FROM `%s` WHERE `tds_username`='%s' OR `tds_username` IS NULL) AND (%s) AND `" TALIESIN_TABLE_DATA_SOURCE "`.`tds_id` = `" TALIESIN_TABLE_MEDIA "`.`tds_id` LIMIT %d", TALIESIN_TABLE_DATA_SOURCE, username_escape, clause_search, TALIESIN_MEDIA_LIMIT_DEFAULT);
  o_free(clause_search);
  res = h_execute_query_json(config->conn, query, &j_result);
  o_free(query);
  if (res == H_OK) {
    json_array_foreach(j_result, index, j_element) {
      j_tags = media_get_tags_from_id(config, json_integer_value(json_object_get(j_element, "tm_id")));
      if (check_result_value(j_tags, T_OK)) {
        json_object_set(j_element, "tags", json_object_get(j_tags, "tags"));
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_advanced_search - Error media_get_tags_from_id");
      }
      json_decref(j_tags);
      json_object_del(j_element, "tm_id");
    }
    j_return = json_pack("{sisO}", "result", T_OK, "media", j_result);
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_search - Error executing query");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  return j_return;
}

/*
 * A simple search looks for the search_pattern in the following categories:
 * - media tags and filename
 * - folder name
 * - stream display name
 * - playlist name
 */
json_t * media_simple_search(struct config_elements * config, const char * username, const char * search_pattern, unsigned short int search_category) {
  json_t * j_result, * j_return = json_pack("{sis{}}", "result", T_OK, "search");
  char * search_pattern_escape, * username_escape;
  
  if (j_return != NULL) {
    search_pattern_escape = h_escape_string(config->conn, search_pattern);
    username_escape = h_escape_string(config->conn, username);
    if (search_pattern_escape != NULL && username_escape != NULL) {
      if (search_category == TALIESIN_SEARCH_CATEGORY_NONE || search_category == TALIESIN_SEARCH_CATEGORY_PLAYLIST) {
        j_result = playlist_search(config, username_escape, search_pattern_escape);
        if (check_result_value(j_result, T_OK)) {
          if (json_array_size(json_object_get(j_result, "playlist")) > 0) {
            json_object_set(json_object_get(j_return, "search"), "playlist", json_object_get(j_result, "playlist"));
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_simple_search - Error playlist_search");
        }
        json_decref(j_result);
      }
      if (search_category == TALIESIN_SEARCH_CATEGORY_NONE || search_category == TALIESIN_SEARCH_CATEGORY_STREAM) {
        j_result = stream_search(config, username, search_pattern);
        if (check_result_value(j_result, T_OK)) {
          if (json_array_size(json_object_get(j_result, "stream")) > 0) {
            json_object_set(json_object_get(j_return, "search"), "stream", json_object_get(j_result, "stream"));
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_simple_search - Error stream_search");
        }
        json_decref(j_result);
      }
      if (search_category == TALIESIN_SEARCH_CATEGORY_NONE || search_category == TALIESIN_SEARCH_CATEGORY_FOLDER) {
        j_result = folder_search(config, username_escape, search_pattern_escape);
        if (check_result_value(j_result, T_OK)) {
          if (json_array_size(json_object_get(j_result, "folder")) > 0) {
            json_object_set(json_object_get(j_return, "search"), "folder", json_object_get(j_result, "folder"));
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_simple_search - Error folder_search");
        }
        json_decref(j_result);
      }
      if (search_category == TALIESIN_SEARCH_CATEGORY_NONE ||
          search_category == TALIESIN_SEARCH_CATEGORY_FILE ||
          search_category == TALIESIN_SEARCH_CATEGORY_TITLE ||
          search_category == TALIESIN_SEARCH_CATEGORY_ARTIST ||
          search_category == TALIESIN_SEARCH_CATEGORY_ALBUM ||
          search_category == TALIESIN_SEARCH_CATEGORY_YEAR ||
          search_category == TALIESIN_SEARCH_CATEGORY_GENRE) {
        j_result = media_search(config, username_escape, search_pattern_escape, search_category);
        if (check_result_value(j_result, T_OK)) {
          if (json_array_size(json_object_get(j_result, "media")) > 0) {
            json_object_set(json_object_get(j_return, "search"), "media", json_object_get(j_result, "media"));
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "media_simple_search - Error media_search");
        }
        json_decref(j_result);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "media_simple_search - Error escaping search_pattern '%s' or username_escape '%s'", search_pattern_escape, username_escape);
      j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
    }
    o_free(search_pattern_escape);
    o_free(username_escape);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_simple_search - Error allocating resources for j_return");
    j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
  }
  return j_return;
}

json_t * is_valid_media_advanced_search(struct config_elements * config, const char * username, json_t * search_criteria) {
  json_t * j_return = json_array(), * j_element, * j_result;
  size_t index;
  
  if (j_return != NULL) {
    if (search_criteria == NULL || !json_is_object(search_criteria)) {
      json_array_append_new(j_return, json_pack("{ss}", "search_criteria", "search_criteria must be a JSON object"));
    } else {
      if (json_object_get(search_criteria, "query") != NULL && (!json_is_string(json_object_get(search_criteria, "query")))) {
        json_array_append_new(j_return, json_pack("{ss}", "query", "query is optional and must be a non null JSON string"));
      }
      if (json_object_get(search_criteria, "type") != NULL && (!json_is_string(json_object_get(search_criteria, "type")) || 
          (
          o_strcmp(json_string_value(json_object_get(search_criteria, "type")), "audio") != 0 && 
          o_strcmp(json_string_value(json_object_get(search_criteria, "type")), "video") != 0 && 
          o_strcmp(json_string_value(json_object_get(search_criteria, "type")), "image") != 0 && 
          o_strcmp(json_string_value(json_object_get(search_criteria, "type")), "subtitle") != 0 && 
          o_strcmp(json_string_value(json_object_get(search_criteria, "type")), "other") != 0 && 
          o_strcmp(json_string_value(json_object_get(search_criteria, "type")), "unknown") != 0 && 
          o_strcmp(json_string_value(json_object_get(search_criteria, "type")), "") != 0
          )
          )) {
        json_array_append_new(j_return, json_pack("{ss}", "type", "type is optional and must be a JSON string with one of the following values: 'audio', 'video', 'image', 'subtitle', 'other', 'unknown'"));
      }
      if (json_object_get(search_criteria, "data_source") != NULL && !json_is_array(json_object_get(search_criteria, "data_source"))) {
        json_array_append_new(j_return, json_pack("{ss}", "data_source", "data_source is optional and must be a JSON array"));
      } else {
        json_array_foreach(json_object_get(search_criteria, "data_source"), index, j_element) {
          if (!json_is_string(j_element)) {
            json_array_append_new(j_return, json_pack("{ss}", "data_source", "data_source element must be a JSON string"));
          } else {
            j_result = data_source_get(config, username, json_string_value(j_element), 1);
            if (check_result_value(j_result, T_ERROR_NOT_FOUND)) {
              json_array_append_new(j_return, json_pack("{ss}", "data_source", "data_source does not exist"));
            }
            json_decref(j_result);
          }
        }
      }
      if (json_object_get(search_criteria, "tags") != NULL && !json_is_array(json_object_get(search_criteria, "tags"))) {
        json_array_append_new(j_return, json_pack("{ss}", "tags", "tags is optional and must be a JSON array"));
      } else {
        json_array_foreach(json_object_get(search_criteria, "tags"), index, j_element) {
          if (!json_is_object(j_element) || json_object_get(j_element, "key") == NULL || !json_is_string(json_object_get(j_element, "key")) || json_string_length(json_object_get(j_element, "key")) > 128) {
            json_array_append_new(j_return, json_pack("{ss}", "tag", "a tag must have at least a key and a value of type string, maximum 128 characters"));
          } else if (0 != o_strcmp(json_string_value(json_object_get(j_element, "operator")), "empty") && (json_object_get(j_element, "value") == NULL || !json_is_string(json_object_get(j_element, "value")) || json_string_length(json_object_get(j_element, "value")) > 512)) {
            json_array_append_new(j_return, json_pack("{ss}", "tag", "a tag must have at least a value of type string, maximum 512 characters"));
          } else {
            if (0 != o_strcmp(json_string_value(json_object_get(j_element, "operator")), "equals") &&
                0 != o_strcmp(json_string_value(json_object_get(j_element, "operator")), "different") &&
                0 != o_strcmp(json_string_value(json_object_get(j_element, "operator")), "contains") &&
                0 != o_strcmp(json_string_value(json_object_get(j_element, "operator")), "lower") &&
                0 != o_strcmp(json_string_value(json_object_get(j_element, "operator")), "higher") &&
                0 != o_strcmp(json_string_value(json_object_get(j_element, "operator")), "between") &&
                0 != o_strcmp(json_string_value(json_object_get(j_element, "operator")), "empty")) {
              json_array_append_new(j_return, json_pack("{ss}", "operator", "operator is optional and must be 'equals', 'different', 'contains', 'lower', 'higher', 'between' or 'empty'"));
            } else if (0 == o_strcmp(json_string_value(json_object_get(j_element, "operator")), "between") && (json_object_get(j_element, "value_max") == NULL || !json_is_string(json_object_get(j_element, "value_max")) || json_string_length(json_object_get(j_element, "value_max")) > 512)) {
              json_array_append_new(j_return, json_pack("{ss}", "between", "when using operator 'between', you must use a 'value_max' string"));
            }
          }
        }
      }
      if (json_object_get(search_criteria, "metrics") != NULL && !json_is_object(json_object_get(search_criteria, "metrics"))) {
        json_array_append_new(j_return, json_pack("{ss}", "metrics", "metrics is optional and must be a JSON object"));
      } else if (json_object_get(search_criteria, "metrics") != NULL) {
        if (json_object_get(json_object_get(search_criteria, "metrics"), "nb_play") != NULL && !json_is_object(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"))) {
          json_array_append_new(j_return, json_pack("{ss}", "nb_play", "nb_play is optional and must be a JSON object"));
        } else if (json_object_get(json_object_get(search_criteria, "metrics"), "nb_play") != NULL && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator") == NULL || (
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "equals") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "different") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "lower") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "higher") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "between")
        ))) {
          json_array_append_new(j_return, json_pack("{ss}", "operator", "nb_play operator is mandatory and must be 'equals', 'different', 'lower', 'higher' or 'between'"));
        } else if (json_object_get(json_object_get(search_criteria, "metrics"), "nb_play") != NULL && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value") == NULL || !json_is_integer(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value")) < 0)) {
          json_array_append_new(j_return, json_pack("{ss}", "operator", "nb_play value is mandatory and must be a positive or null integer"));
        } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "between") && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value_max") == NULL || !json_is_integer(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value_max")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value_max")) < json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value_max")) < 0)) {
          json_array_append_new(j_return, json_pack("{ss}", "between", "when using operator 'between', you must use a 'value_max' positive integer value and inferior or equal to 'value'"));
        }
        if (json_object_get(json_object_get(search_criteria, "metrics"), "played_at") != NULL && !json_is_object(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"))) {
          json_array_append_new(j_return, json_pack("{ss}", "played_at", "played_at is optional and must be a JSON object"));
        } else if (json_object_get(json_object_get(search_criteria, "metrics"), "played_at") != NULL && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator") == NULL || (
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "equals") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "different") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "lower") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "higher") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "between")
        ))) {
          json_array_append_new(j_return, json_pack("{ss}", "operator", "played_at operator is mandatory and must be 'equals', 'different', 'lower', 'higher' or 'between'"));
        } else if (json_object_get(json_object_get(search_criteria, "metrics"), "played_at") != NULL && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value") == NULL || !json_is_integer(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value")) < 0)) {
          json_array_append_new(j_return, json_pack("{ss}", "operator", "played_at value is mandatory and must be a positive or null integer"));
        } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "between") && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value_max") == NULL || !json_is_integer(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value_max")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value_max")) < json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value_max")) < 0)) {
          json_array_append_new(j_return, json_pack("{ss}", "between", "when using operator 'between', you must use a 'value_max' positive integer value and inferior or equal to 'value'"));
        }
        if (json_object_get(json_object_get(search_criteria, "metrics"), "last_seen") != NULL && !json_is_object(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"))) {
          json_array_append_new(j_return, json_pack("{ss}", "last_seen", "last_seen is optional and must be a JSON object"));
        } else if (json_object_get(json_object_get(search_criteria, "metrics"), "last_seen") != NULL && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator") == NULL || (
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "equals") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "different") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "lower") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "higher") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "between")
        ))) {
          json_array_append_new(j_return, json_pack("{ss}", "operator", "last_seen operator is mandatory and must be 'equals', 'different', 'lower', 'higher' or 'between'"));
        } else if (json_object_get(json_object_get(search_criteria, "metrics"), "last_seen") != NULL && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value") == NULL || !json_is_integer(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value")) < 0)) {
          json_array_append_new(j_return, json_pack("{ss}", "operator", "last_seen value is mandatory and must be a positive or null integer"));
        } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "between") && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value_max") == NULL || !json_is_integer(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value_max")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value_max")) < json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value_max")) < 0)) {
          json_array_append_new(j_return, json_pack("{ss}", "between", "when using operator 'between', you must use a 'value_max' positive integer value and inferior or equal to 'value'"));
        }
        if (json_object_get(json_object_get(search_criteria, "metrics"), "last_updated") != NULL && !json_is_object(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"))) {
          json_array_append_new(j_return, json_pack("{ss}", "last_updated", "last_updated is optional and must be a JSON object"));
        } else if (json_object_get(json_object_get(search_criteria, "metrics"), "last_updated") != NULL && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator") == NULL || (
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "equals") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "different") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "lower") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "higher") &&
          0 != o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "between")
        ))) {
          json_array_append_new(j_return, json_pack("{ss}", "operator", "last_updated operator is mandatory and must be 'equals', 'different', 'lower', 'higher' or 'between'"));
        } else if (json_object_get(json_object_get(search_criteria, "metrics"), "last_updated") != NULL && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value") == NULL || !json_is_integer(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value")) < 0)) {
          json_array_append_new(j_return, json_pack("{ss}", "operator", "last_updated value is mandatory and must be a positive or null integer"));
        } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "between") && (json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value_max") == NULL || !json_is_integer(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value_max")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value_max")) < json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value")) || json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value_max")) < 0)) {
          json_array_append_new(j_return, json_pack("{ss}", "between", "when using operator 'between', you must use a 'value_max' positive integer value and inferior or equal to 'value'"));
        }
      }
      if (json_object_get(search_criteria, "sort") != NULL && (!json_is_string(json_object_get(search_criteria, "sort")) ||
      (0 != o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "name") &&
       0 != o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "path") &&
       0 != o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "last_updated") &&
       0 != o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "last_played") &&
       0 != o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "nb_play") &&
       0 != o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "random")))) {
        json_array_append_new(j_return, json_pack("{ss}", "sort", "sort parameter is optional and must be a string with one of the following values: 'name', 'path', 'last_updated', 'last_played', 'nb_play', 'random'"));
      }
      if (json_object_get(search_criteria, "sort_direction") != NULL && (!json_is_string(json_object_get(search_criteria, "sort_direction")) ||
          (0 != o_strcmp(json_string_value(json_object_get(search_criteria, "sort_direction")), "asc") &&
          0 != o_strcmp(json_string_value(json_object_get(search_criteria, "sort_direction")), "desc")))) {
        json_array_append_new(j_return, json_pack("{ss}", "sort_direction", "sort_direction is optional and must be a string of the following values: 'asc' or 'desc'"));
      }
      if (json_object_get(search_criteria, "limit") != NULL && (!json_is_integer(json_object_get(search_criteria, "limit")) || json_integer_value(json_object_get(search_criteria, "limit")) < 0)) {
        json_array_append_new(j_return, json_pack("{ss}", "limit", "limit value is optional and must be a positive or null integer"));
      }
      if (json_object_get(search_criteria, "offset") != NULL && (!json_is_integer(json_object_get(search_criteria, "offset")) || json_integer_value(json_object_get(search_criteria, "offset")) < 0)) {
        json_array_append_new(j_return, json_pack("{ss}", "offset", "offset value is optional and must be a positive or null integer"));
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "is_valid_media_advanced_search - Error allocating resources for j_return");
  }
  return j_return;
}

/*
 * An advanced search look for the following patterns:
 * - tag value
 * - media played history
 * - media last seen date
 */
json_t * media_advanced_search(struct config_elements * config, const char * username, json_t * search_criteria) {
  char * query, * clause_tags = NULL, * clause, * tmp, * escape_key, * escape_value, * escape_value_max, * escape, * data_source_clause;
  json_t * j_element, * j_result = NULL, * j_tags;
  int res;
  size_t index;
  
  escape = h_escape_string(config->conn, username);
  if (json_object_get(json_object_get(search_criteria, "metrics"), "nb_play") != NULL) {
    tmp = "LEFT JOIN (SELECT `"TALIESIN_TABLE_MEDIA_HISTORY"`.`tm_id`, COUNT(`"TALIESIN_TABLE_MEDIA_HISTORY"`.`tmh_id`) AS `nb_h` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` GROUP BY `"TALIESIN_TABLE_MEDIA_HISTORY"`.`tm_id`) AS `th_count` ON `th_count`.`tm_id` = `"TALIESIN_TABLE_MEDIA"`.`tm_id`";
  } else {
    tmp = "";
  }
  query = msprintf("SELECT `"TALIESIN_TABLE_MEDIA"`.`tm_id`, `"TALIESIN_TABLE_MEDIA"`.`tm_type` AS `type`, `"TALIESIN_TABLE_MEDIA"`.`tm_name` AS `name`, `"TALIESIN_TABLE_MEDIA"`.`tm_path` AS `path`, `"TALIESIN_TABLE_DATA_SOURCE"`.`tds_name` AS `data_source`, "
                   "(SELECT COUNT(`tmh_id`) FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE `"TALIESIN_TABLE_MEDIA_HISTORY"`.`tm_id`=`"TALIESIN_TABLE_MEDIA"`.`tm_id`) AS `nb_play`, "
                   "(SELECT %s FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE `"TALIESIN_TABLE_MEDIA_HISTORY"`.`tm_id`=`"TALIESIN_TABLE_MEDIA"`.`tm_id` ORDER BY `tmh_datestamp` DESC LIMIT 1) AS `last_played` "
                   "FROM `"TALIESIN_TABLE_DATA_SOURCE"`, `"TALIESIN_TABLE_MEDIA"` %s"
                   "WHERE `"TALIESIN_TABLE_MEDIA"`.`tds_id`=`"TALIESIN_TABLE_DATA_SOURCE"`.`tds_id` "
                   "AND (`"TALIESIN_TABLE_DATA_SOURCE"`.`tds_username` IS NULL OR `"TALIESIN_TABLE_DATA_SOURCE"`.`tds_username`='%s') AND `"TALIESIN_TABLE_DATA_SOURCE"`.`tds_refresh_status`=%d", 
                   (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), tmp, escape, DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING);
  o_free(escape);
  
  if (json_object_get(search_criteria, "data_source") != NULL && json_array_size(json_object_get(search_criteria, "data_source")) > 0) {
    data_source_clause = NULL;
    json_array_foreach(json_object_get(search_criteria, "data_source"), index, j_element) {
      escape = h_escape_string(config->conn, json_string_value(j_element));
      if (data_source_clause == NULL) {
        tmp = msprintf("AND ( `"TALIESIN_TABLE_DATA_SOURCE"`.`tds_name`='%s'", escape);
      } else {
        tmp = msprintf("%s OR `"TALIESIN_TABLE_DATA_SOURCE"`.`tds_name`='%s'", data_source_clause, escape);
      }
      o_free(escape);
      o_free(data_source_clause);
      data_source_clause = tmp;
    }
    tmp = msprintf("%s %s )", query, data_source_clause);
    o_free(data_source_clause);
    o_free(query);
    query = tmp;
  }
  
  if (json_object_get(search_criteria, "query") != NULL && json_string_length(json_object_get(search_criteria, "query")) > 0) {
    escape = h_escape_string(config->conn, json_string_value(json_object_get(search_criteria, "query")));
    tmp = msprintf("%s AND (`"TALIESIN_TABLE_MEDIA"`.`tm_name` LIKE '%%%s%%' OR `"TALIESIN_TABLE_MEDIA"`.`tm_path` LIKE '%%%s%%' OR `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_META_DATA"` WHERE `tmd_value` LIKE '%%%s%%'))", query, escape, escape, escape);
    o_free(escape);
    o_free(query);
    query = tmp;
  }
  
  if (json_object_get(search_criteria, "type") != NULL && json_string_length(json_object_get(search_criteria, "type")) > 0) {
    escape = h_escape_string(config->conn, json_string_value(json_object_get(search_criteria, "type")));
    tmp = msprintf("%s AND `"TALIESIN_TABLE_MEDIA"`.`tm_type` = '%s'", query, escape);
    o_free(escape);
    o_free(query);
    query = tmp;
  }
  
  if (json_object_get(search_criteria, "tags") != NULL && json_array_size(json_object_get(search_criteria, "tags")) > 0) {
    json_array_foreach(json_object_get(search_criteria, "tags"), index, j_element) {
      if (json_string_length(json_object_get(j_element, "key")) > 0) {
        if (0 == o_strcmp(json_string_value(json_object_get(j_element, "operator")), "equals")) {
          escape_key = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "key")));
          escape_value = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "value")));
          clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_META_DATA"` WHERE `tmd_key` LIKE '%s' AND `tmd_value` LIKE '%%%s%%')", escape_key, escape_value);
          o_free(escape_key);
          o_free(escape_value);
          if (clause_tags == NULL) {
            clause_tags = clause;
          } else {
            tmp = msprintf("%s %s", clause_tags, clause);
            o_free(clause_tags);
            o_free(clause);
            clause_tags = tmp;
          }
        } else if (0 == o_strcmp(json_string_value(json_object_get(j_element, "operator")), "different")) {
          escape_key = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "key")));
          escape_value = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "value")));
          clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_META_DATA"` WHERE `tmd_key` LIKE '%s' AND `tmd_value` NOT LIKE '%%%s%%')", escape_key, escape_value);
          o_free(escape_key);
          o_free(escape_value);
          if (clause_tags == NULL) {
            clause_tags = clause;
          } else {
            tmp = msprintf("%s %s", clause_tags, clause);
            o_free(clause_tags);
            o_free(clause);
            clause_tags = tmp;
          }
        } else if (0 == o_strcmp(json_string_value(json_object_get(j_element, "operator")), "contains")) {
          escape_key = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "key")));
          escape_value = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "value")));
          clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_META_DATA"` WHERE `tmd_key` LIKE '%s' AND `tmd_value` LIKE '%%%s%%')", escape_key, escape_value);
          o_free(escape_key);
          o_free(escape_value);
          if (clause_tags == NULL) {
            clause_tags = clause;
          } else {
            tmp = msprintf("%s %s", clause_tags, clause);
            o_free(clause_tags);
            o_free(clause);
            clause_tags = tmp;
          }
        } else if (0 == o_strcmp(json_string_value(json_object_get(j_element, "operator")), "lower")) {
          escape_key = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "key")));
          escape_value = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "value")));
          clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_META_DATA"` WHERE `tmd_key` LIKE '%s' AND `tmd_value` <= '%s')", escape_key, escape_value);
          o_free(escape_key);
          o_free(escape_value);
          if (clause_tags == NULL) {
            clause_tags = clause;
          } else {
            tmp = msprintf("%s %s", clause_tags, clause);
            o_free(clause_tags);
            o_free(clause);
            clause_tags = tmp;
          }
        } else if (0 == o_strcmp(json_string_value(json_object_get(j_element, "operator")), "higher")) {
          escape_key = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "key")));
          escape_value = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "value")));
          clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_META_DATA"` WHERE `tmd_key` LIKE '%s' AND `tmd_value` >= '%s')", escape_key, escape_value);
          o_free(escape_key);
          o_free(escape_value);
          if (clause_tags == NULL) {
            clause_tags = clause;
          } else {
            tmp = msprintf("%s %s", clause_tags, clause);
            o_free(clause_tags);
            o_free(clause);
            clause_tags = tmp;
          }
        } else if (0 == o_strcmp(json_string_value(json_object_get(j_element, "operator")), "between")) {
          escape_key = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "key")));
          escape_value = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "value")));
          escape_value_max = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "value_max")));
          clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_META_DATA"` WHERE `tmd_key` LIKE '%s' AND `tmd_value` >= '%s' AND `tmd_value` <= '%s')", escape_key, escape_value, escape_value_max);
          o_free(escape_key);
          o_free(escape_value);
          o_free(escape_value_max);
          if (clause_tags == NULL) {
            clause_tags = clause;
          } else {
            tmp = msprintf("%s %s", clause_tags, clause);
            o_free(clause_tags);
            o_free(clause);
            clause_tags = tmp;
          }
        } else if (0 == o_strcmp(json_string_value(json_object_get(j_element, "operator")), "empty")) {
          escape_key = h_escape_string(config->conn, json_string_value(json_object_get(j_element, "key")));
          clause = msprintf("AND (`"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_META_DATA"` WHERE `tmd_key` LIKE '%s' AND (`tmd_value` = '' OR `tmd_value` IS NULL)) OR `"TALIESIN_TABLE_MEDIA"`.`tm_id` NOT IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_META_DATA"` WHERE `tmd_key` LIKE '%s' AND (`tmd_value` != '' AND `tmd_value` IS NOT NULL)))", escape_key, escape_key);
          o_free(escape_key);
          if (clause_tags == NULL) {
            clause_tags = clause;
          } else {
            tmp = msprintf("%s %s", clause_tags, clause);
            o_free(clause_tags);
            o_free(clause);
            clause_tags = tmp;
          }
        }
      }
    }
    tmp = msprintf("%s %s", query, clause_tags);
    o_free(query);
    o_free(clause_tags);
    query = tmp;
  }
  
  if (json_object_size(json_object_get(search_criteria, "metrics")) > 0) {
    if (json_object_get(json_object_get(search_criteria, "metrics"), "nb_play") != NULL) {
      if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "equals")) {
        clause = msprintf("AND IFNULL(`th_count`.`nb_h`, 0) = %" JSON_INTEGER_FORMAT, json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "different")) {
        clause = msprintf("AND IFNULL(`th_count`.`nb_h`, 0) != %" JSON_INTEGER_FORMAT, json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "lower")) {
        clause = msprintf("AND IFNULL(`th_count`.`nb_h`, 0) <= %" JSON_INTEGER_FORMAT, json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "higher")) {
        clause = msprintf("AND IFNULL(`th_count`.`nb_h`, 0) >= %" JSON_INTEGER_FORMAT, json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "operator")), "between")) {
        clause = msprintf("AND IFNULL(`th_count`.`nb_h`, 0) >= %" JSON_INTEGER_FORMAT " AND IFNULL(`th_count`.`nb_h`, 0) <= %" JSON_INTEGER_FORMAT, json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value")), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "nb_play"), "value_max")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      }
    }
    
    if (json_object_get(json_object_get(search_criteria, "metrics"), "played_at") != NULL) {
      if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "equals")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE %s=%" JSON_INTEGER_FORMAT ")", (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "different")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE %s!=%" JSON_INTEGER_FORMAT ")", (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "lower")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE %s<=%" JSON_INTEGER_FORMAT ")", (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "higher")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE %s>=%" JSON_INTEGER_FORMAT ")", (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "operator")), "between")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE %s>=%" JSON_INTEGER_FORMAT " AND %s<=%" JSON_INTEGER_FORMAT ")", (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value")), (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "played_at"), "value_max")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      }
    }
    
    if (json_object_get(json_object_get(search_criteria, "metrics"), "last_seen") != NULL) {
      if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "equals")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE %s=%" JSON_INTEGER_FORMAT " ORDER BY `tmh_datestamp` DESC)", (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "different")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE %s!=%" JSON_INTEGER_FORMAT " ORDER BY `tmh_datestamp` DESC)", (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "lower")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE %s<=%" JSON_INTEGER_FORMAT " ORDER BY `tmh_datestamp` DESC)", (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "higher")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE %s>=%" JSON_INTEGER_FORMAT " ORDER BY `tmh_datestamp` DESC)", (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "operator")), "between")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.`tm_id` IN (SELECT `tm_id` FROM `"TALIESIN_TABLE_MEDIA_HISTORY"` WHERE %s>=%" JSON_INTEGER_FORMAT " AND %s<=%" JSON_INTEGER_FORMAT " ORDER BY %s DESC)", (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value")), (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tmh_datestamp`)":"`tmh_datestamp`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_seen"), "value_max")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      }
    }
    
    if (json_object_get(json_object_get(search_criteria, "metrics"), "last_updated") != NULL) {
      if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "equals")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.%s `=%" JSON_INTEGER_FORMAT, (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tm_last_updated`)":"`tm_last_updated`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "different")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.%s `!=%" JSON_INTEGER_FORMAT, (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tm_last_updated`)":"`tm_last_updated`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "lower")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.%s `<=%" JSON_INTEGER_FORMAT, (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tm_last_updated`)":"`tm_last_updated`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "higher")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.%s `>=%" JSON_INTEGER_FORMAT, (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tm_last_updated`)":"`tm_last_updated`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      } else if (0 == o_strcmp(json_string_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "operator")), "between")) {
        clause = msprintf("AND `"TALIESIN_TABLE_MEDIA"`.%s `>=%" JSON_INTEGER_FORMAT " AND `"TALIESIN_TABLE_MEDIA"`.%s <= %" JSON_INTEGER_FORMAT, (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tm_last_updated`)":"`tm_last_updated`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value_max")), (config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(`tm_last_updated`)":"`tm_last_updated`"), json_integer_value(json_object_get(json_object_get(json_object_get(search_criteria, "metrics"), "last_updated"), "value_max")));
        tmp = msprintf("%s %s", query, clause);
        o_free(query);
        o_free(clause);
        query = tmp;
      }
    }
  }
  
  if (0 == o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "name")) {
    tmp = msprintf("%s ORDER BY `"TALIESIN_TABLE_MEDIA"`.`tm_name` %s", query, (0==o_strcmp(json_string_value(json_object_get(search_criteria, "sort_direction")), "desc")?"DESC":"ASC"));
    o_free(query);
    query = tmp;
  } else if (0 == o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "path") || json_object_get(search_criteria, "sort") == NULL) {
    tmp = msprintf("%s ORDER BY `"TALIESIN_TABLE_MEDIA"`.`tm_path` %s", query, (0==o_strcmp(json_string_value(json_object_get(search_criteria, "sort_direction")), "desc")?"DESC":"ASC"));
    o_free(query);
    query = tmp;
  } else if (0 == o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "last_updated")) {
    tmp = msprintf("%s ORDER BY `"TALIESIN_TABLE_MEDIA"`.`tm_last_updated` %s", query, (0==o_strcmp(json_string_value(json_object_get(search_criteria, "sort_direction")), "desc")?"DESC":"ASC"));
    o_free(query);
    query = tmp;
  } else if (0 == o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "last_played")) {
    tmp = msprintf("%s ORDER BY `last_played` %s", query, (0==o_strcmp(json_string_value(json_object_get(search_criteria, "sort_direction")), "desc")?"DESC":"ASC"));
    o_free(query);
    query = tmp;
  } else if (0 == o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "nb_play")) {
    tmp = msprintf("%s ORDER BY `nb_play` %s", query, (0==o_strcmp(json_string_value(json_object_get(search_criteria, "sort_direction")), "desc")?"DESC":"ASC"));
    o_free(query);
    query = tmp;
  } else if (0 == o_strcmp(json_string_value(json_object_get(search_criteria, "sort")), "random")) {
    if (config->conn->type==HOEL_DB_TYPE_MARIADB) {
      tmp = msprintf("%s ORDER BY RAND()", query);
    } else {
      tmp = msprintf("%s ORDER BY RANDOM()", query);
    }
    o_free(query);
    query = tmp;
  }
  
  if (json_object_get(search_criteria, "limit") != NULL) {
    if (json_integer_value(json_object_get(search_criteria, "limit")) > 0) {
      tmp = msprintf("%s LIMIT %"JSON_INTEGER_FORMAT, query, json_integer_value(json_object_get(search_criteria, "limit")));
      o_free(query);
      query = tmp;
    }
  } else {
    tmp = msprintf("%s LIMIT %d", query, TALIESIN_MEDIA_LIMIT_DEFAULT);
    o_free(query);
    query = tmp;
  }
  
  if (json_object_get(search_criteria, "offset") != NULL) {
    tmp = msprintf("%s OFFSET %"JSON_INTEGER_FORMAT, query, json_integer_value(json_object_get(search_criteria, "offset")));
    o_free(query);
    query = tmp;
  }

	y_log_message(Y_LOG_LEVEL_DEBUG, "search query is %s", query);
  res = h_execute_query_json(config->conn, query, &j_result);
  o_free(query);
  if (res == H_OK) {
    json_array_foreach(j_result, index, j_element) {
      j_tags = media_get_tags_from_id(config, json_integer_value(json_object_get(j_element, "tm_id")));
      if (check_result_value(j_tags, T_OK)) {
        json_object_set(j_element, "tags", json_object_get(j_tags, "tags"));
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "media_advanced_search - Error media_get_tags_from_id");
      }
      json_decref(j_tags);
      json_object_del(j_element, "tm_id");
    }
    return json_pack("{siso}", "result", T_OK, "list", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_advanced_search - Error executing query");
    return json_pack("{si}", "result", T_ERROR_DB);
  }
}
