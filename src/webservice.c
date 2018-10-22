/**
 *
 * Taliesin - Media server
 * 
 * Webservices (endpoints) implementations
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
#include <pthread.h>
#include <sys/time.h>

#include "taliesin.h"

int has_scope(struct config_elements * config, json_t * scope_array, const char * scope) {
  json_t * element;
  size_t index;
  if (!config->use_oauth2_authentication) {
    return 1;
  } else if (scope_array != NULL && scope != NULL && json_is_array(scope_array)) {
    json_array_foreach(scope_array, index, element) {
      if (json_is_string(element) && 0 == o_strcmp(json_string_value(element), scope)) {
        return 1;
      }
    }
  }
  return 0;
}

const char * get_username(const struct _u_request * request, struct _u_response * response, struct config_elements * config) {
  const char * username = NULL;
  if (has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin)) {
    // user can be himself or impersonated
    if (u_map_get(request->map_url, "username") != NULL) {
      username = u_map_get(request->map_url, "username");
    }
  }
  if (username == NULL){
    username =  json_string_value(json_object_get((json_t *)response->shared_data, "username"));
  }
  return username;
}

int set_response_json_body_and_clean(struct _u_response * response, uint status, json_t * json_body) {
  int res = ulfius_set_json_body_response(response, status, json_body);
  json_decref(json_body);
  return res;
}

/**
 * default callback endpoint
 * return an error 404
 */
int callback_default (const struct _u_request * request, struct _u_response * response, void * user_data) {
  set_response_json_body_and_clean(response, 404, json_pack("{ssss}", "error", "resource not found", "message", "no resource available at this address"));
  return U_CALLBACK_CONTINUE;
}

/**
 * OPTIONS callback function
 * Send mandatory parameters for browsers to call REST APIs
 */
