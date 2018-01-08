/**
 *
 * Taliesin - Media server
 * 
 * Declarations for constants and prototypes
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

#ifndef __TALIESIN_H_
#define __TALIESIN_H_

#define _TALIESIN_VERSION_ "1.0.0"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libavresample/avresample.h>

/** Angharad libraries **/
#define U_DISABLE_CURL
#include <ulfius.h>
#include <yder.h>

#define _HOEL_MARIADB
#define _HOEL_SQLITE
#include <hoel.h>

#include "glewlwyd_resource.h"
#include "static_file_callback.h"

/**
 * Application constant definitions
 */
#define TALIESIN_LOG_NAME       "Taliesin"
#define TALIESIN_DEFAULT_PORT   8576
#define TALIESIN_DEFAULT_PREFIX "api"

#define TALIESIN_RUNNING  0
#define TALIESIN_STOP     1
#define TALIESIN_ERROR    2

#define TALIESIN_CALLBACK_PRIORITY_ZERO           0
#define TALIESIN_CALLBACK_PRIORITY_AUTHENTICATION 1
#define TALIESIN_CALLBACK_PRIORITY_APPLICATION    2
#define TALIESIN_CALLBACK_PRIORITY_FILES          3
#define TALIESIN_CALLBACK_PRIORITY_CLEAN          4

#define T_OK                 0
#define T_ERROR              1
#define T_ERROR_UNAUTHORIZED 2
#define T_ERROR_PARAM        3
#define T_ERROR_DB           4
#define T_ERROR_MEMORY       5
#define T_ERROR_NOT_FOUND    6

#define TALIESIN_TABLE_DATA_SOURCE        "t_data_source"
#define TALIESIN_TABLE_FOLDER             "t_folder"
#define TALIESIN_TABLE_MEDIA              "t_media"
#define TALIESIN_TABLE_META_DATA          "t_meta_data"
#define TALIESIN_TABLE_PLAYLIST           "t_playlist"
#define TALIESIN_TABLE_PLAYLIST_ELEMENT   "t_playlist_element"
#define TALIESIN_TABLE_IMAGE_COVER        "t_image_cover"
#define TALIESIN_TABLE_MEDIA_HISTORY      "t_media_history"
#define TALIESIN_TABLE_CONFIG             "t_config"
#define TALIESIN_TABLE_CATEGORY_INFO      "t_category_info"
#define TALIESIN_TABLE_STREAM             "t_stream"
#define TALIESIN_TABLE_STREAM_ELEMENT     "t_stream_element"

#define TALIESIN_SCOPE_ALL "all"
#define TALIESIN_SCOPE_ME  "me"
#define TALIESIN_NO_AUTHENTICATION_USERNAME "Myrddin"

#define TALIESIN_MEDIUMBLOB_MAX_SIZE 16777215

#define TALIESIN_PLAYLIST_NAME_LENGTH 32
#define TALIESIN_TAG_KEY_LENGTH       128
#define TALIESIN_TAG_VALUE_LENGTH     1024

#define TALIESIN_STREAM_DEFAULT_FORMAT               "mp3"
#define TALIESIN_STREAM_DEFAULT_CHANNELS             2
#define TALIESIN_STREAM_DEFAULT_SAMPLE_RATE          44100
#define TALIESIN_STREAM_DEFAULT_BIT_RATE             128000
#define TALIESIN_STREAM_FLAC_BIT_RATE                1411000
#define TALIESIN_STREAM_BUFFER_MAX                   3
#define TALIESIN_PLAYLIST_CLIENT_MAX                 3
#define TALIESIN_STREAM_METADATA_INTERVAL            5
#define TALIESIN_STREAM_INITIAL_CLIENT_BUFFER_LENGTH 5
#define TALIESIN_STREAM_HEADER_SIZE                  1024
#define TALIESIN_STREAM_BUFFER_INC_SIZE              1024

#define TALIESIN_STREAM_STATUS_NOT_STARTED 0
#define TALIESIN_STREAM_STATUS_STARTED     1
#define TALIESIN_STREAM_STATUS_STOPPED     2
#define TALIESIN_STREAM_STATUS_PAUSED      3
#define TALIESIN_STREAM_STATUS_COMPLETED   4

