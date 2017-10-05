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
#define ADMIN_LOGIN         "admin"
#define ADMIN_PASSWORD      "MyAdminPassword2016!"
#define ADMIN_SCOPE_LIST    "taliesin taliesin_admin"
#define TALIESIN_SERVER_URI "http://localhost:8576/api"

#define DATA_SOURCE_VALID       "dataSourceTest"
#define DATA_SOURCE_ADM_VALID   "dataSourceTestAdm"
#define DATA_SOURCE_ALL_VALID   "dataSourceTestAll"
#define DATA_SOURCE_INVALID     "dataSourceTestInvalid"

struct _u_request user_req, admin_req;
char * user_login = NULL, * admin_login = NULL;

START_TEST(test_get_config_user_audio_extension_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/audio_file_extension";
  json_t * j_body = json_pack("[ssssssssssssss]", ".mp3", ".m4a", ".aac", ".ogg", ".oga", ".flac", ".wav", ".wma", ".aif", ".aiff", ".ape", ".mpc", ".shn", ".au");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_user_video_extension_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/video_file_extension";
  json_t * j_body = json_pack("[sssssssssss]", ".avi", ".mpg", ".mpeg", ".mp4", ".m4v", ".mov", ".wmv", ".ogv", ".divx", ".m2ts", ".mkv");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_user_subtitle_extension_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/subtitle_file_extension";
  json_t * j_body = json_pack("[sssss]", ".srt", ".ssa", ".vtt", ".sub", ".sbv");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_user_image_extension_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/image_file_extension";
  json_t * j_body = json_pack("[sssss]", ".jpg", ".jpeg", ".png", ".gif", ".bmp");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_user_cover_file_pattern_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/cover_file_pattern";
  json_t * j_body = json_pack("[ssssssssss]", "folder.jpg", "folder.jpeg", "cover.jpg", "cover.jpeg", "front.jpg", "front.jpeg", "*.jpg", "*.jpeg", "*.gif", "*.png");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_user_not_found)
{
  char * url = TALIESIN_SERVER_URI "/config/invalid";
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_admin_audio_extension_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/audio_file_extension";
  json_t * j_body = json_pack("[ssssssssssssss]", ".mp3", ".m4a", ".aac", ".ogg", ".oga", ".flac", ".wav", ".wma", ".aif", ".aiff", ".ape", ".mpc", ".shn", ".au");
  
  int res = run_simple_authenticated_test(&admin_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_admin_video_extension_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/video_file_extension";
  json_t * j_body = json_pack("[sssssssssss]", ".avi", ".mpg", ".mpeg", ".mp4", ".m4v", ".mov", ".wmv", ".ogv", ".divx", ".m2ts", ".mkv");
  
  int res = run_simple_authenticated_test(&admin_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_admin_subtitle_extension_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/subtitle_file_extension";
  json_t * j_body = json_pack("[sssss]", ".srt", ".ssa", ".vtt", ".sub", ".sbv");
  
  int res = run_simple_authenticated_test(&admin_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_admin_image_extension_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/image_file_extension";
  json_t * j_body = json_pack("[sssss]", ".jpg", ".jpeg", ".png", ".gif", ".bmp");
  
  int res = run_simple_authenticated_test(&admin_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_admin_cover_file_pattern_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/cover_file_pattern";
  json_t * j_body = json_pack("[ssssssssss]", "folder.jpg", "folder.jpeg", "cover.jpg", "cover.jpeg", "front.jpg", "front.jpeg", "*.jpg", "*.jpeg", "*.gif", "*.png");
  
  int res = run_simple_authenticated_test(&admin_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_admin_not_found)
{
  char * url = TALIESIN_SERVER_URI "/config/invalid";
  
  int res = run_simple_authenticated_test(&admin_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_config_user_unauthorized)
{
  char * url = TALIESIN_SERVER_URI "/config/audio_file_extension";
  json_t * j_body = json_pack("[sssssssssssssss]", ".mp3", ".m4a", ".aac", ".ogg", ".oga", ".flac", ".wav", ".wma", ".aif", ".aiff", ".ape", ".mpc", ".shn", ".au", ".ra");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_body, NULL, 401, NULL, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_config_admin_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/audio_file_extension";
  json_t * j_body = json_pack("[sssssssssssssss]", ".mp3", ".m4a", ".aac", ".ogg", ".oga", ".flac", ".wav", ".wma", ".aif", ".aiff", ".ape", ".mpc", ".shn", ".au", ".ra");
  
  int res = run_simple_authenticated_test(&admin_req, "PUT", url, j_body, NULL, 200, NULL, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_config_admin_invalid_parameters)
{
  char * url = TALIESIN_SERVER_URI "/config/audio_file_extension";
  json_t * j_body = json_pack("[sssssssssssssssi]", ".mp3", ".m4a", ".aac", ".ogg", ".oga", ".flac", ".wav", ".wma", ".aif", ".aiff", ".ape", ".mpc", ".shn", ".au", ".ra", 42);
  
  int res = run_simple_authenticated_test(&admin_req, "PUT", url, j_body, NULL, 400, NULL, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_config_user_updated_audio_extension_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/audio_file_extension";
  json_t * j_body = json_pack("[sssssssssssssss]", ".mp3", ".m4a", ".aac", ".ogg", ".oga", ".flac", ".wav", ".wma", ".aif", ".aiff", ".ape", ".mpc", ".shn", ".au", ".ra");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_config_admin_invalid_category)
{
  char * url = TALIESIN_SERVER_URI "/config/invalid";
  json_t * j_body = json_pack("[sssssssssssssss]", ".mp3", ".m4a", ".aac", ".ogg", ".oga", ".flac", ".wav", ".wma", ".aif", ".aiff", ".ape", ".mpc", ".shn", ".au", ".ra");
  
  int res = run_simple_authenticated_test(&admin_req, "PUT", url, j_body, NULL, 404, NULL, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_config_admin_original_ok)
{
  char * url = TALIESIN_SERVER_URI "/config/audio_file_extension";
  json_t * j_body = json_pack("[ssssssssssssss]", ".mp3", ".m4a", ".aac", ".ogg", ".oga", ".flac", ".wav", ".wma", ".aif", ".aiff", ".ape", ".mpc", ".shn", ".au");
  
  int res = run_simple_authenticated_test(&admin_req, "PUT", url, j_body, NULL, 200, NULL, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

static Suite *taliesin_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Taliesin media server Configuration API");
	tc_core = tcase_create("test_config_endpoints");
	tcase_add_test(tc_core, test_get_config_user_audio_extension_ok);
	tcase_add_test(tc_core, test_get_config_user_video_extension_ok);
	tcase_add_test(tc_core, test_get_config_user_subtitle_extension_ok);
	tcase_add_test(tc_core, test_get_config_user_image_extension_ok);
	tcase_add_test(tc_core, test_get_config_user_cover_file_pattern_ok);
	tcase_add_test(tc_core, test_get_config_user_not_found);
	tcase_add_test(tc_core, test_get_config_admin_audio_extension_ok);
	tcase_add_test(tc_core, test_get_config_admin_video_extension_ok);
	tcase_add_test(tc_core, test_get_config_admin_subtitle_extension_ok);
	tcase_add_test(tc_core, test_get_config_admin_image_extension_ok);
	tcase_add_test(tc_core, test_get_config_admin_cover_file_pattern_ok);
	tcase_add_test(tc_core, test_get_config_admin_not_found);
	tcase_add_test(tc_core, test_set_config_user_unauthorized);
	tcase_add_test(tc_core, test_set_config_admin_ok);
	tcase_add_test(tc_core, test_set_config_admin_invalid_parameters);
	tcase_add_test(tc_core, test_get_config_user_updated_audio_extension_ok);
	tcase_add_test(tc_core, test_set_config_admin_invalid_category);
	tcase_add_test(tc_core, test_set_config_admin_original_ok);
	tcase_add_test(tc_core, test_get_config_user_audio_extension_ok);
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
	
  ulfius_init_request(&auth_req);
  ulfius_init_request(&admin_req);
  ulfius_init_response(&auth_resp);
  auth_req.http_verb = strdup("POST");
  auth_req.http_url = msprintf("%s/token/", argc>7?argv[7]:AUTH_SERVER_URI);
  u_map_put(auth_req.map_post_body, "grant_type", "password");
	admin_login = argc>4?argv[4]:ADMIN_LOGIN;
  u_map_put(auth_req.map_post_body, "username", admin_login);
  u_map_put(auth_req.map_post_body, "password", argc>5?argv[5]:ADMIN_PASSWORD);
  u_map_put(auth_req.map_post_body, "scope", argc>6?argv[6]:ADMIN_SCOPE_LIST);
  res = ulfius_send_http_request(&auth_req, &auth_resp);
  if (res == U_OK && auth_resp.status == 200) {
    json_t * json_body = ulfius_get_json_body_response(&auth_resp, NULL);
    char * bearer_token = msprintf("Bearer %s", (json_string_value(json_object_get(json_body, "access_token"))));
    y_log_message(Y_LOG_LEVEL_INFO, "User %s authenticated", ADMIN_LOGIN);
    u_map_put(admin_req.map_header, "Authorization", bearer_token);
    free(bearer_token);
    json_decref(json_body);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error authentication user %s", argc>4?argv[4]:ADMIN_LOGIN);
  }
  ulfius_clean_request(&auth_req);
  ulfius_clean_response(&auth_resp);

	s = taliesin_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	
  ulfius_clean_request(&user_req);
  ulfius_clean_request(&admin_req);
  
  y_close_logs();
  
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
