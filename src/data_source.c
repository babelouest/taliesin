/**
 *
 * Taliesin - Media server
 * 
 * Data source functions implementations
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

#include <zlib.h>
#include "taliesin.h"

/**
 * Data Source CRUD
 */
json_t * data_source_list(struct config_elements * config, const char * username) {
  json_t * j_query, * j_result = NULL, * j_return, * j_element;
  int res;
  char * profile_query, * escaped, * data_source_scope;
  size_t index;
  
  escaped = h_escape_string(config->conn, username);
  profile_query = msprintf("(`tds_username` IS NULL OR `tds_username`= '%s')", escaped);
  h_free(escaped);
  j_query = json_pack("{sss[sssss]s{s{ssss}}}",
                      "table",
                      TALIESIN_TABLE_DATA_SOURCE,
                      "columns",
                        "tds_username",
                        "tds_name AS name",
                        "tds_description AS description",
                        "tds_icon AS icon",
                        config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(tds_last_updated) AS last_updated":"tds_last_updated AS last_updated",
                      "where",
                        " ",
                          "operator",
                          "raw",
                          "value",
                          profile_query);
  o_free(profile_query);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    json_array_foreach(j_result, index, j_element) {
      if (json_object_get(j_element, "tds_username") != json_null()) {
        data_source_scope = TALIESIN_SCOPE_ME;
      } else {
        data_source_scope = TALIESIN_SCOPE_ALL;
      }
      json_object_set_new(j_element, "scope", json_string(data_source_scope));
      json_object_del(j_element, "tds_username");
    }
    j_return = json_pack("{sisO}", "result", T_OK, "data_source", j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_list - Error getting data source list");
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  json_decref(j_result);
  return j_return;
}

json_t * data_source_get(struct config_elements * config, const char * username, const char * name, int get_id) {
  json_t * j_query, * j_result = NULL, * j_return;
  int res;
  char * profile_query, * escaped, * data_source_scope;
  
  if (username != NULL) {
    escaped = h_escape_string(config->conn, username);
    profile_query = msprintf("(`tds_username` IS NULL OR `tds_username`= '%s')", escaped);
    h_free(escaped);
    j_query = json_pack("{sss[sssss]s{s{ssss}ss}}",
                        "table",
                        TALIESIN_TABLE_DATA_SOURCE,
                        "columns",
                          "tds_username",
                          "tds_name AS name",
                          "tds_description AS description",
                          "tds_icon AS icon",
                          config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(tds_last_updated) AS last_updated":"tds_last_updated AS last_updated",
                        "where",
                          " ",
                            "operator",
                            "raw",
                            "value",
                            profile_query,
                          "tds_name",
                          name);
    o_free(profile_query);
  } else {
    j_query = json_pack("{sss[sssss]s{ss}}",
                        "table",
                        TALIESIN_TABLE_DATA_SOURCE,
                        "columns",
                          "tds_username",
                          "tds_name AS name",
                          "tds_description AS description",
                          "tds_icon AS icon",
                          config->conn->type==HOEL_DB_TYPE_MARIADB?"UNIX_TIMESTAMP(tds_last_updated) AS last_updated":"tds_last_updated AS last_updated",
                        "where",
                          "tds_name",
                          name);
  }
  if (j_query != NULL) {
    if (get_id) {
      json_array_append_new(json_object_get(j_query, "columns"), json_string("tds_username AS username"));
      json_array_append_new(json_object_get(j_query, "columns"), json_string("tds_id"));
      json_array_append_new(json_object_get(j_query, "columns"), json_string("tds_refresh_status AS refresh_status"));
      json_array_append_new(json_object_get(j_query, "columns"), json_string("tds_path AS path"));
    }
    res = h_select(config->conn, j_query, &j_result, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      if (json_array_size(j_result) > 0) {
        if (json_object_get(json_array_get(j_result, 0), "tds_username") != json_null()) {
          data_source_scope = TALIESIN_SCOPE_ME;
        } else {
          data_source_scope = TALIESIN_SCOPE_ALL;
        }
        json_object_set_new(json_array_get(j_result, 0), "scope", json_string(data_source_scope));
        json_object_del(json_array_get(j_result, 0), "tds_username");
        j_return = json_pack("{sisO}", "result", T_OK, "data_source", json_array_get(j_result, 0));
      } else {
        j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "data_source_get - Error getting data source list");
      j_return = json_pack("{si}", "result", T_ERROR_DB);
    }
    json_decref(j_result);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_get - Error allocating resources for j_query");
    j_return = json_pack("{si}", "result", T_ERROR_MEMORY);
  }
  return j_return;
}

int data_source_can_update(json_t * j_data_source, int is_admin) {
  return (is_admin || o_strcmp(TALIESIN_SCOPE_ME, json_string_value(json_object_get(j_data_source, "scope"))) == 0);
}

json_t * is_data_source_valid(struct config_elements * config, const char * username, int is_admin, json_t * j_data_source, int update) {
  json_t * j_result = json_array(), * j_element, * j_element_all;
  
  if (j_result != NULL) {
    if (j_data_source != NULL && json_is_object(j_data_source)) {
      if (!update) {
        if (json_object_get(j_data_source, "name") != NULL &&
            json_is_string(json_object_get(j_data_source, "name")) &&
            json_string_length(json_object_get(j_data_source, "name")) > 0 && 
            json_string_length(json_object_get(j_data_source, "name")) < 128) {
          if (is_admin && o_strcmp(json_string_value(json_object_get(j_data_source, "scope")), TALIESIN_SCOPE_ALL) == 0) {
            j_element = data_source_get(config, NULL, json_string_value(json_object_get(j_data_source, "name")), 0);
          } else {
            j_element = data_source_get(config, username, json_string_value(json_object_get(j_data_source, "name")), 0);
          }
          if (check_result_value(j_element, T_OK)) {
            json_array_append_new(j_result, json_pack("{ss}", "name", "A data_source with the same name already exist"));
          } else if (!check_result_value(j_element, T_ERROR_NOT_FOUND)) {
            y_log_message(Y_LOG_LEVEL_ERROR, "is_data_source_valid - Error looking for a data_source");
            json_array_append_new(j_result, json_pack("{ss}", "name", "Internal error"));
          } else {
            j_element_all = data_source_get(config, NULL, json_string_value(json_object_get(j_data_source, "name")), 0);
            if (check_result_value(j_element_all, T_OK)) {
              json_array_append_new(j_result, json_pack("{ss}", "name", "A data_source with the same name already exist"));
            } else if (!check_result_value(j_element_all, T_ERROR_NOT_FOUND)) {
              y_log_message(Y_LOG_LEVEL_ERROR, "is_data_source_valid - Error looking for a data_source");
              json_array_append_new(j_result, json_pack("{ss}", "name", "Internal error"));
            }
            json_decref(j_element_all);
          }
          json_decref(j_element);
        }
        
        if (json_object_get(j_data_source, "path") == NULL ||
            !json_is_string(json_object_get(j_data_source, "path")) ||
            json_string_length(json_object_get(j_data_source, "path")) > 1024
          ) {
          json_array_append_new(j_result, json_pack("{ss}", "path", "Path is an mandatory string up to 1024 characters"));
        } else {
          if (!is_fs_directory_readable(json_string_value(json_object_get(j_data_source, "path")))) {
            json_array_append_new(j_result, json_pack("{ss}", "path", "Path is not accessible from the server"));
          }
        }

        if (json_object_get(j_data_source, "scope") != NULL &&
            (
              !json_is_string(json_object_get(j_data_source, "scope")) ||
              (
                o_strcmp(json_string_value(json_object_get(j_data_source, "scope")), TALIESIN_SCOPE_ALL) != 0 &&
                o_strcmp(json_string_value(json_object_get(j_data_source, "scope")), TALIESIN_SCOPE_ME) != 0
              )
              || (o_strcmp(json_string_value(json_object_get(j_data_source, "scope")), TALIESIN_SCOPE_ALL) == 0 && !is_admin)
            )
          ) {
          json_array_append_new(j_result, json_pack("{ss}", "scope", "scope value is an optional string and can be only " TALIESIN_SCOPE_ALL " or " TALIESIN_SCOPE_ME ", only administrator can add data source for all users"));
        } else if (!config->user_can_create_data_source && o_strcmp(json_string_value(json_object_get(j_data_source, "scope")), TALIESIN_SCOPE_ME) == 0) {
          json_array_append_new(j_result, json_pack("{ss}", "scope", "only administrator can add data source"));
        }
      }
      if (json_object_get(j_data_source, "description") != NULL &&
          (
            !json_is_string(json_object_get(j_data_source, "description")) ||
            json_string_length(json_object_get(j_data_source, "description")) > 512
          )
        ) {
        json_array_append_new(j_result, json_pack("{ss}", "description", "Description is an optional string up to 512 characters"));
      }
      
      if (json_object_get(j_data_source, "icon") != NULL &&
          (
            !json_is_string(json_object_get(j_data_source, "icon")) ||
            json_string_length(json_object_get(j_data_source, "icon")) > 128
          )
        ) {
        json_array_append_new(j_result, json_pack("{ss}", "icon", "icon is an optional string up to 128 characters"));
      }
    } else {
      json_array_append_new(j_result, json_pack("{ss}", "data_source", "data_source must be a valid JSON object"));
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "is_data_source_valid - Error allocating resources for j_result");
  }
  return j_result;
}

int data_source_add(struct config_elements * config, const char * username, json_t * j_data_source) {
  json_t * j_query, * j_profile_clause;
  int res, to_return;
  
  if (!o_strcmp(json_string_value(json_object_get(j_data_source, "scope")), TALIESIN_SCOPE_ALL)) {
    j_profile_clause = json_null();
  } else {
    j_profile_clause = json_string(username);
  }
  j_query = json_pack("{sss{sossssssss}}",
                      "table",
                      TALIESIN_TABLE_DATA_SOURCE,
                      "values",
                        "tds_username",
                        j_profile_clause,
                        "tds_name",
                        json_string_value(json_object_get(j_data_source, "name")),
                        "tds_description",
                        json_object_get(j_data_source, "description")!=NULL?json_string_value(json_object_get(j_data_source, "description")):"",
                        "tds_path",
                        json_string_value(json_object_get(j_data_source, "path")),
                        "tds_icon",
                        json_object_get(j_data_source, "icon")!=NULL?json_string_value(json_object_get(j_data_source, "icon")):"");
  if (j_query != NULL) {
    res = h_insert(config->conn, j_query, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      to_return = T_OK;
    } else {
      to_return = T_ERROR_DB;
    }
  } else {
    to_return = T_ERROR_MEMORY;
  }
  return to_return;
}

int data_source_set(struct config_elements * config, const char * username, const char * data_source, json_t * j_data_source) {
  json_t * j_query, * j_profile_clause;
  int res, to_return;
  
  if (!o_strcmp(json_string_value(json_object_get(j_data_source, "scope")), TALIESIN_SCOPE_ALL)) {
    j_profile_clause = json_null();
  } else {
    j_profile_clause = json_string(username);
  }
  j_query = json_pack("{sss{ssss}s{ssso}}",
                      "table",
                      TALIESIN_TABLE_DATA_SOURCE,
                      "set",
                        "tds_description",
                        json_object_get(j_data_source, "description")!=NULL?json_string_value(json_object_get(j_data_source, "description")):"",
                        "tds_icon",
                        json_object_get(j_data_source, "icon")!=NULL?json_string_value(json_object_get(j_data_source, "icon")):"",
                      "where",
                        "tds_name",
                        data_source,
                        "tds_username",
                        j_profile_clause);
  if (j_query != NULL) {
    res = h_update(config->conn, j_query, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      to_return = T_OK;
    } else {
      to_return = T_ERROR_DB;
    }
  } else {
    to_return = T_ERROR_MEMORY;
  }
  return to_return;
}

int data_source_delete(struct config_elements * config, const char * username, const char * data_source) {
  json_t * j_query;
  int res, to_return;
  char * escaped, * profile_clause;
  
  escaped = h_escape_string(config->conn, username);
  profile_clause = msprintf("(`tds_username` IS NULL OR `tds_username`= '%s')", escaped);
  h_free(escaped);
  j_query = json_pack("{sss{sss{ssss}}}",
                      "table",
                      TALIESIN_TABLE_DATA_SOURCE,
                      "where",
                        "tds_name",
                        data_source,
                        " ",
                          "operator",
                          "raw",
                          "value",
                          profile_clause);
  o_free(profile_clause);
  if (j_query != NULL) {
    res = h_delete(config->conn, j_query, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      to_return = T_OK;
    } else {
      to_return = T_ERROR_DB;
    }
  } else {
    to_return = T_ERROR_MEMORY;
  }
  return to_return;
}

/**
 * Data Source scan and refresh data source functions
 */
int set_folder_cover(struct config_elements * config, json_int_t tds_id, json_int_t tf_id, json_int_t tic_id) {
  json_t * j_query;
  int res;
  
  j_query = json_pack("{sss{so}s{sIsI}}",
                      "table",
                      TALIESIN_TABLE_FOLDER,
                      "set",
                        "tic_id",
                        tic_id?json_integer(tic_id):json_null(),
                      "where",
                        "tf_id",
                        tf_id,
                        "tds_id",
                        tds_id);
  if (j_query != NULL) {
    res = h_update(config->conn, j_query, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      return T_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "set_folder_cover - Error executing j_query");
      return T_ERROR_DB;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "set_folder_cover - Error allocating resources for j_query");
    return T_ERROR_MEMORY;
  }
}