#define TALIESIN_STREAM_TRANSCODE_STATUS_NOT_STARTED 0
#define TALIESIN_STREAM_TRANSCODE_STATUS_STARTED     1
#define TALIESIN_STREAM_TRANSCODE_STATUS_PAUSED      2
#define TALIESIN_STREAM_TRANSCODE_STATUS_COMPLETE    3
#define TALIESIN_STREAM_TRANSCODE_STATUS_ERROR       4

#define DATA_SOURCE_REFRESH_STATUS_NOT_RUNNING 0
#define DATA_SOURCE_REFRESH_STATUS_PENDING     1
#define DATA_SOURCE_REFRESH_STATUS_PREPARING   2
#define DATA_SOURCE_REFRESH_STATUS_RUNNING     3
#define DATA_SOURCE_REFRESH_STATUS_CLEANING    4
#define DATA_SOURCE_REFRESH_STATUS_ERROR       5
#define DATA_SOURCE_REFRESH_STATUS_STOP        6
#define DATA_SOURCE_REFRESH_STATUS_NOT_FOUND   7

#define DATA_SOURCE_REFRESH_MODE_PROCESSED  0
#define DATA_SOURCE_REFRESH_MODE_IN_REFRESH 1

#define TALIESIN_PLAYLIST_VISIBILITY_PUBLIC  0
#define TALIESIN_PLAYLIST_VISIBILITY_PRIVATE 1

#define TALIESIN_STREAM_COMMAND_NONE     0
#define TALIESIN_STREAM_COMMAND_PREVIOUS 1
#define TALIESIN_STREAM_COMMAND_NEXT     2
#define TALIESIN_STREAM_COMMAND_JUMP     3

#define TALIESIN_CONFIG_AUDIO_FILE_EXTENSION    "audio_file_extension"
#define TALIESIN_CONFIG_VIDEO_FILE_EXTENSION    "video_file_extension"
#define TALIESIN_CONFIG_SUBTITLE_FILE_EXTENSION "subtitle_file_extension"
#define TALIESIN_CONFIG_IMAGE_FILE_EXTENSION    "image_file_extension"
#define TALIESIN_CONFIG_COVER_FILE_PATTERN      "cover_file_pattern"
#define TALIESIN_CONFIG_EXTERNAL_PLAYER         "external_player"

#define TALIESIN_FILE_TYPE_UNKNOWN  0
#define TALIESIN_FILE_TYPE_AUDIO    1
#define TALIESIN_FILE_TYPE_VIDEO    2
#define TALIESIN_FILE_TYPE_SUBTITLE 3
#define TALIESIN_FILE_TYPE_IMAGE    4

#define TALIESIN_PLAYLIST_MESSAGE_TYPE_NONE      0
#define TALIESIN_PLAYLIST_MESSAGE_TYPE_NEW_MEDIA 1
#define TALIESIN_PLAYLIST_MESSAGE_TYPE_CLOSING   2
#define TALIESIN_PLAYLIST_MESSAGE_TYPE_CLOSE     3
#define TALIESIN_PLAYLIST_MESSAGE_TYPE_TRASH     4

#define TALIESIN_STORED_PLAYLIST_WEBRADIO_STARTUP_NO_UPDATE -1
#define TALIESIN_STORED_PLAYLIST_WEBRADIO_STARTUP_OFF        0
#define TALIESIN_STORED_PLAYLIST_WEBRADIO_STARTUP_NO_RANDOM  1
#define TALIESIN_STORED_PLAYLIST_WEBRADIO_STARTUP_RANDOM     2

#define TALIESIN_WEBSOCKET_PLAYLIST_STATUS_OPEN    0
#define TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSING 1
#define TALIESIN_WEBSOCKET_PLAYLIST_STATUS_CLOSE   2

#define TALIESIN_COVER_THUMB_WIDTH  100
#define TALIESIN_COVER_THUMB_HEIGHT 100

