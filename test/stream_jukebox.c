/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <check.h>
#include <ulfius.h>
#include <orcania.h>
#include <yder.h>

#include "unit-tests.h"

#define AUTH_SERVER_URI     "http://localhost:4593/api"
#define USER_LOGIN          "user1"
#define USER_PASSWORD       "MyUser1Password!"
#define USER_SCOPE_LIST     "taliesin"
#define TALIESIN_SERVER_URI "http://localhost:8576/api"

#define DATA_SOURCE_VALID         "dataSourceTest"
#define STREAM_DISPLAY_NAME_ORIG  "J.S. Bach: \"Open\" Goldberg Variations, BWV 988 (Piano)"
#define STREAM_DISPLAY_NAME_MODIF "Kimiko Ishizaka"
#define PLAYLIST_USER_VALID       "jukeboxTest"
#define PLAYLIST_SAVE_VALID       "saveTest"

struct _u_request user_req;
char * user_login = NULL, valid_stream_name[33] = {0};
int stream_res = 0;

static int get_stream_name() {
  struct _u_response resp;
  json_t * j_result;
  
  ulfius_init_response(&resp);
  
  o_free(user_req.http_url);
  user_req.http_url = o_strdup(TALIESIN_SERVER_URI "/stream/");
  if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
    j_result = ulfius_get_json_body_response(&resp, NULL);
    o_strcpy(valid_stream_name, "");
    o_strcpy(valid_stream_name, json_string_value(json_object_get(json_array_get(j_result, 0), "name")));
    json_decref(j_result);
    return strlen(valid_stream_name);
  } else {
    return 0;
  }
  
}

static size_t fony_reader(void * contents, size_t size, size_t nmemb, void * user_data) {
  return size*nmemb;
}

static void * fony_reader_thread(void * args) {
  struct _u_request req;
  struct _u_response resp;
  
  if (get_stream_name()) {
    ulfius_init_request(&req);
    ulfius_init_response(&resp);
    req.http_url = msprintf(TALIESIN_SERVER_URI "/stream/%s", valid_stream_name);
    ulfius_send_http_streaming_request(&req, &resp, &fony_reader, NULL);
    ulfius_clean_request(&req);
    ulfius_clean_response(&resp);
  }
  return NULL;
}

START_TEST(test_create_jukebox_ok)
{
  struct _u_response resp;
  json_t * j_result = NULL;
  
  ulfius_init_response(&resp);
  o_free(user_req.http_url);
  user_req.http_url = msprintf(TALIESIN_SERVER_URI "/data_source/" DATA_SOURCE_VALID "/browse/path/Kimiko Ishizaka/J.S. Bach: \"Open\" Goldberg Variations, BWV 988 (Piano)/?jukebox");
  
  if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
    j_result = ulfius_get_json_body_response(&resp, NULL);
  }
  
  ck_assert_int_eq(resp.status, 200);
  ck_assert_ptr_ne(json_string_value(json_object_get(j_result, "name")), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "webradio"), json_false());
  ck_assert_str_eq(json_string_value(json_object_get(j_result, "display_name")), "J.S. Bach: \"Open\" Goldberg Variations, BWV 988 (Piano)");

  ulfius_clean_response(&resp);
  json_decref(j_result);
}
END_TEST

START_TEST(test_create_jukebox_from_category_ok)
{
  struct _u_response resp;
  json_t * j_result = NULL;
  char * url;
  json_t * j_command;
  
  ulfius_init_response(&resp);
  o_free(user_req.http_url);
  user_req.http_url = msprintf(TALIESIN_SERVER_URI "/data_source/" DATA_SOURCE_VALID "/browse/category/artist/Kimiko Ishizaka/?jukebox");
  if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
    j_result = ulfius_get_json_body_response(&resp, NULL);
  }
  ck_assert_int_eq(resp.status, 200);
  ck_assert_ptr_ne(json_string_value(json_object_get(j_result, "name")), NULL);
  ck_assert_ptr_eq(json_object_get(j_result, "webradio"), json_false());
  ck_assert_str_eq(json_string_value(json_object_get(j_result, "display_name")), "Kimiko Ishizaka");
  url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", json_string_value(json_object_get(j_result, "name")));
  j_command = json_pack("{ss}", "command", "stop");
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
  ck_assert_int_eq(res, 1);
  ulfius_clean_response(&resp);
  o_free(url);
  json_decref(j_result);
  json_decref(j_command);
}
END_TEST