json_int_t insert_new_folder(struct config_elements * config, json_int_t tds_id, json_int_t tf_parent_id, const char * name, const char * path) {
  json_t * j_query, * j_last_id;
  int res;
  json_int_t ret = 0;
  
  j_query = json_pack("{sss{sssssI}}",
                      "table",
                      TALIESIN_TABLE_FOLDER,
                      "values",
                        "tf_name",
                        name,
                        "tf_path",
                        path,
                        "tds_id",
                        tds_id);
  if (j_query != NULL) {
    if (tf_parent_id != 0) {
      json_object_set_new(json_object_get(j_query, "values"), "tf_parent_id", json_integer(tf_parent_id));
    }
    res = h_insert(config->conn, j_query, NULL);
    json_decref(j_query);
    if (res == H_OK) {
      j_last_id = h_last_insert_id(config->conn);
      if (j_last_id != NULL) {
        ret = json_integer_value(j_last_id);
        json_decref(j_last_id);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "insert_new_folder - Error getting last id");
        ret = T_ERROR_DB;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "insert_new_folder - Error inserting in database");
      ret = T_ERROR_DB;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "insert_new_folder - Error allocating resources for j_query");
    ret = T_ERROR_MEMORY;
  }
  return ret;
}

int folder_set_refresh_status(struct config_elements * config, json_int_t tf_id, int refresh_mode) {
  json_t * j_query;
  int res;
  
  j_query = json_pack("{sss{si}s{sI}}",
                      "table",
                      TALIESIN_TABLE_FOLDER,
                      "set",
                        "tf_refresh_status",
                        refresh_mode,
                      "where",
                        "tf_id",
                        tf_id);
  res = h_update(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    return T_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "folder_set_refresh_status - Error executing j_query");
    return T_ERROR_DB;
  }
}

