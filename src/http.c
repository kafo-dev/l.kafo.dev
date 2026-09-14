#include "http.h"
#include "maths.h"
#include <assert.h>
#include <string.h>

static const size_t HTTP_MAX_METHOD_LEN = 32;
static const size_t HTTP_MAX_PATH_LEN = HTTP_REQUEST_PATH_SIZE;

enum http_error http_parse_request(const char *data, size_t data_len,
                                   struct http_request *request) {
	assert(data != nullptr);
	assert(request != nullptr);

	const char *method_space =
	    memchr(data, ' ', maths_min(HTTP_MAX_METHOD_LEN, data_len));
	if (method_space == nullptr) {
		return HTTP_ERROR_INVALID_SYNTAX;
	}

	static const char GET[] = "GET";
	static const char POST[] = "POST";

	size_t method_len = method_space - data;
	if (method_len == sizeof(GET) - 1 &&
	    memcmp(data, GET, method_len) == 0) {
		request->method = HTTP_METHOD_GET;
	} else if (method_len == sizeof(POST) - 1 &&
	           memcmp(data, POST, method_len) == 0) {
		request->method = HTTP_METHOD_POST;
	} else {
		return HTTP_ERROR_INVALID_METHOD;
	}

	data = method_space + 1;
	data_len -= method_len;

	const char *path_space =
	    memchr(data, ' ', maths_min(HTTP_MAX_PATH_LEN, data_len));
	if (path_space == nullptr) {
		return HTTP_ERROR_INVALID_SYNTAX;
	}

	size_t path_len = path_space - data;
	if (path_len > HTTP_REQUEST_PATH_SIZE - 1) {
		return HTTP_ERROR_PATH_TOO_LARGE;
	}
	memcpy(request->path, method_space + 1, path_len);
	request->path[path_len] = 0;

	// TODO: Version parsing & return current position.

	return HTTP_ERROR_NONE;
}
