#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

enum { HTTP_REQUEST_PATH_SIZE = (1024 * 8) };

enum http_error {
	HTTP_ERROR_NONE,
	HTTP_ERROR_INVALID_SYNTAX,
	HTTP_ERROR_INVALID_METHOD,
	HTTP_ERROR_PATH_TOO_LARGE,
};

enum http_method {
	HTTP_METHOD_UNKNOWN,
	HTTP_METHOD_GET,
	HTTP_METHOD_POST,
};

struct http_request {
	enum http_method method;
	char path[HTTP_REQUEST_PATH_SIZE];
};

enum http_error http_parse_request(const char *data, size_t data_len,
                                   struct http_request *request);

#endif /* HTTP_H */
