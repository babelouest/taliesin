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

#define DATA_SOURCE_VALID       "dataSourceTest"
#define SUBPATH "fss"

struct _u_request user_req;
char * user_login = NULL;

START_TEST(test_broswe_path_root)
{
  char * url = msprintf("%s/data_source/%s/browse/path/", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_browse_check_path_ok)
{
	struct _u_response resp;
	json_t * j_result = NULL, * j_expect;
	
	ulfius_init_response(&resp);
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/data_source/%s/browse/path/fss", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_gt(json_array_size(j_result), 0);
	j_expect = json_pack("{ss}", "name", "FreeSWSong.ogg");
  ck_assert_ptr_ne(json_search(j_result, j_expect), NULL);
	json_decref(j_expect);
	json_decref(j_result);
}
END_TEST

START_TEST(test_browse_check_subpath_ok)
{
	struct _u_response resp;
	json_t * j_result = NULL, * j_expect;
	
	ulfius_init_response(&resp);
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/data_source/%s/browse/path/%s", TALIESIN_SERVER_URI, DATA_SOURCE_VALID, SUBPATH);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_gt(json_array_size(j_result), 0);
	j_expect = json_pack("{ss}", "name", "FreeSWSong.ogg");
  ck_assert_ptr_ne(json_search(j_result, j_expect), NULL);
	json_decref(j_expect);
	json_decref(j_result);
}
END_TEST

START_TEST(test_browse_check_subpath_folder_not_found)
{
  char * url = msprintf("%s/data_source/%s/browse/path/%s", TALIESIN_SERVER_URI, DATA_SOURCE_VALID, "invalid");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_browse_check_path_not_found)
{
  char * url = msprintf("%s/data_source/%s/browse/path/%s", TALIESIN_SERVER_URI, DATA_SOURCE_VALID, "invalid.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_browse_check_path_folder_cover_ok)
{
  char * url = msprintf("%s/data_source/%s/browse/path/folder_cover/?cover", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
  
	url = msprintf("%s/data_source/%s/browse/path/media_cover/?cover", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_browse_check_path_folder_cover_not_found)
{
  char * url = msprintf("%s/data_source/%s/browse/path/folder_no_cover/?cover", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
  
	url = msprintf("%s/data_source/%s/browse/path/media_no_cover/?cover", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_browse_check_path_media_cover_ok)
{
  char * url = msprintf("%s/data_source/%s/browse/path/media_cover/01 - Aria.mp3?cover", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_browse_check_path_media_cover_not_found)
{
  char * url = msprintf("%s/data_source/%s/browse/path/media_no_cover/Stallman%201.ogg?cover", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

static Suite *taliesin_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Taliesin media server test browse path");
	tc_core = tcase_create("test_data_source_browse_path");
	tcase_add_test(tc_core, test_broswe_path_root);
	tcase_add_test(tc_core, test_browse_check_path_ok);
	tcase_add_test(tc_core, test_browse_check_subpath_ok);
	tcase_add_test(tc_core, test_browse_check_subpath_folder_not_found);
	tcase_add_test(tc_core, test_browse_check_path_not_found);
	tcase_add_test(tc_core, test_browse_check_path_folder_cover_ok);
	tcase_add_test(tc_core, test_browse_check_path_folder_cover_not_found);
	tcase_add_test(tc_core, test_browse_check_path_media_cover_ok);
	tcase_add_test(tc_core, test_browse_check_path_media_cover_not_found);
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
