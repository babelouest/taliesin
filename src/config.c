/**
 *
 * Taliesin - Media server
 * 
 * Taliesin configuratrion functions
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

/**
 * Return the filename extension
 */
static const char * get_filename_ext(const char *path) {
    const char *dot = strrchr(path, '.');
    if(!dot || dot == path) return "*";
    if (strchr(dot, '?') != NULL) {
      *strchr(dot, '?') = '\0';
    }
    return dot;
}

/**
 * Return a char array from a json string array
 */
char ** get_array_from_json_list(json_t * j_config_values) {
  char ** char_array = malloc((json_array_size(j_config_values)+1) * sizeof(char *));
  size_t index;
  json_t * j_element;
  
  if (char_array != NULL) {
    json_array_foreach(j_config_values, index, j_element) {
      char_array[index] = o_strdup(json_string_value(j_element));
    }
    char_array[json_array_size(j_config_values)] = NULL;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "get_array_from_json_list - Error allocating resources for char_array");
  }
  return char_array;
}

/**
 * Get config values from the database based on the type of value requested
 */
json_t * config_get_db_values(struct config_elements * config, const char * config_type) {
  json_t * j_query, * j_result, * j_return;
  int res;
  
  j_query = json_pack("{sss[ss]s{ss}ss}",
                      "table",
                      TALIESIN_TABLE_CONFIG,
                      "columns",
                        "tc_value AS conf_value",
                        "tc_order AS conf_order",
                      "where",
                        "tc_type",
                        config_type,
                      "order_by",
                      "tc_order, tc_id");
  if (j_query != NULL) {
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      j_return = json_pack("{siso}", "result", T_OK, "config", j_result);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "config_get_db_values - Error executing j_query");
      j_return = json_pack("{si}", "result", T_ERROR_DB);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "config_get_db_values - Error allocating resources for j_query");
    j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
  }
  return j_return;
}

/**
 * Get config values and return as a JSON array of strings
 */