START_TEST(test_create_jukebox_from_category_not_found)
{
  char * url = msprintf(TALIESIN_SERVER_URI "/data_source/" DATA_SOURCE_VALID "/browse/category/artist/invalid/?jukebox");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_jukebox_path_error_not_found)
{
  char * url = msprintf(TALIESIN_SERVER_URI "/data_source/" DATA_SOURCE_VALID "/browse/path/invalid/?jukebox");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_jukebox_path_error_empty)
{
  char * url = msprintf(TALIESIN_SERVER_URI "/data_source/" DATA_SOURCE_VALID "/browse/path/Kimiko Ishizaka/?jukebox");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_jukebox_path_error_invalid_format_parameter)
{
  char * url = msprintf(TALIESIN_SERVER_URI "/data_source/" DATA_SOURCE_VALID "/browse/path/Kimiko Ishizaka/J.S. Bach: \"Open\" Goldberg Variations, BWV 988 (Piano)/?jukebox&format=invalid");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 400, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_jukebox_path_error_invalid_bitrate_parameter)
{
  char * url = msprintf(TALIESIN_SERVER_URI "/data_source/" DATA_SOURCE_VALID "/browse/path/Kimiko Ishizaka/J.S. Bach: \"Open\" Goldberg Variations, BWV 988 (Piano)/?jukebox&bitrate=42");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 400, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_jukebox_command_invalid)
{
  char * url;
  json_t * j_command = json_pack("{ss}", "command", "invalid");
  int res = 0;
  
  if (get_stream_name()) {
    url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 400, NULL, NULL, NULL);
  }
  
  ck_assert_int_eq(res, 1);

  json_decref(j_command);
}
END_TEST

START_TEST(test_jukebox_command_info_ok)
{
  struct _u_response resp;
  json_t * j_command = json_pack("{ss}", "command", "info"),  * j_result = NULL;
  
  ulfius_init_response(&resp);
  o_free(user_req.http_url);
  if (get_stream_name()) {
    user_req.http_url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    o_free(user_req.http_verb);
    user_req.http_verb = o_strdup("PUT");
    ulfius_set_json_body_request(&user_req, j_command);

    if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
      j_result = ulfius_get_json_body_response(&resp, NULL);
    }
  }
  
  ck_assert_int_eq(resp.status, 200);
  ck_assert_str_eq(STREAM_DISPLAY_NAME_ORIG, json_string_value(json_object_get(j_result, "display_name")));
  ck_assert_str_eq(valid_stream_name, json_string_value(json_object_get(j_result, "name")));
  ck_assert_ptr_eq(json_object_get(j_result, "webradio"), json_false());

  json_decref(j_command);
  json_decref(j_result);
  ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_jukebox_command_list_ok)
{
  struct _u_response resp;
  json_t * j_command = json_pack("{ss}", "command", "list"),  * j_result = NULL;
  
  ulfius_init_response(&resp);
  o_free(user_req.http_url);
  if (get_stream_name()) {
    user_req.http_url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    o_free(user_req.http_verb);
    user_req.http_verb = o_strdup("PUT");
    ulfius_set_json_body_request(&user_req, j_command);

    if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
      j_result = ulfius_get_json_body_response(&resp, NULL);
    }
  }
  
  ck_assert_int_eq(resp.status, 200);
  ck_assert_int_eq(json_array_size(j_result), 31);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "data_source")), DATA_SOURCE_VALID);
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "path")), "Kimiko Ishizaka/J.S. Bach: \"Open\" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 01 Aria.mp3");
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "name")), "Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 01 Aria.mp3");
  ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "type")), "audio");

  json_decref(j_command);
  json_decref(j_result);
  ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_jukebox_command_append_list_ok)
{
  if (get_stream_name()) {
    struct _u_response resp;
    char * url;
    json_t * j_command = json_pack("{sss[{ssss}]}", 
                                    "command", 
                                    "append_list", 
                                    "parameters", 
                                      "data_source", 
                                      DATA_SOURCE_VALID, 
                                      "path", 
                                      "fss/FreeSWSong.ogg");
    json_t * j_result = NULL;
    int res = 0;
    
    url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
    
    ck_assert_int_eq(res, 1);

    json_decref(j_command);

    j_command = json_pack("{ss}", "command", "list");
    
    ulfius_init_response(&resp);
    o_free(user_req.http_url);
    user_req.http_url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    o_free(user_req.http_verb);
    user_req.http_verb = o_strdup("PUT");
    ulfius_set_json_body_request(&user_req, j_command);

    if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
      j_result = ulfius_get_json_body_response(&resp, NULL);
    }
    
    ck_assert_int_eq(resp.status, 200);
    ck_assert_int_eq(json_array_size(j_result), 32);
    ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 31), "data_source")), DATA_SOURCE_VALID);
    ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 31), "path")), "fss/FreeSWSong.ogg");

    json_decref(j_command);
    json_decref(j_result);
    ulfius_clean_response(&resp);
  }
}
END_TEST

