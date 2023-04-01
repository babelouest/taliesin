/**
 *
 * Taliesin - Media server
 *
 * Main file
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

#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <libconfig.h>
#include <string.h>
#include <libavformat/avformat.h>

#include "taliesin.h"

/**
 * global variables to store close condition and mutex
 */
pthread_mutex_t global_handler_close_lock;
pthread_cond_t  global_handler_close_cond;

#ifndef DISABLE_OAUTH2

static char * read_file(const char * filename, size_t * filesize) {
  char * buffer = NULL;
  size_t length;
  FILE * f;
  if (filename != NULL) {
    f = fopen (filename, "rb");
    if (f) {
      fseek (f, 0, SEEK_END);
      length = (size_t)ftell (f);
      if (filesize != NULL) {
        *filesize = length;
      }
      fseek (f, 0, SEEK_SET);
      buffer = o_malloc (length + 1);
      if (buffer != NULL) {
        if (fread (buffer, 1, length, f)) {
          buffer[length] = '\0';
        }
      }
      fclose (f);
    }
    return buffer;
  } else {
    return NULL;
  }
}
#endif

/**
 *
 * Main function
 *
 * Initialize config structure, parse the arguments and the config file
 * Then run the webservice
 *
 */
int main (int argc, char ** argv) {
  struct config_elements * config = o_malloc(sizeof(struct config_elements));
  int res;
  pthread_mutexattr_t mutexattr;
  json_t * j_stream_list, * j_element;
  size_t index, i;
  int ret_thread_webradio = 0, detach_thread_webradio = 0;
  int ret_thread_icecast = 0, detach_thread_icecast = 0;
  pthread_t thread_webradio, thread_icecast;
  struct _t_webradio * webradio;
#ifndef DISABLE_OAUTH2
  char * str_jwks;
  json_t * j_jwks;
#endif

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
  av_register_all();
#endif
  shout_init();

  srand((unsigned int)time(NULL));
  if (config == NULL) {
    fprintf(stderr, "Memory error - config\n");
    return 1;
  }

  // Init config structure with default values
  config->config_file = NULL;
  config->status = TALIESIN_RUNNING;
  config->api_prefix = NULL;
  config->log_mode = Y_LOG_MODE_NONE;
  config->log_level = Y_LOG_LEVEL_NONE;
  config->log_file = NULL;
  config->server_remote_address = NULL;
  config->conn = NULL;
  config->instance = o_malloc(sizeof(struct _u_instance));
  if (config->instance == NULL) {
    fprintf(stderr, "Memory error - instance\n");
    return 1;
  }
  config->instance->port = TALIESIN_DEFAULT_PORT;
  config->allow_origin = NULL;
  config->use_oidc_authentication = 0;
#ifndef DISABLE_OAUTH2
  config->iddawc_resource_config = o_malloc(sizeof(struct _iddawc_resource_config));
  config->iddawc_resource_config_admin = o_malloc(sizeof(struct _iddawc_resource_config));
  config->oidc_server_remote_config = NULL;
  config->oidc_server_remote_config_verify_cert = 1;
  config->oidc_server_public_jwks = NULL;
  config->oidc_realm = NULL;
  config->oidc_iss = NULL;
  config->oidc_aud = NULL;
  config->oidc_dpop_max_iat = 0;
  config->oidc_configured = 0;
#endif
  config->static_file_config = o_malloc(sizeof(struct _u_compressed_inmemory_website_config));
  config->use_secure_connection = 0;
  config->secure_connection_key_file = NULL;
  config->secure_connection_pem_file = NULL;
  config->oauth_scope_user = NULL;
  config->oauth_scope_admin = NULL;
  config->nb_webradio = 0;
  config->webradio_set = NULL;
  config->nb_jukebox  = 0;
  config->jukebox_set = NULL;
  config->nb_refresh_status = 0;
  config->refresh_status_list = NULL;
  config->user_can_create_data_source = 0;
  config->audio_file_extension = NULL;
  config->video_file_extension = NULL;
  config->subtitle_file_extension = NULL;
  config->image_file_extension = NULL;
  config->cover_file_pattern = NULL;
  config->external_player = NULL;

  pthread_mutexattr_init ( &mutexattr );
  pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
  if (pthread_mutex_init(&config->playlist_lock, &mutexattr)) {
    fprintf(stderr, "Error setting playlist_lock\n");
    return 1;
  }
  config->stream_format = o_strdup(TALIESIN_STREAM_DEFAULT_FORMAT);
  config->stream_channels = TALIESIN_STREAM_DEFAULT_CHANNELS;
  config->stream_sample_rate = TALIESIN_STREAM_DEFAULT_SAMPLE_RATE;
  config->stream_bitrate = TALIESIN_STREAM_DEFAULT_BIT_RATE;
#ifndef DISABLE_OAUTH2
  if (config->instance == NULL || config->iddawc_resource_config == NULL || config->iddawc_resource_config_admin == NULL || config->static_file_config == NULL)
#else
  if (config->instance == NULL || config->static_file_config == NULL)
#endif
  {
    fprintf(stderr, "Memory error - config->instance || config->iddawc_resource_config || config->iddawc_resource_config_admin || config->static_file_config\n");
    return 1;
  }
  if (pthread_mutex_init(&config->stream_stop_lock, NULL) ||
      pthread_cond_init(&config->stream_stop_cond, NULL)) {
    fprintf(stderr, "Error init stream_stop_lock or stream_stop_cond\n");
    return 1;
  }
  if (pthread_mutex_init(&config->refresh_lock, NULL) ||
      pthread_cond_init(&config->refresh_cond, NULL)) {
    fprintf(stderr, "Error init refresh_lock or refresh_cond\n");
    return 1;
  }
  i_global_init();
  ulfius_init_instance(config->instance, TALIESIN_DEFAULT_PORT, NULL, NULL);
  config->timeout = TALIESIN_DEFAULT_HTTP_TIMEOUT;
  config->icecast_host = NULL;
  config->icecast_port = 0;
  config->icecast_user = NULL;
  config->icecast_password = NULL;
  config->icecast_mount_prefix = NULL;
  config->icecast_remote_address = NULL;

  if (u_init_compressed_inmemory_website_config(config->static_file_config) != U_OK) {
    fprintf(stderr, "Error u_init_compressed_inmemory_website_config\n");
    return 1;
  }
  u_map_put(&config->static_file_config->mime_types, "*", "application/octet-stream");
  u_map_put(&config->static_file_config->map_header, "Cache-Control", "public,max-age=31536000,immutable");

  if (pthread_mutex_init(&global_handler_close_lock, NULL) ||
      pthread_cond_init(&global_handler_close_cond, NULL)) {
    fprintf(stderr, "init - Error initializing global_handler_close_lock or global_handler_close_cond\n");
  }
  // Catch end signals to make a clean exit
  signal (SIGQUIT, exit_handler);
  signal (SIGINT, exit_handler);
  signal (SIGTERM, exit_handler);
  signal (SIGHUP, exit_handler);

  // First we parse command line arguments
  if (!build_config_from_args(argc, argv, config)) {
    fprintf(stderr, "Error reading command-line parameters\n");
    print_help(stderr);
    exit_server(&config, TALIESIN_ERROR);
  }

  // Then we parse configuration file
  // They have lower priority than command line parameters
  if (!build_config_from_file(config)) {
    fprintf(stderr, "Error config file\n");
    exit_server(&config, TALIESIN_ERROR);
  }

  // Check if all mandatory configuration variables are present and correctly typed
  if (!check_config(config)) {
    fprintf(stderr, "Error initializing configuration\n");
    exit_server(&config, TALIESIN_ERROR);
  }

  if (load_config_values(config) != T_OK) {
    fprintf(stderr, "Error load_config_values\n");
    exit_server(&config, TALIESIN_ERROR);
  }

#ifndef DISABLE_OAUTH2
  if (config->use_oidc_authentication) {
    y_log_message(Y_LOG_LEVEL_INFO, "OIDC Authentication: enabled");
    if (i_jwt_profile_access_token_init_config(config->iddawc_resource_config, I_METHOD_HEADER, NULL, NULL, config->oauth_scope_user, config->server_remote_address, config->oidc_dpop_max_iat) != I_TOKEN_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error i_jwt_profile_access_token_init_config");
      exit_server(&config, TALIESIN_ERROR);
    } else {
      config->oidc_configured = 1;
      if (config->oidc_server_remote_config != NULL) {
        if (!i_jwt_profile_access_token_load_config(config->iddawc_resource_config, config->oidc_server_remote_config, config->oidc_server_remote_config_verify_cert)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error i_jwt_profile_access_token_load_config");
          exit_server(&config, TALIESIN_ERROR);
        }
        y_log_message(Y_LOG_LEVEL_INFO, "Load remote authentification config: %s", config->oidc_server_remote_config);
      } else if (config->oidc_server_public_jwks != NULL) {
        res = 1;
        if ((str_jwks = read_file(config->oidc_server_public_jwks, NULL)) != NULL) {
          if ((j_jwks = json_loads(str_jwks, JSON_DECODE_ANY, NULL)) != NULL) {
            if (!i_jwt_profile_access_token_load_jwks(config->iddawc_resource_config, j_jwks, config->oidc_iss)) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Error i_jwt_profile_access_token_load_jwks");
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "Error parsing jwks");
            res = 0;
          }
          json_decref(j_jwks);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error reading jwks file");
          res = 0;
        }
        o_free(str_jwks);
        if (!res) {
          exit_server(&config, TALIESIN_ERROR);
        }
        y_log_message(Y_LOG_LEVEL_INFO, "Load signature key from file: %s", config->oidc_server_public_jwks);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error oidc config");
        exit_server(&config, TALIESIN_ERROR);
      }
    }
    if (i_jwt_profile_access_token_init_config(config->iddawc_resource_config_admin, I_METHOD_HEADER, NULL, NULL, config->oauth_scope_admin, config->server_remote_address, config->oidc_dpop_max_iat) != I_TOKEN_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error i_jwt_profile_access_token_init_config");
      exit_server(&config, TALIESIN_ERROR);
    } else {
      config->oidc_configured = 1;
      if (config->oidc_server_remote_config != NULL) {
        if (!i_jwt_profile_access_token_load_config(config->iddawc_resource_config_admin, config->oidc_server_remote_config, config->oidc_server_remote_config_verify_cert)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error i_jwt_profile_access_token_load_config");
          exit_server(&config, TALIESIN_ERROR);
        }
        y_log_message(Y_LOG_LEVEL_INFO, "Load remote authentification config: %s", config->oidc_server_remote_config);
      } else if (config->oidc_server_public_jwks != NULL) {
        res = 1;
        if ((str_jwks = read_file(config->oidc_server_public_jwks, NULL)) != NULL) {
          if ((j_jwks = json_loads(str_jwks, JSON_DECODE_ANY, NULL)) != NULL) {
            if (!i_jwt_profile_access_token_load_jwks(config->iddawc_resource_config_admin, j_jwks, config->oidc_iss)) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Error i_jwt_profile_access_token_load_jwks");
            }
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "Error parsing jwks");
            res = 0;
          }
          json_decref(j_jwks);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error reading jwks file");
          res = 0;
        }
        o_free(str_jwks);
        if (!res) {
          exit_server(&config, TALIESIN_ERROR);
        }
        y_log_message(Y_LOG_LEVEL_INFO, "Load signature key from file: %s", config->oidc_server_public_jwks);
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error oidc config");
        exit_server(&config, TALIESIN_ERROR);
      }
    }
  } else {
    y_log_message(Y_LOG_LEVEL_INFO, "OIDC Authentication: disabled");
  }