int media_set_refresh_status(struct config_elements * config, json_int_t tm_id, int refresh_mode) {
  json_t * j_query;
  int res;
  
  j_query = json_pack("{sss{si}s{sI}}",
                      "table",
                      TALIESIN_TABLE_MEDIA,
                      "set",
                        "tm_refresh_status",
                        refresh_mode,
                      "where",
                        "tm_id",
                        tm_id);
  res = h_update(config->conn, j_query, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    return T_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "media_set_refresh_status - Error executing j_query");
    return T_ERROR_DB;
  }
}

int data_source_set_refresh_mode(struct config_elements * config, json_int_t tds_id, json_int_t tf_parent_id, int refresh_mode) {
  json_t * j_query;
  
  j_query = json_pack("{sss{si}s{sIso}}",
                      "table",
                      TALIESIN_TABLE_FOLDER,
                      "set",
                        "tf_refresh_status",
                        refresh_mode,
                      "where",
                        "tds_id",
                        tds_id,
                        "tf_parent_id",
                        tf_parent_id?json_integer(tf_parent_id):json_null());
  if (h_update(config->conn, j_query, NULL) != H_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_set_refresh_mode - error cleaning table %s", TALIESIN_TABLE_FOLDER);
  }
  json_decref(j_query);
  
  j_query = json_pack("{sss{si}s{sIso}}",
                      "table",
                      TALIESIN_TABLE_MEDIA,
                      "set",
                        "tm_refresh_status",
                        refresh_mode,
                      "where",
                        "tds_id",
                        tds_id,
                        "tf_id",
                        tf_parent_id?json_integer(tf_parent_id):json_null());
  if (h_update(config->conn, j_query, NULL) != H_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_set_refresh_mode - error cleaning table %s", TALIESIN_TABLE_MEDIA);
  }
  json_decref(j_query);
  return T_OK;
}