int callback_taliesin_options (const struct _u_request * request, struct _u_response * response, void * user_data) {
  u_map_put(response->map_header, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  u_map_put(response->map_header, "Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Bearer, Authorization");
  u_map_put(response->map_header, "Access-Control-Max-Age", "1800");
  return U_CALLBACK_COMPLETE;
}

/**
 * api description endpoint
 * send the location of prefixes
 */
int callback_taliesin_server_configuration (const struct _u_request * request, struct _u_response * response, void * user_data) {
  set_response_json_body_and_clean(response, 200, json_pack("{sssssssssisisiso}", 
                        "api_prefix", 
                        ((struct config_elements *)user_data)->api_prefix,
                        "oauth_scope_user",
                        ((struct config_elements *)user_data)->oauth_scope_user,
                        "oauth_scope_admin",
                        ((struct config_elements *)user_data)->oauth_scope_admin,
                        "default_stream_format",
                        ((struct config_elements *)user_data)->stream_format,
                        "default_stream_channels",
                        ((struct config_elements *)user_data)->stream_channels,
                        "default_stream_sample_rate",
                        ((struct config_elements *)user_data)->stream_sample_rate,
                        "default_stream_bitrate",
                        ((struct config_elements *)user_data)->stream_bitrate,
                        "use_websockets",
#ifdef U_DISABLE_WEBSOCKET
                        json_false()
#else
                        json_true()
#endif
                        ));
  return U_CALLBACK_COMPLETE;
};

/**
 * Authentication endpoint, user scope, redirect to glelwyd authentication callback or build a fake valid user depending on the configuration
 */
int callback_taliesin_check_access (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  char * scope;
  
  if (config->use_oauth2_authentication) {
#ifdef DISABLE_OAUTH2
    return U_CALLBACK_UNAUTHORIZED;
#else
    return callback_check_glewlwyd_access_token(request, response, config->glewlwyd_resource_config);
#endif
  } else {
    scope = msprintf("%s %s", config->oauth_scope_user, config->oauth_scope_admin);
    response->shared_data = (void*)json_pack("{ssss}", "username", TALIESIN_NO_AUTHENTICATION_USERNAME, "scope", scope);
    o_free(scope);
    return U_CALLBACK_CONTINUE;
  }
}

/**
 * Authentication endpoint, admin scope, redirect to glelwyd authentication callback or build a fake valid user depending on the configuration
 */
int callback_taliesin_check_admin_access (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  char * scope;
#ifndef DISABLE_OAUTH2
  int res, found = 0;
  json_t * j_element;
  size_t index;
#endif
  
  if (config->use_oauth2_authentication) {
#ifdef DISABLE_OAUTH2
    return U_CALLBACK_UNAUTHORIZED;
#else
    if ((res = callback_check_glewlwyd_access_token(request, response, config->glewlwyd_resource_config)) == U_CALLBACK_CONTINUE) {
      json_array_foreach(json_object_get(((json_t *)response->shared_data), "scope"), index, j_element) {
        if (0 ==  o_strcmp(json_string_value(j_element), config->oauth_scope_admin)) {
          found = 1;
          break;
        }
      }
      if (found) {
        return U_CALLBACK_CONTINUE;
      } else {
        json_decref((json_t *)response->shared_data);
        response->shared_data = NULL;
        return U_CALLBACK_UNAUTHORIZED;
      }
    } else {
      return res;
    }
#endif
  } else {
    scope = msprintf("%s %s", config->oauth_scope_user, config->oauth_scope_admin);
    response->shared_data = (void*)json_pack("{ssss}", "username", TALIESIN_NO_AUTHENTICATION_USERNAME, "scope", scope);
    o_free(scope);
    return U_CALLBACK_CONTINUE;
  }
}

/**
 * Last endpoint called, clean response->shared_data
 */
int callback_clean (const struct _u_request * request, struct _u_response * response, void * user_data) {
  if (response->shared_data != NULL) {
    json_decref((json_t *)response->shared_data);
  }
  return U_CALLBACK_COMPLETE;
}

/**
 * Data source callbacks
 */
int callback_taliesin_data_source_list (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_result;
  int res = U_CALLBACK_CONTINUE;
  
  j_result = data_source_list(config, get_username(request, response, config));
  if (!check_result_value(j_result, T_OK)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_list - Error getting current user data sources");
    res = U_CALLBACK_ERROR;
  } else if (ulfius_set_json_body_response(response, 200, json_object_get(j_result, "data_source")) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_list - Error setting json response");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_result);
  return res;
}

int callback_taliesin_data_source_get (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_result;
  int res = U_CALLBACK_CONTINUE;
  
  j_result = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 0);
  if (check_result_value(j_result, T_OK)) {
    if (ulfius_set_json_body_response(response, 200, json_object_get(j_result, "data_source")) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_get - Error setting json response");
      res = U_CALLBACK_ERROR;
    }
  } else if (check_result_value(j_result, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_get - Error getting data source");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_result);
  return res;
}

int callback_taliesin_data_source_add (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_body = ulfius_get_json_body_request(request, NULL), * j_is_valid;
  int res = U_CALLBACK_CONTINUE;
  
  j_is_valid = is_data_source_valid(config, get_username(request, response, config), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin), j_body, 0);
  if (j_is_valid != NULL) {
    if (json_array_size(j_is_valid) == 0) {
      if (data_source_add(config, get_username(request, response, config), j_body) != T_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_add - Error data_source_add");
        res = U_CALLBACK_ERROR;
      }
    } else {
      if (ulfius_set_json_body_response(response, 400, j_is_valid) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_add - Error setting json response");
        res = U_CALLBACK_ERROR;
      }
    }
    json_decref(j_is_valid);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_add - Error is_data_source_valid");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_body);
  return res;
}

int callback_taliesin_data_source_set (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_body = ulfius_get_json_body_request(request, NULL), * j_is_valid, * j_data_source;
  int res = U_CALLBACK_CONTINUE, is_admin = has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin);
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (data_source_can_update(json_object_get(j_data_source, "data_source"), is_admin)) {
      j_is_valid = is_data_source_valid(config, get_username(request, response, config), is_admin, j_body, 1);
      if (j_is_valid != NULL) {
        if (json_array_size(j_is_valid) == 0) {
          if (data_source_set(config, (!o_strcmp(TALIESIN_SCOPE_ALL, json_string_value(json_object_get(json_object_get(j_data_source, "data_source"), "scope")))?NULL:get_username(request, response, config)), u_map_get(request->map_url, "data_source"), j_body) != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_set - Error data_source_set");
            res = U_CALLBACK_ERROR;
          }
        } else {
          if (ulfius_set_json_body_response(response, 400, j_is_valid) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_set - Error setting json response");
            res = U_CALLBACK_ERROR;
          }
        }
        json_decref(j_is_valid);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_set - Error is_data_source_valid");
        res = U_CALLBACK_ERROR;
      }
    } else {
      if (set_response_json_body_and_clean(response, 403, json_pack("[{ss}]", "scope", "User can't modify a data source for all users, admin is required")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_set - Error setting json response");
        res = U_CALLBACK_ERROR;
      }
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_set - Error getting data_source");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_data_source);
  json_decref(j_body);
  
  return res;
}

int callback_taliesin_data_source_delete (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source;
  int res = U_CALLBACK_CONTINUE;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (data_source_can_update(json_object_get(j_data_source, "data_source"), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin))) {
      if (data_source_delete(config, get_username(request, response, config), u_map_get(request->map_url, "data_source")) != T_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_delete - Error data_source_delete");
        res = U_CALLBACK_ERROR;
      }
    } else {
      if (set_response_json_body_and_clean(response, 403, json_pack("[{ss}]", "scope", "User can't delete a data source for all users, admin is required")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_delete - Error setting json response");
        res = U_CALLBACK_ERROR;
      }
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_delete - Error getting data_source");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_data_source);
  
  return res;
}

int callback_taliesin_data_source_refresh_run (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source, * j_media;
  int res = U_CALLBACK_CONTINUE;
  char * decoded_url, * path, * decode_path, * decode_path_save, * last;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (data_source_can_update(json_object_get(j_data_source, "data_source"), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin))) {
      decoded_url = url_decode(request->http_url);
      if (decoded_url != NULL) {
        path = o_strdup((decoded_url + snprintf(NULL, 0, "/%s/data_source/%s/refresh/", config->api_prefix, u_map_get(request->map_url, "data_source"))));
        last = strrchr(path, '?');
        if (last != NULL) {
          path[last - path] = '\0';
        }
        decode_path = decode_path_save = url_decode(path);
        while (decode_path[0] == '/') {
          decode_path++;
        }
        if (o_strlen(decode_path) == 0) {
          if (data_source_refresh_run(config, json_deep_copy(json_object_get(j_data_source, "data_source")), decode_path, 0) != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_run - Error data_source_refresh_run");
            res = U_CALLBACK_ERROR;
          }
        } else {
          j_media = media_get(config, json_object_get(j_data_source, "data_source"), decode_path);
          if (check_result_value(j_media, T_OK) && json_is_array(json_object_get(j_media, "media"))) {
            if (data_source_refresh_run(config, json_deep_copy(json_object_get(j_data_source, "data_source")), decode_path, 0) != T_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_run - Error data_source_refresh_run");
              res = U_CALLBACK_ERROR;
            }
          } else if (check_result_value(j_media, T_ERROR_NOT_FOUND)) {
            response->status = 404;
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_run - Error media_get");
            res = U_CALLBACK_ERROR;
          }
          json_decref(j_media);
        }
        o_free(decode_path_save);
        o_free(path);
      }
      o_free(decoded_url);
    } else {
      if (set_response_json_body_and_clean(response, 403, json_pack("[{ss}]", "scope", "User can't refresh a data source for all users, admin is required")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_run - Error setting json response");
        res = U_CALLBACK_ERROR;
      }
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_run - Error getting data_source");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_data_source);
  
  return res;
}

int callback_taliesin_data_source_refresh_status (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source, * j_status;
  int res = U_CALLBACK_CONTINUE;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    j_status = data_source_get_refresh_status(config, json_integer_value(json_object_get(json_object_get(j_data_source, "data_source"), "tds_id")));
    if (check_result_value(j_status, T_OK)) {
      if (ulfius_set_json_body_response(response, 200, json_object_get(j_status, "refresh")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_status - error ulfius_set_json_body_response");
        response->status = 500;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_status - error data_source_get_refresh_status");
      response->status = 500;
    }
    json_decref(j_status);
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_status - Error getting data_source");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_data_source);
  
  return res;
}

int callback_taliesin_data_source_refresh_stop (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source;
  int res;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (data_source_can_update(json_object_get(j_data_source, "data_source"), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin))) {
      if ((res = data_source_refresh_stop(config, json_object_get(j_data_source, "data_source"))) == T_ERROR) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_stop - Error data_source_refresh_stop");
        response->status = 500;
      } else if (res == T_ERROR_PARAM) {
        response->status = 400;
      }
    } else {
      if (set_response_json_body_and_clean(response, 403, json_pack("[{ss}]", "scope", "User can't refresh a data source for all users, admin is required")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_stop - Error setting json response");
        response->status = 500;
      }
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_stop - Error getting data_source");
    response->status = 500;
  }
  json_decref(j_data_source);
  
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_data_source_clean (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (data_source_can_update(json_object_get(j_data_source, "data_source"), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin))) {
      if (data_source_refresh_run(config, json_deep_copy(json_object_get(j_data_source, "data_source")), NULL, 1) != T_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_clean - Error data_source_refresh_run");
        response->status = 500;
      }
    } else {
      if (set_response_json_body_and_clean(response, 403, json_pack("[{ss}]", "scope", "User can't refresh a data source for all users, admin is required")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_stop - Error setting json response");
        response->status = 500;
      }
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_data_source_refresh_stop - Error getting data_source");
    response->status = 500;
  }
  json_decref(j_data_source);
  
  return U_CALLBACK_CONTINUE;
}

/**
 * Media callbacks
 */
int callback_taliesin_media_get_path (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source, * j_result = NULL, * j_stream_info = NULL, * j_valid, * j_cover;
  char * path, * last, * decode_path, * decode_path_save, * decoded_url;
  const char * format;
  unsigned short int channels;
  unsigned int sample_rate, bit_rate;
  struct _t_webradio * webradio;
  int ret_thread_webradio = 0, detach_thread_webradio = 0;
  pthread_t thread_webradio;
  unsigned char * cover_decoded;
  size_t cover_decoded_len, cover_b64_len;
  const char * cover_b64;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (json_integer_value(json_object_get(json_object_get(j_data_source, "data_source"), "refresh_status")) == DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) {
      decoded_url = url_decode(request->http_url);
      if (decoded_url != NULL) {
        path = o_strdup((decoded_url + snprintf(NULL, 0, "/%s/data_source/%s/media/path/", config->api_prefix, u_map_get(request->map_url, "data_source"))));
        last = strrchr(path, '?');
        if (last != NULL) {
          path[last - path] = '\0';
        }
        decode_path = o_strdup(path);
        decode_path_save = decode_path;
        while (decode_path[0] == '/') {
          decode_path++;
        }
        while (decode_path[strlen(decode_path) - 1] == '/') {
          decode_path[strlen(decode_path) - 1] = '\0';
        }
        j_result = media_get(config, json_object_get(j_data_source, "data_source"), decode_path);
        if (check_result_value(j_result, T_OK)) {
          if (u_map_get(request->map_url, "webradio") != NULL || u_map_get(request->map_url, "jukebox") != NULL) {
            format = u_map_get(request->map_url, "format");
            channels = u_map_get(request->map_url, "channels")!=NULL?strtol(u_map_get(request->map_url, "channels"), NULL, 10):TALIESIN_STREAM_DEFAULT_CHANNELS;
            sample_rate = u_map_get(request->map_url, "samplerate")!=NULL?strtol(u_map_get(request->map_url, "samplerate"), NULL, 10):TALIESIN_STREAM_DEFAULT_SAMPLE_RATE;
            if (0 == o_strcmp(format, "flac")) {
              bit_rate = TALIESIN_STREAM_FLAC_BIT_RATE;
            } else {
              bit_rate = u_map_get(request->map_url, "bitrate")!=NULL?strtol(u_map_get(request->map_url, "bitrate"), NULL, 10):TALIESIN_STREAM_DEFAULT_BIT_RATE;
            }
            
            if (format == NULL) {
              format = TALIESIN_STREAM_DEFAULT_FORMAT;
            }
            if (!channels) {
              sample_rate = TALIESIN_STREAM_DEFAULT_CHANNELS;
            }
            if (!sample_rate) {
              channels = TALIESIN_STREAM_DEFAULT_SAMPLE_RATE;
            }
            if (!bit_rate) {
              bit_rate = TALIESIN_STREAM_DEFAULT_BIT_RATE;
            }
            j_valid = is_stream_parameters_valid((u_map_get(request->map_url, "webradio") != NULL), format, channels, sample_rate, bit_rate);
            if (j_valid != NULL && json_array_size(j_valid) == 0) {
              if (u_map_get(request->map_url, "webradio") != NULL) {
                format = "mp3";
                j_stream_info = add_webradio_from_path(config, json_object_get(j_data_source, "data_source"), decode_path, get_username(request, response, config), format, channels, sample_rate, bit_rate, (u_map_get(request->map_url, "recursive")!=NULL), (u_map_get(request->map_url, "random")!=NULL), u_map_get(request->map_url, "name"), &webradio);
                if (check_result_value(j_stream_info, T_OK)) {
                  ret_thread_webradio = pthread_create(&thread_webradio, NULL, webradio_run_thread, (void *)webradio);
                  detach_thread_webradio = pthread_detach(thread_webradio);
                  if (ret_thread_webradio || detach_thread_webradio) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "Error running thread webradio");
                    response->status = 500;
                  } else {
                    ulfius_set_json_body_response(response, 200, json_object_get(j_stream_info, "stream"));
                  }
                } else if (check_result_value(j_stream_info, T_ERROR_NOT_FOUND)) {
                  response->status = 404;
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error streaming file");
                  response->status = 500;
                }
                json_decref(j_stream_info);
              } else if (u_map_get(request->map_url, "jukebox") != NULL) {
                j_stream_info = add_jukebox_from_path(config, json_object_get(j_data_source, "data_source"), decode_path, get_username(request, response, config), format, channels, sample_rate, bit_rate, (u_map_get(request->map_url, "recursive")!=NULL), u_map_get(request->map_url, "name"));
                if (check_result_value(j_stream_info, T_OK)) {
                  ulfius_set_json_body_response(response, 200, json_object_get(j_stream_info, "stream"));
                } else if (check_result_value(j_stream_info, T_ERROR_NOT_FOUND)) {
                  response->status = 404;
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error creating jukebox");
                  response->status = 500;
                }
                json_decref(j_stream_info);
              }
            } else if (j_valid != NULL && json_array_size(j_valid) > 0) {
              ulfius_set_json_body_response(response, 400, j_valid);
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error is_stream_parameters_valid");
              response->status = 500;
            }
            json_decref(j_valid);
          } else if (u_map_get(request->map_url, "cover") != NULL) {
            j_cover = media_cover_get(config, json_object_get(j_data_source, "data_source"), decode_path, (u_map_get(request->map_url, "thumbnail")!=NULL));
            if (check_result_value(j_cover, T_OK)) {
              cover_b64 = json_string_value(json_object_get(j_cover, "cover"));
              if (u_map_get(request->map_url, "base64")!=NULL) {
                ulfius_set_string_body_response(response, 200, cover_b64);
              } else {
                cover_b64_len = o_strlen(cover_b64);
                cover_decoded = o_malloc(cover_b64_len);
                if (o_base64_decode((const unsigned char *)cover_b64, cover_b64_len, cover_decoded, &cover_decoded_len)) {
                  ulfius_set_binary_body_response(response, 200, (const char *)cover_decoded, cover_decoded_len);
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_cover_get - Error decoding cover");
                  response->status = 500;
                }
                o_free(cover_decoded);
              }
            } else if (check_result_value(j_cover, T_ERROR_NOT_FOUND)) {
              response->status = 404;
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_cover_get - Error getting cover");
              response->status = 500;
            }
            json_decref(j_cover);
          } else {
            ulfius_set_json_body_response(response, 200, json_object_get(j_result, "media"));
          }
        } else if (check_result_value(j_result, T_ERROR_NOT_FOUND)) {
          response->status = 404;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error getting media");
          response->status = 500;
        }
        json_decref(j_result);
        o_free(decode_path_save);
        o_free(path);
        o_free(decoded_url);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error getting decoded_url");
        response->status = 500;
      }
    } else {
      response->status = 503;
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error getting data_source");
    response->status = 500;
  }
  json_decref(j_data_source);
  
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_category_get_info (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source, * j_result = NULL;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (json_integer_value(json_object_get(json_object_get(j_data_source, "data_source"), "refresh_status")) == DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) {
      if (0 == o_strcmp(u_map_get(request->map_url, "level"), "artist") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "album") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "year") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "genre")) {
        j_result = media_category_get_info(config, json_object_get(j_data_source, "data_source"), u_map_get(request->map_url, "level"), u_map_get(request->map_url, "category"));
        if (check_result_value(j_result, T_OK)) {
          ulfius_set_json_body_response(response, 200, json_object_get(j_result, "info"));
        } else if (check_result_value(j_result, T_ERROR_NOT_FOUND)) {
          response->status = 404;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_get_info - Error media_category_get_info");
          response->status = 500;
        }
        json_decref(j_result);
      } else {
        response->status = 404;
      }
    } else {
      response->status = 503;
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_get_info - Error getting data_source");
    response->status = 500;
  }
  json_decref(j_data_source);
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_category_set_info (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source, * j_body = ulfius_get_json_body_request(request, NULL), * j_is_valid;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (json_integer_value(json_object_get(json_object_get(j_data_source, "data_source"), "refresh_status")) == DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) {
      if (0 == o_strcmp(u_map_get(request->map_url, "level"), "artist") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "album") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "year") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "genre")) {
        j_is_valid = is_media_category_info_valid(config, j_body);
        if (j_is_valid != NULL && json_array_size(j_is_valid) == 0) {
          if (media_category_set_info(config, json_object_get(j_data_source, "data_source"), u_map_get(request->map_url, "level"), u_map_get(request->map_url, "category"), j_body) != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_set_info - Error media_category_set_info");
            response->status = 500;
          }
        } else if (j_is_valid != NULL) {
          ulfius_set_json_body_response(response, 400, j_is_valid);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_set_info - Error is_media_category_info_valid");
          response->status = 500;
        }
        json_decref(j_is_valid);
      } else {
        response->status = 404;
      }
    } else {
      response->status = 503;
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_set_info - Error getting data_source");
    response->status = 500;
  }
  json_decref(j_data_source);
  json_decref(j_body);
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_category_delete_info (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (json_integer_value(json_object_get(json_object_get(j_data_source, "data_source"), "refresh_status")) == DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) {
      if (0 == o_strcmp(u_map_get(request->map_url, "level"), "artist") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "album") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "year") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "genre")) {
        if (media_category_delete_info(config, json_object_get(j_data_source, "data_source"), u_map_get(request->map_url, "level"), u_map_get(request->map_url, "category")) != T_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_delete_info - Error media_category_set_info");
          response->status = 500;
        }
      } else {
        response->status = 404;
      }
    } else {
      response->status = 503;
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_delete_info - Error getting data_source");
    response->status = 500;
  }
  json_decref(j_data_source);
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_category_get (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source, * j_result = NULL;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (json_integer_value(json_object_get(json_object_get(j_data_source, "data_source"), "refresh_status")) == DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) {
      if (0 == o_strcmp(u_map_get(request->map_url, "level"), "artist") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "album") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "year") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "genre")) {
        j_result = media_category_get(config, json_object_get(j_data_source, "data_source"), u_map_get(request->map_url, "level"));
        if (check_result_value(j_result, T_OK)) {
          ulfius_set_json_body_response(response, 200, json_object_get(j_result, "list"));
        } else if (check_result_value(j_result, T_ERROR_NOT_FOUND)) {
          response->status = 404;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_get - Error media_category_get");
          response->status = 500;
        }
        json_decref(j_result);
      } else {
        response->status = 404;
      }
    } else {
      response->status = 503;
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_get - Error getting data_source");
    response->status = 500;
  }
  json_decref(j_data_source);
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_category_list (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source, * j_result = NULL, * j_stream_info, * j_valid, * j_fake_jukebox, * j_element, * j_data_source_path;
  const char * format;
  unsigned short int channels;
  unsigned int sample_rate, bit_rate;
  struct _t_webradio * webradio;
  int ret_thread_webradio = 0, detach_thread_webradio = 0;
  pthread_t thread_webradio;
  size_t index;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (json_integer_value(json_object_get(json_object_get(j_data_source, "data_source"), "refresh_status")) == DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) {
      if (0 == o_strcmp(u_map_get(request->map_url, "level"), "artist") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "album") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "year") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "genre")) {
        j_result = media_category_list(config, json_object_get(j_data_source, "data_source"), u_map_get(request->map_url, "level"), u_map_get(request->map_url, "category"), (u_map_get(request->map_url, "webradio") != NULL || u_map_get(request->map_url, "jukebox") != NULL));
        if (check_result_value(j_result, T_OK)) {
          if (u_map_get(request->map_url, "webradio") != NULL || u_map_get(request->map_url, "jukebox") != NULL) {
            format = u_map_get(request->map_url, "format");
            channels = u_map_get(request->map_url, "channels")!=NULL?strtol(u_map_get(request->map_url, "channels"), NULL, 10):TALIESIN_STREAM_DEFAULT_CHANNELS;
            sample_rate = u_map_get(request->map_url, "samplerate")!=NULL?strtol(u_map_get(request->map_url, "samplerate"), NULL, 10):TALIESIN_STREAM_DEFAULT_SAMPLE_RATE;
            if (0 == o_strcmp(format, "flac")) {
              bit_rate = TALIESIN_STREAM_FLAC_BIT_RATE;
            } else {
              bit_rate = u_map_get(request->map_url, "bitrate")!=NULL?strtol(u_map_get(request->map_url, "bitrate"), NULL, 10):TALIESIN_STREAM_DEFAULT_BIT_RATE;
            }
            if (format == NULL) {
              format = TALIESIN_STREAM_DEFAULT_FORMAT;
            }
            if (!channels) {
              sample_rate = TALIESIN_STREAM_DEFAULT_CHANNELS;
            }
            if (!sample_rate) {
              channels = TALIESIN_STREAM_DEFAULT_SAMPLE_RATE;
            }
            if (!bit_rate) {
              bit_rate = TALIESIN_STREAM_DEFAULT_BIT_RATE;
            }
            j_valid = is_stream_parameters_valid((u_map_get(request->map_url, "webradio") != NULL), format, channels, sample_rate, bit_rate);
            if (j_valid != NULL && json_array_size(j_valid) == 0) {
              j_data_source_path = json_object_get(json_object_get(j_data_source, "data_source"), "path");
              json_array_foreach(json_object_get(j_result, "media"), index, j_element) {
                json_object_set(j_element, "tds_path", j_data_source_path);
              }
              if (u_map_get(request->map_url, "webradio") != NULL) {
                format = "mp3";
                j_fake_jukebox = json_pack("{sssssisO}", "description", u_map_get(request->map_url, "category"), "name", u_map_get(request->map_url, "category"), "tpl_id", 0, "media", json_object_get(j_result, "media"));
                j_stream_info = add_webradio_from_playlist(config, j_fake_jukebox, get_username(request, response, config), format, channels, sample_rate, bit_rate, (u_map_get(request->map_url, "random")!=NULL), u_map_get(request->map_url, "name"), &webradio);
                if (check_result_value(j_stream_info, T_OK)) {
                  ret_thread_webradio = pthread_create(&thread_webradio, NULL, webradio_run_thread, (void *)webradio);
                  detach_thread_webradio = pthread_detach(thread_webradio);
                  if (ret_thread_webradio || detach_thread_webradio) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "Error running thread webradio");
                    response->status = 500;
                  } else {
                    ulfius_set_json_body_response(response, 200, json_object_get(j_stream_info, "stream"));
                  }
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error streaming file");
                  response->status = 500;
                }
                json_decref(j_stream_info);
                json_decref(j_fake_jukebox);
              } else if (u_map_get(request->map_url, "jukebox") != NULL) {
                j_fake_jukebox = json_pack("{sssssisO}", "description", u_map_get(request->map_url, "category"), "name", u_map_get(request->map_url, "category"), "tpl_id", 0, "media", json_object_get(j_result, "media"));
                j_stream_info = add_jukebox_from_playlist(config, j_fake_jukebox, get_username(request, response, config), format, channels, sample_rate, bit_rate, u_map_get(request->map_url, "name"));
                if (check_result_value(j_stream_info, T_OK)) {
                  ulfius_set_json_body_response(response, 200, json_object_get(j_stream_info, "stream"));
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error creating jukebox");
                  response->status = 500;
                }
                json_decref(j_stream_info);
                json_decref(j_fake_jukebox);
              }
            } else if (j_valid != NULL && json_array_size(j_valid) > 0) {
              ulfius_set_json_body_response(response, 400, j_valid);
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error is_stream_parameters_valid");
              response->status = 500;
            }
            json_decref(j_valid);
          } else {
            ulfius_set_json_body_response(response, 200, json_object_get(j_result, "media"));
          }
        } else if (check_result_value(j_result, T_ERROR_NOT_FOUND)) {
          response->status = 404;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_get - Error media_category_list");
          response->status = 500;
        }
        json_decref(j_result);
      } else {
        response->status = 404;
      }
    } else {
      response->status = 503;
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_get_list - Error getting data_source");
    response->status = 500;
  }
  json_decref(j_data_source);
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_subcategory_get (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source, * j_result = NULL;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (json_integer_value(json_object_get(json_object_get(j_data_source, "data_source"), "refresh_status")) == DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) {
      if ((0 == o_strcmp(u_map_get(request->map_url, "level"), "artist") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "album") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "year") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "genre")) &&
          (0 == o_strcmp(u_map_get(request->map_url, "sublevel"), "artist") ||
          0 == o_strcmp(u_map_get(request->map_url, "sublevel"), "album") ||
          0 == o_strcmp(u_map_get(request->map_url, "sublevel"), "year") ||
          0 == o_strcmp(u_map_get(request->map_url, "sublevel"), "genre"))) {
        if (o_strcmp(u_map_get(request->map_url, "level"), u_map_get(request->map_url, "sublevel")) != 0) {
          j_result = media_subcategory_get(config, json_object_get(j_data_source, "data_source"), u_map_get(request->map_url, "level"), u_map_get(request->map_url, "category"), u_map_get(request->map_url, "sublevel"));
          if (check_result_value(j_result, T_OK)) {
            ulfius_set_json_body_response(response, 200, json_object_get(j_result, "list"));
          } else if (check_result_value(j_result, T_ERROR_NOT_FOUND)) {
            response->status = 404;
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_get - Error media_subcategory_get");
            response->status = 500;
          }
          json_decref(j_result);
        } else {
          response->status = 400;
        }
      } else {
        response->status = 404;
      }
    } else {
      response->status = 503;
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_subcategory_get - Error getting data_source");
    response->status = 500;
  }
  json_decref(j_data_source);
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_subcategory_list (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_data_source, * j_result = NULL, * j_stream_info, * j_valid, * j_fake_jukebox, * j_element, * j_data_source_path;
  const char * format;
  unsigned short int channels;
  unsigned int sample_rate, bit_rate;
  struct _t_webradio * webradio;
  int ret_thread_webradio = 0, detach_thread_webradio = 0;
  pthread_t thread_webradio;
  size_t index;
  
  j_data_source = data_source_get(config, get_username(request, response, config), u_map_get(request->map_url, "data_source"), 1);
  if (check_result_value(j_data_source, T_OK)) {
    if (json_integer_value(json_object_get(json_object_get(j_data_source, "data_source"), "refresh_status")) == DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING) {
      if ((0 == o_strcmp(u_map_get(request->map_url, "level"), "artist") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "album") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "year") ||
          0 == o_strcmp(u_map_get(request->map_url, "level"), "genre")) &&
          (0 == o_strcmp(u_map_get(request->map_url, "sublevel"), "artist") ||
          0 == o_strcmp(u_map_get(request->map_url, "sublevel"), "album") ||
          0 == o_strcmp(u_map_get(request->map_url, "sublevel"), "year") ||
          0 == o_strcmp(u_map_get(request->map_url, "sublevel"), "genre"))) {
        if (o_strcmp(u_map_get(request->map_url, "level"), u_map_get(request->map_url, "sublevel")) != 0) {
          j_result = media_subcategory_list(config, json_object_get(j_data_source, "data_source"), u_map_get(request->map_url, "level"), u_map_get(request->map_url, "category"), u_map_get(request->map_url, "sublevel"), u_map_get(request->map_url, "subcategory"), (u_map_get(request->map_url, "webradio") != NULL || u_map_get(request->map_url, "jukebox") != NULL));
          if (check_result_value(j_result, T_OK)) {
            if (u_map_get(request->map_url, "webradio") != NULL || u_map_get(request->map_url, "jukebox") != NULL) {
              format = u_map_get(request->map_url, "format");
              channels = u_map_get(request->map_url, "channels")!=NULL?strtol(u_map_get(request->map_url, "channels"), NULL, 10):TALIESIN_STREAM_DEFAULT_CHANNELS;
              sample_rate = u_map_get(request->map_url, "samplerate")!=NULL?strtol(u_map_get(request->map_url, "samplerate"), NULL, 10):TALIESIN_STREAM_DEFAULT_SAMPLE_RATE;
              if (0 == o_strcmp(format, "flac")) {
                bit_rate = TALIESIN_STREAM_FLAC_BIT_RATE;
              } else {
                bit_rate = u_map_get(request->map_url, "bitrate")!=NULL?strtol(u_map_get(request->map_url, "bitrate"), NULL, 10):TALIESIN_STREAM_DEFAULT_BIT_RATE;
              }
              if (format == NULL) {
                format = TALIESIN_STREAM_DEFAULT_FORMAT;
              }
              if (!channels) {
                sample_rate = TALIESIN_STREAM_DEFAULT_CHANNELS;
              }
              if (!sample_rate) {
                channels = TALIESIN_STREAM_DEFAULT_SAMPLE_RATE;
              }
              if (!bit_rate) {
                bit_rate = TALIESIN_STREAM_DEFAULT_BIT_RATE;
              }
              j_valid = is_stream_parameters_valid((u_map_get(request->map_url, "webradio") != NULL), format, channels, sample_rate, bit_rate);
              if (j_valid != NULL && json_array_size(j_valid) == 0) {
                j_data_source_path = json_object_get(json_object_get(j_data_source, "data_source"), "path");
                json_array_foreach(json_object_get(j_result, "media"), index, j_element) {
                  json_object_set(j_element, "tds_path", j_data_source_path);
                }
                if (u_map_get(request->map_url, "webradio") != NULL) {
                  format = "mp3";
                  j_fake_jukebox = json_pack("{sssssiso}", "description", u_map_get(request->map_url, "subcategory"), "name", u_map_get(request->map_url, "subcategory"), "tpl_id", 0, "media", json_object_get(j_result, "list"));
                  j_stream_info = add_webradio_from_playlist(config, j_fake_jukebox, get_username(request, response, config), format, channels, sample_rate, bit_rate, (u_map_get(request->map_url, "random")!=NULL), u_map_get(request->map_url, "name"), &webradio);
                  if (check_result_value(j_stream_info, T_OK)) {
                    ret_thread_webradio = pthread_create(&thread_webradio, NULL, webradio_run_thread, (void *)webradio);
                    detach_thread_webradio = pthread_detach(thread_webradio);
                    if (ret_thread_webradio || detach_thread_webradio) {
                      y_log_message(Y_LOG_LEVEL_ERROR, "Error running thread webradio");
                      response->status = 500;
                    } else {
                      ulfius_set_json_body_response(response, 200, json_object_get(j_stream_info, "stream"));
                    }
                  } else {
                    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error streaming file");
                    response->status = 500;
                  }
                  json_decref(j_stream_info);
                  json_decref(j_fake_jukebox);
                } else if (u_map_get(request->map_url, "jukebox") != NULL) {
                  j_fake_jukebox = json_pack("{sssssiso}", "description", u_map_get(request->map_url, "subcategory"), "name", u_map_get(request->map_url, "subcategory"), "tpl_id", 0, "media", json_object_get(j_result, "list"));
                  j_stream_info = add_jukebox_from_playlist(config, j_fake_jukebox, get_username(request, response, config), format, channels, sample_rate, bit_rate, u_map_get(request->map_url, "name"));
                  if (check_result_value(j_stream_info, T_OK)) {
                    ulfius_set_json_body_response(response, 200, json_object_get(j_stream_info, "stream"));
                  } else {
                    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error creating jukebox");
                    response->status = 500;
                  }
                  json_decref(j_stream_info);
                  json_decref(j_fake_jukebox);
                }
              } else if (j_valid != NULL && json_array_size(j_valid) > 0) {
                ulfius_set_json_body_response(response, 400, j_valid);
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error is_stream_parameters_valid");
                response->status = 500;
              }
              json_decref(j_valid);
            } else {
              ulfius_set_json_body_response(response, 200, json_object_get(j_result, "media"));
            }
          } else if (check_result_value(j_result, T_ERROR_NOT_FOUND)) {
            response->status = 404;
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_get - Error media_subcategory_list");
            response->status = 500;
          }
          json_decref(j_result);
        } else {
          response->status = 400;
        }
      } else {
        response->status = 404;
      }
    } else {
      response->status = 503;
    }
  } else if (check_result_value(j_data_source, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_category_subcategory_get - Error getting data_source");
    response->status = 500;
  }
  json_decref(j_data_source);
  return U_CALLBACK_CONTINUE;
}

