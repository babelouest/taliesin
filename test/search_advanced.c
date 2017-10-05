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

#define DATA_SOURCE_VALID       "dataSourceTest"

struct _u_request user_req;
char * user_login = NULL;

START_TEST(test_advanced_search_no_criteria)
{
  char * url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	json_t * j_criteria = json_object();
  
  int res = run_simple_authenticated_test(&user_req, "PUT", url, j_criteria, NULL, 200, NULL, NULL, NULL);
  free(url);
	json_decref(j_criteria);
	ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_advanced_search_no_criteria_limit_5)
{
	json_t * j_criteria = json_pack("{si}", "limit", 5);
	struct _u_response resp;
	json_t * j_result = NULL;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_eq(json_array_size(j_result), 5);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_no_criteria_limit_invalid)
{
	json_t * j_criteria = json_pack("{ss}", "limit", "invalid");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 400);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_no_criteria_offset_5)
{
	json_t * j_criteria = json_pack("{si}", "offset", 5);
	struct _u_response resp;
	json_t * j_result = NULL;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_no_criteria_offset_invalid)
{
	json_t * j_criteria = json_pack("{ss}", "offset", "invalid");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 400);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_data_source_valid)
{
	json_t * j_criteria = json_pack("{s[s]}", "data_source", DATA_SOURCE_VALID);
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 200);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_data_source_invalid)
{
	json_t * j_criteria = json_pack("{s[s]}", "data_source", "invalid");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 400);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_query_valid)
{
	json_t * j_criteria = json_pack("{ss}", "query", "Kimiko");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 200);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_query_invalid)
{
	json_t * j_criteria = json_pack("{si}", "query", 42);
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 400);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_type_valid)
{
	json_t * j_criteria = json_pack("{ss}", "type", "audio");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 200);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_type_invalid)
{
	json_t * j_criteria = json_pack("{ss}", "type", "invalid");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 400);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_tag_invalid)
{
	json_t * j_criteria = json_pack("{s[s]}", "tags", "invalid");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 400);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_tag_eq_valid)
{
	json_t * j_criteria = json_pack("{s[{ssssss}]}", "tags", "key", "artist", "operator", "equals", "value", "Kimiko"), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_tag_eq_valid_result_empty)
{
	json_t * j_criteria = json_pack("{s[{ssssss}]}", "tags", "key", "artist", "operator", "equals", "value", "Batman"), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_eq(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_tag_ne_valid)
{
	json_t * j_criteria = json_pack("{s[{ssssss}]}", "tags", "key", "artist", "operator", "different", "value", "Kimiki"), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_tag_contains_valid)
{
	json_t * j_criteria = json_pack("{s[{ssssss}]}", "tags", "key", "artist", "operator", "contains", "value", "imik"), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_tag_lower_valid)
{
	json_t * j_criteria = json_pack("{s[{ssssss}]}", "tags", "key", "date", "operator", "lower", "value", "2019"), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_tag_higher_valid)
{
	json_t * j_criteria = json_pack("{s[{ssssss}]}", "tags", "key", "date", "operator", "higher", "value", "2000"), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_tag_between_valid)
{
	json_t * j_criteria = json_pack("{s[{ssssssss}]}", "tags", "key", "date", "operator", "between", "value", "2000", "value_max", "2019"), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_tag_empty_valid)
{
	json_t * j_criteria = json_pack("{s[{ssss}]}", "tags", "key", "artist", "operator", "empty"), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_metrics_nb_play_eq_valid)
{
	json_t * j_criteria = json_pack("{s{sssssi}}", "metrics", "key", "nb_play", "operator", "equals", "value", 1), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_metrics_nb_play_ne_valid)
{
	json_t * j_criteria = json_pack("{s{sssssi}}", "metrics", "key", "nb_play", "operator", "different", "value", 1), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_metrics_nb_play_lower_valid)
{
	json_t * j_criteria = json_pack("{s{sssssi}}", "metrics", "key", "nb_play", "operator", "lower", "value", 10), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_metrics_nb_play_higher_valid)
{
	json_t * j_criteria = json_pack("{s{sssssi}}", "metrics", "key", "nb_play", "operator", "higher", "value", 0), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_metrics_nb_play_between_valid)
{
	json_t * j_criteria = json_pack("{s{sssssisi}}", "metrics", "key", "nb_play", "operator", "between", "value", 0, "value_max", 10), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_metrics_played_at_higher_valid)
{
	json_t * j_criteria = json_pack("{s{sssssi}}", "metrics", "key", "played_at", "operator", "higher", "value", 0), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_metrics_last_seen_higher_valid)
{
	json_t * j_criteria = json_pack("{s{sssssi}}", "metrics", "key", "last_seen", "operator", "higher", "value", 0), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_metrics_last_updated_higher_valid)
{
	json_t * j_criteria = json_pack("{s{sssssi}}", "metrics", "key", "last_updated", "operator", "higher", "value", 0), * j_result = NULL;
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	if (ulfius_send_http_request(&user_req, &resp) == U_OK) {
		j_result = ulfius_get_json_body_response(&resp, NULL);
	}
	
	ck_assert_int_eq(resp.status, 200);
	ck_assert_int_gt(json_array_size(j_result), 0);

	json_decref(j_criteria);
	json_decref(j_result);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_sort_invalid)
{
	json_t * j_criteria = json_pack("{ss}", "sort", "invalid");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 400);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_sort_name_valid)
{
	json_t * j_criteria = json_pack("{ss}", "sort", "name");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 200);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_sort_path_valid)
{
	json_t * j_criteria = json_pack("{ss}", "sort", "path");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 200);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_sort_last_updated_valid)
{
	json_t * j_criteria = json_pack("{ss}", "sort", "last_updated");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 200);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_sort_last_played_valid)
{
	json_t * j_criteria = json_pack("{ss}", "sort", "last_played");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 200);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_sort_nb_play_valid)
{
	json_t * j_criteria = json_pack("{ss}", "sort", "nb_play");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 200);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