int data_source_scan_directory(struct config_elements * config, struct _refresh_config * refresh_config, AVCodecContext * thumbnail_cover_codec_context, json_int_t tds_id, json_t * j_data_source, const char * sub_path, int tf_id) {
  char * path, * new_sub_path, * file_path, * relative_path;
  json_t * j_element, * j_folder, * j_folder_cover, * j_media;
  size_t index;
  int res = T_OK;
  json_int_t new_tf_id;
  
  if (refresh_config != NULL && refresh_config->refresh_status == DATA_SOURCE_REFRESH_STATUS_RUNNING) {
    if (data_source_set_refresh_mode(config, tds_id, tf_id, DATA_SOURCE_REFRESH_MODE_IN_REFRESH) == T_OK) {
      path = msprintf("%s/%s", json_string_value(json_object_get(j_data_source, "path")), sub_path!=NULL?sub_path:"");
      if (o_strlen(path) <= PATH_MAX) {
        j_folder = fs_directory_read(path);
        if (j_folder != NULL) {
          json_array_foreach(j_folder, index, j_element) {
            if (o_strcmp(json_string_value(json_object_get(j_element, "type")), "folder") == 0) {
              // Look for folder in database
              new_tf_id = folder_get_id(config, j_data_source, tf_id, json_string_value(json_object_get(j_element, "name")));
              if (new_tf_id < 0) {
                // Folder does not exist yet, we create it
                if (sub_path != NULL && o_strlen(sub_path)) {
                  new_sub_path = msprintf("%s/%s", sub_path, json_string_value(json_object_get(j_element, "name")));
                } else {
                  new_sub_path = o_strdup(json_string_value(json_object_get(j_element, "name")));
                }
                new_tf_id = insert_new_folder(config, tds_id, tf_id, json_string_value(json_object_get(j_element, "name")), new_sub_path);
                o_free(new_sub_path);
              } else {
                if (folder_set_refresh_status(config, new_tf_id, DATA_SOURCE_REFRESH_MODE_PROCESSED) != T_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Error updating folder status");
                }
              }
              if (new_tf_id > 0) {
                if (sub_path != NULL && o_strlen(sub_path)) {
                  new_sub_path = msprintf("%s/%s", sub_path, json_string_value(json_object_get(j_element, "name")));
                } else {
                  new_sub_path = o_strdup(json_string_value(json_object_get(j_element, "name")));
                }
                if (data_source_scan_directory(config, refresh_config, thumbnail_cover_codec_context, tds_id, j_data_source, new_sub_path, new_tf_id) != T_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Error scanning subdirectory %s", new_sub_path);
                }
                j_folder_cover = media_folder_detect_cover_by_id(config, j_data_source, new_tf_id, new_sub_path);
                if (check_result_value(j_folder_cover, T_OK)) {
                  if (set_folder_cover(config, tds_id, new_tf_id, json_integer_value(json_object_get(json_object_get(j_folder_cover, "cover"), "tic_id"))) != T_OK) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Error updating folder with cover");
                  }
                } else {
                  if (set_folder_cover(config, tds_id, new_tf_id, 0) != T_OK) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Error cleaning folder cover");
                  }
                }
                json_decref(j_folder_cover);
                j_folder_cover = NULL;
                o_free(new_sub_path);
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Error insert new folder %s", json_string_value(json_object_get(j_element, "name")));
              }
            } else if (o_strcmp(json_string_value(json_object_get(j_element, "type")), "file") == 0) {
              if (json_string_length(json_object_get(j_element, "name")) <= NAME_MAX) {
                file_path = msprintf("%s/%s", path, json_string_value(json_object_get(j_element, "name")));
                relative_path = msprintf("%s/%s", sub_path!=NULL?sub_path:"", json_string_value(json_object_get(j_element, "name")));
                j_media = media_get_full(config, j_data_source, relative_path);
                if (check_result_value(j_media, T_ERROR_NOT_FOUND)) {
                  json_object_set_new(j_element, "metadata", media_get_metadata(config, thumbnail_cover_codec_context, file_path));
                  if (media_add(config, tds_id, tf_id, relative_path, j_element) != T_OK) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Error insert new media %s", json_string_value(json_object_get(j_element, "name")));
                  }
                } else if (check_result_value(j_media, T_OK) && !json_is_array(json_object_get(j_media, "media"))) {
                  // Check if last_modified is newer than the one in the database, if so, update current file rather than creating a new one
                  if (json_integer_value(json_object_get(json_object_get(j_media, "media"), "last_updated")) < json_integer_value(json_object_get(j_element, "last_modified"))) {
                    if (0 == o_strcmp("audio", json_string_value(json_object_get(json_object_get(j_media, "media"), "type"))) ||
                        0 == o_strcmp("video", json_string_value(json_object_get(json_object_get(j_media, "media"), "type"))) ||
                        0 == o_strcmp("image", json_string_value(json_object_get(json_object_get(j_media, "media"), "type")))) {
                      json_object_set_new(j_element, "metadata", media_get_metadata(config, thumbnail_cover_codec_context, file_path));
                    }
                    if (media_update(config, json_integer_value(json_object_get(json_object_get(j_media, "media"), "tm_id")), j_element) != T_OK) {
                      y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Error update media %s", json_string_value(json_object_get(j_element, "name")));
                    }
                  } else {
                    if (media_set_refresh_status(config, json_integer_value(json_object_get(json_object_get(j_media, "media"), "tm_id")), DATA_SOURCE_REFRESH_MODE_PROCESSED) != T_OK) {
                      y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Error updating media status");
                    }
                  }
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Error getting media %s from data source %s", relative_path, json_string_value(json_object_get(j_data_source, "name")));
                }
                o_free(file_path);
                o_free(relative_path);
                json_decref(j_media);
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Can't evaluate '%s', name too long", json_string_value(json_object_get(j_element, "name")));
              }
              refresh_config->nb_files_read++;
            }
          }
        } else {
          res = T_ERROR_PARAM;
        }
        json_decref(j_folder);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Can't scan '%s', path too long", path);
      }
      o_free(path);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "data_source_scan_directory - Error data_source_set_refresh_mode");
    }
  }
  return res;
}

