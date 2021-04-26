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

struct _u_request user_req;
char * user_login = NULL;

START_TEST(test_broswe_category_artist_root)
{
  char * url = msprintf("%s/data_source/%s/browse/category/artist/", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_album_root)
{
  char * url = msprintf("%s/data_source/%s/browse/category/album/", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_year_root)
{
  char * url = msprintf("%s/data_source/%s/browse/category/year/", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_genre_root)
{
  char * url = msprintf("%s/data_source/%s/browse/category/genre/", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_not_found)
{
  char * url = msprintf("%s/data_source/%s/browse/category/invalid/", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_list)
{
  char * url = msprintf("%s/data_source/%s/browse/category/artist/Kimiko Ishizaka/", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_set_invalid_info)
{
  char * url = msprintf("%s/data_source/%s/info/category/artist/Kimiko Ishizaka", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  json_t * j_info = json_pack("{si}", "content", 42);
	
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_info, NULL, 400, NULL, NULL, NULL);
  free(url);
	json_decref(j_info);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_set_invalid_cover)
{
  char * url = msprintf("%s/data_source/%s/info/category/artist/Kimiko Ishizaka", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  json_t * j_info = json_pack("{ssss}", "content", "Kimiko Ishizaka Open Goldberg Variations", "cover", "invalid");
	
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_info, NULL, 400, NULL, NULL, NULL);
  free(url);
	json_decref(j_info);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_set_info)
{
  char * url = msprintf("%s/data_source/%s/info/category/artist/Kimiko Ishizaka", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  json_t * j_info = json_pack("{ss}", "content", "Kimiko Ishizaka Open Goldberg Variations");
	
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_info, NULL, 200, NULL, NULL, NULL);
  free(url);
	json_decref(j_info);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_get_info)
{
  char * url = msprintf("%s/data_source/%s/info/category/artist/Kimiko Ishizaka/", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
	
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_cover)
{
  char * url = msprintf("%s/data_source/%s/info/category/artist/Kimiko Ishizaka?cover", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_cover_base64)
{
  char * url = msprintf("%s/data_source/%s/info/category/artist/Kimiko Ishizaka?cover&base64", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_cover_thumbnail)
{
  char * url = msprintf("%s/data_source/%s/info/category/artist/Kimiko Ishizaka?cover&thumbnail", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_year_get_no_info)
{
  char * url = msprintf("%s/data_source/%s/info/category/year/2015", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
	
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_year_no_cover)
{
  char * url = msprintf("%s/data_source/%s/info/category/year/2015?cover", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_not_found)
{
  char * url = msprintf("%s/data_source/%s/browse/category/artist/invalid/", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_subcategories)
{
  char * url = msprintf("%s/data_source/%s/browse/category/artist/Kimiko Ishizaka/year", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_invalid_sublevel)
{
  char * url = msprintf("%s/data_source/%s/browse/category/artist/Kimiko Ishizaka/artist", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 400, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_subcategory_year)
{
  char * url = msprintf("%s/data_source/%s/browse/category/artist/Kimiko Ishizaka/year/2012", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 200, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_broswe_category_artist_subcategory_year_not_found)
{
  char * url = msprintf("%s/data_source/%s/browse/category/artist/Kimiko Ishizaka/year/invalid", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
  
  int res = run_simple_authenticated_test(&user_req, "GET", url, NULL, NULL, 404, NULL, NULL, NULL);
  free(url);
	ck_assert_int_eq(res, 1);
}
END_TEST

static Suite *taliesin_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Taliesin media server test browse category");
	tc_core = tcase_create("test_data_source_browse_category");
	tcase_add_test(tc_core, test_broswe_category_artist_root);
	tcase_add_test(tc_core, test_broswe_category_album_root);
	tcase_add_test(tc_core, test_broswe_category_year_root);
	tcase_add_test(tc_core, test_broswe_category_genre_root);
	tcase_add_test(tc_core, test_broswe_category_not_found);
	tcase_add_test(tc_core, test_broswe_category_artist_list);
	tcase_add_test(tc_core, test_broswe_category_artist_set_invalid_info);
	tcase_add_test(tc_core, test_broswe_category_artist_set_invalid_cover);
	tcase_add_test(tc_core, test_broswe_category_artist_set_info);
	tcase_add_test(tc_core, test_broswe_category_artist_get_info);
	tcase_add_test(tc_core, test_broswe_category_artist_cover);
	tcase_add_test(tc_core, test_broswe_category_artist_cover_base64);
	tcase_add_test(tc_core, test_broswe_category_artist_cover_thumbnail);
	tcase_add_test(tc_core, test_broswe_category_year_get_no_info);
	tcase_add_test(tc_core, test_broswe_category_year_no_cover);
	tcase_add_test(tc_core, test_broswe_category_artist_not_found);
	tcase_add_test(tc_core, test_broswe_category_artist_subcategories);
	tcase_add_test(tc_core, test_broswe_category_artist_invalid_sublevel);
	tcase_add_test(tc_core, test_broswe_category_artist_subcategory_year);
	tcase_add_test(tc_core, test_broswe_category_artist_subcategory_year_not_found);
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
