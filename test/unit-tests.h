/* Public domain, no copyright. Use at your own risk. */

#include <ulfius.h>

char * print_map(const struct _u_map * map);
void print_response(struct _u_response * response);
int test_request(struct _u_request * req, long int expected_status, json_t * expected_json_body, const char * exptected_string_body, const char * expected_redirect_uri_contains);
int run_simple_authenticated_test(struct _u_request * req, const char * method, const char * url, json_t * json_body, const struct _u_map * body, int expected_status, json_t * expected_json_body, const char * exptected_string_body, const char * expected_redirect_uri_contains);
char * url_decode(const char * str);
char * url_encode(const char * str);
json_t * json_search(json_t * haystack, json_t * needle);
char * read_file(const char * filename);
