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

#define PLAYLIST_USER_VALID    "playlistTest"
#define PLAYLIST_USER_INVALID  "playlistTestInvalid"
#define PLAYLIST_ADMIN_VALID   "playlistAdmTest"

struct _u_request user_req, admin_req;
char * user_login = NULL, * admin_login = NULL;

START_TEST(test_list_playlist_ok)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
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

START_TEST(test_create_playlist_error_input_scope)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{sssssss[{ssss}{ssss}]}",
                                  "name", PLAYLIST_USER_INVALID,
                                  "description", "description for "PLAYLIST_USER_INVALID,
                                  "scope", "all",
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_error_input_description_type)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{sssisss[{ssss}{ssss}]}",
                                  "name", PLAYLIST_USER_INVALID,
                                  "description", 42,
                                  "scope", "all",
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_error_input_media_empty)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{sssssss[]}",
                                  "name", PLAYLIST_USER_INVALID,
                                  "description", "description for "PLAYLIST_USER_INVALID,
                                  "scope", "all",
                                  "media");
  
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_error_input_name_exist)
{
  char * url = msprintf("%s/playlist", TALIESIN_SERVER_URI);
  json_t * j_playlist = json_pack("{sssssss[{ssss}{ssss}]}",
                                  "name", PLAYLIST_USER_VALID,
                                  "description", "description for "PLAYLIST_USER_VALID,
                                  "scope", "all",
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_create_playlist_adm_ok)
{
  char * url = msprintf("%s/playlist?username=%s", TALIESIN_SERVER_URI, user_login);
  json_t * j_playlist = json_pack("{sssssss[{ssss}{ssss}]}",
                                  "name", PLAYLIST_ADMIN_VALID,
                                  "description", "description for "PLAYLIST_ADMIN_VALID,
                                  "scope", "all",
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");

  int res = run_simple_authenticated_test(&admin_req, "POST", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_playlist_ok)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{sssssssis[{ssss}{ssss}]s[]}",
                                  "name", PLAYLIST_USER_VALID,
                                  "description", "description for "PLAYLIST_USER_VALID,
                                  "scope", "me",
                                  "elements", 2,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "fss/free-software-song.ogg",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "fss/FreeSWSong.ogg",
                                  "stream");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_playlist, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_playlist_not_found)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, "invalid");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_playlist_ok)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{sss[{ssss}]}",
                                  "description", "new description for "PLAYLIST_USER_VALID,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/FreeSWSong.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_playlist_error_input)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{sis[{ssss}]}",
                                  "description", 42,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/eternal-flame.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_playlist_error_media)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{sss[{ssss}]}",
                                  "description", "new description for "PLAYLIST_USER_VALID,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "invalid");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_get_updated_playlist_ok)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{sssssssis[{ssss}]s[]}",
                                  "name", PLAYLIST_USER_VALID,
                                  "description", "new description for "PLAYLIST_USER_VALID,
                                  "scope", "me",
                                  "elements", 1,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "fss/FreeSWSong.ogg",
                                  "stream");
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, j_playlist, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_set_playlist_adm_no_credentials)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_ADMIN_VALID);
  json_t * j_playlist = json_pack("{sss[{ssss}]}",
                                  "description", "new description for "PLAYLIST_ADMIN_VALID,
                                  "media",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/eternal-flame.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 403, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_playlist_add_media_ok)
{
  char * url = msprintf("%s/playlist/%s/add_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("[{ssss}]",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_playlist_add_media_error_input)
{
  char * url = msprintf("%s/playlist/%s/add_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("{ssss}",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_playlist_add_media_error_media)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("[{ssss}]",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "invalid");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 400, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_playlist_add_media_error_credentials)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_ADMIN_VALID);
  json_t * j_playlist = json_pack("[{ssss}]",
                                    "data_source", DATA_SOURCE_VALID,
                                    "path", "/fss/free-software-song.ogg");
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 403, NULL, NULL, NULL);
  free(url);
  json_decref(j_playlist);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_playlist_has_media_ok)
{
  char * url = msprintf("%s/playlist/%s/add_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("[{ssss}]",
                                  "data_source", DATA_SOURCE_VALID,
                                  "path", "/fss"), * j_result;
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  ck_assert_int_eq(res, 1);
  free(url);
  json_decref(j_playlist);
  
  url = msprintf("%s/playlist/%s/has_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  j_playlist = json_pack("[{ssss}]",
                         "data_source", DATA_SOURCE_VALID,
                         "path", "/fss/free-software-song.ogg");
  j_result = json_pack("{ss}", "path", "fss/free-software-song.ogg");
  res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 200, j_result, NULL, NULL);
  ck_assert_int_eq(res, 1);
  free(url);
  json_decref(j_playlist);
}
END_TEST

START_TEST(test_playlist_has_media_not_found)
{
  char * url = msprintf("%s/playlist/%s/has_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  json_t * j_playlist = json_pack("[{ssss}]",
                         "data_source", DATA_SOURCE_VALID,
                         "path", "short/short1.mp3");
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_playlist, NULL, 404, NULL, NULL, NULL);
  ck_assert_int_eq(res, 1);
  free(url);
  json_decref(j_playlist);
  
  url = msprintf("%s/playlist/%s/remove_media", TALIESIN_SERVER_URI, PLAYLIST_USER_VALID);
  j_playlist = json_pack("[{ssss}]",
                         "data_source", DATA_SOURCE_VALID,
                         "path", "/fss");
  res = run_simple_authenticated_test(&user_req, "PUT", url, j_playlist, NULL, 200, NULL, NULL, NULL);
  ck_assert_int_eq(res, 1);
  free(url);
  json_decref(j_playlist);
}
END_TEST