int data_source_clean_removed(struct config_elements * config, json_int_t tds_id) {
  json_t * j_query;
  
  if (data_source_set_refresh_status(config, tds_id, DATA_SOURCE_REFRESH_STATUS_RUNNING) == T_OK) {
    j_query = json_pack("{sss{sIsi}}",
                        "table",
                        TALIESIN_TABLE_FOLDER,
                        "where",
                          "tds_id",
                          tds_id,
                          "tf_refresh_status",
                          DATA_SOURCE_REFRESH_MODE_IN_REFRESH);
    if (h_delete(config->conn, j_query, NULL) != H_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "data_source_clean_removed - error cleaning table %s", TALIESIN_TABLE_FOLDER);
    }
    json_decref(j_query);
    
    j_query = json_pack("{sss{sIsi}}",
                        "table",
                        TALIESIN_TABLE_MEDIA,
                        "where",
                          "tds_id",
                          tds_id,
                          "tm_refresh_status",
                          DATA_SOURCE_REFRESH_MODE_IN_REFRESH);
    if (h_delete(config->conn, j_query, NULL) != H_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "data_source_clean_removed - error cleaning table %s", TALIESIN_TABLE_MEDIA);
    }
    json_decref(j_query);
    
    if (media_image_cover_clean_orphan(config, tds_id, 0) != T_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "data_source_clean_removed - error media_image_cover_clean_orphan");
    }
    
    if (data_source_set_refresh_status(config, tds_id, DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) == T_OK) {
      return T_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "data_source_clean_removed - Error data_source_set_refresh_status not running");
      return T_ERROR;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_clean_removed - Error data_source_set_refresh_status running");
    return T_ERROR;
  }
}