/**
 * Stream callbacks
 */
int callback_taliesin_stream_media (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  struct _t_webradio * current_webradio = NULL;
  struct _t_jukebox * current_jukebox = NULL;
  unsigned int i, jukebox_index = 0;
  struct _client_data_webradio * client_data_webradio = NULL;
  struct _client_data_jukebox * client_data_jukebox;
  struct _t_file * file;
  char metaint[17] = {0}, * m3u_data = NULL, * content_disposition, * escaped_filename;
  uint64_t time_offset;
  int ret_thread_jukebox = 0, detach_thread_jukebox = 0;
  pthread_t thread_jukebox;
  
  for (i=0; i<config->nb_webradio; i++) {
    if (0 == o_strcmp(u_map_get(request->map_url, "stream_name"), config->webradio_set[i]->name)) {
      current_webradio = config->webradio_set[i];
      break;
    }
  }
  if (current_webradio == NULL) {
    for (i=0; i<config->nb_jukebox; i++) {
      if (0 == o_strcmp(u_map_get(request->map_url, "stream_name"), config->jukebox_set[i]->name)) {
        current_jukebox = config->jukebox_set[i];
        break;
      }
    }
  }
  
  if (u_map_get_case(request->map_url, "index") != NULL) {
    jukebox_index = strtol(u_map_get_case(request->map_url, "index"), NULL, 10);
  }
  if (current_webradio != NULL) {
    response->status = 200;
    // TODO specify timeout when MHD will support a timeout on a streaming response
    //if (config->timeout) {
      //response->timeout = config->timeout;
    //}
    y_log_message(Y_LOG_LEVEL_DEBUG, "Start stream");
    client_data_webradio = o_malloc(sizeof (struct _client_data_webradio));
    if (client_data_webradio != NULL) {
      if (client_data_webradio_init(client_data_webradio) == T_OK) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &client_data_webradio->start);
        o_strcpy(client_data_webradio->stream_name, u_map_get(request->map_url, "stream_name"));
        client_data_webradio->audio_stream = current_webradio->audio_stream;
        client_data_webradio->current_buffer = client_data_webradio->audio_stream->header_buffer;
        client_data_webradio->metadata_send = (0 == o_strcmp(client_data_webradio->audio_stream->stream_format, "mp3") && o_strcasecmp("1", u_map_get_case(request->map_header, "Icy-MetaData")))?-1:0;
        client_data_webradio->client_address = get_ip_source(request);
        client_data_webradio->user_agent = o_strdup(u_map_get_case(request->map_header, "User-Agent"));
        if (client_data_webradio->audio_stream->nb_client_connected >= TALIESIN_PLAYLIST_CLIENT_MAX) {
          response->status = 503;
        } else if (jukebox_index >= current_webradio->file_list->nb_files) {
          response->status = 404;
        } else {
          client_data_webradio->audio_stream->client_list = o_realloc(client_data_webradio->audio_stream->client_list, (client_data_webradio->audio_stream->nb_client_connected + 1)*sizeof(struct _client_data_webradio *));
          if (client_data_webradio->audio_stream->client_list != NULL) {
            client_data_webradio->audio_stream->client_list[client_data_webradio->audio_stream->nb_client_connected] = client_data_webradio;
            client_data_webradio->audio_stream->nb_client_connected++;
            if (client_data_webradio->audio_stream->first_buffer) {
              client_data_webradio->first_buffer_counter = client_data_webradio->audio_stream->first_buffer->counter;
            } else {
              client_data_webradio->first_buffer_counter = 0;
            }
            if (client_data_webradio->audio_stream->first_buffer != NULL) {
              time_offset = client_data_webradio->audio_stream->first_buffer->offset_list[client_data_webradio->audio_stream->first_buffer->last_offset];
              i=0;
              while (i < client_data_webradio->audio_stream->first_buffer->nb_offset && client_data_webradio->audio_stream->first_buffer->offset_list[i] < time_offset) {
                i++;
              }
              if (i < client_data_webradio->audio_stream->first_buffer->nb_offset) {
                client_data_webradio->buffer_offset = client_data_webradio->audio_stream->first_buffer->offset_list[i];
              } else {
                // This shouldn't happen
                client_data_webradio->buffer_offset = 0;
              }
            } else {
              client_data_webradio->buffer_offset = 0;
            }
            if (ulfius_set_stream_response(response, 200, u_webradio_stream, u_webradio_stream_free, U_STREAM_SIZE_UNKOWN, client_data_webradio->audio_stream->stream_bitrate / 8, client_data_webradio) == U_OK) {
              pthread_setname_np(pthread_self(), request->http_url);
              if (0 == o_strcmp(client_data_webradio->audio_stream->stream_format, "vorbis")) {
                u_map_put(response->map_header, "Content-Type", "application/ogg");
              } else if (0 == o_strcmp(client_data_webradio->audio_stream->stream_format, "flac")) {
                u_map_put(response->map_header, "Content-Type", "audio/flac");
              } else {
                u_map_put(response->map_header, "Content-Type", "audio/mpeg");
              }
              if (!client_data_webradio->metadata_send) {
                snprintf(metaint, 16, "%d", client_data_webradio->audio_stream->stream_bitrate / 8 * TALIESIN_STREAM_METADATA_INTERVAL);
                u_map_put(response->map_header, "icy-metaint", metaint);
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_media - Error ulfius_set_stream_response");
              response->status = 500;
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_media - Error allocating resources for client_list");
            response->status = 500;
          }
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_media - Error initializing client_data_webradio");
        response->status = 500;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_media - Error allocating resources for client_data_webradio");
      response->status = 500;
    }
    if (response->status != 200) {
      client_data_webradio_clean(client_data_webradio);
    }
  } else if (current_jukebox != NULL) {
    if (u_map_get_case(request->map_url, "index") != NULL) {
      file = file_list_get_file(current_jukebox->file_list, jukebox_index);
      if (file != NULL) {
        client_data_jukebox = o_malloc(sizeof(struct _client_data_jukebox));
        if (client_data_jukebox != NULL) {
          if (init_client_data_jukebox(client_data_jukebox) == T_OK) {
            client_data_jukebox->jukebox = current_jukebox;
            o_strcpy(client_data_jukebox->stream_name, u_map_get(request->map_url, "stream_name"));
            client_data_jukebox->audio_buffer = o_malloc(sizeof(struct _jukebox_audio_buffer));
            if (client_data_jukebox->audio_buffer != NULL && jukebox_audio_buffer_init(client_data_jukebox->audio_buffer) == T_OK) {
              client_data_jukebox->audio_buffer->client_address = get_ip_source(request);
              client_data_jukebox->audio_buffer->user_agent = o_strdup(u_map_get_case(request->map_header, "User-Agent"));
              client_data_jukebox->audio_buffer->file = file;
              client_data_jukebox->audio_buffer->jukebox = current_jukebox;
              ret_thread_jukebox = pthread_create(&thread_jukebox, NULL, jukebox_run_thread, (void *)client_data_jukebox);
              detach_thread_jukebox = pthread_detach(thread_jukebox);
              if (ret_thread_jukebox || detach_thread_jukebox) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Error running thread jukebox");
                response->status = 500;
              } else {
                client_data_jukebox->client_present = 1;
                client_data_jukebox->audio_buffer->status = TALIESIN_STREAM_TRANSCODE_STATUS_STARTED;
                if (ulfius_set_stream_response(response, 200, u_jukebox_stream, u_jukebox_stream_free, U_STREAM_SIZE_UNKOWN, current_jukebox->stream_bitrate / 8, client_data_jukebox) == U_OK) {
                  current_jukebox->nb_client++;
                  time(&current_jukebox->last_seen);
                  if (0 == o_strcmp(current_jukebox->stream_format, "vorbis")) {
                    u_map_put(response->map_header, "Content-Type", "application/ogg");
                  } else if (0 == o_strcmp(current_jukebox->stream_format, "flac")) {
                    u_map_put(response->map_header, "Content-Type", "audio/flac");
                  } else {
                    u_map_put(response->map_header, "Content-Type", "audio/mpeg");
                  }
                } else {
                  client_data_jukebox->client_present = 0;
                  y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_media - Error ulfius_set_stream_response");
                  response->status = 500;
                }
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_media - Error initializing client_data_jukebox->audio_buffer");
              response->status = 500;
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_media - Error initializing client_data_jukebox");
            response->status = 500;
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_media - Error allocating resources for client_data_jukebox");
          response->status = 500;
        }
      } else {
        response->status = 404;
      }
    } else {
      // Build m3u jukebox
      if (jukebox_build_m3u(config, current_jukebox, u_map_get(request->map_url, "url_prefix"), &m3u_data) == T_OK) {
        escaped_filename = url_encode(current_jukebox->display_name);
        content_disposition = msprintf("attachment; filename=%s.m3u", escaped_filename);
        u_map_put(response->map_header, "Content-Type", "audio/mpegurl");
        u_map_put(response->map_header, "Content-Disposition", content_disposition);
        if (ulfius_set_string_body_response(response, 200, m3u_data) != U_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_media - Error ulfius_set_string_body_response");
          response->status = 500;
        }
        o_free(escaped_filename);
        o_free(content_disposition);
        o_free(m3u_data);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_media - Error jukebox_build_m3u");
        response->status = 500;
      }
    }
  } else {
    response->status = 404;
  }
  if (response->shared_data != NULL) {
    json_decref((json_t *)response->shared_data);
  }
  return U_CALLBACK_COMPLETE;
}

int callback_taliesin_stream_get_list (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_result = stream_list(config, get_username(request, response, config));
  int res = U_CALLBACK_CONTINUE;
  
  if (!check_result_value(j_result, T_OK)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_get_list - Error getting current user data sources");
    res = U_CALLBACK_ERROR;
  } else if (ulfius_set_json_body_response(response, 200, json_object_get(j_result, "stream")) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_get_list - Error setting json response");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_result);
  return res;
}

int callback_taliesin_stream_manage (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_body = ulfius_get_json_body_request(request, NULL), * j_is_valid, * j_result_command;
  int res = U_CALLBACK_COMPLETE, i;
  struct _t_webradio * current_webradio = NULL;
  struct _t_jukebox * current_playlist = NULL;
  
  for (i=0; i<config->nb_webradio; i++) {
    if (0 == o_strcmp(u_map_get(request->map_url, "stream_name"), config->webradio_set[i]->name)) {
      current_webradio = config->webradio_set[i];
      break;
    }
  }
  if (current_webradio == NULL) {
    for (i=0; i<config->nb_jukebox; i++) {
      if (0 == o_strcmp(u_map_get(request->map_url, "stream_name"), config->jukebox_set[i]->name)) {
        current_playlist = config->jukebox_set[i];
        break;
      }
    }
  }
  if (current_webradio != NULL) {
    j_is_valid = is_webradio_command_valid(config, current_webradio, j_body, get_username(request, response, config), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin));
    if (j_is_valid != NULL) {
      if (json_array_size(j_is_valid) == 0) {
        j_result_command = webradio_command(config, current_webradio, get_username(request, response, config), j_body);
        if (!check_result_value(j_result_command, T_OK) && !check_result_value(j_result_command, T_ERROR_NOT_FOUND) && !check_result_value(j_result_command, T_ERROR_PARAM)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_manage - Error webradio_command");
          res = U_CALLBACK_ERROR;
        }
        if (check_result_value(j_result_command, T_OK) && json_object_get(j_result_command, "command") != NULL) {
          if (ulfius_set_json_body_response(response, 200, json_object_get(j_result_command, "command")) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_manage - Error setting json response");
            res = U_CALLBACK_ERROR;
          }
        } else if (check_result_value(j_result_command, T_ERROR_NOT_FOUND)) {
          response->status = 404;
        } else if (check_result_value(j_result_command, T_ERROR_PARAM)) {
          response->status = 400;
        }
        json_decref(j_result_command);
      } else {
        if (ulfius_set_json_body_response(response, 400, j_is_valid) != U_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_manage - Error setting json response");
          res = U_CALLBACK_ERROR;
        }
      }
      json_decref(j_is_valid);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_manage - Error is_webradio_command_valid");
      res = U_CALLBACK_ERROR;
    }
  } else if (current_playlist != NULL) {
    j_is_valid = is_jukebox_command_valid(config, current_playlist, j_body, get_username(request, response, config), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin));
    if (j_is_valid != NULL) {
      if (json_array_size(j_is_valid) == 0) {
        j_result_command = jukebox_command(config, current_playlist, get_username(request, response, config), j_body);
        if (!check_result_value(j_result_command, T_OK) && !check_result_value(j_result_command, T_ERROR_NOT_FOUND) && !check_result_value(j_result_command, T_ERROR_PARAM)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_manage - Error webradio_command");
          res = U_CALLBACK_ERROR;
        }
        if (check_result_value(j_result_command, T_OK) && json_object_get(j_result_command, "command") != NULL) {
          if (ulfius_set_json_body_response(response, 200, json_object_get(j_result_command, "command")) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_manage - Error setting json response");
            res = U_CALLBACK_ERROR;
          }
        } else if (check_result_value(j_result_command, T_ERROR_NOT_FOUND)) {
          response->status = 404;
        }
        json_decref(j_result_command);
      } else {
        if (ulfius_set_json_body_response(response, 400, j_is_valid) != U_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_manage - Error setting json response");
          res = U_CALLBACK_ERROR;
        }
      }
      json_decref(j_is_valid);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_manage - Error is_jukebox_command_valid");
      res = U_CALLBACK_ERROR;
    }
  } else {
    response->status = 404;
  }
  json_decref(j_body);
  if (response->shared_data != NULL) {
    json_decref((json_t *)response->shared_data);
  }
  return res;
}

#ifndef U_DISABLE_WEBSOCKET
int callback_taliesin_stream_manage_ws (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  struct _t_webradio * current_webradio = NULL;
  struct _t_jukebox * current_playlist = NULL;
  struct _ws_stream * ws_stream;
  int ret = U_CALLBACK_COMPLETE, i;
  
  for (i=0; i<config->nb_webradio; i++) {
    if (0 == o_strcmp(u_map_get(request->map_url, "stream_name"), config->webradio_set[i]->name)) {
      current_webradio = config->webradio_set[i];
      break;
    }
  }
  if (current_webradio == NULL) {
    for (i=0; i<config->nb_jukebox; i++) {
      if (0 == o_strcmp(u_map_get(request->map_url, "stream_name"), config->jukebox_set[i]->name)) {
        current_playlist = config->jukebox_set[i];
        break;
      }
    }
  }
  
  if (current_webradio != NULL || current_playlist != NULL) {
    ws_stream = o_malloc(sizeof(struct _ws_stream));
    if (ws_stream != NULL) {
      ws_stream->config = config;
      ws_stream->is_admin = has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin);
      if (ws_stream->is_admin && u_map_get(request->map_url, "username") != NULL) {
        ws_stream->username = o_strdup(u_map_get(request->map_url, "username"));
      } else {
        ws_stream->username = NULL;
      }
      ws_stream->is_authenticated = 0;
      ws_stream->expiration = 0;
      ws_stream->webradio = current_webradio;
      ws_stream->jukebox = current_playlist;
      ws_stream->status = TALIESIN_WEBSOCKET_PLAYLIST_STATUS_OPEN;
      if (ulfius_set_websocket_response(response, "taliesin", "permessage-deflate", &callback_websocket_stream_manager, ws_stream, &callback_websocket_stream_incoming_message, ws_stream, &callback_websocket_stream_onclose, ws_stream) == U_OK) {
        pthread_setname_np(pthread_self(), request->http_url);
        ret = U_CALLBACK_COMPLETE;
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_manage_ws - Error ulfius_set_websocket_response");
        o_free(ws_stream);
        ret = U_CALLBACK_ERROR;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_manage_ws - Error allocating resources for ws_stream");
      ret = U_CALLBACK_ERROR;
    }
  } else {
    response->status = 404;
  }
  if (response->shared_data != NULL) {
    json_decref((json_t *)response->shared_data);
  }
  return ret;
}

void callback_websocket_stream_manager (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_user_data) {
  struct _ws_stream * ws_stream = (struct _ws_stream *)websocket_user_data;
  char * message;
  json_t * j_result, * j_message;
  time_t now;
  struct timespec wait;
  struct timeval tv_now;
  int ret_wait;
  
  if (ws_stream->webradio != NULL) {
    ws_stream->webradio->nb_websocket++;
    // Loop until websocket is closed by the server or the client
    while (websocket_manager->connected && ws_stream->status == TALIESIN_WEBSOCKET_PLAYLIST_STATUS_OPEN && ws_stream->webradio->audio_stream->status != TALIESIN_STREAM_STATUS_STOPPED) {
      gettimeofday(&tv_now, NULL);
      wait.tv_sec = tv_now.tv_sec + 5;
      wait.tv_nsec = tv_now.tv_usec * 1000;
      pthread_mutex_lock(&ws_stream->webradio->message_lock);
      ret_wait = pthread_cond_timedwait(&ws_stream->webradio->message_cond, &ws_stream->webradio->message_lock, &wait);
      pthread_mutex_unlock(&ws_stream->webradio->message_lock);
      
      time(&now);
      if (ret_wait != ETIMEDOUT && websocket_manager->connected && ((ws_stream->is_authenticated && ws_stream->expiration && ws_stream->expiration > now) || !ws_stream->config->use_oauth2_authentication)) {
        if (ws_stream->webradio->message_type == TALIESIN_PLAYLIST_MESSAGE_TYPE_NEW_MEDIA) {
          j_result = media_get_by_id(ws_stream->config, ws_stream->webradio->audio_stream->first_buffer->file->tm_id);
          if (check_result_value(j_result, T_OK)) {
            json_object_del(json_object_get(j_result, "media"), "tm_id");
            j_message = json_pack("{sssO}", "command", "now", "result", json_object_get(j_result, "media"));
            message = json_dumps(j_message, JSON_COMPACT);
            if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_manager - Error ulfius_websocket_send_message");
            }
            o_free(message);
            json_decref(j_message);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_manager - Error executing command now");
          }
          json_decref(j_result);
        } else if (websocket_manager->connected && ws_stream->webradio->message_type == TALIESIN_PLAYLIST_MESSAGE_TYPE_CLOSE) {
          j_message = json_pack("{ss}", "command", "quit");
          message = json_dumps(j_message, JSON_COMPACT);
          if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_manager - Error ulfius_websocket_send_message");
          }
          o_free(message);
          json_decref(j_message);
        }
      }
    }
    
    if (ws_stream->status == TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSING) {
      ws_stream->status = TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSE;
      ws_stream->webradio->message_type = TALIESIN_PLAYLIST_MESSAGE_TYPE_NONE;
      pthread_mutex_lock(&ws_stream->webradio->message_lock);
      pthread_cond_broadcast(&ws_stream->webradio->message_cond);
      pthread_mutex_unlock(&ws_stream->webradio->message_lock);
    } else {
      ws_stream->status = TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSE;
    }
  } else if (ws_stream->jukebox != NULL) {
    ws_stream->jukebox->nb_websocket++;
    // Loop until websocket is closed by the server or the client
    while (websocket_manager->connected && ws_stream->status == TALIESIN_WEBSOCKET_PLAYLIST_STATUS_OPEN) {
      pthread_mutex_lock(&ws_stream->jukebox->message_lock);
      pthread_cond_wait(&ws_stream->jukebox->message_cond, &ws_stream->jukebox->message_lock);
      pthread_mutex_unlock(&ws_stream->jukebox->message_lock);
      
      if (websocket_manager->connected && ws_stream->is_authenticated) {
        if (ws_stream->jukebox->message_type == TALIESIN_PLAYLIST_MESSAGE_TYPE_CLOSING) {
          j_message = json_pack("{ss}", "command", "quit");
          message = json_dumps(j_message, JSON_COMPACT);
          if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_manager - Error ulfius_websocket_send_message");
          }
          o_free(message);
          json_decref(j_message);
          ws_stream->status = TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSING;
        }
      }
    }
    
    if (ws_stream->status == TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSING) {
      ws_stream->status = TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSE;
    } else {
      ws_stream->status = TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSE;
    }
  }
}