START_TEST(test_delete_playlist_not_found)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, "invalid");
  
  int res = run_simple_authenticated_test(&user_req, "DELETE", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_delete_playlist_error_credentials)
{
  char * url = msprintf("%s/playlist/%s", TALIESIN_SERVER_URI, PLAYLIST_ADMIN_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "DELETE", url, NULL, NULL, 403, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
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

START_TEST(test_delete_playlist_adm_ok)
{
  char * url = msprintf("%s/playlist/%s?username=%s", TALIESIN_SERVER_URI, PLAYLIST_ADMIN_VALID, user_login);
  
  int res = run_simple_authenticated_test(&admin_req, "DELETE", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
  ck_assert_int_eq(res, 1);
}
END_TEST

static Suite *taliesin_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Taliesin media server test playlist CRUD");
  tc_core = tcase_create("test_playlist_crud");
  tcase_add_test(tc_core, test_list_playlist_ok);
  tcase_add_test(tc_core, test_create_playlist_ok);
  tcase_add_test(tc_core, test_create_playlist_error_input_scope);
  tcase_add_test(tc_core, test_create_playlist_error_input_description_type);
  tcase_add_test(tc_core, test_create_playlist_error_input_media_empty);
  tcase_add_test(tc_core, test_create_playlist_error_input_name_exist);
  tcase_add_test(tc_core, test_create_playlist_adm_ok);
  tcase_add_test(tc_core, test_get_playlist_ok);
  tcase_add_test(tc_core, test_get_playlist_not_found);
  tcase_add_test(tc_core, test_set_playlist_ok);
  tcase_add_test(tc_core, test_set_playlist_error_input);
  tcase_add_test(tc_core, test_set_playlist_error_media);
  tcase_add_test(tc_core, test_get_updated_playlist_ok);
  tcase_add_test(tc_core, test_set_playlist_adm_no_credentials);
  tcase_add_test(tc_core, test_playlist_add_media_ok);
  tcase_add_test(tc_core, test_playlist_add_media_error_input);
  tcase_add_test(tc_core, test_playlist_add_media_error_media);
  tcase_add_test(tc_core, test_playlist_add_media_error_credentials);
  tcase_add_test(tc_core, test_playlist_has_media_ok);
  tcase_add_test(tc_core, test_playlist_has_media_not_found);
  tcase_add_test(tc_core, test_delete_playlist_not_found);
  tcase_add_test(tc_core, test_delete_playlist_error_credentials);
  tcase_add_test(tc_core, test_delete_playlist_ok);
  tcase_add_test(tc_core, test_delete_playlist_adm_ok);
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
    ulfius_init_request(&admin_req);
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