START_TEST(test_advanced_search_sort_random_valid)
{
	json_t * j_criteria = json_pack("{ss}", "sort", "random");
	struct _u_response resp;
  
	ulfius_init_response(&resp);
	o_free(user_req.http_verb);
	user_req.http_verb = o_strdup("PUT");
	o_free(user_req.http_url);
	user_req.http_url = msprintf("%s/search/", TALIESIN_SERVER_URI);
	ulfius_set_json_body_request(&user_req, j_criteria);
	
	ulfius_send_http_request(&user_req, &resp);
	
	ck_assert_int_eq(resp.status, 200);

	json_decref(j_criteria);
	ulfius_clean_response(&resp);
}
END_TEST

static Suite *taliesin_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Taliesin media server Advanced Search");
	tc_core = tcase_create("test_search_advanced");
	tcase_add_test(tc_core, test_advanced_search_no_criteria);
	tcase_add_test(tc_core, test_advanced_search_no_criteria_limit_5);
	tcase_add_test(tc_core, test_advanced_search_no_criteria_limit_invalid);
	tcase_add_test(tc_core, test_advanced_search_no_criteria_offset_5);
	tcase_add_test(tc_core, test_advanced_search_no_criteria_offset_invalid);
	tcase_add_test(tc_core, test_advanced_search_data_source_valid);
	tcase_add_test(tc_core, test_advanced_search_data_source_invalid);
	tcase_add_test(tc_core, test_advanced_search_query_valid);
	tcase_add_test(tc_core, test_advanced_search_query_invalid);
	tcase_add_test(tc_core, test_advanced_search_type_valid);
	tcase_add_test(tc_core, test_advanced_search_type_invalid);
	tcase_add_test(tc_core, test_advanced_search_tag_invalid);
	tcase_add_test(tc_core, test_advanced_search_tag_eq_valid);
	tcase_add_test(tc_core, test_advanced_search_tag_eq_valid_result_empty);
	tcase_add_test(tc_core, test_advanced_search_tag_ne_valid);
	tcase_add_test(tc_core, test_advanced_search_tag_contains_valid);
	tcase_add_test(tc_core, test_advanced_search_tag_lower_valid);
	tcase_add_test(tc_core, test_advanced_search_tag_higher_valid);
	tcase_add_test(tc_core, test_advanced_search_tag_between_valid);
	tcase_add_test(tc_core, test_advanced_search_tag_empty_valid);
	tcase_add_test(tc_core, test_advanced_search_metrics_nb_play_eq_valid);
	tcase_add_test(tc_core, test_advanced_search_metrics_nb_play_ne_valid);
	tcase_add_test(tc_core, test_advanced_search_metrics_nb_play_lower_valid);
	tcase_add_test(tc_core, test_advanced_search_metrics_nb_play_higher_valid);
	tcase_add_test(tc_core, test_advanced_search_metrics_nb_play_between_valid);
	tcase_add_test(tc_core, test_advanced_search_metrics_played_at_higher_valid);
	tcase_add_test(tc_core, test_advanced_search_metrics_last_seen_higher_valid);
	tcase_add_test(tc_core, test_advanced_search_metrics_last_updated_higher_valid);
	tcase_add_test(tc_core, test_advanced_search_sort_invalid);
	tcase_add_test(tc_core, test_advanced_search_sort_name_valid);
	tcase_add_test(tc_core, test_advanced_search_sort_path_valid);
	tcase_add_test(tc_core, test_advanced_search_sort_last_updated_valid);
	tcase_add_test(tc_core, test_advanced_search_sort_last_played_valid);
	tcase_add_test(tc_core, test_advanced_search_sort_nb_play_valid);
	tcase_add_test(tc_core, test_advanced_search_sort_random_valid);
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