START_TEST(test_jukebox_command_remove_list_ok)
{
  if (get_stream_name()) {
    struct _u_response resp;
    char * url;
    json_t * j_command = json_pack("{sss{si}}", "command", "remove_list", "parameters", "index", 31),  * j_result = NULL;
    int res;
    
    url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
    
    ck_assert_int_eq(res, 1);

    json_decref(j_command);

    j_command = json_pack("{ss}", "command", "list");
    
    ulfius_init_response(&resp);
    o_free(user_req.http_url);
    user_req.http_url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    o_free(user_req.http_verb);
    user_req.http_verb = o_strdup("PUT");
    ulfius_set_json_body_request(&user_req, j_command);

    if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
      j_result = ulfius_get_json_body_response(&resp, NULL);
    }
    
    ck_assert_int_eq(resp.status, 200);
    ck_assert_int_eq(json_array_size(j_result), 31);

    json_decref(j_command);
    json_decref(j_result);
    ulfius_clean_response(&resp);
  }
}
END_TEST

START_TEST(test_jukebox_command_move_ok)
{
  if (get_stream_name()) {
    struct _u_response resp;
    char * url;
    json_t * j_command = json_pack("{sss{sisi}}", "command", "move", "parameters", "index", 0, "target", 3),  * j_result = NULL;
    int res;
    
    url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
    
    ck_assert_int_eq(res, 1);

    json_decref(j_command);

    j_command = json_pack("{ss}", "command", "list");
    
    ulfius_init_response(&resp);
    o_free(user_req.http_url);
    user_req.http_url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    o_free(user_req.http_verb);
    user_req.http_verb = o_strdup("PUT");
    ulfius_set_json_body_request(&user_req, j_command);

    if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
      j_result = ulfius_get_json_body_response(&resp, NULL);
    }
    
    ck_assert_int_eq(resp.status, 200);
    ck_assert_int_eq(json_array_size(j_result), 31);
    ck_assert_str_ne(json_string_value(json_object_get(json_array_get(j_result, 0), "path")), "Kimiko Ishizaka/J.S. Bach: \"Open\" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 01 Aria.mp3");
    ck_assert_str_ne(json_string_value(json_object_get(json_array_get(j_result, 3), "path")), "Kimiko Ishizaka/J.S. Bach: \"Open\" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 01 Aria.mp3");

    json_decref(j_command);
    json_decref(j_result);
    ulfius_clean_response(&resp);
  }
}
END_TEST

START_TEST(test_jukebox_command_rename_ok)
{
  if (get_stream_name()) {
    struct _u_response resp;
    char * url;
    json_t * j_command = json_pack("{sss{ss}}", "command", "rename", "parameters", "name", STREAM_DISPLAY_NAME_MODIF),  * j_result = NULL;
    int res;
    
    url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
    
    ck_assert_int_eq(res, 1);

    json_decref(j_command);

    j_command = json_pack("{ss}", "command", "info");
    
    ulfius_init_response(&resp);
    o_free(user_req.http_url);
    user_req.http_url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    o_free(user_req.http_verb);
    user_req.http_verb = o_strdup("PUT");
    ulfius_set_json_body_request(&user_req, j_command);

    if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
      j_result = ulfius_get_json_body_response(&resp, NULL);
    }
    
    ck_assert_int_eq(resp.status, 200);
    ck_assert_str_eq(STREAM_DISPLAY_NAME_MODIF, json_string_value(json_object_get(j_result, "display_name")));

    json_decref(j_command);
    json_decref(j_result);
    ulfius_clean_response(&resp);
  }
}
END_TEST