void callback_websocket_stream_onclose (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_user_data) {
  struct _ws_stream * ws_stream = (struct _ws_stream *)websocket_user_data;
  
  if (ws_stream->status != TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSE) {
    ws_stream->status = TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSING;
    if (ws_stream->jukebox != NULL) {
      ws_stream->jukebox->message_type = TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSING;
      pthread_mutex_lock(&ws_stream->jukebox->message_lock);
      pthread_cond_broadcast(&ws_stream->jukebox->message_cond);
      pthread_mutex_unlock(&ws_stream->jukebox->message_lock);
      
      while (ws_stream->status != TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSE) {
        pthread_mutex_lock(&ws_stream->jukebox->message_lock);
        pthread_cond_wait(&ws_stream->jukebox->message_cond, &ws_stream->jukebox->message_lock);
        pthread_mutex_unlock(&ws_stream->jukebox->message_lock);
      }
    } else if (ws_stream->webradio != NULL) {
      ws_stream->webradio->message_type = TALIESIN_PLAYLIST_MESSAGE_TYPE_CLOSING;
      pthread_mutex_lock(&ws_stream->webradio->message_lock);
      pthread_cond_broadcast(&ws_stream->webradio->message_cond);
      pthread_mutex_unlock(&ws_stream->webradio->message_lock);
      
      while (ws_stream->status != TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSE) {
        pthread_mutex_lock(&ws_stream->webradio->message_lock);
        pthread_cond_wait(&ws_stream->webradio->message_cond, &ws_stream->webradio->message_lock);
        pthread_mutex_unlock(&ws_stream->webradio->message_lock);
      }
    }
  }
  
  if (ws_stream->jukebox != NULL) {
    ws_stream->jukebox->nb_websocket--;
    ws_stream->jukebox->message_type = TALIESIN_PLAYLIST_MESSAGE_TYPE_TRASH;
    pthread_mutex_lock(&ws_stream->jukebox->websocket_lock);
    pthread_cond_broadcast(&ws_stream->jukebox->websocket_cond);
    pthread_mutex_unlock(&ws_stream->jukebox->websocket_lock);
  } else if (ws_stream->webradio != NULL) {
    ws_stream->webradio->nb_websocket--;
    ws_stream->webradio->message_type = TALIESIN_PLAYLIST_MESSAGE_TYPE_TRASH;
    pthread_mutex_lock(&ws_stream->webradio->websocket_lock);
    pthread_cond_broadcast(&ws_stream->webradio->websocket_cond);
    pthread_mutex_unlock(&ws_stream->webradio->websocket_lock);
  }
  o_free(ws_stream->username);
  o_free(ws_stream);
}