#define TALIESIN_SEARCH_CATEGORY_INVALID  0
#define TALIESIN_SEARCH_CATEGORY_NONE     1
#define TALIESIN_SEARCH_CATEGORY_PLAYLIST 2
#define TALIESIN_SEARCH_CATEGORY_STREAM   3
#define TALIESIN_SEARCH_CATEGORY_FOLDER   4
#define TALIESIN_SEARCH_CATEGORY_FILE     5
#define TALIESIN_SEARCH_CATEGORY_TITLE    6
#define TALIESIN_SEARCH_CATEGORY_ARTIST   7
#define TALIESIN_SEARCH_CATEGORY_ALBUM    8
#define TALIESIN_SEARCH_CATEGORY_YEAR     9
#define TALIESIN_SEARCH_CATEGORY_GENRE    10

#define TALIESIN_MEDIA_LIMIT_DEFAULT 100

/**
 * global variables to store close condition and mutex
 */
pthread_mutex_t global_handler_close_lock;
pthread_cond_t  global_handler_close_cond;

/**
 * Application structures
 */
struct _t_file {
  char                    * path;
  json_int_t                tm_id;
  struct _t_file          * next;
};

struct _t_file_list {
  unsigned int     nb_files;
  pthread_mutex_t  file_lock;
  
  struct _t_file * start;
  struct _t_file * end;
};

struct _audio_buffer {
  size_t                 size;
  size_t                 max_size;
  short int              complete;
  uint8_t              * data;
  size_t               * offset_list;
  unsigned int           nb_offset;
  size_t                 last_offset;
  
  unsigned int           nb_client;
  struct timespec        start;
  
  struct _t_file       * file;
  char                 * title;
  unsigned int           index;
  uint64_t               counter;
  short int              skip;
  unsigned short int     read;
  
  struct _audio_buffer * next;
};

struct _audio_stream {
  pthread_mutex_t                  write_lock;
  pthread_mutex_t                  transcode_lock;
  int                              nb_buffer;
  struct _audio_buffer          *  first_buffer;
  struct _audio_buffer          *  last_buffer;
  short int                        transcode_status;
  
  short int                        is_header;
  struct _audio_buffer           * header_buffer;
  
  short                            status;
  
  pthread_mutex_t                  buffer_lock;
  pthread_cond_t                   buffer_cond;

  unsigned int                     nb_client_connected;
  struct _client_data_webradio  ** client_list;
  pthread_mutex_t                  client_lock;
  pthread_cond_t                   client_cond;
  
  AVFormatContext                * output_format_context;
  AVCodecContext                 * output_codec_context;
  AVAudioFifo                    * fifo;
  int64_t                          pts;

  char                           * stream_format;
  unsigned short int               stream_channels;
  unsigned int                     stream_sample_rate;
  unsigned int                     stream_bitrate;
};

struct _t_webradio {
  char                      name[TALIESIN_PLAYLIST_NAME_LENGTH + 1];
  char                    * display_name;
  char                    * username;
  json_int_t                tpl_id;
  char                    * playlist_name;
  
  struct _audio_stream    * audio_stream;
  struct timespec           start;
  
  struct _t_file_list     * file_list;
  unsigned long             current_index;

  pthread_mutex_t           message_lock;
  pthread_cond_t            message_cond;
  short int                 message_type;
  
  pthread_mutex_t           websocket_lock;
  pthread_cond_t            websocket_cond;
  unsigned int              nb_websocket;
  
  short int                 random;
  
  struct config_elements  * config;
};

struct _client_data_webradio {
  struct _audio_stream * audio_stream;
  struct _audio_buffer * current_buffer;
  
  uint64_t               global_offset;
  uint64_t               first_buffer_counter;
  struct timespec        start;
  
  short int              send_header;
  size_t                 header_offset;
  
  size_t                 buffer_offset;
  
  int                    metadata_send;
  size_t                 metadata_offset;
  size_t                 metadata_len;
  size_t                 metadata_current_offset;
  char *                 metadata_buffer;
  
  char                   stream_name[TALIESIN_PLAYLIST_NAME_LENGTH + 1];
  char                 * client_address;
  char                 * user_agent;
  short unsigned int     command;
  
  char                 * server_remote_address;
  char                 * api_prefix;
};

struct _client_data_jukebox {
  struct _jukebox_audio_buffer  * audio_buffer;
  struct _t_jukebox             * jukebox;
  size_t                          buffer_offset;
  
  char                            stream_name[TALIESIN_PLAYLIST_NAME_LENGTH + 1];
  short unsigned int              command;
  short unsigned int              client_present;
  
  char                          * server_remote_address;
  char                          * api_prefix;
};

