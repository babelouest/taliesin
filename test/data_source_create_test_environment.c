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

#define DATA_SOURCE_VALID "dataSourceTest"
#define DATA_SOURCE_PATH  "/tmp/media"

struct _u_request user_req;
char * user_login = NULL, * data_source_path;

START_TEST(test_create_data_source_user_ok)
{
  char * url = msprintf("%s/data_source/", TALIESIN_SERVER_URI);
	json_t * j_data_source = json_pack("{ssssssssss}",
																		 "name", DATA_SOURCE_VALID,
																		 "description", "description for " DATA_SOURCE_VALID,
																		 "scope", "me",
																		 "path", data_source_path,
																		 "icon", "testIcon");
	
  int res = run_simple_authenticated_test(&user_req, "POST", url, j_data_source, NULL, 200, NULL, NULL, NULL);
  free(url);
	json_decref(j_data_source);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_data_source_user_refresh_complete_ok)
{
	struct _u_response resp;
  char * url = msprintf("%s/data_source/%s/refresh/", TALIESIN_SERVER_URI, DATA_SOURCE_VALID);
	json_t * j_result = NULL, * j_expect = json_pack("{ss}", "status", "not running");
	int i = 0;
	
  int res = run_simple_authenticated_test(&user_req, "PUT", url, NULL, NULL, 200, NULL, NULL, NULL);
	ck_assert_int_eq(res, 1);
	
	ulfius_init_response(&resp);
	o_free(user_req.http_url);
	user_req.http_url = o_strdup(url);
	
	do {
		json_decref(j_result);
		ulfius_init_response(&resp);
		if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
			j_result = ulfius_get_json_body_response(&resp, NULL);
			ulfius_clean_response(&resp);
		}
		i++;
		sleep(2);
	} while (!json_search(j_result, j_expect) && i < 40);
	ck_assert_ptr_ne(json_search(j_result, j_expect), NULL);
	json_decref(j_result);
  free(url);
}
END_TEST

static Suite *taliesin_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Taliesin media server test data source creation");
	tc_core = tcase_create("test_data_source_create_test_environment");
	tcase_add_test(tc_core, test_create_data_source_user_ok);
	tcase_add_test(tc_core, test_data_source_user_refresh_complete_ok);
	tcase_set_timeout(tc_core, 120);
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

  data_source_path = argc>8?argv[8]:DATA_SOURCE_PATH;
  
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