void callback_websocket_stream_incoming_message (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * last_message, void * websocket_user_data) {
  struct _ws_stream * ws_stream = (struct _ws_stream *)websocket_user_data;
  json_t * j_message = json_loadb(last_message->data, last_message->data_len, 0, NULL), * j_is_valid = NULL, * j_result_command, * j_out_message;
  char * message;
  const char * token_value;
  time_t now;
#ifndef DISABLE_OAUTH2
  json_t * j_res_scope, * j_access_token;
  int res_validity;
#endif
  
  time(&now);
  if (json_is_object(j_message) && json_is_string(json_object_get(j_message, "command")) && 0 == o_strcasecmp("authorization", json_string_value(json_object_get(j_message, "command")))) {
    token_value = json_string_value(json_object_get(j_message, "token"));
    if (token_value != NULL) {
#ifndef DISABLE_OAUTH2
      j_access_token = access_token_check_signature(ws_stream->config->glewlwyd_resource_config, token_value);
      if (check_result_value(j_access_token, G_OK)) {
        res_validity = access_token_check_validity(ws_stream->config->glewlwyd_resource_config, json_object_get(j_access_token, "grants"));
        if (res_validity == G_OK) {
          j_res_scope = access_token_check_scope(ws_stream->config->glewlwyd_resource_config, json_object_get(j_access_token, "grants"));
          if (check_result_value(j_res_scope, G_ERROR_INSUFFICIENT_SCOPE)) {
            j_out_message = json_pack("{ssss}", "command", "authorization", "result", "insufficient_scope");
            message = json_dumps(j_out_message, JSON_COMPACT);
            if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
            }
            o_free(message);
            json_decref(j_out_message);
            ws_stream->is_authenticated = 0;
          } else if (!check_result_value(j_res_scope, G_OK)) {
            j_out_message = json_pack("{ssss}", "command", "authorization", "result", "invalid_request");
            message = json_dumps(j_out_message, JSON_COMPACT);
            if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
            }
            o_free(message);
            json_decref(j_out_message);
            ws_stream->is_authenticated = 0;
          } else {
            j_out_message = json_pack("{ssss}", "command", "authorization", "result", "connected");
            message = json_dumps(j_out_message, JSON_COMPACT);
            if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
            }
            o_free(message);
            json_decref(j_out_message);
            ws_stream->is_authenticated = 1;
            ws_stream->expiration = json_integer_value(json_object_get(json_object_get(j_access_token, "grants"), "iat")) + json_integer_value(json_object_get(json_object_get(j_access_token, "grants"), "expires_in"));
            if (ws_stream->username == NULL) {
              ws_stream->username = o_strdup(json_string_value(json_object_get(json_object_get(j_access_token, "grants"), "username")));
            }
          }
          json_decref(j_res_scope);
        } else {
          j_out_message = json_pack("{ssss}", "command", "authorization", "result", "invalid_request");
          message = json_dumps(j_out_message, JSON_COMPACT);
          if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
          }
          o_free(message);
          json_decref(j_out_message);
          ws_stream->is_authenticated = 0;
        }
      } else {
        j_out_message = json_pack("{ssss}", "command", "authorization", "result", "invalid_request");
        message = json_dumps(j_out_message, JSON_COMPACT);
        if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
        }
        o_free(message);
        json_decref(j_out_message);
        ws_stream->is_authenticated = 0;
      }
      json_decref(j_access_token);