START_TEST(test_jukebox_command_reset_url_ok)
{
  char old_stream_name[33] = {0};
  if (get_stream_name()) {
    strcpy(old_stream_name, valid_stream_name);
    char * url;
    json_t * j_command = json_pack("{ss}", "command", "reset_url");
    int res;
    
    url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
    
    ck_assert_int_eq(res, 1);

    json_decref(j_command);
    ck_assert_int_ne(get_stream_name(), 0);
    
    ck_assert_str_ne(old_stream_name, valid_stream_name);
  }
}
END_TEST

START_TEST(test_jukebox_command_save_as_playlist_ok)
{
  if (get_stream_name()) {
    char * url;
    json_t * j_command = json_pack("{sss{ssss}}", "command", "save", "parameters", "name", PLAYLIST_SAVE_VALID, "description", "description for "PLAYLIST_SAVE_VALID);
    int res;
    
    url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
    
    ck_assert_int_eq(res, 1);

    json_decref(j_command);
  }
}
END_TEST

START_TEST(test_jukebox_get_playlist_saved_ok)
{
  if (get_stream_name()) {
    char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_SAVE_VALID);
    
    int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
    free(url);
    ck_assert_int_eq(res, 1);
  }
}
END_TEST

START_TEST(test_jukebox_delete_playlist_saved_ok)
{
  if (get_stream_name()) {
    char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_SAVE_VALID);
    
    int res = run_simple_authenticated_test(&user_req, "DELETE", url, NULL, NULL, 200, NULL, NULL, NULL);
    free(url);
    ck_assert_int_eq(res, 1);
  }
}
END_TEST

START_TEST(test_jukebox_get_m3u)
{
  if (get_stream_name()) {
    char * url = msprintf(TALIESIN_SERVER_URI "/stream/%s", valid_stream_name);
    
    int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, "#EXTINF:", NULL);
    free(url);
    ck_assert_int_eq(res, 1);
  }
}
END_TEST

START_TEST(test_jukebox_play_ok)
{
  if (get_stream_name()) {
    int ret_thread, ret_detach;
    pthread_t thread;
    
    ret_thread = pthread_create(&thread, NULL, fony_reader_thread, NULL);
    ret_detach = pthread_detach(thread);
    ck_assert_int_eq(ret_thread, 0);
    ck_assert_int_eq(ret_detach, 0);
  }
}
END_TEST

START_TEST(test_jukebox_play_index_not_found)
{
  if (get_stream_name()) {
    char * url = msprintf(TALIESIN_SERVER_URI "/stream/%s?index=42", valid_stream_name);
    
    int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
    free(url);
    ck_assert_int_eq(res, 1);
  }
}
END_TEST

START_TEST(test_jukebox_play_not_found)
{
  if (get_stream_name()) {
    char * url = msprintf(TALIESIN_SERVER_URI "/stream/invalid?index=0");
    
    int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
    free(url);
    ck_assert_int_eq(res, 1);
  }
}
END_TEST

