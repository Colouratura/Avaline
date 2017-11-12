/**
* Avaline -  a hacky c playground for mediawiki
* Copyright (C) 2017  Avaline Caughdough
*
* This file is part of Avaline.
*
* Avaline is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Avaline is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Avaline.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <curl/curl.h>

typedef enum HTTPStatus {
	HTTPOK,
	HTTPPENDING,
	HTTPFATALERROR,
	HTTPWRITEMEMERROR,
	HTTPCOOKIEERR
} HTTPStatus;

typedef enum CookieOptions {
	KEEPCOOKIES,
	DELETECOOKIES
} CookieOptions;

typedef struct HTTPResponse {
	HTTPStatus         status;
	size_t             buff_size;
	char              *buff;
	struct curl_slist *cookies;
} HTTPResponse;

HTTPResponse http_get(char *url, char *query, struct curl_slist *cookies);
HTTPResponse http_post(char *url, char *form, struct curl_slist *cookies);

void http_delete(HTTPResponse *response, CookieOptions options);

// Private methods the user doesn't need to know about
// char *http_join(char *url, char *query);
// static struct curl_slist *get_cookies(CURL *curl_handle);
// static size_t http_write(void *contents, size_t old_size, size_t new_size, void *response);