#else
      if (ws_stream->config->use_oauth2_authentication) {
        j_out_message = json_pack("{ssss}", "command", "authorization", "result", "invalid_request");
        message = json_dumps(j_out_message, JSON_COMPACT);
        if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
        }
        o_free(message);
        json_decref(j_out_message);
        ws_stream->is_authenticated = 0;
      } else {
        j_out_message = json_pack("{ssss}", "command", "authorization", "result", "connected");
        message = json_dumps(j_out_message, JSON_COMPACT);
        if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
        }
        o_free(message);
        json_decref(j_out_message);
        ws_stream->is_authenticated = 1;
        ws_stream->expiration = now + 3600;;
        if (ws_stream->username == NULL) {
          ws_stream->username = o_strdup(TALIESIN_NO_AUTHENTICATION_USERNAME);
        }
      }
#endif
    } else {
      j_out_message = json_pack("{ssss}", "command", "authorization", "result", "invalid_request");
      message = json_dumps(j_out_message, JSON_COMPACT);
      if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
      }
      o_free(message);
      json_decref(j_out_message);
      ws_stream->is_authenticated = 0;
    }
  } else if (ws_stream->is_authenticated || !ws_stream->config->use_oauth2_authentication) {
    if (ws_stream->expiration > now) {
      if (ws_stream->webradio != NULL) {
        j_is_valid = is_webradio_command_valid(ws_stream->config, ws_stream->webradio, j_message, ws_stream->username, ws_stream->is_admin);
      } else if (ws_stream->jukebox != NULL) {
        j_is_valid = is_jukebox_command_valid(ws_stream->config, ws_stream->jukebox, j_message, ws_stream->username, ws_stream->is_admin);
      }
      if (j_is_valid != NULL) {
        if (json_array_size(j_is_valid) == 0) {
          if (ws_stream->webradio != NULL) {
            j_result_command = webradio_command(ws_stream->config, ws_stream->webradio, ws_stream->username, j_message);
            if (!check_result_value(j_result_command, T_OK) && !check_result_value(j_result_command, T_ERROR_NOT_FOUND) && !check_result_value(j_result_command, T_ERROR_PARAM)) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error webradio_command");
            }
          } else if (ws_stream->jukebox != NULL) {
            j_result_command = jukebox_command(ws_stream->config, ws_stream->jukebox, ws_stream->username, j_message);
            if (!check_result_value(j_result_command, T_OK) && !check_result_value(j_result_command, T_ERROR_NOT_FOUND) && !check_result_value(j_result_command, T_ERROR_PARAM)) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error jukebox_command");
            }
          }
          if (check_result_value(j_result_command, T_OK)) {
            j_out_message = json_pack("{sssO}", "command", json_string_value(json_object_get(j_message, "command")), "result", (json_object_get(j_result_command, "command") != NULL)?json_object_get(j_result_command, "command"):json_true());
            message = json_dumps(j_out_message, JSON_COMPACT);
            if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
            }
            o_free(message);
            json_decref(j_out_message);
          } else if (check_result_value(j_result_command, T_ERROR_NOT_FOUND)) {
            j_out_message = json_pack("{ssss}", "command", json_string_value(json_object_get(j_message, "command")), "result", "not_found");
            message = json_dumps(j_out_message, JSON_COMPACT);
            if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
            }
            o_free(message);
            json_decref(j_out_message);
          } else if (check_result_value(j_result_command, T_ERROR_PARAM)) {
            j_out_message = json_pack("{ssss}", "command", json_string_value(json_object_get(j_message, "command")), "result", "error_param");
            message = json_dumps(j_out_message, JSON_COMPACT);
            if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
            }
            o_free(message);
            json_decref(j_out_message);
          } else {
            j_out_message = json_pack("{ssss}", "command", json_string_value(json_object_get(j_message, "command")), "result", "error");
            message = json_dumps(j_out_message, JSON_COMPACT);
            if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
            }
            o_free(message);
            json_decref(j_out_message);
          }
          json_decref(j_result_command);
        } else {
          j_out_message = json_pack("{sssssO}", "command", json_string_value(json_object_get(j_message, "command")), "result", "invalid_param", "error", j_is_valid);
          message = json_dumps(j_out_message, JSON_COMPACT);
          if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
          }
          o_free(message);
          json_decref(j_out_message);
        }
        json_decref(j_is_valid);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error is_webradio_command_valid");
      }
    } else {
      j_out_message = json_pack("{ssss}", "command", json_string_value(json_object_get(j_message, "command")), "result", "token_expired");
      message = json_dumps(j_out_message, JSON_COMPACT);
      if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
      }
      o_free(message);
      json_decref(j_out_message);
    }
  } else {
    j_out_message = json_pack("{ssss}", "command", json_string_value(json_object_get(j_message, "command")), "result", "not_authenticated");
    message = json_dumps(j_out_message, JSON_COMPACT);
    if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_websocket_stream_incoming_message - Error ulfius_websocket_send_message");
    }
    o_free(message);
    json_decref(j_out_message);
  }
  json_decref(j_message);
}
#endif

int callback_taliesin_stream_cover (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  struct _t_webradio * current_webradio = NULL;
  struct _t_jukebox * current_playlist = NULL;
  struct _t_file * file;
  unsigned int playlist_index;
  json_t * j_cover;
  unsigned char * cover_decoded;
  const char * cover_b64;
  size_t cover_decoded_len, cover_b64_len;
  int i;
  
  for (i=0; i<config->nb_webradio; i++) {
    if (0 == o_strcmp(u_map_get(request->map_url, "stream_name"), config->webradio_set[i]->name)) {
      current_webradio = config->webradio_set[i];
      break;
    }
  }
  if (current_webradio == NULL) {
    for (i=0; i<config->nb_jukebox; i++) {
      if (0 == o_strcmp(u_map_get(request->map_url, "stream_name"), config->jukebox_set[i]->name)) {
        current_playlist = config->jukebox_set[i];
        break;
      }
    }
  }
  
  if (u_map_get_case(request->map_url, "index") == NULL) {
    playlist_index = 0;
  } else {
    playlist_index = strtol(u_map_get_case(request->map_url, "index"), NULL, 10);
  }
  
  if (current_webradio != NULL &&
      current_webradio->audio_stream != NULL &&
      current_webradio->audio_stream->first_buffer != NULL &&
      current_webradio->audio_stream->first_buffer->file != NULL) {
    j_cover = media_cover_get_by_id(config, current_webradio->audio_stream->first_buffer->file->tm_id, (u_map_get(request->map_url, "thumbnail")!=NULL));
    if (check_result_value(j_cover, T_OK)) {
      if (u_map_get(request->map_url, "thumbnail") != NULL) {
        cover_b64 = json_string_value(json_object_get(json_object_get(j_cover, "cover"), "thumbnail"));
      } else {
        cover_b64 = json_string_value(json_object_get(json_object_get(j_cover, "cover"), "full"));
      }
      if (u_map_get(request->map_url, "base64")!=NULL) {
        ulfius_set_string_body_response(response, 200, cover_b64);
      } else {
        cover_b64_len = o_strlen(cover_b64);
        cover_decoded = o_malloc(cover_b64_len);
        if (o_base64_decode((const unsigned char *)cover_b64, cover_b64_len, cover_decoded, &cover_decoded_len)) {
          ulfius_set_binary_body_response(response, 200, (const char *)cover_decoded, cover_decoded_len);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_cover - Error decoding cover");
          response->status = 500;
        }
        o_free(cover_decoded);
      }
    } else if (check_result_value(j_cover, T_ERROR_NOT_FOUND)) {
      response->status = 404;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_cover - Error getting cover");
      response->status = 500;
    }
    json_decref(j_cover);
  } else if (current_playlist != NULL && (file = file_list_get_file(current_playlist->file_list, playlist_index)) != NULL) {
    j_cover = media_cover_get_by_id(config, file->tm_id, (u_map_get(request->map_url, "thumbnail")!=NULL));
    if (check_result_value(j_cover, T_OK)) {
      if (u_map_get(request->map_url, "thumbnail") != NULL) {
        cover_b64 = json_string_value(json_object_get(json_object_get(j_cover, "cover"), "thumbnail"));
      } else {
        cover_b64 = json_string_value(json_object_get(json_object_get(j_cover, "cover"), "full"));
      }
      if (u_map_get(request->map_url, "base64")!=NULL) {
        ulfius_set_string_body_response(response, 200, cover_b64);
      } else {
        cover_b64_len = o_strlen(cover_b64);
        cover_decoded = o_malloc(cover_b64_len);
        if (o_base64_decode((const unsigned char *)cover_b64, cover_b64_len, cover_decoded, &cover_decoded_len)) {
          ulfius_set_binary_body_response(response, 200, (const char *)cover_decoded, cover_decoded_len);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_cover - Error decoding cover");
          response->status = 500;
        }
        o_free(cover_decoded);
      }
    } else if (check_result_value(j_cover, T_ERROR_NOT_FOUND)) {
      response->status = 404;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_stream_cover - Error getting cover");
      response->status = 500;
    }
    json_decref(j_cover);
  } else {
    response->status = 404;
  }
  if (response->shared_data != NULL) {
    json_decref((json_t *)response->shared_data);
  }
  return U_CALLBACK_COMPLETE;
}

/**
 * Playlist callbacks
 */