void run_data_source_clean(struct config_elements * config, json_int_t tds_id) {
  json_t * j_query;
  char * cover_clause = msprintf("`tic_id` IN (SELECT `tic_id` FROM `%s` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ") OR `tic_id` IN (SELECT `tic_id` FROM `%s` WHERE `tds_id`=%" JSON_INTEGER_FORMAT ")", TALIESIN_TABLE_MEDIA, tds_id, TALIESIN_TABLE_FOLDER, tds_id);
  
  j_query = json_pack("{sss{s{ssss}sI}}",
                      "table",
                      TALIESIN_TABLE_IMAGE_COVER,
                      "where",
                        " ",
                          "operator",
                          "raw",
                          "value",
                          cover_clause,
                        "tds_id",
                        tds_id);
  o_free(cover_clause);
  if (h_delete(config->conn, j_query, NULL) != H_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_clean - error cleaning table %s", TALIESIN_TABLE_IMAGE_COVER);
  }
  json_decref(j_query);

  j_query = json_pack("{sss{sI}}",
                      "table",
                      TALIESIN_TABLE_FOLDER,
                      "where",
                        "tds_id",
                        tds_id);
  if (h_delete(config->conn, j_query, NULL) != H_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_clean - error cleaning table %s", TALIESIN_TABLE_FOLDER);
  }
  json_decref(j_query);
  
  j_query = json_pack("{sss{sI}}",
                      "table",
                      TALIESIN_TABLE_MEDIA,
                      "where",
                        "tds_id",
                        tds_id);
  if (h_delete(config->conn, j_query, NULL) != H_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "data_source_clean - error cleaning table %s", TALIESIN_TABLE_MEDIA);
  }
  json_decref(j_query);
}

