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

struct _u_request user_req, admin_req;
char * user_login = NULL, * admin_login = NULL, * data_source_path;

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

START_TEST(test_create_data_source_user_ok)
{
  char * url = msprintf("%s/data_source/", TALIESIN_SERVER_URI);
	json_t * j_data_source = json_pack("{ssssssssss}",
																		 "name", DATA_SOURCE_VALID,
																		 "description", "description for " DATA_SOURCE_VALID,
																		 "scope", "me",
																		 "path", data_source_path,
																		 "icon", "testIcon");
	
  int res = run_simple_authenticated_test(&admin_req, "POST", url, j_data_source, NULL, 200, NULL, NULL, NULL);
  free(url);
	json_decref(j_data_source);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_username_list_ok)
{
  char * url = TALIESIN_SERVER_URI "/users";
  json_t * j_body = json_pack("{ss}", "username", ADMIN_LOGIN);
  
  int res = run_simple_authenticated_test(&admin_req, "GET", url, NULL, NULL, 200, j_body, NULL, NULL);
  json_decref(j_body);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_username_list_unauthorized)
{
  char * url = TALIESIN_SERVER_URI "/users";
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 401, NULL, NULL, NULL);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_delete_data_source_user_ok)
{
  char * url = msprintf("%s/data_source/%s", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
	
  int res = run_simple_authenticated_test(&admin_req, "DELETE", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
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
	tcase_add_test(tc_core, test_create_data_source_user_ok);
	tcase_add_test(tc_core, test_get_username_list_ok);
	tcase_add_test(tc_core, test_get_username_list_unauthorized);
	tcase_add_test(tc_core, test_delete_data_source_user_ok);
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
  
  data_source_path = argc>2?argv[2]:DATA_SOURCE_PATH;
  
  y_init_logs("Taliesin test", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Taliesin test");
  
  if (argv[1] != NULL) {
    str_jwks = read_file(argv[1]);
    
    // Generate user and admin access tokens
    ulfius_init_request(&user_req);
    ulfius_init_request(&admin_req);
    r_jwt_init(&jwt);
    r_jwt_set_header_str_value(jwt, "typ", "at+jwt");
    r_jwks_init(&jwks);
    r_jwks_import_from_str(jwks, str_jwks);
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
    
    json_object_set_new(j_claims, "scope", json_string("taliesin taliesin_admin"));
    json_object_set_new(j_claims, "sub", json_string(ADMIN_LOGIN));
    r_jwt_set_full_claims_json_t(jwt, j_claims);
    token = r_jwt_serialize_signed(jwt, NULL, 0);
    bearer_token = msprintf("Bearer %s", token);
    u_map_put(admin_req.map_header, "Authorization", bearer_token);
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
    ulfius_clean_request(&admin_req);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error, no jwks file path specified");
    number_failed = 1;
  }
  
  y_close_logs();
  
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