int callback_taliesin_playlist_list (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_result;
  int res = U_CALLBACK_CONTINUE;
  
  j_result = playlist_list(config, get_username(request, response, config));
  if (!check_result_value(j_result, T_OK)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_list - Error getting current user playlists");
    res = U_CALLBACK_ERROR;
  } else if (ulfius_set_json_body_response(response, 200, json_object_get(j_result, "playlist")) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_list - Error setting json response");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_result);
  return res;
}

int callback_taliesin_playlist_get (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_result;
  int res = U_CALLBACK_CONTINUE;
  long offset = u_map_get(request->map_url, "offset")!=NULL?strtol(u_map_get(request->map_url, "offset"), NULL, 10):0,
       limit = (u_map_get(request->map_url, "limit")!=NULL&&strtol(u_map_get(request->map_url, "limit"), NULL, 10)>0)?strtol(u_map_get(request->map_url, "limit"), NULL, 10):TALIESIN_MEDIA_LIMIT_DEFAULT;
  
  if (offset < 0) {
    offset = 0;
  }
  if (limit < 0) {
    limit = TALIESIN_MEDIA_LIMIT_DEFAULT;
  }
  j_result = playlist_get(config, get_username(request, response, config), u_map_get(request->map_url, "playlist"), 0, offset, limit);
  if (check_result_value(j_result, T_OK)) {
    json_object_del(json_object_get(j_result, "playlist"), "tpl_id");
    if (ulfius_set_json_body_response(response, 200, json_object_get(j_result, "playlist")) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_get - Error setting json response");
      res = U_CALLBACK_ERROR;
    }
  } else if (check_result_value(j_result, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_get - Error getting data source");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_result);
  return res;
}

int callback_taliesin_playlist_add (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_body = ulfius_get_json_body_request(request, NULL), * j_is_valid, * j_result;
  int res = U_CALLBACK_CONTINUE;
  json_int_t tpl_id;
  
  j_is_valid = is_playlist_valid(config, get_username(request, response, config), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin), j_body, 0, 1);
  if (j_is_valid != NULL) {
    if (json_array_size(j_is_valid) == 0) {
      j_result = media_append_list_to_media_list(config, json_object_get(j_body, "media"), get_username(request, response, config));
      if (check_result_value(j_result, T_OK)) {
        json_object_del(j_body, "media");
        if ((tpl_id = playlist_add(config, get_username(request, response, config), j_body, NULL)) == -1) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add - Error playlist_add");
          res = U_CALLBACK_ERROR;
        } else {
          if (playlist_add_media(config, tpl_id, json_object_get(j_result, "media")) != T_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add - Error playlist_add_media");
            res = U_CALLBACK_ERROR;
          }
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add - Error media_append_list_to_media_list");
        res = U_CALLBACK_ERROR;
      }
      json_decref(j_result);
    } else {
      if (ulfius_set_json_body_response(response, 400, j_is_valid) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add - Error setting json response");
        res = U_CALLBACK_ERROR;
      }
    }
    json_decref(j_is_valid);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add - Error is_playlist_valid");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_body);
  return res;
}

int callback_taliesin_playlist_set (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_body = ulfius_get_json_body_request(request, NULL), * j_is_valid, * j_playlist;
  int res = U_CALLBACK_CONTINUE, is_admin = has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin);
  
  j_playlist = playlist_get(config, get_username(request, response, config), u_map_get(request->map_url, "playlist"), 1, 0, 1);
  if (check_result_value(j_playlist, T_OK)) {
    if (playlist_can_update(json_object_get(j_playlist, "playlist"), is_admin)) {
        j_is_valid = is_playlist_valid(config, get_username(request, response, config), is_admin, j_body, 1, 1);
        if (j_is_valid != NULL) {
          if (json_array_size(j_is_valid) == 0) {
            if (playlist_set(config, json_integer_value(json_object_get(json_object_get(j_playlist, "playlist"), "tpl_id")), j_body) != T_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_set - Error playlist_set");
              res = U_CALLBACK_ERROR;
            }
          } else {
            if (ulfius_set_json_body_response(response, 400, j_is_valid) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_set - Error setting json response");
              res = U_CALLBACK_ERROR;
            }
          }
          json_decref(j_is_valid);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_set - Error is_playlist_valid");
          res = U_CALLBACK_ERROR;
        }
    } else {
      if (set_response_json_body_and_clean(response, 403, json_pack("[{ss}]", "scope", "User can't update a playlist for all users, admin is required")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_set - Error setting json response");
        res = U_CALLBACK_ERROR;
      }
    }
  } else if (check_result_value(j_playlist, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_set - Error getting playlist");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_playlist);
  json_decref(j_body);
  
  return res;
}

int callback_taliesin_playlist_delete (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_playlist;
  int res = U_CALLBACK_CONTINUE, is_admin = has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin);
  
  j_playlist = playlist_get(config, get_username(request, response, config), u_map_get(request->map_url, "playlist"), 1, 0, 1);
  if (check_result_value(j_playlist, T_OK)) {
    if (playlist_can_update(json_object_get(j_playlist, "playlist"), is_admin)) {
      if (playlist_delete(config, json_integer_value(json_object_get(json_object_get(j_playlist, "playlist"), "tpl_id"))) != T_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_delete - Error playlist_delete");
        res = U_CALLBACK_ERROR;
      }
    } else {
      if (set_response_json_body_and_clean(response, 403, json_pack("[{ss}]", "scope", "User can't delete a playlist for all users, admin is required")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_delete - Error setting json response");
        res = U_CALLBACK_ERROR;
      }
    }
  } else if (check_result_value(j_playlist, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_delete - Error getting playlist");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_playlist);
  
  return res;
}

int callback_taliesin_playlist_add_media (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_playlist, * j_is_valid, * j_body = ulfius_get_json_body_request(request, NULL), * j_result;
  int res = U_CALLBACK_CONTINUE, is_admin = has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin);
  
  j_playlist = playlist_get(config, get_username(request, response, config), u_map_get(request->map_url, "playlist"), 1, 0, 1);
  if (check_result_value(j_playlist, T_OK)) {
    if (playlist_can_update(json_object_get(j_playlist, "playlist"), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin))) {
      j_is_valid = is_playlist_element_list_valid(config, is_admin, get_username(request, response, config), j_body);
      if (j_is_valid != NULL) {
        if (json_array_size(j_is_valid) == 0) {
          j_result = media_append_list_to_media_list(config, j_body, get_username(request, response, config));
          if (check_result_value(j_result, T_OK)) {
            if (json_array_size(json_object_get(j_result, "media")) > 0) {
              if (playlist_add_media(config, json_integer_value(json_object_get(json_object_get(j_playlist, "playlist"), "tpl_id")), json_object_get(j_result, "media")) != T_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error playlist_add_media");
                res = U_CALLBACK_ERROR;
              }
            } else {
              response->status = 404;
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error media_append_list_to_media_list");
            res = U_CALLBACK_ERROR;
          }
          json_decref(j_result);
        } else {
          if (ulfius_set_json_body_response(response, 400, j_is_valid) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error setting json response");
            res = U_CALLBACK_ERROR;
          }
        }
        json_decref(j_is_valid);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error is_playlist_valid");
        res = U_CALLBACK_ERROR;
      }
    } else {
      if (set_response_json_body_and_clean(response, 403, json_pack("[{ss}]", "scope", "User can't delete a playlist for all users, admin is required")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error setting json response");
        res = U_CALLBACK_ERROR;
      }
    }
  } else if (check_result_value(j_playlist, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error getting playlist");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_playlist);
  json_decref(j_body);
  
  return res;
}

int callback_taliesin_playlist_delete_media (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_playlist, * j_is_valid, * j_body = ulfius_get_json_body_request(request, NULL), * j_result;
  int res = U_CALLBACK_CONTINUE, is_admin = has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin);
  
  j_playlist = playlist_get(config, get_username(request, response, config), u_map_get(request->map_url, "playlist"), 1, 0, 1);
  if (check_result_value(j_playlist, T_OK)) {
    if (playlist_can_update(json_object_get(j_playlist, "playlist"), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin))) {
      j_is_valid = is_playlist_element_list_valid(config, is_admin, get_username(request, response, config), j_body);
      if (j_is_valid != NULL) {
        if (json_array_size(j_is_valid) == 0) {
          j_result = media_append_list_to_media_list(config, j_body, get_username(request, response, config));
          if (check_result_value(j_result, T_OK) && json_array_size(json_object_get(j_result, "media")) > 0) {
            if (playlist_delete_media(config, json_integer_value(json_object_get(json_object_get(j_playlist, "playlist"), "tpl_id")), json_object_get(j_result, "media")) != T_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error playlist_add_media");
              res = U_CALLBACK_ERROR;
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error media_append_list_to_media_list");
            res = U_CALLBACK_ERROR;
          }
          json_decref(j_result);
        } else {
          if (ulfius_set_json_body_response(response, 400, j_is_valid) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error setting json response");
            res = U_CALLBACK_ERROR;
          }
        }
        json_decref(j_is_valid);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error is_playlist_element_list_valid");
        res = U_CALLBACK_ERROR;
      }
    } else {
      if (set_response_json_body_and_clean(response, 403, json_pack("[{ss}]", "scope", "User can't delete a playlist for all users, admin is required")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error setting json response");
        res = U_CALLBACK_ERROR;
      }
    }
  } else if (check_result_value(j_playlist, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_add_media - Error getting playlist");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_playlist);
  json_decref(j_body);
  
  return res;
}

int callback_taliesin_playlist_has_media (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_playlist, * j_is_valid, * j_body = ulfius_get_json_body_request(request, NULL), * j_result, * j_media_list;
  int res = U_CALLBACK_CONTINUE, is_admin = has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin);
  size_t offset = u_map_get(request->map_url, "offset")!=NULL?strtol(u_map_get(request->map_url, "offset"), NULL, 10):0,
         limit = (u_map_get(request->map_url, "limit")!=NULL&&strtol(u_map_get(request->map_url, "limit"), NULL, 10)>0)?strtol(u_map_get(request->map_url, "limit"), NULL, 10):TALIESIN_MEDIA_LIMIT_DEFAULT;
  
  j_playlist = playlist_get(config, get_username(request, response, config), u_map_get(request->map_url, "playlist"), 1, 0, 1);
  if (check_result_value(j_playlist, T_OK)) {
    if (playlist_can_update(json_object_get(j_playlist, "playlist"), has_scope(config, json_object_get((json_t *)response->shared_data, "scope"), config->oauth_scope_admin))) {
      j_is_valid = is_playlist_element_list_valid(config, is_admin, get_username(request, response, config), j_body);
      if (j_is_valid != NULL) {
        if (json_array_size(j_is_valid) == 0) {
          j_result = media_append_list_to_media_list(config, j_body, get_username(request, response, config));
          if (check_result_value(j_result, T_OK) && json_array_size(json_object_get(j_result, "media")) > 0) {
            j_media_list = playlist_has_media(config, json_integer_value(json_object_get(json_object_get(j_playlist, "playlist"), "tpl_id")), json_object_get(j_result, "media"), offset, limit);
            if (check_result_value(j_media_list, T_OK)) {
              if (json_array_size(json_object_get(j_media_list, "media")) > 0) {
                if (ulfius_set_json_body_response(response, 200, json_object_get(j_media_list, "media")) != U_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_has_media - Error setting json response");
                  res = U_CALLBACK_ERROR;
                }
              } else {
                response->status = 404;
              }
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_has_media - Error playlist_add_media");
              res = U_CALLBACK_ERROR;
            }
            json_decref(j_media_list);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_has_media - Error media_append_list_to_media_list");
            res = U_CALLBACK_ERROR;
          }
          json_decref(j_result);
        } else {
          if (ulfius_set_json_body_response(response, 400, j_is_valid) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_has_media - Error setting json response");
            res = U_CALLBACK_ERROR;
          }
        }
        json_decref(j_is_valid);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_has_media - Error is_playlist_element_list_valid");
        res = U_CALLBACK_ERROR;
      }
    } else {
      if (set_response_json_body_and_clean(response, 403, json_pack("[{ss}]", "scope", "User can't delete a playlist for all users, admin is required")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_has_media - Error setting json response");
        res = U_CALLBACK_ERROR;
      }
    }
  } else if (check_result_value(j_playlist, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_has_media - Error getting playlist");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_playlist);
  json_decref(j_body);
  
  return res;
}

int callback_taliesin_playlist_load (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_playlist, * j_stream_info, * j_valid;
  int res = U_CALLBACK_CONTINUE;
  const char * format;
  unsigned short int channels;
  unsigned int sample_rate, bit_rate;
  struct _t_webradio * webradio;
  int ret_thread_webradio = 0, detach_thread_webradio = 0;
  pthread_t thread_webradio;
  
  j_playlist = playlist_get(config, get_username(request, response, config), u_map_get(request->map_url, "playlist"), 1, 0, 0);
  if (check_result_value(j_playlist, T_OK)) {
    if (json_array_size(json_object_get(json_object_get(j_playlist, "playlist"), "media"))) {
      format = u_map_get(request->map_url, "format");
      channels = u_map_get(request->map_url, "channels")!=NULL?strtol(u_map_get(request->map_url, "channels"), NULL, 10):TALIESIN_STREAM_DEFAULT_CHANNELS;
      sample_rate = u_map_get(request->map_url, "samplerate")!=NULL?strtol(u_map_get(request->map_url, "samplerate"), NULL, 10):TALIESIN_STREAM_DEFAULT_SAMPLE_RATE;
      if (0 == o_strcmp(format, "flac")) {
        bit_rate = TALIESIN_STREAM_FLAC_BIT_RATE;
      } else {
        bit_rate = u_map_get(request->map_url, "bitrate")!=NULL?strtol(u_map_get(request->map_url, "bitrate"), NULL, 10):TALIESIN_STREAM_DEFAULT_BIT_RATE;
      }
      
      if (format == NULL) {
        format = TALIESIN_STREAM_DEFAULT_FORMAT;
      }
      if (!channels) {
        sample_rate = TALIESIN_STREAM_DEFAULT_CHANNELS;
      }
      if (!sample_rate) {
        channels = TALIESIN_STREAM_DEFAULT_SAMPLE_RATE;
      }
      if (!bit_rate) {
        bit_rate = TALIESIN_STREAM_DEFAULT_BIT_RATE;
      }
      j_valid = is_stream_parameters_valid((u_map_get(request->map_url, "webradio") != NULL), format, channels, sample_rate, bit_rate);
      if (j_valid != NULL && json_array_size(j_valid) == 0) {
        if (u_map_get(request->map_url, "webradio") != NULL) {
          format = "mp3";
          j_stream_info = add_webradio_from_playlist(config, json_object_get(j_playlist, "playlist"), get_username(request, response, config), format, channels, sample_rate, bit_rate, (u_map_get(request->map_url, "random")!=NULL), u_map_get(request->map_url, "name"), &webradio);
          if (check_result_value(j_stream_info, T_OK)) {
            ret_thread_webradio = pthread_create(&thread_webradio, NULL, webradio_run_thread, (void *)webradio);
            detach_thread_webradio = pthread_detach(thread_webradio);
            if (ret_thread_webradio || detach_thread_webradio) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Error running thread webradio");
              response->status = 500;
            } else {
              ulfius_set_json_body_response(response, 200, json_object_get(j_stream_info, "stream"));
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error streaming file");
            response->status = 500;
          }
          json_decref(j_stream_info);
        } else {
          j_stream_info = add_jukebox_from_playlist(config, json_object_get(j_playlist, "playlist"), get_username(request, response, config), format, channels, sample_rate, bit_rate, u_map_get(request->map_url, "name"));
          if (check_result_value(j_stream_info, T_OK)) {
            ulfius_set_json_body_response(response, 200, json_object_get(j_stream_info, "stream"));
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error creating playlist");
            response->status = 500;
          }
          json_decref(j_stream_info);
        }
      } else if (j_valid != NULL && json_array_size(j_valid) > 0) {
        ulfius_set_json_body_response(response, 400, j_valid);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_media_get_path - Error is_stream_parameters_valid");
        response->status = 500;
      }
      json_decref(j_valid);
    } else {
      j_valid = json_pack("{ss}", "error", "playlist has no media");
      ulfius_set_json_body_response(response, 400, j_valid);
      json_decref(j_valid);
    }
  } else if (check_result_value(j_playlist, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_get - Error getting data source");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_playlist);
  return res;
}

int callback_taliesin_playlist_export (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_result;
  int res = U_CALLBACK_CONTINUE;
  char * escaped_filename, * content_disposition;

  j_result = playlist_get(config, get_username(request, response, config), u_map_get(request->map_url, "playlist"), 0, 0, 0);
  if (check_result_value(j_result, T_OK)) {
    escaped_filename = url_encode(json_string_value(json_object_get(json_object_get(j_result, "playlist"), "name")));
    content_disposition = msprintf("attachment; filename=%s.m3u", escaped_filename);
    u_map_put(response->map_header, "Content-Type", "application/json");
    u_map_put(response->map_header, "Content-Disposition", content_disposition);
    if (ulfius_set_json_body_response(response, 200, json_object_get(json_object_get(j_result, "playlist"), "media")) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_get - Error setting json response");
      res = U_CALLBACK_ERROR;
    }
    o_free(escaped_filename);
    o_free(content_disposition);
  } else if (check_result_value(j_result, T_ERROR_NOT_FOUND)) {
    response->status = 404;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_playlist_get - Error getting data source");
    res = U_CALLBACK_ERROR;
  }
  json_decref(j_result);
  return res;
}

/**
 * Search callbacks
 */
int callback_taliesin_search (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  unsigned short int category = TALIESIN_SEARCH_CATEGORY_NONE;
  json_t * j_result;
  
  if (u_map_get_case(request->map_url, "q") != NULL) {
    if (u_map_get_case(request->map_url, "category") != NULL) {
      if (0 == o_strcmp("playlist", u_map_get_case(request->map_url, "category"))) {
        category = TALIESIN_SEARCH_CATEGORY_PLAYLIST;
      } else if (0 == o_strcmp("stream", u_map_get_case(request->map_url, "category"))) {
        category = TALIESIN_SEARCH_CATEGORY_STREAM;
      } else if (0 == o_strcmp("folder", u_map_get_case(request->map_url, "category"))) {
        category = TALIESIN_SEARCH_CATEGORY_FOLDER;
      } else if (0 == o_strcmp("file", u_map_get_case(request->map_url, "category"))) {
        category = TALIESIN_SEARCH_CATEGORY_FILE;
      } else if (0 == o_strcmp("title", u_map_get_case(request->map_url, "category"))) {
        category = TALIESIN_SEARCH_CATEGORY_TITLE;
      } else if (0 == o_strcmp("artist", u_map_get_case(request->map_url, "category"))) {
        category = TALIESIN_SEARCH_CATEGORY_ARTIST;
      } else if (0 == o_strcmp("album", u_map_get_case(request->map_url, "category"))) {
        category = TALIESIN_SEARCH_CATEGORY_ALBUM;
      } else if (0 == o_strcmp("year", u_map_get_case(request->map_url, "category"))) {
        category = TALIESIN_SEARCH_CATEGORY_YEAR;
      } else if (0 == o_strcmp("genre", u_map_get_case(request->map_url, "category"))) {
        category = TALIESIN_SEARCH_CATEGORY_GENRE;
      } else if (u_map_get_case(request->map_url, "category") != NULL) {
        category = TALIESIN_SEARCH_CATEGORY_INVALID;
      }
    }
    if (category != TALIESIN_SEARCH_CATEGORY_INVALID) {
      j_result = media_simple_search(config, get_username(request, response, config), u_map_get_case(request->map_url, "q"), category);
      if (check_result_value(j_result, T_OK)) {
        ulfius_set_json_body_response(response, 200, json_object_get(j_result, "search"));
      } else {
        response->status = 500;
      }
      json_decref(j_result);
    } else {
      response->status = 400;
    }
  } else {
    j_result = json_pack("{ssss}", "result", "error", "reason", "You must provide at least a search pattern in the url");
    ulfius_set_json_body_response(response, 400, j_result);
    json_decref(j_result);
  }
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_advanced_search (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_is_valid, * j_result = NULL, * j_body;
  
  j_body = ulfius_get_json_body_request(request, NULL);
  j_is_valid = is_valid_media_advanced_search(config, get_username(request, response, config), j_body);
  if (j_is_valid != NULL && json_array_size(j_is_valid) == 0) {
    j_result = media_advanced_search(config, get_username(request, response, config), j_body);
    if (check_result_value(j_result, T_OK)) {
      ulfius_set_json_body_response(response, 200, json_object_get(j_result, "list"));
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_advanced_search - Error media_advanced_search");
      response->status = 500;
    }
    json_decref(j_result);
  } else if (j_is_valid != NULL && json_array_size(j_is_valid) > 0) {
    ulfius_set_json_body_response(response, 400, j_is_valid);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_advanced_search - Error is_valid_media_advanced_search");
    response->status = 500;
  }
  json_decref(j_is_valid);
  json_decref(j_body);
  return U_CALLBACK_CONTINUE;
}

/**
 * Config callbacks
 */
int callback_taliesin_username_get_list (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_result = username_get_list(config);
  
  if (check_result_value(j_result, T_OK)) {
    if (ulfius_set_json_body_response(response, 200, json_object_get(j_result, "username")) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_username_get_list - Error ulfius_set_json_body_response");
      response->status = 500;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_username_get_list - Error username_get_list");
    response->status = 500;
  }
  json_decref(j_result);
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_config_type_get (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_result = NULL;
  
  if (0 == o_strcmp(TALIESIN_CONFIG_AUDIO_FILE_EXTENSION, u_map_get_case(request->map_url, "type")) ||
      0 == o_strcmp(TALIESIN_CONFIG_VIDEO_FILE_EXTENSION, u_map_get_case(request->map_url, "type")) ||
      0 == o_strcmp(TALIESIN_CONFIG_SUBTITLE_FILE_EXTENSION, u_map_get_case(request->map_url, "type")) ||
      0 == o_strcmp(TALIESIN_CONFIG_IMAGE_FILE_EXTENSION, u_map_get_case(request->map_url, "type")) ||
      0 == o_strcmp(TALIESIN_CONFIG_COVER_FILE_PATTERN, u_map_get_case(request->map_url, "type")) ||
      0 == o_strcmp(TALIESIN_CONFIG_EXTERNAL_PLAYER, u_map_get_case(request->map_url, "type"))) {
    j_result = config_get_values(config, u_map_get_case(request->map_url, "type"));
    if (check_result_value(j_result, T_OK)) {
      if (ulfius_set_json_body_response(response, 200, json_object_get(j_result, "config")) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_config_type_get - Error ulfius_set_json_body_response");
        response->status = 500;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_config_type_get - Error config_get_values");
      response->status = 500;
    }
    json_decref(j_result);
  } else {
    response->status = 404;
  }
  return U_CALLBACK_CONTINUE;
}

int callback_taliesin_config_type_set (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct config_elements * config = (struct config_elements *)user_data;
  json_t * j_is_valid, * j_body;
  
  if (0 == o_strcmp(TALIESIN_CONFIG_AUDIO_FILE_EXTENSION, u_map_get_case(request->map_url, "type")) ||
      0 == o_strcmp(TALIESIN_CONFIG_VIDEO_FILE_EXTENSION, u_map_get_case(request->map_url, "type")) ||
      0 == o_strcmp(TALIESIN_CONFIG_SUBTITLE_FILE_EXTENSION, u_map_get_case(request->map_url, "type")) ||
      0 == o_strcmp(TALIESIN_CONFIG_IMAGE_FILE_EXTENSION, u_map_get_case(request->map_url, "type")) ||
      0 == o_strcmp(TALIESIN_CONFIG_COVER_FILE_PATTERN, u_map_get_case(request->map_url, "type")) ||
      0 == o_strcmp(TALIESIN_CONFIG_EXTERNAL_PLAYER, u_map_get_case(request->map_url, "type"))) {
    j_body = ulfius_get_json_body_request(request, NULL);
    j_is_valid = is_valid_config_elements(j_body);
    if (j_is_valid != NULL && json_array_size(j_is_valid) == 0) {
      if (config_set_values(config, u_map_get_case(request->map_url, "type"), j_body) != T_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_config_type_set - Error config_set_values");
        response->status = 500;
      }
    } else if (j_is_valid != NULL && json_array_size(j_is_valid) > 0) {
      ulfius_set_json_body_response(response, 400, j_is_valid);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "callback_taliesin_config_type_set - Error is_valid_config_elements");
      response->status = 500;
    }
    json_decref(j_is_valid);
    json_decref(j_body);
  } else {
    response->status = 404;
  }
  return U_CALLBACK_CONTINUE;
}