struct _jukebox_audio_buffer {
  size_t               size;
  size_t               max_size;
  short int            complete;
  uint8_t            * data;
  
  struct _t_file     * file;
  char               * client_address;
  char               * user_agent;
  uint64_t             duration;
  
  short                status;
  pthread_mutex_t      buffer_lock;
  pthread_cond_t       buffer_cond;
  
  pthread_mutex_t      write_lock;
  
  struct _t_jukebox * jukebox;
};

struct _t_jukebox {
  char                             name[TALIESIN_PLAYLIST_NAME_LENGTH + 1];
  char                           * display_name;
  char                           * username;
  json_int_t                       tpl_id;
  char                           * playlist_name;
  short int                        status;
  
  struct _jukebox_audio_buffer  ** jukebox_audio_buffer;
  unsigned int                     nb_jukebox_audio_buffer;
  
  struct _t_file_list            * file_list;

  pthread_mutex_t                  message_lock;
  pthread_cond_t                   message_cond;
  short int                        message_type;
  
  pthread_mutex_t                  websocket_lock;
  pthread_cond_t                   websocket_cond;
  unsigned int                     nb_websocket;
  
  struct config_elements         * config;

  char                           * stream_format;
  unsigned short int               stream_channels;
  unsigned int                     stream_sample_rate;
  unsigned int                     stream_bitrate;
  
  unsigned int                     nb_client;
  time_t                           last_seen;
};

struct _refresh_config {
  struct config_elements * config;
  json_t                 * j_data_source;
  char                   * path;
  unsigned int             index;
  int                      refresh_status;
  int                      refresh_action;
  ssize_t                  nb_files_total;
  ssize_t                  nb_files_read;
};

struct _ws_stream {
  struct config_elements * config;
  char                   * username;
  int                      is_admin;
  int                      is_authenticated;
  time_t                   expiration;
  struct _t_webradio     * webradio;
  struct _t_jukebox      * jukebox;
  int                      status;
};

struct _close_jukebox {
  struct config_elements * config;
  struct _t_jukebox      * jukebox;
};

struct config_elements {
  char                             * config_file;
  char                             * server_remote_address;
  char                             * api_prefix;
  unsigned long                      log_mode;
  unsigned long                      log_level;
  char                             * log_file;
  char                             * allow_origin;
  unsigned int                       use_secure_connection;
  char                             * secure_connection_key_file;
  char                             * secure_connection_pem_file;
  struct _h_connection             * conn;
  struct _u_instance               * instance;
  unsigned short                     use_oauth2_authentication;
  struct _glewlwyd_resource_config * glewlwyd_resource_config;
  struct _static_file_config       * static_file_config;
  char                             * oauth_scope_user;
  char                             * oauth_scope_admin;
  pthread_mutex_t                    playlist_lock;
  unsigned int                       nb_webradio;
  struct _t_webradio              ** webradio_set;
  unsigned int                       nb_jukebox;
  struct _t_jukebox               ** jukebox_set;
  char                             * stream_format;
  unsigned short int                 stream_channels;
  unsigned int                       stream_sample_rate;
  unsigned int                       stream_bitrate;
  pthread_mutex_t                    stream_stop_lock;
  pthread_cond_t                     stream_stop_cond;
  uint                               nb_refresh_status;
  struct _refresh_config          ** refresh_status_list;
  pthread_mutex_t                    refresh_lock;
  pthread_cond_t                     refresh_cond;
  unsigned short                     user_can_create_data_source;
  char                            ** audio_file_extension;
  char                            ** video_file_extension;
  char                            ** subtitle_file_extension;
  char                            ** image_file_extension;
  char                            ** cover_file_pattern;
  char                            ** external_player;
};

/**
 * Functions declarations
 */

// Main functions and misc functions
int    build_config_from_args(int argc, char ** argv, struct config_elements * config);
int    build_config_from_file(struct config_elements * config);
int    check_config(struct config_elements * config);
void   exit_handler(int handler);
void   exit_server(struct config_elements ** config, int exit_value);
void   print_help(FILE * output);
char * get_file_content(const char * file_path);
char * url_decode(const char * str);
char * url_encode(const char * str);
long   random_at_most(long max);
char * rand_string(char * str, size_t size);
void   redirect_libav_logs(void * avcl, int level, const char * fmt, va_list vl);
char * get_ip_source(const struct _u_request * request);