START_TEST(test_create_playlist_ok)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{ss ss ss s[{ssss}{ssss}]}",
                                  "name", PLAYLIST_USER_VALID,
                                  "description", "description for "PLAYLIST_USER_VALID,
                                  "scope", "me",
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");

  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_jukebox_command_attach_playlist_ok)
{
  if (get_stream_name()) {
    struct _u_response resp;
    char * url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    json_t * j_command = json_pack("{sss{ss}}", "command", "attach_playlist", "parameters", "name", PLAYLIST_USER_VALID), * j_result = NULL;
    
    int res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
    
    ck_assert_int_eq(res, 1);

    json_decref(j_command);

    j_command = json_pack("{ss}", "command", "info");
    
    ulfius_init_response(&resp);
    o_free(user_req.http_url);
    user_req.http_url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    o_free(user_req.http_verb);
    user_req.http_verb = o_strdup("PUT");
    ulfius_set_json_body_request(&user_req, j_command);

    if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
      j_result = ulfius_get_json_body_response(&resp, NULL);
    }
    
    ck_assert_int_eq(resp.status, 200);
    ck_assert_str_eq(PLAYLIST_USER_VALID, json_string_value(json_object_get(j_result, "stored_playlist")));

    json_decref(j_command);
    json_decref(j_result);
    ulfius_clean_response(&resp);
  }
}
END_TEST

START_TEST(test_jukebox_command_reload_playlist_ok)
{
  if (get_stream_name()) {
    struct _u_response resp;
    char * url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    json_t * j_command = json_pack("{ss}", "command", "reload"),  * j_result = NULL;
    
    int res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
    
    ck_assert_int_eq(res, 1);

    json_decref(j_command);

    j_command = json_pack("{ss}", "command", "list");
    
    ulfius_init_response(&resp);
    o_free(user_req.http_url);
    user_req.http_url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    o_free(user_req.http_verb);
    user_req.http_verb = o_strdup("PUT");
    ulfius_set_json_body_request(&user_req, j_command);

    if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
      j_result = ulfius_get_json_body_response(&resp, NULL);
    }
    
    ck_assert_int_eq(resp.status, 200);
    ck_assert_int_eq(json_array_size(j_result), 2);
    ck_assert_str_eq(json_string_value(json_object_get(json_array_get(j_result, 0), "path")), "fss/free-software-song.ogg");

    json_decref(j_command);
    json_decref(j_result);
    ulfius_clean_response(&resp);
  }
}
END_TEST

START_TEST(test_get_playlist_ok)
{
  if (get_stream_name()) {
    char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
    json_t * j_jukebox = json_pack("{sssssssis[{ssss}{ssss}]s[{ssssso}]}",
                                    "name", PLAYLIST_USER_VALID,
                                    "description", "description for "PLAYLIST_USER_VALID,
                                    "scope", "me",
                                    "elements", 2,
                                    "media",
                                      "data_source", DATA_SOURCE_VALID,
                                      "path", "fss/free-software-song.ogg",
                                      "data_source", DATA_SOURCE_VALID,
                                      "path", "fss/FreeSWSong.ogg",
                                    "stream",
                                      "name", valid_stream_name,
                                      "display_name", STREAM_DISPLAY_NAME_MODIF,
                                      "webradio", json_false());
    
    int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_jukebox, NULL, NULL);
    free(url);
    json_decref(j_jukebox);
    ck_assert_int_eq(res, 1);
  }
}
END_TEST

START_TEST(test_delete_playlist_ok)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "DELETE", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_jukebox_command_delete_playlist_ok)
{
  if (get_stream_name()) {
    struct _u_response resp;
    char * url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", valid_stream_name);
    json_t * j_command = json_pack("{ss}", "command", "stop"),  * j_result = NULL;
    
    int res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
    
    ck_assert_int_eq(res, 1);
    
    sleep(2);

    json_decref(j_command);

    ulfius_init_response(&resp);
    o_free(user_req.http_url);
    user_req.http_url = msprintf(TALIESIN_SERVER_URI "/stream/", valid_stream_name);
    o_free(user_req.http_verb);
    user_req.http_verb = o_strdup("PUT");

    if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
      j_result = ulfius_get_json_body_response(&resp, NULL);
    }
    
    ck_assert_int_eq(resp.status, 200);
    ck_assert_int_eq(json_array_size(j_result), 0);

    json_decref(j_result);
    ulfius_clean_response(&resp);
  }
}
END_TEST

START_TEST(test_stream_list_empty)
{
  char * url = msprintf("%s/stream", TALIESIN_SERVER_URI);
  json_t * j_result = json_array();
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_result, NULL, NULL);
  free(url);
  json_decref(j_result);
  ck_assert_int_eq(res, 1);
}
END_TEST