#endif

  // TODO remove instance timeout when MHD will support a timeout on a streaming response
  config->instance->timeout = (unsigned int)config->timeout;

  av_log_set_callback(&redirect_libav_logs);

  // At this point, we declare all API endpoints and configure

  // Data source endpoints
  ulfius_add_endpoint_by_val(config->instance, "*", config->api_prefix, "/data_source/*", TALIESIN_CALLBACK_PRIORITY_AUTHENTICATION, &callback_taliesin_check_access, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/data_source/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_data_source_list, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/data_source/:data_source", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_data_source_get, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "POST", config->api_prefix, "/data_source/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_data_source_add, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "POST", config->api_prefix, "/data_source/:data_source", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_default, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "PUT", config->api_prefix, "/data_source/:data_source/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_data_source_set, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "DELETE", config->api_prefix, "/data_source/:data_source/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_data_source_delete, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "PUT", config->api_prefix, "/data_source/:data_source/refresh/*", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_data_source_refresh_run, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/data_source/:data_source/refresh", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_data_source_refresh_status, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "DELETE", config->api_prefix, "/data_source/:data_source/refresh", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_data_source_refresh_stop, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "POST", config->api_prefix, "/data_source/:data_source/clean", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_data_source_clean, (void*)config);

  // Browse media endpoints
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/data_source/:data_source/browse/path/*", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_media_get_path, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/data_source/:data_source/browse/category/:level", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_category_get, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/data_source/:data_source/browse/category/:level/:category/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_category_list, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/data_source/:data_source/info/category/:level/:category", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_category_get_info, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "PUT", config->api_prefix, "/data_source/:data_source/info/category/:level/:category", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_category_set_info, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "DELETE", config->api_prefix, "/data_source/:data_source/info/category/:level/:category", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_category_delete_info, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/data_source/:data_source/browse/category/:level/:category/:sublevel/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_subcategory_get, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/data_source/:data_source/browse/category/:level/:category/:sublevel/:subcategory", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_subcategory_list, (void*)config);

  // Playlist endpoints
  ulfius_add_endpoint_by_val(config->instance, "*", config->api_prefix, "/playlist/*", TALIESIN_CALLBACK_PRIORITY_AUTHENTICATION, &callback_taliesin_check_access, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/playlist/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_playlist_list, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/playlist/:playlist", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_playlist_get, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "POST", config->api_prefix, "/playlist/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_playlist_add, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "PUT", config->api_prefix, "/playlist/:playlist/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_playlist_set, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "DELETE", config->api_prefix, "/playlist/:playlist/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_playlist_delete, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "PUT", config->api_prefix, "/playlist/:playlist/add_media", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_playlist_add_media, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "DELETE", config->api_prefix, "/playlist/:playlist/delete_media", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_playlist_delete_media, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "POST", config->api_prefix, "/playlist/:playlist/has_media", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_playlist_has_media, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/playlist/:playlist/load", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_playlist_load, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/playlist/:playlist/export", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_playlist_export, (void*)config);

  // Config endpoints
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/users/", TALIESIN_CALLBACK_PRIORITY_AUTHENTICATION, &callback_taliesin_check_admin_access, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/users/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_username_get_list, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/config/:type/", TALIESIN_CALLBACK_PRIORITY_AUTHENTICATION, &callback_taliesin_check_access, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/config/:type/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_config_type_get, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "PUT", config->api_prefix, "/config/:type/", TALIESIN_CALLBACK_PRIORITY_AUTHENTICATION, &callback_taliesin_check_admin_access, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "PUT", config->api_prefix, "/config/:type/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_config_type_set, (void*)config);

  // Streaming endpoint
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/stream/:stream_name", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_stream_media, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/stream/:stream_name/cover", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_stream_cover, (void*)config);

  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/stream/", TALIESIN_CALLBACK_PRIORITY_AUTHENTICATION, &callback_taliesin_check_access, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/stream/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_stream_get_list, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "PUT", config->api_prefix, "/stream/:stream_name/manage", TALIESIN_CALLBACK_PRIORITY_AUTHENTICATION, &callback_taliesin_check_access, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "PUT", config->api_prefix, "/stream/:stream_name/manage", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_stream_manage, (void*)config);
#ifndef U_DISABLE_WEBSOCKET
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/stream/:stream_name/ws", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_stream_manage_ws, (void*)config);
#endif

  // Search endpoints
  ulfius_add_endpoint_by_val(config->instance, "*", config->api_prefix, "/search/", TALIESIN_CALLBACK_PRIORITY_AUTHENTICATION, &callback_taliesin_check_access, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "GET", config->api_prefix, "/search/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_search, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "PUT", config->api_prefix, "/search/", TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_advanced_search, (void*)config);

  // Other endpoints
  if (config->static_file_config->files_path != NULL) {
    ulfius_add_endpoint_by_val(config->instance, "GET", NULL, "*", TALIESIN_CALLBACK_PRIORITY_FILES, &callback_static_compressed_inmemory_website, (void*)config->static_file_config);
  }
  ulfius_add_endpoint_by_val(config->instance, "GET", NULL, "*", TALIESIN_CALLBACK_PRIORITY_POST_FILE, &callback_404_if_necessary, NULL);
  ulfius_add_endpoint_by_val(config->instance, "GET", "/.well-known/taliesin-configuration/", NULL, TALIESIN_CALLBACK_PRIORITY_APPLICATION, &callback_taliesin_server_configuration, (void*)config);
  ulfius_add_endpoint_by_val(config->instance, "OPTIONS", NULL, "*", TALIESIN_CALLBACK_PRIORITY_ZERO, &callback_taliesin_options, NULL);

  // API output compression
  ulfius_add_endpoint_by_val(config->instance, "*", config->api_prefix, "*", TALIESIN_CALLBACK_PRIORITY_COMPRESSION, &callback_http_compression, NULL);

  // Set default headers
  u_map_put(config->instance->default_headers, "Access-Control-Allow-Origin", config->allow_origin);
  u_map_put(config->instance->default_headers, "Access-Control-Allow-Credentials", "true");
  u_map_put(config->instance->default_headers, "Cache-Control", "no-store");
  u_map_put(config->instance->default_headers, "Pragma", "no-cache");

  y_log_message(Y_LOG_LEVEL_INFO, "Loading streams");
  j_stream_list = db_stream_list(config);
  if (check_result_value(j_stream_list, T_OK)) {
    json_array_foreach(json_object_get(j_stream_list, "stream"), index, j_element) {
      if (json_integer_value(json_object_get(j_element, "webradio"))) {
        if (add_webradio_from_db_stream(config, j_element, &webradio) != T_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error adding webradio stream");
        } else {
          if (webradio->icecast) {
            ret_thread_webradio = pthread_create(&thread_webradio, NULL, webradio_icecast_run_thread, (void *)webradio);
            detach_thread_webradio = pthread_detach(thread_webradio);
            if (ret_thread_webradio || detach_thread_webradio) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Error running thread webradio icecast");
            } else {
              ret_thread_icecast = pthread_create(&thread_icecast, NULL, icecast_push_run_thread, (void *)webradio);
              detach_thread_icecast = pthread_detach(thread_icecast);
              if (ret_thread_icecast || detach_thread_icecast) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Error running thread icecast push");
                webradio->audio_stream->status = TALIESIN_STREAM_STATUS_STOPPED;
                if (webradio->audio_stream->first_buffer != NULL) {
                  pthread_mutex_lock(&webradio->audio_stream->stream_lock);
                  pthread_cond_signal(&webradio->audio_stream->stream_cond);
                  pthread_mutex_unlock(&webradio->audio_stream->stream_lock);
                }
                pthread_mutex_lock(&config->stream_stop_lock);
                pthread_cond_wait(&config->stream_stop_cond, &config->stream_stop_lock);
                pthread_mutex_unlock(&config->stream_stop_lock);
              }
            }
          } else {
            ret_thread_webradio = pthread_create(&thread_webradio, NULL, webradio_run_thread, (void *)webradio);
            detach_thread_webradio = pthread_detach(thread_webradio);
            if (ret_thread_webradio || detach_thread_webradio) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Error running thread webradio");
            }
          }
        }
      } else {
        if (add_jukebox_from_db_stream(config, j_element) != T_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Error adding jukebox stream");
        }
      }
      y_log_message(Y_LOG_LEVEL_INFO, "stream %s (%s) loaded", json_string_value(json_object_get(j_element, "display_name")), json_string_value(json_object_get(j_element, "name")));
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error getting startup stream lists");
  }
  json_decref(j_stream_list);

  if (config->use_secure_connection) {
    char * key_file = get_file_content(config->secure_connection_key_file);
    char * pem_file = get_file_content(config->secure_connection_pem_file);
    if (key_file != NULL && pem_file != NULL) {
      res = ulfius_start_secure_framework(config->instance, key_file, pem_file);
    } else {
      res = U_ERROR_PARAMS;
    }
    o_free(key_file);
    o_free(pem_file);
  } else {
    res = ulfius_start_framework(config->instance);
  }

  if (res == U_OK) {
    y_log_message(Y_LOG_LEVEL_INFO, "Taliesin started on port %d, prefix: %s, secure: %s, remote address: %s", config->instance->port, config->api_prefix, config->use_secure_connection?"true":"false", config->server_remote_address);
    // Wait until stop signal is broadcasted
    pthread_mutex_lock(&global_handler_close_lock);
    pthread_cond_wait(&global_handler_close_cond, &global_handler_close_lock);
    pthread_mutex_unlock(&global_handler_close_lock);
    config->status = TALIESIN_STOP;
    for (i=0; i<config->nb_webradio; i++) {
      config->webradio_set[i]->audio_stream->status = TALIESIN_STREAM_STATUS_STOPPED;
      if (config->webradio_set[i]->audio_stream->first_buffer != NULL) {
        pthread_mutex_lock(&config->webradio_set[i]->audio_stream->stream_lock);
        pthread_cond_signal(&config->webradio_set[i]->audio_stream->stream_cond);
        pthread_mutex_unlock(&config->webradio_set[i]->audio_stream->stream_lock);
      }
      pthread_mutex_lock(&config->stream_stop_lock);
      pthread_cond_wait(&config->stream_stop_cond, &config->stream_stop_lock);
      pthread_mutex_unlock(&config->stream_stop_lock);
    }
    for (i=0; i<config->nb_jukebox; i++) {
      if (jukebox_close(config, config->jukebox_set[i]) != T_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error closing playlist");
      }
    }
    for (i=0; i<config->nb_refresh_status; i++) {
      config->refresh_status_list[i]->refresh_status = DATA_SOURCE_REFRESH_STATUS_STOP;
      config->refresh_status_list[i]->index = 0;
    }
    pthread_mutex_lock(&config->refresh_lock);
    pthread_cond_signal(&config->refresh_cond);
    pthread_mutex_unlock(&config->refresh_lock);
    while (config->nb_refresh_status) {
      pthread_mutex_lock(&config->refresh_lock);
      pthread_cond_wait(&config->refresh_cond, &config->refresh_lock);
      pthread_mutex_unlock(&config->refresh_lock);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error starting taliesin webservice");
    exit_server(&config, TALIESIN_ERROR);
  }
  if (pthread_mutex_destroy(&global_handler_close_lock) ||
      pthread_cond_destroy(&global_handler_close_cond)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error destroying global_handler_close_lock or global_handler_close_cond");
  }
  y_log_message(Y_LOG_LEVEL_INFO, "Taliesin stopped");
  exit_server(&config, TALIESIN_STOP);
  return 0;
}

/**
 * Exit properly the server by closing opened connections, databases and files
 */
void exit_server(struct config_elements ** config, int exit_value) {

  if (config != NULL && *config != NULL) {
    // Cleaning data
    ulfius_stop_framework((*config)->instance);
    ulfius_clean_instance((*config)->instance);
    h_close_db((*config)->conn);
    h_clean_connection((*config)->conn);
    pthread_mutex_destroy(&(*config)->playlist_lock);
    o_free((*config)->config_file);
    o_free((*config)->server_remote_address);
    o_free((*config)->api_prefix);
    o_free((*config)->log_file);
    o_free((*config)->allow_origin);
    o_free((*config)->secure_connection_key_file);
    o_free((*config)->secure_connection_pem_file);
    o_free((*config)->oauth_scope_user);
    o_free((*config)->oauth_scope_admin);
    o_free((*config)->oidc_claim_user_id);
#ifndef DISABLE_OAUTH2
    if ((*config)->oidc_configured) {
      i_jwt_profile_access_token_close_config((*config)->iddawc_resource_config);
    }
    o_free((*config)->iddawc_resource_config);
    o_free((*config)->iddawc_resource_config_admin);
    o_free((*config)->oidc_server_remote_config);
    o_free((*config)->oidc_server_public_jwks);
    o_free((*config)->oidc_iss);
    o_free((*config)->oidc_realm);
    o_free((*config)->oidc_aud);
#endif
    o_free((*config)->stream_format);
    free_string_array((*config)->audio_file_extension);
    free_string_array((*config)->video_file_extension);
    free_string_array((*config)->subtitle_file_extension);
    free_string_array((*config)->image_file_extension);
    free_string_array((*config)->cover_file_pattern);
    free_string_array((*config)->external_player);

    u_clean_compressed_inmemory_website_config((*config)->static_file_config);
    o_free((*config)->static_file_config->files_path);
    o_free((*config)->static_file_config);
    o_free((*config)->instance);
    o_free((*config)->webradio_set);

    o_free((*config)->icecast_host);
    o_free((*config)->icecast_user);
    o_free((*config)->icecast_password);
    o_free((*config)->icecast_mount_prefix);
    o_free((*config)->icecast_remote_address);

    o_free(*config);
    (*config) = NULL;
  }
  i_global_close();
  y_close_logs();
  shout_shutdown();
  exit(exit_value==TALIESIN_STOP?0:1);
}

/**
 * Initialize the application configuration based on the command line parameters
 */
int build_config_from_args(int argc, char ** argv, struct config_elements * config) {
  int next_option;
  const char * short_options = "c::p::u::m::l::f::h::v::";
  char * tmp = NULL, * to_free = NULL, * one_log_mode = NULL;
  static const struct option long_options[]= {
    {"config-file", optional_argument, NULL, 'c'},
    {"port", optional_argument, NULL, 'p'},
    {"url-prefix", optional_argument, NULL, 'u'},
    {"log-mode", optional_argument, NULL, 'm'},
    {"log-level", optional_argument, NULL, 'l'},
    {"log-file", optional_argument, NULL, 'f'},
    {"help", optional_argument, NULL, 'h'},
    {"version", optional_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}
  };

  if (config != NULL) {
    do {
      next_option = getopt_long(argc, argv, short_options, long_options, NULL);

      switch (next_option) {
        case 'c':
          if (optarg != NULL) {
            config->config_file = o_strdup(optarg);
            if (config->config_file == NULL) {
              fprintf(stderr, "Error allocating config->config_file, exiting\n");
              exit_server(&config, TALIESIN_STOP);
            }
          } else {
            fprintf(stderr, "Error!\nNo config file specified\n");
            return 0;
          }
          break;
        case 'p':
          if (optarg != NULL) {
            config->instance->port = (unsigned int)strtol(optarg, NULL, 10);
            if (config->instance->port <= 0 || config->instance->port > 65535) {
              fprintf(stderr, "Error!\nInvalid TCP Port number\n\tPlease specify an integer value between 1 and 65535");
              return 0;
            }
          } else {
            fprintf(stderr, "Error!\nNo TCP Port number specified\n");
            return 0;
          }
          break;
        case 'u':
          if (optarg != NULL) {
            config->api_prefix = o_strdup(optarg);
            if (config->api_prefix == NULL) {
              fprintf(stderr, "Error allocating config->api_prefix, exiting\n");
              exit_server(&config, TALIESIN_STOP);
            }
          } else {
            fprintf(stderr, "Error!\nNo URL prefix specified\n");
            return 0;
          }
          break;
        case 'm':
          if (optarg != NULL) {
            tmp = o_strdup(optarg);
            if (tmp == NULL) {
              fprintf(stderr, "Error allocating log_mode, exiting\n");
              exit_server(&config, TALIESIN_STOP);
            }
            one_log_mode = strtok(tmp, ",");
            while (one_log_mode != NULL) {
              if (0 == o_strncmp("console", one_log_mode, o_strlen("console"))) {
                config->log_mode |= Y_LOG_MODE_CONSOLE;
              } else if (0 == o_strncmp("syslog", one_log_mode, o_strlen("syslog"))) {
                config->log_mode |= Y_LOG_MODE_SYSLOG;
              } else if (0 == o_strncmp("journald", one_log_mode, o_strlen("journald"))) {
                config->log_mode += Y_LOG_MODE_JOURNALD;
              } else if (0 == o_strncmp("file", one_log_mode, o_strlen("file"))) {
                config->log_mode |= Y_LOG_MODE_FILE;
              }
              one_log_mode = strtok(NULL, ",");
            }
            o_free(to_free);
          } else {
            fprintf(stderr, "Error!\nNo mode specified\n");
            return 0;
          }
          break;
        case 'l':
          if (optarg != NULL) {
            if (0 == o_strncmp("NONE", optarg, o_strlen("NONE"))) {
              config->log_level = Y_LOG_LEVEL_NONE;
            } else if (0 == o_strncmp("ERROR", optarg, o_strlen("ERROR"))) {
              config->log_level = Y_LOG_LEVEL_ERROR;
            } else if (0 == o_strncmp("WARNING", optarg, o_strlen("WARNING"))) {
              config->log_level = Y_LOG_LEVEL_WARNING;
            } else if (0 == o_strncmp("INFO", optarg, o_strlen("INFO"))) {
              config->log_level = Y_LOG_LEVEL_INFO;
            } else if (0 == o_strncmp("DEBUG", optarg, o_strlen("DEBUG"))) {
              config->log_level = Y_LOG_LEVEL_DEBUG;
            }
          } else {
            fprintf(stderr, "Error!\nNo log level specified\n");
            return 0;
          }
          break;
        case 'f':
          if (optarg != NULL) {
            config->log_file = o_strdup(optarg);
            if (config->log_file == NULL) {
              fprintf(stderr, "Error allocating config->log_file, exiting\n");
              exit_server(&config, TALIESIN_STOP);
            }
          } else {
            fprintf(stderr, "Error!\nNo log file specified\n");
            return 0;
          }
          break;
        case 'h':
          print_help(stdout);
          exit_server(&config, TALIESIN_STOP);
          break;
        case 'v':
          fprintf(stdout, "\nTaliesin - Streaming server\n");
          fprintf(stdout, "\n");
          fprintf(stdout, "Version %s\n", _TALIESIN_VERSION_);
          fprintf(stdout, "\n");
          fprintf(stdout, "Copyright 2017-2018 Nicolas Mora <mail@babelouest.org>\n");
          exit_server(&config, TALIESIN_STOP);
          break;
      }

    } while (next_option != -1);

    // If none exists, exit failure
    if (config->config_file == NULL) {
      fprintf(stderr, "No configuration file found, please specify a configuration file path\n");
      return 0;
    }

    return 1;
  } else {
    return 0;
  }

}

/**
 * Print help message to output file specified
 */
void print_help(FILE * output) {
  fprintf(output, "\nTaliesin - Streaming server\n");
  fprintf(output, "\n");
  fprintf(output, "Version %s\n", _TALIESIN_VERSION_);
  fprintf(output, "\n");
  fprintf(output, "Copyright 2017-2018 Nicolas Mora <mail@babelouest.org>\n");
  fprintf(output, "\n");
  fprintf(output, "This program is free software; you can redistribute it and/or\n");
  fprintf(output, "modify it under the terms of the GNU GENERAL PUBLIC LICENSE\n");
  fprintf(output, "License as published by the Free Software Foundation;\n");
  fprintf(output, "version 3 of the License.\n");
  fprintf(output, "\n");
  fprintf(output, "Command-line options:\n");
  fprintf(output, "\n");
  fprintf(output, "-c --config-file=PATH\n");
  fprintf(output, "\tPath to configuration file\n");
  fprintf(output, "-p --port=PORT\n");
  fprintf(output, "\tPort to listen to\n");
  fprintf(output, "-u --url-prefix=PREFIX\n");
  fprintf(output, "\tAPI URL prefix\n");
  fprintf(output, "-m --log-mode=MODE\n");
  fprintf(output, "\tLog Mode\n");
  fprintf(output, "\tconsole, syslog, journald or file\n");
  fprintf(output, "\tIf you want multiple modes, separate them with a comma \",\"\n");
  fprintf(output, "\tdefault: console\n");
  fprintf(output, "-l --log-level=LEVEL\n");
  fprintf(output, "\tLog level\n");
  fprintf(output, "\tNONE, ERROR, WARNING, INFO, DEBUG\n");
  fprintf(output, "\tdefault: ERROR\n");
  fprintf(output, "-f --log-file=PATH\n");
  fprintf(output, "\tPath for log file if log mode file is specified\n");
  fprintf(output, "-h --help\n");
  fprintf(output, "-v --version\n");
  fprintf(output, "\tPrint this message\n\n");
}

/**
 * handles signal catch to exit properly when ^C is used for example
 * I don't like global variables but it looks fine to people who designed this
 */
void exit_handler(int signal) {
  y_log_message(Y_LOG_LEVEL_INFO, "Taliesin caught a stop or kill signal (%d), exiting", signal);
  pthread_mutex_lock(&global_handler_close_lock);
  pthread_cond_signal(&global_handler_close_cond);
  pthread_mutex_unlock(&global_handler_close_lock);
}

/**
 * Initialize the application configuration based on the config file content
 * Read the config file, get mandatory variables and devices
 */
int build_config_from_file(struct config_elements * config) {

  config_t cfg;
  config_setting_t * root, * database, * mime_type_list, * mime_type, * icecast;
  const char * cur_server_remote_address = NULL,
             * cur_prefix = NULL,
             * cur_log_mode = NULL,
             * cur_log_level = NULL,
             * cur_log_file = NULL,
             * one_log_mode = NULL,
             * cur_allow_origin = NULL,
             * db_type = NULL,
             * db_sqlite_path = NULL,
             * db_mariadb_host = NULL,
             * db_mariadb_user = NULL,
             * db_mariadb_password = NULL,
             * db_mariadb_dbname = NULL,
             * cur_static_files_path = NULL,
             * cur_oauth_scope_user = NULL,
             * cur_oauth_scope_admin = NULL,
             * extension = NULL,
             * mime_type_value = NULL,
             * cur_stream_format = NULL,
             * cur_oidc_claim_user_id = NULL,
             * icecast_host = NULL,
             * icecast_user = NULL,
             * icecast_password = NULL,
             * icecast_mount_prefix = NULL,
             * icecast_remote_address = NULL;
  int db_mariadb_port = 0,
      cur_stream_channels = 0,
      cur_stream_sample_rate = 0,
      cur_stream_bit_rate = 0,
      cur_use_oidc_authentication = 1,
      cur_user_can_create_data_source = 0,
      cur_timeout = 0,
      compress = 0,
      icecast_port = 0;
  unsigned int i = 0;
#ifndef DISABLE_OAUTH2
  config_setting_t * oidc_cfg;
  const char * cur_oidc_server_remote_config = NULL, * cur_oidc_server_public_jwks = NULL, * cur_oidc_iss = NULL, * cur_oidc_realm = NULL, * cur_oidc_aud = NULL;
  int cur_oidc_dpop_max_iat = 0, cur_oidc_server_remote_config_verify_cert = 0;
#endif

  config_init(&cfg);

  if (!config_read_file(&cfg, config->config_file)) {
    fprintf(stderr, "Error parsing config file %s\nOn line %d error: %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    config_destroy(&cfg);
    return 0;
  }

  if (config->instance->port == TALIESIN_DEFAULT_PORT) {
    // Get Port number to listen to
    int port;
    if (config_lookup_int(&cfg, "port", &port) == CONFIG_TRUE) {
      config->instance->port = (uint)port;
    }
  }

  if (config->server_remote_address == NULL) {
    // Get allow-origin value for CORS
    if (config_lookup_string(&cfg, "server_remote_address", &cur_server_remote_address)) {
      config->server_remote_address = o_strdup(cur_server_remote_address);
      if (config->server_remote_address == NULL) {
        fprintf(stderr, "Error allocating config->server_remote_address, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    }
  }

  if (config->api_prefix == NULL) {
    // Get prefix url for angharad
    if (config_lookup_string(&cfg, "api_prefix", &cur_prefix)) {
      config->api_prefix = o_strdup(cur_prefix);
      if (config->api_prefix == NULL) {
        fprintf(stderr, "Error allocating config->api_prefix, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    }
  }

  if (config->allow_origin == NULL) {
    // Get allow-origin value for CORS
    if (config_lookup_string(&cfg, "allow_origin", &cur_allow_origin)) {
      config->allow_origin = o_strdup(cur_allow_origin);
      if (config->allow_origin == NULL) {
        fprintf(stderr, "Error allocating config->allow_origin, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    }
  }

  if (config->log_mode == Y_LOG_MODE_NONE) {
    // Get log mode
    if (config_lookup_string(&cfg, "log_mode", &cur_log_mode)) {
      one_log_mode = strtok((char *)cur_log_mode, ",");
      while (one_log_mode != NULL) {
        if (0 == o_strncmp("console", one_log_mode, o_strlen("console"))) {
          config->log_mode |= Y_LOG_MODE_CONSOLE;
        } else if (0 == o_strncmp("syslog", one_log_mode, o_strlen("syslog"))) {
          config->log_mode |= Y_LOG_MODE_SYSLOG;
        } else if (0 == o_strncmp("journald", one_log_mode, o_strlen("journald"))) {
          config->log_mode += Y_LOG_MODE_JOURNALD;
        } else if (0 == o_strncmp("file", one_log_mode, o_strlen("file"))) {
          config->log_mode |= Y_LOG_MODE_FILE;
          // Get log file path
          if (config->log_file == NULL) {
            if (config_lookup_string(&cfg, "log_file", &cur_log_file)) {
              config->log_file = o_strdup(cur_log_file);
              if (config->log_file == NULL) {
                fprintf(stderr, "Error allocating config->log_file, exiting\n");
                config_destroy(&cfg);
                return 0;
              }
            }
          }
        }
        one_log_mode = strtok(NULL, ",");
      }
    }
  }

  if (config->log_level == Y_LOG_LEVEL_NONE) {
    // Get log level
    if (config_lookup_string(&cfg, "log_level", &cur_log_level)) {
      if (0 == o_strncmp("NONE", cur_log_level, o_strlen("NONE"))) {
        config->log_level = Y_LOG_LEVEL_NONE;
      } else if (0 == o_strncmp("ERROR", cur_log_level, o_strlen("ERROR"))) {
        config->log_level = Y_LOG_LEVEL_ERROR;
      } else if (0 == o_strncmp("WARNING", cur_log_level, o_strlen("WARNING"))) {
        config->log_level = Y_LOG_LEVEL_WARNING;
      } else if (0 == o_strncmp("INFO", cur_log_level, o_strlen("INFO"))) {
        config->log_level = Y_LOG_LEVEL_INFO;
      } else if (0 == o_strncmp("DEBUG", cur_log_level, o_strlen("DEBUG"))) {
        config->log_level = Y_LOG_LEVEL_DEBUG;
      }
    }
  }

  if (!y_init_logs(TALIESIN_LOG_NAME, config->log_mode, config->log_level, config->log_file, "Starting Taliesin Server")) {
    fprintf(stderr, "Error initializing logs\n");
    exit_server(&config, TALIESIN_ERROR);
  }

  root = config_root_setting(&cfg);
  database = config_setting_get_member(root, "database");
  if (database != NULL) {
    if (config_setting_lookup_string(database, "type", &db_type) == CONFIG_TRUE) {
      if (0 == o_strncmp(db_type, "sqlite3", o_strlen("sqlite3"))) {
        if (config_setting_lookup_string(database, "path", &db_sqlite_path) == CONFIG_TRUE) {
          config->conn = h_connect_sqlite(db_sqlite_path);
          if (config->conn == NULL) {
            fprintf(stderr, "Error opening sqlite database %s\n", db_sqlite_path);
            config_destroy(&cfg);
            return 0;
          } else {
            if (h_execute_query_sqlite(config->conn, "PRAGMA foreign_keys = ON;") != H_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Error executing sqlite3 query 'PRAGMA foreign_keys = ON;'");
              config_destroy(&cfg);
              return 0;
            }
          }
        } else {
          config_destroy(&cfg);
          fprintf(stderr, "Error, no sqlite database specified\n");
          return 0;
        }
      } else if (0 == o_strncmp(db_type, "mariadb", o_strlen("mariadb"))) {
        config_setting_lookup_string(database, "host", &db_mariadb_host);
        config_setting_lookup_string(database, "user", &db_mariadb_user);
        config_setting_lookup_string(database, "password", &db_mariadb_password);
        config_setting_lookup_string(database, "dbname", &db_mariadb_dbname);
        config_setting_lookup_int(database, "port", &db_mariadb_port);
        config->conn = h_connect_mariadb(db_mariadb_host, db_mariadb_user, db_mariadb_password, db_mariadb_dbname, (unsigned int)db_mariadb_port, NULL);
        if (config->conn == NULL) {
          fprintf(stderr, "Error opening mariadb database %s\n", db_mariadb_dbname);
          config_destroy(&cfg);
          return 0;
        }
      } else {
        config_destroy(&cfg);
        fprintf(stderr, "Error, database type unknown\n");
        return 0;
      }
    } else {
      config_destroy(&cfg);
      fprintf(stderr, "Error, no database type found\n");
      return 0;
    }
  } else {
    config_destroy(&cfg);
    fprintf(stderr, "Error, no database setting found\n");
    return 0;
  }

  if (config->static_file_config->files_path == NULL) {
    // Get path that serve static files
    if (config_lookup_string(&cfg, "app_files_path", &cur_static_files_path)) {
      config->static_file_config->files_path = o_strdup(cur_static_files_path);
      if (config->static_file_config->files_path == NULL) {
        fprintf(stderr, "Error allocating config->static_file_config->files_path, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    }
  }

  // Populate mime types u_map
  mime_type_list = config_lookup(&cfg, "app_files_mime_types");
  if (mime_type_list != NULL) {
    for (i=0; i<(unsigned int)config_setting_length(mime_type_list); i++) {
      mime_type = config_setting_get_elem(mime_type_list, i);
      if (mime_type != NULL) {
        if (config_setting_lookup_string(mime_type, "extension", &extension) == CONFIG_TRUE &&
            config_setting_lookup_string(mime_type, "mime_type", &mime_type_value) == CONFIG_TRUE) {
          u_map_put(&config->static_file_config->mime_types, extension, mime_type_value);
          if (config_setting_lookup_int(mime_type, "compress", &compress) == CONFIG_TRUE) {
            if (compress && u_add_mime_types_compressed(config->static_file_config, mime_type_value) != U_OK) {
              fprintf(stderr, "Error setting mime_type %s to compressed list, exiting\n", mime_type_value);
              config_destroy(&cfg);
              return 0;
            }
          }
        }
      }
    }
  }

  if (config_lookup_bool(&cfg, "use_oidc_authentication", &cur_use_oidc_authentication) == CONFIG_TRUE) {
    config->use_oidc_authentication = (short unsigned int)cur_use_oidc_authentication;
  }
  
  icecast = config_lookup(&cfg, "icecast");
  if (icecast != NULL) {
    if (config_setting_lookup_string(icecast, "host", &icecast_host) == CONFIG_TRUE) {
      if ((config->icecast_host = o_strdup(icecast_host)) == NULL) {
        fprintf(stderr, "Error allocating config->icecast_host, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    } else {
      fprintf(stderr, "Error icecast_host mandatory, exiting\n");
      config_destroy(&cfg);
      return 0;
    }
    if (config_setting_lookup_int(icecast, "port", &icecast_port) == CONFIG_TRUE) {
      config->icecast_port = (unsigned int)icecast_port;
    } else {
      fprintf(stderr, "Error icecast_port mandatory, exiting\n");
      config_destroy(&cfg);
      return 0;
    }
    if (config_setting_lookup_string(icecast, "user", &icecast_user) == CONFIG_TRUE) {
      if ((config->icecast_user = o_strdup(icecast_user)) == NULL) {
        fprintf(stderr, "Error allocating config->icecast_user, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    } else {
      fprintf(stderr, "Error icecast_user mandatory, exiting\n");
      config_destroy(&cfg);
      return 0;
    }
    if (config_setting_lookup_string(icecast, "password", &icecast_password) == CONFIG_TRUE) {
      if ((config->icecast_password = o_strdup(icecast_password)) == NULL) {
        fprintf(stderr, "Error allocating config->icecast_password, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    } else {
      fprintf(stderr, "Error icecast_password mandatory, exiting\n");
      config_destroy(&cfg);
      return 0;
    }
    if (config_setting_lookup_string(icecast, "mount_prefix", &icecast_mount_prefix) == CONFIG_TRUE) {
      if ((config->icecast_mount_prefix = o_strdup(icecast_mount_prefix)) == NULL) {
        fprintf(stderr, "Error allocating config->icecast_mount_prefix, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    } else {
      fprintf(stderr, "Error icecast_mount_prefix mandatory, exiting\n");
      config_destroy(&cfg);
      return 0;
    }
    if (config_setting_lookup_string(icecast, "remote_address", &icecast_remote_address) == CONFIG_TRUE) {
      if ((config->icecast_remote_address = o_strdup(icecast_remote_address)) == NULL) {
        fprintf(stderr, "Error allocating config->icecast_remote_address, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    } else {
      fprintf(stderr, "Error icecast_remote_address mandatory, exiting\n");
      config_destroy(&cfg);
      return 0;
    }
  }

  if (config->use_oidc_authentication) {
#ifndef DISABLE_OAUTH2
    oidc_cfg = config_lookup(&cfg, "oidc");
    if (config_setting_lookup_string(oidc_cfg, "server_remote_config", &cur_oidc_server_remote_config) == CONFIG_TRUE) {
      if ((config->oidc_server_remote_config = o_strdup(cur_oidc_server_remote_config)) == NULL) {
        fprintf(stderr, "Error allocating config->oidc_server_remote_config, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    }
    if (config_setting_lookup_bool(oidc_cfg, "server_remote_config_verify_cert", &cur_oidc_server_remote_config_verify_cert) == CONFIG_TRUE) {
      config->oidc_server_remote_config_verify_cert = (short unsigned int)cur_oidc_server_remote_config_verify_cert;
    }
    if (config_setting_lookup_string(oidc_cfg, "server_public_jwks", &cur_oidc_server_public_jwks) == CONFIG_TRUE) {
      if ((config->oidc_server_public_jwks = o_strdup(cur_oidc_server_public_jwks)) == NULL) {
        fprintf(stderr, "Error allocating config->oidc_server_public_jwks, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    }
    if (config_setting_lookup_string(oidc_cfg, "iss", &cur_oidc_iss) == CONFIG_TRUE) {
      if ((config->oidc_iss = o_strdup(cur_oidc_iss)) == NULL) {
        fprintf(stderr, "Error allocating config->oidc_iss, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    }
    if (config_setting_lookup_string(oidc_cfg, "realm", &cur_oidc_realm) == CONFIG_TRUE) {
      if ((config->oidc_realm = o_strdup(cur_oidc_realm)) == NULL) {
        fprintf(stderr, "Error allocating config->oidc_realm, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    }
    if (config_setting_lookup_string(oidc_cfg, "aud", &cur_oidc_aud) == CONFIG_TRUE) {
      if ((config->oidc_aud = o_strdup(cur_oidc_aud)) == NULL) {
        fprintf(stderr, "Error allocating config->oidc_aud, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    }
    if (config_setting_lookup_int(oidc_cfg, "dpop_max_iat", &cur_oidc_dpop_max_iat) == CONFIG_TRUE) {
      config->oidc_dpop_max_iat = (time_t)cur_oidc_dpop_max_iat;
    }
#else
    fprintf(stderr, "Error, Taliesin was built without oidc support\n");
    return 0;
#endif
  }

  if (config_lookup_string(&cfg, "oauth_scope_user", &cur_oauth_scope_user)) {
    config->oauth_scope_user = o_strdup(cur_oauth_scope_user);
    if (config->oauth_scope_user == NULL) {
      fprintf(stderr, "Error allocating config->oauth_scope_user, exiting\n");
      config_destroy(&cfg);
      return 0;
    }
  }

  if (config_lookup_string(&cfg, "oauth_scope_admin", &cur_oauth_scope_admin)) {
    config->oauth_scope_admin = o_strdup(cur_oauth_scope_admin);
    if (config->oauth_scope_admin == NULL) {
      fprintf(stderr, "Error allocating config->oauth_scope_admin, exiting\n");
      config_destroy(&cfg);
      return 0;
    }
  }

  if (config_lookup_string(&cfg, "oidc_claim_user_id", &cur_oidc_claim_user_id)) {
    config->oidc_claim_user_id = o_strdup(cur_oidc_claim_user_id);
    if (config->oidc_claim_user_id == NULL) {
      fprintf(stderr, "Error allocating config->oidc_claim_user_id, exiting\n");
      config_destroy(&cfg);
      return 0;
    }
  }

  if (config_lookup_string(&cfg, "stream_format", &cur_stream_format)) {
    if (0 == o_strcasecmp(cur_stream_format, "mp3") || 0 == o_strcasecmp(cur_stream_format, "vorbis") || 0 == o_strcasecmp(cur_stream_format, "flac")) {
      o_free(config->stream_format);
      config->stream_format = o_strdup(cur_stream_format);
      if (config->stream_format == NULL) {
        fprintf(stderr, "Error allocating resources for stream_format, exiting\n");
        config_destroy(&cfg);
        return 0;
      }
    } else {
      fprintf(stderr, "Error stream_format unknown, use values 'mp3', 'vorbis' or 'flac'\n");
      config_destroy(&cfg);
      return 0;
    }
  }

  if (config_lookup_int(&cfg, "stream_channels", &cur_stream_channels) == CONFIG_TRUE) {
    if (cur_stream_channels == 1 || cur_stream_channels == 2) {
      config->stream_channels = (short unsigned int)cur_stream_channels;
    } else {
      fprintf(stderr, "Error stream_channels, use values 1 or 2\n");
      config_destroy(&cfg);
      return 0;
    }
  }

  if (config_lookup_int(&cfg, "stream_sample_rate", &cur_stream_sample_rate) == CONFIG_TRUE) {
    if (cur_stream_sample_rate != 8000 && cur_stream_sample_rate != 11025 && cur_stream_sample_rate != 22050 && cur_stream_sample_rate != 32000 && cur_stream_sample_rate != 44100 && cur_stream_sample_rate != 48000) {
      fprintf(stderr, "Error stream_sample_rate, use values 8000, 11025, 22050, 32000, 44100 or 48000\n");
      config_destroy(&cfg);
      return 0;
    } else {
      config->stream_sample_rate = (unsigned int)cur_stream_sample_rate;
    }
  }

  if (config_lookup_int(&cfg, "stream_bitrate", &cur_stream_bit_rate) == CONFIG_TRUE) {
    if (0 != o_strcasecmp("flac", config->stream_format) && cur_stream_bit_rate != 32000 && cur_stream_bit_rate != 96000 && cur_stream_bit_rate != 128000 && cur_stream_bit_rate != 192000 && cur_stream_bit_rate != 256000 && cur_stream_bit_rate != 320000) {
      fprintf(stderr, "Error stream_bitrate, use values 32000, 96000, 128000, 192000, 256000 or 320000\n");
      config_destroy(&cfg);
      return 0;
    } else {
      config->stream_bitrate = (unsigned int)cur_stream_bit_rate;
    }
  }

  if (config_lookup_int(&cfg, "timeout", &cur_timeout)) {
    if (cur_timeout >= 0) {
      config->timeout = cur_timeout;
    } else {
      fprintf(stderr, "Error timeout must be positive or null\n");
      config_destroy(&cfg);
      return 0;
    }
  }

  if (config_lookup_bool(&cfg, "user_can_create_data_source", &cur_user_can_create_data_source) == CONFIG_TRUE) {
    config->user_can_create_data_source = (short unsigned int)cur_user_can_create_data_source;
  }

  config_destroy(&cfg);
  return 1;
}

/**
 * Check if all mandatory configuration parameters are present and correct
 * Initialize some parameters with default value if not set
 */
int check_config(struct config_elements * config) {

  if (config->api_prefix == NULL) {
    config->api_prefix = o_strdup(TALIESIN_DEFAULT_PREFIX);
    if (config->api_prefix == NULL) {
      fprintf(stderr, "Error allocating api_prefix, exit\n");
      return 0;
    }
  }

  if (config->log_mode == Y_LOG_MODE_NONE) {
    config->log_mode = Y_LOG_MODE_CONSOLE;
  }

  if (config->log_level == Y_LOG_LEVEL_NONE) {
    config->log_level = Y_LOG_LEVEL_ERROR;
  }

  if (config->log_mode == Y_LOG_MODE_FILE && config->log_file == NULL) {
    fprintf(stderr, "Error, you must specify a log file if log mode is set to file\n");
    print_help(stderr);
    return 0;
  }

  if (config->oidc_claim_user_id == NULL) {
    config->oidc_claim_user_id = o_strdup(TALIESIN_DEFAULT_CLAIM_USER_ID);
  }
  
  if (config->icecast_host != NULL) {
    if (!config->icecast_port || config->icecast_user == NULL || config->icecast_password == NULL || config->icecast_mount_prefix == NULL || config->icecast_remote_address == NULL) {
      fprintf(stderr, "Error, invalid icecast parameters\n");
      return 0;
    }
  }

  return 1;
}

/**
 *
 * Read the content of a file and return it as a char *
 * returned value must be free'd after use
 *
 */
char * get_file_content(const char * file_path) {
  char * buffer = NULL;
  size_t length, res;
  FILE * f;

  f = fopen (file_path, "rb");
  if (f) {
    fseek (f, 0, SEEK_END);
    length = (size_t)ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = o_malloc((length+1)*sizeof(char));
    if (buffer) {
      res = fread (buffer, 1, length, f);
      if (res != length) {
        fprintf(stderr, "fread warning, reading %zu while expecting %zu", res, length);
      }
      // Add null character at the end of buffer, just in case
      buffer[length] = '\0';
    }
    fclose (f);
  }

  return buffer;
}

/**
 * Redirect libav logs to yder
 * Except there is a bug that makes taliesin crash in some cases
 * So I disabled it...
 */
void redirect_libav_logs(void * avcl, int level, const char * fmt, va_list vl) {
  UNUSED(avcl);
  va_list args_cpy, args_cpy2;
  size_t out_len = 0;
  char * out = NULL, * new_fmt;
  unsigned long y_level;

  switch (level) {
  case AV_LOG_INFO:
    y_level = Y_LOG_LEVEL_INFO;
    break;
  case AV_LOG_WARNING:
    y_level = Y_LOG_LEVEL_WARNING;
    break;
  case AV_LOG_ERROR:
  case AV_LOG_FATAL:
  case AV_LOG_PANIC:
    y_level = Y_LOG_LEVEL_ERROR;
    break;
  case AV_LOG_VERBOSE:
  case AV_LOG_TRACE:
  case AV_LOG_DEBUG:
  default:
    y_level = Y_LOG_LEVEL_NONE;
    break;
  }

  if (y_level != Y_LOG_LEVEL_NONE) {
    va_copy(args_cpy, vl);
    va_copy(args_cpy2, vl);
    new_fmt = msprintf("LIBAV - %s", fmt);
    out_len = (size_t)vsnprintf(NULL, 0, new_fmt, args_cpy);
    out = o_malloc((out_len)*sizeof(char));
    if (out != NULL) {
      vsnprintf(out, (out_len), new_fmt, args_cpy2);
      y_log_message(y_level, out);
      o_free(out);
    }
    o_free(new_fmt);
    va_end(args_cpy);
    va_end(args_cpy2);
  }
}