json_t * config_get_values(struct config_elements * config, const char * config_type) {
  json_t * j_result = config_get_db_values(config, config_type), * j_return, * j_element;
  size_t index;
  
  if (check_result_value(j_result, T_OK)) {
    j_return = json_pack("{sis[]}", "result", T_OK, "config");
    if (j_return != NULL) {
      json_array_foreach(json_object_get(j_result, "config"), index, j_element) {
        json_array_append(json_object_get(j_return, "config"), json_object_get(j_element, "conf_value"));
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error allocating resources for j_return");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "config_get_values - Error config_get_db_values");
    j_return = json_pack("{si}", "result", T_ERROR);
  }
  json_decref(j_result);
  return j_return;
}

/**
 * Check that config values is correct
 */
json_t * is_valid_config_elements(json_t * j_config_values) {
  json_t * j_return = json_array(), * j_element;
  size_t index;
  
  if (j_return != NULL) {
    if (j_config_values == NULL || !json_is_array(j_config_values)) {
      json_array_append_new(j_return, json_pack("{ss}", "body", "body must be a JSON array of string values"));
    } else {
      json_array_foreach(j_config_values, index, j_element) {
        if (!json_is_string(j_element) || json_string_length(j_element) > 512) {
          json_array_append_new(j_return, json_pack("{ss}", "value", "config value must be a string of maximum 512 characters"));
        }
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "is_valid_config_elements - Error allocating resources for j_return");
  }
  return j_return;
}

/**
 * Set config values in the database for the type requested
 */
int config_set_values(struct config_elements * config, const char * config_type, json_t * j_config_values) {
  json_t * j_query, * j_element, * j_config_value;
  int res, ret;
  size_t index;
  char ** new_config_array = NULL;
  
  j_query = json_pack("{sss{ss}}",
                      "table",
                      TALIESIN_TABLE_CONFIG,
                      "where",
                        "tc_type",
                        config_type);
  res = h_delete(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    j_query = json_pack("{sss[]}",
                        "table",
                        TALIESIN_TABLE_CONFIG,
                        "values");
    if (j_query != NULL) {
      json_array_foreach(j_config_values, index, j_element) {
        j_config_value = json_pack("{sssO}", "tc_type", config_type, "tc_value", j_element);
        json_array_append_new(json_object_get(j_query, "values"), j_config_value);
      }
      res = h_insert(config->conn, j_query, NULL);
      json_decref(j_query);
      if (res == H_OK) {
        new_config_array = get_array_from_json_list(j_config_values);
        if (new_config_array != NULL) {
          if (0 == o_strcmp(config_type, TALIESIN_CONFIG_EXTERNAL_PLAYER)) {
            free_string_array(config->external_player);
            config->external_player = new_config_array;
            ret = T_OK;
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "config_get_db_values - Error config_type (this shouldn't happen)");
            free_string_array(new_config_array);
            new_config_array = NULL;
            ret = T_ERROR_PARAM;
          }
        } else {
          ret = T_ERROR_MEMORY;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "config_get_db_values - Error executing j_query");
        ret = T_ERROR_DB;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "config_get_db_values - Error allocating resources for j_query");
      ret = T_ERROR_MEMORY;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "config_set_values - Error executing j_query (1)");
    ret = T_ERROR_DB;
  }
  return ret;
}

/**
 * Load config values at startup
 */
int load_config_values(struct config_elements * config) {
  json_t * j_result, * j_element;
  size_t index;
  int ret = T_OK;
  
  j_result = config_get_db_values(config, TALIESIN_CONFIG_EXTERNAL_PLAYER);
  if (check_result_value(j_result, T_OK)) {
    config->external_player = o_malloc((json_array_size(json_object_get(j_result, "config")) + 1) * sizeof (char*));
    if (config->external_player != NULL) {
      json_array_foreach(json_object_get(j_result, "config"), index, j_element) {
        config->external_player[index] = o_strdup(json_string_value(json_object_get(j_element, "conf_value")));
      }
      config->external_player[json_array_size(json_object_get(j_result, "config"))] = NULL;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "load_config_values - Error allocating resources for config->external_player");
      ret = T_ERROR_MEMORY;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "load_config_values - Error config_get_db_values for audio file extension");
    ret = T_ERROR_DB;
  }
  json_decref(j_result);
  
  return ret;
}

/**
 * Get the type of a file based on its path
 */
int config_get_type_from_path(struct config_elements * config, const char * path) {
  char * save_path = o_strdup(path);
  const char * ext = get_filename_ext(save_path);
  int ret = TALIESIN_FILE_TYPE_UNKNOWN;
  
  // Check for audio extension
  if (u_map_has_key_case(config->audio_file_extension, ext)) {
    ret = TALIESIN_FILE_TYPE_AUDIO;
  }
  
  if (ret == TALIESIN_FILE_TYPE_UNKNOWN) {
    // Check for video extension
    if (u_map_has_key_case(config->video_file_extension, ext)) {
      ret = TALIESIN_FILE_TYPE_VIDEO;
    }
  }
  
  if (ret == TALIESIN_FILE_TYPE_UNKNOWN) {
    // Check for subtitle extension
    if (u_map_has_key_case(config->subtitle_file_extension, ext)) {
      ret = TALIESIN_FILE_TYPE_SUBTITLE;
    }
  }
  
  if (ret == TALIESIN_FILE_TYPE_UNKNOWN) {
    // Check for image extension
    if (u_map_has_key_case(config->image_file_extension, ext)) {
      ret = TALIESIN_FILE_TYPE_IMAGE;
    }
  }
  
  o_free(save_path);
  return ret;
}

/**
 * Get the type of a file based on its path
 */
const char * config_get_content_type_from_path(struct config_elements * config, const char * path) {
  char * save_path = o_strdup(path);
  const char * ext = get_filename_ext(save_path), * content_type = NULL;
  int type = config_get_type_from_path(config, path);
  
  switch (type) {
    case TALIESIN_FILE_TYPE_AUDIO:
      content_type = u_map_get_case(config->audio_file_extension, ext);
      break;
    case TALIESIN_FILE_TYPE_VIDEO:
      content_type = u_map_get_case(config->video_file_extension, ext);
      break;
    case TALIESIN_FILE_TYPE_SUBTITLE:
      content_type = u_map_get_case(config->subtitle_file_extension, ext);
      break;
    case TALIESIN_FILE_TYPE_IMAGE:
      content_type = u_map_get_case(config->image_file_extension, ext);
      break;
    default:
      content_type = NULL;
      break;
  }
  
  o_free(save_path);
  return content_type;
}

/**
 * Return the different usernames that have been connected to the application
 */
json_t * username_get_list(struct config_elements * config) {
  json_t * j_result;
  int res;
  char * query;
  
  query = msprintf("SELECT DISTINCT(`username`) AS `username` FROM (\
SELECT DISTINCT(`tds_username`) AS `username` FROM " TALIESIN_TABLE_DATA_SOURCE " WHERE `tds_username` IS NOT NULL"
" UNION ALL \
SELECT DISTINCT(`tpl_username`) AS `username` FROM " TALIESIN_TABLE_PLAYLIST " WHERE `tpl_username` IS NOT NULL"
" UNION ALL \
SELECT DISTINCT(`ts_username`) AS `username` FROM " TALIESIN_TABLE_STREAM " WHERE `ts_username` IS NOT NULL"
") AS `t_username` ORDER BY `username`");
  res = h_execute_query_json(config->conn, query, &j_result);
  o_free(query);
  if (res == H_OK) {
    return json_pack("{siso}", "result", T_OK, "username", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "username_get_list - Error executing query");
    return json_pack("{si}", "result", T_ERROR_DB);
  }
}