// Filesystem functions
int      is_fs_directory_readable(const char * path);
json_t * fs_directory_read(const char * path);
json_t * media_get_metadata(struct config_elements * config, AVCodecContext  * thumbnail_cover_codec_context, const char * path);
ssize_t  fs_directory_count_files_recursive(const char * path);

// Config functions
int      load_config_values(struct config_elements * config);
int      config_get_type_from_path(struct config_elements * config, const char * path);
json_t * config_get_values(struct config_elements * config, const char * config_type);
json_t * is_valid_config_elements(json_t * j_config_values);
int      config_set_values(struct config_elements * config, const char * config_type, json_t * j_config_values);
json_t * username_get_list(struct config_elements * config);

// Data source functions
json_t   * data_source_list(struct config_elements * config, const char * username);
json_t   * data_source_get(struct config_elements * config, const char * username, const char * data_source, int get_id);
int        data_source_add(struct config_elements * config, const char * username, json_t * j_data_source);
int        data_source_set(struct config_elements * config, const char * username, const char * data_source, json_t * j_data_source);
int        data_source_delete(struct config_elements * config, const char * username, const char * data_source);
int        data_source_refresh_run(struct config_elements * config, json_t * j_data_source, const char * path, int clean);
int        data_source_refresh_stop(struct config_elements * config, json_t * j_data_source);
json_t   * is_data_source_valid(struct config_elements * config, const char * username, int is_admin, json_t * j_data_source, int update);
json_t   * data_source_get_refresh_status(struct config_elements * config, json_int_t tds_id);
int        data_source_set_refresh_status(struct config_elements * config, json_int_t tds_id, int new_status);
int        data_source_clean(struct config_elements * config, json_int_t tds_id);
int        data_source_can_update(json_t * j_data_source, int is_admin);
json_t *   media_scan_path(struct config_elements * config, json_t * j_data_source, const char * path, int recursive);

// struct _t_file_list functions
int              file_list_enqueue_new_file(struct _t_file_list * file_list, const char * path, json_int_t tm_id);
int              file_list_enqueue_file(struct _t_file_list * file_list, struct _t_file * file);
int              file_list_enqueue_new_file_nolock(struct _t_file_list * file_list, const char * path, json_int_t tm_id);
int              file_list_enqueue_file_nolock(struct _t_file_list * file_list, struct _t_file * file);
int              file_list_insert_file_at(struct _t_file_list * file_list, struct _t_file * file, unsigned long index);
struct _t_file * file_list_dequeue_file(struct _t_file_list * file_list, unsigned long index);
struct _t_file * file_list_get_file(struct _t_file_list * file_list, unsigned long index);
int              file_list_init(struct _t_file_list * file_list);
void             file_list_clean(struct _t_file_list * file_list);
void             file_list_clean_file(struct _t_file * file);
int              file_list_add_media_list(struct config_elements * config, struct _t_file_list * file_list, json_t * media_list);
json_t *         file_list_has_media_list(struct config_elements * config, struct _t_file_list * file_list, json_t * media_list, json_int_t offset, json_int_t limit);
int              file_list_remove_media_list(struct config_elements * config, struct _t_file_list * file_list, json_t * j_media_list);
int              file_list_empty_nolock(struct _t_file_list * file_list);

// Jukebox audio buffer
json_t * is_stream_parameters_valid(int webradio, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate);
void     audio_stream_clean (struct _audio_stream * audio_stream);
int      jukebox_audio_buffer_init (struct _jukebox_audio_buffer * jukebox_audio_buffer);
void     jukebox_audio_buffer_clean (struct _jukebox_audio_buffer * jukebox_audio_buffer);
int      audio_stream_add_data(struct _audio_stream * stream, uint8_t *buf, int buf_size);

// Common stream functions
json_t * stream_list(struct config_elements * config, const char * username);
char * build_icy_title(json_t * media);
char * build_m3u_title(json_t * media);