void * thread_run_refresh_data_source(void * data) {
  struct _refresh_config * refresh_config = (struct _refresh_config *)data;
  struct config_elements * config = refresh_config->config;
  AVCodecContext  * thumbnail_cover_codec_context = NULL;
  json_int_t tds_id = json_integer_value(json_object_get(refresh_config->j_data_source, "tds_id"));
  int i;
  char * root_path;

  // Wait for previous scans to be completed
  while (refresh_config->index && refresh_config->refresh_status == DATA_SOURCE_REFRESH_STATUS_PENDING) {
    pthread_mutex_lock(&config->refresh_lock);
    pthread_cond_wait(&config->refresh_cond, &config->refresh_lock);
    pthread_mutex_unlock(&config->refresh_lock);
  }
  if (tds_id > 0) {
    refresh_config->refresh_status = refresh_config->refresh_action;
    if (data_source_set_refresh_status(config, tds_id, DATA_SOURCE_REFRESH_STATUS_RUNNING) == T_OK) {
      if (refresh_config->refresh_status == DATA_SOURCE_REFRESH_STATUS_RUNNING) {
        // Refreshing
        if (init_output_jpeg_image(&thumbnail_cover_codec_context, TALIESIN_COVER_THUMB_WIDTH, TALIESIN_COVER_THUMB_HEIGHT) != T_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "thread_run_refresh_data_source - Error init_output_jpeg_image");
          if (data_source_set_refresh_status(config, tds_id, DATA_SOURCE_REFRESH_STATUS_ERROR) != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "thread_run_refresh_data_source - Error setting data source refresh status");
          }
        } else {
          root_path = msprintf("%s/%s", json_string_value(json_object_get(refresh_config->j_data_source, "path")), refresh_config->path);
          refresh_config->nb_files_total = fs_directory_count_files_recursive(root_path);
          o_free(root_path);
          if (refresh_config->nb_files_total >= 0) {
            if (data_source_scan_directory(config, refresh_config, thumbnail_cover_codec_context, tds_id, refresh_config->j_data_source, refresh_config->path, folder_get_id(config, refresh_config->j_data_source, 0, refresh_config->path)) == T_OK) {
              if (data_source_set_refresh_status(config, tds_id, DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) != T_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "thread_run_refresh_data_source - Error setting data source refresh status");
              }
            } else {
              if (data_source_set_refresh_status(config, tds_id, DATA_SOURCE_REFRESH_STATUS_ERROR) != T_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "thread_run_refresh_data_source - Error setting data source refresh status");
              }
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "thread_run_refresh_data_source - Error calculating the number of files in the data_source");
            if (data_source_set_refresh_status(config, tds_id, DATA_SOURCE_REFRESH_STATUS_ERROR) != T_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "thread_run_refresh_data_source - Error setting data source refresh status");
            }
          }
        }
        avcodec_close(thumbnail_cover_codec_context);
        avcodec_free_context(&thumbnail_cover_codec_context);
        if (data_source_clean_removed(config, tds_id) != T_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "thread_run_refresh_data_source - Error data_source_clean_removed");
        }
      } else {
        // Cleaning
        run_data_source_clean(config, tds_id);
        if (data_source_set_refresh_status(config, tds_id, DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) != T_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "thread_run_refresh_data_source - Error setting data source refresh status");
        }
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "thread_run_refresh_data_source - Error setting data source refresh status");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "thread_run_refresh_data_source - Error getting tds_id");
  }
  for (i=0; i<config->nb_refresh_status - 1; i++) {
    if (json_integer_value(json_object_get(config->refresh_status_list[i]->j_data_source, "tds_id")) == tds_id) {
      break;
    }
  }
  
  for (;i<config->nb_refresh_status - 1; i++) {
    config->refresh_status_list[i + 1]->index--;
    config->refresh_status_list[i] = config->refresh_status_list[i + 1];
  }
  pthread_mutex_lock(&config->refresh_lock);
  pthread_cond_signal(&config->refresh_cond);
  pthread_mutex_unlock(&config->refresh_lock);
  json_decref(refresh_config->j_data_source);
  o_free(refresh_config->path);
  o_free(refresh_config);
  if (config->nb_refresh_status > 1) {
    config->refresh_status_list = o_realloc(config->refresh_status_list, (config->nb_refresh_status - 1)*sizeof(struct _refresh_config));
  } else {
    o_free(config->refresh_status_list);
    config->refresh_status_list = NULL;
  }
  config->nb_refresh_status--;
  return NULL;
}

int data_source_refresh_run(struct config_elements * config, json_t * j_data_source, const char * path, int clean) {
  int thread_refresh_ret, thread_refresh_detach, ret, index;
  pthread_t thread_refresh;
  json_int_t tds_id = json_integer_value(json_object_get(j_data_source, "tds_id"));
  json_t * j_status;
  struct _refresh_config * refresh_config;
  
  j_status = data_source_get_refresh_status(config, tds_id);
  if (check_result_value(j_status, T_OK) && (0 == o_strcmp("not running", json_string_value(json_object_get(json_object_get(j_status, "refresh"), "status"))) || 0 == o_strcmp("error", json_string_value(json_object_get(json_object_get(j_status, "refresh"), "status"))))) {
    index = config->nb_refresh_status;
    refresh_config = o_malloc(sizeof(struct _refresh_config));
    config->refresh_status_list = o_realloc(config->refresh_status_list, (config->nb_refresh_status + 1)*sizeof(struct _refresh_config *));
    if (refresh_config != NULL && config->refresh_status_list != NULL) {
      config->nb_refresh_status++;
      config->refresh_status_list[index] = refresh_config;
      refresh_config->config = config;
      refresh_config->j_data_source = j_data_source;
      refresh_config->path = o_strdup(path);
      if (index) {
        refresh_config->refresh_status = DATA_SOURCE_REFRESH_STATUS_PENDING;
      } else {
        refresh_config->refresh_status = DATA_SOURCE_REFRESH_STATUS_RUNNING;
      }
      if (clean) {
        refresh_config->refresh_action = DATA_SOURCE_REFRESH_STATUS_CLEANING;
      } else {
        refresh_config->refresh_action = DATA_SOURCE_REFRESH_STATUS_RUNNING;
      }
      refresh_config->nb_files_read = 0;
      refresh_config->nb_files_total = 0;
      refresh_config->index = index;
      if (!index || data_source_set_refresh_status(config, tds_id, DATA_SOURCE_REFRESH_STATUS_PENDING) == T_OK) {
        thread_refresh_ret = pthread_create(&thread_refresh, NULL, thread_run_refresh_data_source, (void *)refresh_config);
        thread_refresh_detach = pthread_detach(thread_refresh);
        if (thread_refresh_ret || thread_refresh_detach) {
          y_log_message(Y_LOG_LEVEL_ERROR, "data_source_refresh_run - Error creating or detaching data source refresh thread, return code: %d, detach code: %d",
                        thread_refresh_ret, thread_refresh_detach);
          ret = T_ERROR;
        } else {
          ret = T_OK;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "data_source_refresh_run - Error setting status pending to data_sources");
        ret = T_ERROR;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "data_source_refresh_run - Error allocating resources for refresh_config->refresh_status_list");
      ret = T_ERROR;
    }
  } else if (check_result_value(j_status, T_OK) && (0 == o_strcmp("running", json_string_value(json_object_get(json_object_get(j_status, "refresh"), "status"))) || 0 == o_strcmp("preparing", json_string_value(json_object_get(json_object_get(j_status, "refresh"), "status"))))) {
    ret = T_ERROR_PARAM;
  } else {
    ret = T_ERROR_NOT_FOUND;
  }
  json_decref(j_status);
  return ret;
}

