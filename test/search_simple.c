/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <check.h>
#include <orcania.h>
#include <yder.h>
#include <ulfius.h>
#include <rhonabwy.h>

#include "unit-tests.h"

#define USER_LOGIN          "dev"
#define ADMIN_LOGIN         "admin"
#define TALIESIN_SERVER_URI "http://localhost:8576/api"
#define DATA_SOURCE_VALID   "dataSourceTest"
#define DATA_SOURCE_PATH    "/tmp/media"

#define DATA_SOURCE_VALID     "dataSourceTest"
#define WEBRADIO_DISPLAY_NAME "short"
#define PLAYLIST_DISPLAY_NAME "Kimiko Ishizaka"
#define PLAYLIST_USER_VALID   "playlistTest"

// TODO
struct _u_request user_req;
char * user_login = NULL;

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

static char * get_stream_name(const char * display_name) {
  struct _u_response resp;
  json_t * j_result, * j_element;
	char * name = NULL;
	size_t index;
  
  ulfius_init_response(&resp);
  
  o_free(user_req.http_url);
  user_req.http_url = o_strdup(TALIESIN_SERVER_URI "/stream/");
  if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
    j_result = ulfius_get_json_body_response(&resp, NULL);
		json_array_foreach(j_result, index, j_element) {
			if (0 == o_strcmp(display_name, json_string_value(json_object_get(j_element, "display_name")))) {
				name = o_strdup(json_string_value(json_object_get(j_element, "name")));
				break;
			}
		}
    json_decref(j_result);
  }
  return name;
}

START_TEST(test_create_webradio_ok)
{
	int res = run_simple_authenticated_test(&user_req, "GET", TALIESIN_SERVER_URI "/data_source/" DATA_SOURCE_VALID "/browse/path/" WEBRADIO_DISPLAY_NAME "/?webradio", NULL, NULL, 200, NULL, NULL, NULL);
	
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_jukebox_ok)
{
	int res = run_simple_authenticated_test(&user_req, "GET", TALIESIN_SERVER_URI "/data_source/" DATA_SOURCE_VALID "/browse/path/" PLAYLIST_DISPLAY_NAME "/?jukebox&recursive", NULL, NULL, 200, NULL, NULL, NULL);
	
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_search_simple_ok)
{
	int res = run_simple_authenticated_test(&user_req, "GET", TALIESIN_SERVER_URI "/search/?q=Kimiko", NULL, NULL, 200, NULL, NULL, NULL);
	
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_search_simple_empty)
{
	struct _u_response resp;
	json_t * j_result = NULL;
	
	ulfius_init_response(&resp);
	o_free(user_req.http_url);
	user_req.http_url = msprintf(TALIESIN_SERVER_URI "/search/?q=invalid");

	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_ptr_eq(json_object_get(j_result, "playlist"), NULL);
	ck_assert_ptr_eq(json_object_get(j_result, "stream"), NULL);
	ck_assert_ptr_eq(json_object_get(j_result, "folder"), NULL);
	ck_assert_ptr_eq(json_object_get(j_result, "media"), NULL);

	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_search_simple_media)
{
	struct _u_response resp;
	json_t * j_result = NULL;
	
	ulfius_init_response(&resp);
	o_free(user_req.http_url);
	user_req.http_url = msprintf(TALIESIN_SERVER_URI "/search/?q=free-software-song");

	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);

	ck_assert_int_gt(json_array_size(json_object_get(j_result, "media")), 0);

	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_search_simple_folder)
{
	struct _u_response resp;
	json_t * j_result = NULL;
	
	ulfius_init_response(&resp);
	o_free(user_req.http_url);
	user_req.http_url = msprintf(TALIESIN_SERVER_URI "/search/?q=Kimiko");

	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(json_object_get(j_result, "folder")), 0);

	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_search_simple_stream)
{
	struct _u_response resp;
	json_t * j_result = NULL;
	
	ulfius_init_response(&resp);
	o_free(user_req.http_url);
	user_req.http_url = msprintf(TALIESIN_SERVER_URI "/search/?q=%s", WEBRADIO_DISPLAY_NAME);

	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(json_object_get(j_result, "stream")), 0);

	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_search_simple_playlist)
{
	struct _u_response resp;
	json_t * j_result = NULL;
	
	ulfius_init_response(&resp);
	o_free(user_req.http_url);
	user_req.http_url = msprintf(TALIESIN_SERVER_URI "/search/?q=%s", PLAYLIST_USER_VALID);

	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(json_object_get(j_result, "playlist")), 0);

	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_search_simple_filter_category_ok)
{
	struct _u_response resp;
	json_t * j_result = NULL;
	
	ulfius_init_response(&resp);
	o_free(user_req.http_url);
	user_req.http_url = msprintf(TALIESIN_SERVER_URI "/search/?q=free&category=file");

	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_eq(json_array_size(json_object_get(j_result, "playlist")), 0);
	ck_assert_int_eq(json_array_size(json_object_get(j_result, "stream")), 0);
	ck_assert_int_eq(json_array_size(json_object_get(j_result, "folder")), 0);
	ck_assert_int_gt(json_array_size(json_object_get(j_result, "media")), 0);

	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_search_simple_filter_category_error)
{
	ck_assert_int_eq(run_simple_authenticated_test(&user_req, "GET", TALIESIN_SERVER_URI "/search/?q=free&category=invalid", NULL, NULL, 400, NULL, NULL, NULL), 1);
}
END_TEST