// Webradio functions
int              webradio_init(struct _t_webradio * webradio, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate);
void             webradio_clean(struct _t_webradio * webradio);
struct _t_file * webradio_get_next_file(struct _t_webradio * webradio, unsigned int * index/*, unsigned short * play_after*/);
json_t         * webradio_get_clients(struct _t_webradio * webradio);
json_t         * webradio_get_info(struct _t_webradio * webradio);
json_t         * webradio_get_file_list(struct config_elements * config, struct _t_webradio * webradio, json_int_t offset, json_int_t limit);
int              webradio_remove_media_by_index(struct _t_webradio * webradio, int index, json_int_t * tm_id);

int      webradio_open_output_buffer(struct _audio_stream * audio_stream);
ssize_t  webradio_buffer_metadata(char * buf, size_t max, struct _client_data_webradio * client_data);
int      audio_buffer_init(struct _audio_buffer * audio_buffer);
void     audio_buffer_clean(struct _audio_buffer * audio_buffer, int recursive);
void   * webradio_run_thread(void * args);
json_t * is_webradio_command_valid(struct config_elements * config, struct _t_webradio * webradio, json_t * j_command, const char * username, int is_admin);
json_t * webradio_command(struct config_elements * config, struct _t_webradio * webradio, const char * username, json_t * j_command);
json_t * add_webradio_from_path(struct config_elements * config, json_t * j_data_source, const char * path, const char * username, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate, int recursive, short int random, const char * name, struct _t_webradio ** new_webradio);
json_t * add_webradio_from_playlist(struct config_elements * config, json_t * j_playlist, const char * username, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate, short int random, const char * name, struct _t_webradio ** new_webradio);
int      add_webradio_from_db_stream(struct config_elements * config, json_t * j_stream, struct _t_webradio ** new_webradio);
int      scan_path_to_webradio(struct config_elements * config, json_t * j_data_source, const char * path, int recursive, struct _t_webradio * webradio);

int     client_data_webradio_init(struct _client_data_webradio * client_data);
void    client_data_webradio_clean(struct _client_data_webradio * client_data);
ssize_t u_webradio_stream (void * cls, uint64_t pos, char * buf, size_t max);
void    u_webradio_stream_free(void * cls);

// Jukebox functions
json_t * is_jukebox_command_valid(struct config_elements * config, struct _t_jukebox * jukebox, json_t * j_command, const char * username, int is_admin);
json_t * jukebox_command(struct config_elements * config, struct _t_jukebox * jukebox, const char * username, json_t * j_command);
int      jukebox_close(struct config_elements * config, struct _t_jukebox * jukebox);
int      jukebox_audio_buffer_add_data(struct _jukebox_audio_buffer * jukebox_audio_buffer, uint8_t *buf, int buf_size);
int      jukebox_build_m3u(struct config_elements * config, struct _t_jukebox * jukebox, char ** m3u_data);
int      is_valid_path_element_parameter(struct config_elements * config, json_t * jukebox_element, const char * username, int is_admin);
int      is_valid_category_element_parameter(struct config_elements * config, json_t * category_element, const char * username, int is_admin);
int      is_valid_playlist_element_parameter(struct config_elements * config, json_t * j_playlist, const char * username);
void   * jukebox_run_thread(void * args);
json_t * add_jukebox_from_path(struct config_elements * config, json_t * j_data_source, const char * path, const char * username, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate, int recursive, const char * name);
json_t * add_jukebox_from_playlist(struct config_elements * config, json_t * j_playlist, const char * username, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate, const char * name);
int      add_jukebox_from_db_stream(struct config_elements * config, json_t * j_stream);

int     init_client_data_jukebox(struct _client_data_jukebox * client_data_jukebox);
void    clean_client_data_jukebox(struct _client_data_jukebox * client_data_jukebox);
ssize_t u_jukebox_stream (void * cls, uint64_t pos, char * buf, size_t max);
void    u_jukebox_stream_free(void * cls);

int      jukebox_init(struct _t_jukebox * jukebox, const char * format, unsigned short channels, unsigned int sample_rate, unsigned int bit_rate);
void     jukebox_clean(struct _t_jukebox * jukebox);
json_t * jukebox_get_info(struct _t_jukebox * jukebox);
json_t * jukebox_get_clients(struct _t_jukebox * jukebox);
json_t * jukebox_get_file_list(struct config_elements * config, struct _t_jukebox * jukebox, json_int_t offset, json_int_t limit);
int      jukebox_remove_media_by_index(struct _t_jukebox * jukebox, int index, json_int_t * tm_id);