int data_source_refresh_stop(struct config_elements * config, json_t * j_data_source) {
  int i;
  json_int_t tds_id = json_integer_value(json_object_get(j_data_source, "tds_id"));
  
  for (i=0; i<config->nb_refresh_status; i++) {
    if (json_integer_value(json_object_get(config->refresh_status_list[i]->j_data_source, "tds_id")) == tds_id) {
      config->refresh_status_list[i]->refresh_status = DATA_SOURCE_REFRESH_STATUS_STOP;
    }
  }
  return data_source_set_refresh_status(config, tds_id, DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING);
}

json_t * data_source_get_refresh_status(struct config_elements * config, json_int_t tds_id) {
  json_t * j_query, * j_result = NULL, * j_return = NULL;
  int res, i;
  json_int_t status;
  char * str_status;
  
  j_query = json_pack("{sss[s]s{sI}}",
                      "table",
                      TALIESIN_TABLE_DATA_SOURCE,
                      "columns",
                        "tds_refresh_status",
                      "where",
                        "tds_id",
                        tds_id);
  res = h_select(config->conn, j_query, &j_result, NULL);
  json_decref(j_query);
  if (res == H_OK) {
    if (json_array_size(j_result) > 0) {
      status = json_integer_value(json_object_get(json_array_get(j_result, 0), "tds_refresh_status"));
      if (status == DATA_SOURCE_REFRESH_STATUS_RUNNING) {
        for (i=0; i<config->nb_refresh_status; i++) {
          if (json_integer_value(json_object_get(config->refresh_status_list[i]->j_data_source, "tds_id")) == tds_id) {
            j_return = json_pack("{sis{sssisi}}", "result", T_OK, "refresh", "status", "running", "read", config->refresh_status_list[i]->nb_files_read, "total", config->refresh_status_list[i]->nb_files_total);
          }
        }
        if (j_return == NULL) {
          j_return = json_pack("{sis{ss}}", "result", T_OK, "refresh", "status", "error");
        }
      } else {
        switch (status) {
          case DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING:
            str_status = "not running";
            break;
          case DATA_SOURCE_REFRESH_STATUS_PREPARING:
            str_status = "preparing";
            break;
          case DATA_SOURCE_REFRESH_STATUS_PENDING:
            str_status = "pending";
            break;
          default:
            str_status = "error";
            break;
        }
        j_return = json_pack("{sis{ss}}", "result", T_OK, "refresh", "status", str_status);
      }
    } else {
      j_return = json_pack("{si}", "result", T_ERROR_NOT_FOUND);
    }
  } else {
    j_return = json_pack("{si}", "result", T_ERROR_DB);
  }
  json_decref(j_result);
  
  return j_return;
}

int data_source_set_refresh_status(struct config_elements * config, json_int_t tds_id, int new_status) {
  json_t * j_query;
  int res;
  
  j_query = json_pack("{sss{si}s{sI}}",
                      "table",
                      TALIESIN_TABLE_DATA_SOURCE,
                      "set",
                        "tds_refresh_status",
                        new_status,
                      "where",
                        "tds_id",
                        tds_id);
  if (j_query != NULL) {
    if (new_status == DATA_SOURCE_REFRESH_STATUS_RUNNING) {
      json_object_set_new(json_object_get(j_query, "set"), "tds_last_updated", json_pack("{ss}", "raw", config->conn->type==HOEL_DB_TYPE_MARIADB?"NOW()":"(strftime('%s','now'))"));
    }
    res = h_update(config->conn, j_query, NULL);
    json_decref(j_query);
    return res==H_OK?T_OK:T_ERROR_DB;
  } else {
    return T_ERROR;
  }
}