static Suite *taliesin_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Taliesin media server Stream Playlist tests");
  tc_core = tcase_create("test_stream_jukebox");
  tcase_add_test(tc_core, test_create_jukebox_ok);
  tcase_add_test(tc_core, test_create_jukebox_from_category_ok);
  tcase_add_test(tc_core, test_create_jukebox_from_category_not_found);
  tcase_add_test(tc_core, test_create_jukebox_path_error_not_found);
  tcase_add_test(tc_core, test_create_jukebox_path_error_empty);
  tcase_add_test(tc_core, test_create_jukebox_path_error_invalid_format_parameter);
  tcase_add_test(tc_core, test_create_jukebox_path_error_invalid_bitrate_parameter);
  tcase_add_test(tc_core, test_jukebox_command_invalid);
  tcase_add_test(tc_core, test_jukebox_command_info_ok);
  tcase_add_test(tc_core, test_jukebox_command_list_ok);
  tcase_add_test(tc_core, test_jukebox_command_append_list_ok);
  tcase_add_test(tc_core, test_jukebox_command_remove_list_ok);
  tcase_add_test(tc_core, test_jukebox_command_move_ok);
  tcase_add_test(tc_core, test_jukebox_command_rename_ok);
  tcase_add_test(tc_core, test_jukebox_command_reset_url_ok);
  tcase_add_test(tc_core, test_jukebox_command_save_as_playlist_ok);
  tcase_add_test(tc_core, test_jukebox_get_playlist_saved_ok);
  tcase_add_test(tc_core, test_jukebox_delete_playlist_saved_ok);
  tcase_add_test(tc_core, test_jukebox_get_m3u);
  tcase_add_test(tc_core, test_jukebox_play_ok);
  tcase_add_test(tc_core, test_jukebox_play_not_found);
  tcase_add_test(tc_core, test_jukebox_play_index_not_found);
  tcase_add_test(tc_core, test_create_playlist_ok);
  tcase_add_test(tc_core, test_jukebox_command_attach_playlist_ok);
  tcase_add_test(tc_core, test_jukebox_command_reload_playlist_ok);
  tcase_add_test(tc_core, test_get_playlist_ok);
  tcase_add_test(tc_core, test_delete_playlist_ok);
  tcase_add_test(tc_core, test_jukebox_command_delete_playlist_ok);
  tcase_add_test(tc_core, test_stream_list_empty);
  tcase_set_timeout(tc_core, 30);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  struct _u_request auth_req;
  struct _u_response auth_resp;
  int res;
  
  y_init_logs("Taliesin test", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Taliesin test");
  
  // Getting a refresh_token
  ulfius_init_request(&auth_req);
  ulfius_init_request(&user_req);
  ulfius_init_response(&auth_resp);
  auth_req.http_verb = strdup("POST");
  auth_req.http_url = msprintf("%s/token/", argc>7?argv[7]:AUTH_SERVER_URI);
  u_map_put(auth_req.map_post_body, "grant_type", "password");
  user_login = argc>1?argv[1]:USER_LOGIN;
  u_map_put(auth_req.map_post_body, "username", user_login);
  u_map_put(auth_req.map_post_body, "password", argc>2?argv[2]:USER_PASSWORD);
  u_map_put(auth_req.map_post_body, "scope", argc>3?argv[3]:USER_SCOPE_LIST);
  res = ulfius_send_http_request(&auth_req, &auth_resp);
  if (res == U_OK && auth_resp.status == 200) {
    json_t * json_body = ulfius_get_json_body_response(&auth_resp, NULL);
    char * bearer_token = msprintf("Bearer %s", (json_string_value(json_object_get(json_body, "access_token"))));
    y_log_message(Y_LOG_LEVEL_INFO, "User %s authenticated", USER_LOGIN);
    u_map_put(user_req.map_header, "Authorization", bearer_token);
    free(bearer_token);
    json_decref(json_body);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error authentication user %s", argc>1?argv[1]:USER_LOGIN);
  }
  ulfius_clean_request(&auth_req);
  ulfius_clean_response(&auth_resp);
  
  s = taliesin_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  ulfius_clean_request(&user_req);
  
  y_close_logs();
  
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