// Media functions
json_t   * media_get(struct config_elements * config, json_t * j_data_source, const char * path);
json_t   * media_get_by_id(struct config_elements * config, json_int_t tm_id);
json_t   * media_get_full(struct config_elements * config, json_t * j_data_source, const char * path);
json_t   * media_get_file_list_from_path(struct config_elements * config, json_t * j_data_source, const char * path, int recursive);
json_t   * media_get_audio_list_from_path(struct config_elements * config, json_t * j_data_source, const char * path, int recursive);
json_t   * media_cover_get(struct config_elements * config, json_t * j_data_source, const char * path, int thumbnail);
json_t   * media_cover_get_by_id(struct config_elements * config, json_int_t tm_id, int thumbnail);
json_t   * media_list_folder(struct config_elements * config, json_t * j_data_source, json_int_t tf_id, int get_id);
int        media_add(struct config_elements * config, json_int_t tds_id, json_int_t tf_id, const char * path, json_t * j_media);
int        media_update(struct config_elements * config, json_int_t tm_id, json_t * j_media);
json_t   * media_folder_get_cover(struct config_elements * config, json_t * j_data_source, const char * path);
json_t   * media_folder_detect_cover_by_id(struct config_elements * config, json_t * j_data_source, json_int_t tf_id, const char * path);
json_t   * media_cover_get_all(struct config_elements * config, json_t * j_data_source, const char * path);
json_int_t folder_get_id(struct config_elements * config, json_t * j_data_source, json_int_t tf_parent_id, const char * path);
json_t   * folder_get_all(struct config_elements * config, json_t * j_data_source, json_int_t tf_id);
int        media_add_history(struct config_elements * config, const char * stream_name, json_int_t tpl_id, json_int_t tm_id);
json_t   * media_get_history(struct config_elements * config, const char * stream_name, json_int_t offset, json_int_t limit);
int        media_image_cover_clean_orphan(struct config_elements * config, json_int_t tds_id, json_int_t tic_id);
int        is_valid_b64_image(const unsigned char * base64_image);
json_int_t media_cover_save(struct config_elements * config, json_int_t tds_id, const unsigned char * image_base64);
json_t *   media_get_tags_from_id(struct config_elements * config, json_int_t tm_id);
json_t *   media_append_list_to_media_list(struct config_elements * config, json_t * append_list, const char * username);

// db stream functions
json_t * db_stream_list(struct config_elements * config);
int      db_stream_reload_file_lists(struct config_elements * config);

// Search functions
json_t * media_simple_search(struct config_elements * config, const char * username, const char * search_pattern, unsigned short int search_category);
json_t * is_valid_media_advanced_search(struct config_elements * config, const char * username, json_t * search_criteria);
json_t * media_advanced_search(struct config_elements * config, const char * username, json_t * search_criteria);

// Browse category functions
json_t * media_category_get(struct config_elements * config, json_t * j_data_source, const char * level);
json_t * media_category_list(struct config_elements * config, json_t * j_data_source, const char * level, const char * category, int with_id);
json_t * media_subcategory_get(struct config_elements * config, json_t * j_data_source, const char * level, const char * category, const char * sublevel);
json_t * media_subcategory_list(struct config_elements * config, json_t * j_data_source, const char * level, const char * category, const char * sublevel, const char * subcategory, int with_id);
json_t * media_category_get_info(struct config_elements * config, json_t * j_data_source, const char * level, const char * category);
int      media_category_set_info(struct config_elements * config, json_t * j_data_source, const char * level, const char * category, json_t * j_info);
int      media_category_delete_info(struct config_elements * config, json_t * j_data_source, const char * level, const char * category);
json_t * is_media_category_info_valid(struct config_elements * config, json_t * j_info);
json_t * media_category_cover_get(struct config_elements * config, json_t * j_data_source, const char * level, const char * category, int thumbnail);