START_TEST(test_delete_webradio_ok)
{
	char * name = get_stream_name(WEBRADIO_DISPLAY_NAME), * url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", name);
	json_t * j_command = json_pack("{ss}", "command", "stop");
	
	int res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
	
	ck_assert_int_eq(res, 1);
	o_free(name);
	o_free(url);
}
END_TEST

START_TEST(test_delete_jukebox_ok)
{
	char * name = get_stream_name(PLAYLIST_DISPLAY_NAME), * url = msprintf(TALIESIN_SERVER_URI "/stream/%s/manage", name);
	json_t * j_command = json_pack("{ss}", "command", "stop");
	
	int res = run_simple_authenticated_test(&user_req, "PUT", url, j_command, NULL, 200, NULL, NULL, NULL);
	
	ck_assert_int_eq(res, 1);
	o_free(name);
	o_free(url);
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

static Suite *taliesin_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Taliesin media server Simple Search");
	tc_core = tcase_create("test_simple_search");
	tcase_add_test(tc_core, test_create_playlist_ok);
	tcase_add_test(tc_core, test_create_webradio_ok);
	tcase_add_test(tc_core, test_create_jukebox_ok);
	tcase_add_test(tc_core, test_search_simple_ok);
	tcase_add_test(tc_core, test_search_simple_empty);
	tcase_add_test(tc_core, test_search_simple_media);
	tcase_add_test(tc_core, test_search_simple_folder);
	tcase_add_test(tc_core, test_search_simple_stream);
	tcase_add_test(tc_core, test_search_simple_playlist);
	tcase_add_test(tc_core, test_search_simple_filter_category_ok);
	tcase_add_test(tc_core, test_search_simple_filter_category_error);
	tcase_add_test(tc_core, test_delete_webradio_ok);
	tcase_add_test(tc_core, test_delete_jukebox_ok);
	tcase_add_test(tc_core, test_delete_playlist_ok);
	tcase_set_timeout(tc_core, 30);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  jwt_t * jwt;
  jwks_t * jwks;
  char * str_jwks, * token, * bearer_token;
  json_t * j_claims;
  time_t now;
  
  y_init_logs("Taliesin test", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Taliesin test");
  
  if (argv[1] != NULL) {
    str_jwks = read_file(argv[1]);
    
    // Generate user and admin access tokens
    ulfius_init_request(&user_req);
    r_jwt_init(&jwt);
    r_jwt_set_header_str_value(jwt, "typ", "at+jwt");
    r_jwks_init(&jwks);
    r_jwks_import_from_json_str(jwks, str_jwks);
    r_jwt_add_sign_jwks(jwt, jwks, NULL);
    o_free(str_jwks);
    
    time(&now);
    j_claims = json_pack("{ss ss ss ss ss si si si ss}",
                         "iss", "https://glewlwyd.tld/",
                         "sub", USER_LOGIN,
                         "client_id", "client",
                         "jti", "abcdxyz1234",
                         "type", "access_token",
                         "iat", now,
                         "exp", now+3600,
                         "nbf", now,
                         "scope", "taliesin");
    r_jwt_set_full_claims_json_t(jwt, j_claims);
    token = r_jwt_serialize_signed(jwt, NULL, 0);
    bearer_token = msprintf("Bearer %s", token);
    u_map_put(user_req.map_header, "Authorization", bearer_token);
    o_free(bearer_token);
    o_free(token);
    
    json_decref(j_claims);
    r_jwt_free(jwt);
    r_jwks_free(jwks);
    
    s = taliesin_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    ulfius_clean_request(&user_req);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error, no jwks file path specified");
    number_failed = 1;
  }
  
  y_close_logs();
  
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