// Database Playlists functions
json_t   * playlist_list(struct config_elements * config, const char * username);
json_t   * playlist_list_webradio_startup(struct config_elements * config);
json_t   * playlist_get(struct config_elements * config, const char * username, const char * name, int with_id, size_t offset, size_t limit);
json_t   * playlist_get_by_id(struct config_elements * config, json_int_t tpl_id);
json_t   * is_playlist_valid(struct config_elements * config, const char * username, int is_admin, json_t * j_playlist, int update, int with_media);
json_t   * is_playlist_element_list_valid(struct config_elements * config, int is_admin, const char * username, json_t * j_element_list);
json_int_t playlist_add(struct config_elements * config, const char * username, json_t * j_playlist, struct _t_file_list * file_list);
int        playlist_set(struct config_elements * config, json_int_t tpl_id, json_t * j_playlist);
int        playlist_delete(struct config_elements * config, json_int_t tpl_id);
int        playlist_can_update(json_t * j_playlist, int is_admin);
int        playlist_add_media(struct config_elements * config, json_int_t tpl_id, json_t * media_list);
int        playlist_delete_media(struct config_elements * config, json_int_t tpl_id, json_t * media_list);
json_t *   playlist_has_media(struct config_elements * config, json_int_t tpl_id, json_t * media_list, size_t offset, size_t limit);
json_t   * playlist_media_cover_get(struct config_elements * config, const char * username, const char * name, int thumbnail);

// Libav functions
int open_input_file(const char *filename, AVFormatContext **input_format_context, AVCodecContext **input_codec_context, int type);
int init_resampler(AVCodecContext *input_codec_context,
                   AVCodecContext *output_codec_context,
                   AVAudioResampleContext **resample_context);
int read_decode_convert_and_store(AVAudioFifo *fifo,
                                  AVFormatContext *input_format_context,
                                  AVCodecContext *input_codec_context,
                                  AVCodecContext *output_codec_context,
                                  AVAudioResampleContext *resample_context,
                                  int *finished);
int load_encode_and_return(AVAudioFifo *fifo,
                           AVCodecContext *output_codec_context,
                           AVFormatContext * output_format_context,
                           int64_t * pts,
                           int * data_present);
int encode_audio_frame_and_return(AVFrame * frame,
                                  AVCodecContext * output_codec_context,
                                  AVFormatContext * output_format_context,
                                  int64_t * pts,
                                  int * data_present);
int init_output_jpeg_image(AVCodecContext ** thumbnail_cover_codec_context, int dst_width, int dst_height);
int resize_image(AVCodecContext * full_size_cover_codec_context, AVCodecContext * thumbnail_cover_codec_context, AVPacket * full_size_cover_packet, AVPacket * thumbnail_cover_packet, int dst_width, int dst_height);
int open_output_buffer_jukebox(struct _jukebox_audio_buffer * jukebox_audio_buffer, AVFormatContext ** output_format_context, AVCodecContext ** output_codec_context, AVAudioFifo ** fifo);
int open_input_buffer(const unsigned char * base64_buffer, AVFormatContext **image_format_context, AVCodecContext **image_codec_context, int * codec_index, int type);

/**
 * Ulfius callback functions
 */
int callback_taliesin_data_source_list (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_data_source_get (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_data_source_add (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_data_source_set (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_data_source_delete (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_data_source_refresh_run (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_data_source_refresh_status (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_data_source_refresh_stop (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_data_source_clean (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_taliesin_media_list_folder (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_media_get_path (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_category_get (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_category_list (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_category_get_info (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_category_set_info (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_category_delete_info (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_subcategory_get (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_subcategory_list (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_advanced_search (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_taliesin_playlist_list (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_playlist_get (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_playlist_add (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_playlist_set (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_playlist_delete (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_playlist_add_media (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_playlist_delete_media (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_playlist_has_media (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_playlist_load (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_taliesin_stream_media (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_stream_get_list (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_stream_manage (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_stream_manage_ws (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_stream_cover (const struct _u_request * request, struct _u_response * response, void * user_data);

void callback_websocket_stream_manager (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_user_data);
void callback_websocket_stream_onclose (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_user_data);
void callback_websocket_stream_incoming_message (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * last_message, void * websocket_user_data);

int callback_taliesin_search (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_taliesin_username_get_list (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_config_type_get (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_config_type_set (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_taliesin_check_access (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_check_admin_access (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_server_configuration (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_taliesin_options (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_default (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_clean (const struct _u_request * request, struct _u_response * response, void * user_data);

#endif
