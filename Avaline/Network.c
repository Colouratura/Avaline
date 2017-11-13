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

#include <stdlib.h>
#include <string.h>

#include "Network.h"

HTTPResponse http_request(HTTPType method, char *url, char *data, struct curl_slist *cookies) {
	HTTPResponse response;

	CURL *handle;
	CURLcode res;

	char *url_query;

	// add the query string if this is a GET
	if (method == HTTP_GET) {
		url_query = http_join(url, strlen(url), data, strlen(data));
	} else {
		url_query = url;
	}

	// setup the response object
	response.buff = (char *)malloc(1);
	response.buff_size = 0;
	response.status = HTTP_PENDING;
	// response.cookies = NULL;

	// setup the request
	curl_global_init(CURL_GLOBAL_ALL);

	handle = curl_easy_init();

	if (!handle) {
		http_delete(&response, KEEP_COOKIES);
		response.status = HTTP_FATALERROR;

		return response;
	}

	curl_easy_setopt(handle, CURLOPT_URL, url_query);
	curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "");
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, http_write);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&response); // that last bit is taking the address of our HTTPRequest and casting it as a void pointer
	curl_easy_setopt(handle, CURLOPT_USERAGENT, "Avaline/0.0.1");

	// if our request is a POST then append the data to the form
	if (method == HTTP_POST) {
		curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, (long)strlen(data));
	}

	// set our cookies
	while (cookies) {
		res = curl_easy_setopt(handle, CURLOPT_COOKIELIST, cookies->data);
		cookies = cookies->next;
	}

	// make the request
	res = curl_easy_perform(handle);

	if (res != CURLE_OK) {
		curl_easy_cleanup(handle);
		http_delete(&response, KEEP_COOKIES);
		response.status = HTTP_FATALERROR;

		return response;
	} else {
		response.cookies = get_cookies(handle);

		// checking for a cookie write error
		if (!response.cookies) {
			http_delete(&response, KEEP_COOKIES);
			response.status = HTTP_COOKIEERROR;

			// clean up the curl stuff too
			curl_easy_cleanup(handle);

			return response;
		}
	}

	// cleanup request
	curl_easy_cleanup(handle);

	response.status = HTTP_OK;

	return response;
}

/**
 * http_get
 *     this function performs a GET request.
 *
 * @visibility public
 *
 * @param (char *)              url     - the url to fetch
 * @param (char *)              query   - the query string to append
 * @param (struct curl_slist *) cookies - the cookies list to include with the request (nullable)
 *
 * @return (HTTPRequest) the response struct with response data and optional error codes
 */
HTTPResponse http_get(char *url, char *query, struct curl_slist *cookies) {
	return http_request(HTTP_GET, url, query, cookies);
}

/**
* http_get
*     this function performs a POST request.
*
* @visibility public
*
* @param (char *)              url     - the url to fetch
* @param (char *)              query   - the query string to append as a form
* @param (struct curl_slist *) cookies - the cookies list to include with the request (nullable)
*
* @return (HTTPRequest) the response struct with response data and optional error codes
*/
HTTPResponse http_post(char *url, char *query, struct curl_slist *cookies) {
	return http_request(HTTP_POST, url, query, cookies);
}

/**
 * http_delete
 *     this function provides a simple method to be used in order to destroy and free the memory
 *     from an allocated HTTPRequest structure.
 *
 *     this function also allows you to specify whether or not yu want to delete or keep the
 *     associated cookies.
 *
 * @visibility public
 *
 * @param (HTTPResponse *) response - the response structure to perform the free on
 * @param (CookieOptions)  options  - the enum value that toggles cookie deletion
 *
 * @return (void)
 */
void http_delete(HTTPResponse *response, CookieOptions *option) {
	response->buff_size = 0;

	if (response->buff) {
		free(response->buff);
	}

	if (response->cookies && option == DELETE_COOKIES) {
		curl_slist_free_all(response->cookies);
	}
}

/**
* http_write
*     this function controlls the writting of the network response to the buffer
*     of the HTTPRequest structure.
*
*     you may notice that the resonse is passed as a void and that is because it
*     is passed directly to this function from libcurl and given that it doesnt know
*     the type of the structure it only makes sense to pass it without any more
*     information than that it is indeed a memory address.

* @visibility private
*
* @param (void *) contents - the contents of the request as received by libcurl
* @param (size_t) size     - the size of each data type in bytes
* @param (size_t) nmemb    - the number of elements being passed
* @param (void *) response - the HTTPResponse structure to write to
*
* @return (static size_t) the new buffer size used for some record keeping
*/
static size_t http_write(void *contents, size_t size, size_t nmemb, void *response) {
	HTTPResponse *mem = (HTTPResponse *)response;
	size_t buff_size = size * nmemb;

	// allocate the size needed to store all the data
	mem->buff = (char *)realloc(mem->buff, mem->buff_size + buff_size + 1);

	if (mem->buff == NULL) {
		mem->status = HTTP_WRITEMEMERROR;
		return 0;
	}

	// copy everything into the response struct
	memcpy(&(mem->buff[mem->buff_size]), contents, buff_size);

	mem->buff_size += buff_size;
	mem->buff[mem->buff_size] = 0;

	return buff_size;
}

/**
* get_cookies
*     this function grabs the cookies set by the server and saves them for future requests
*
* @visibility private
*
* @param (CURL *) handle - the libcurl request handle to grab the cookies from
*
* @return (static struct curl_slist *) a new libcurl list of cookies
*/
static struct curl_slist *get_cookies(CURL *curl_handle) {
	struct curl_slist *cookies;
	CURLcode res;

	res = curl_easy_getinfo(curl_handle, CURLINFO_COOKIELIST, &cookies);

	if (res != CURLE_OK) {
		return NULL;
	} else {
		return cookies;
	}
}

/**
* http_join
*     this function is a helper function for GET requests that allows you to build
*     forms like you would for POST requests and then merge it into the url as the
*     query string.
*
* @todo:
*     - handle cases where malloc may fail to allocate memory
*
* @visibility public
*
* @param (char *) url   - the url to merge onto
* @param (char *) query - the url encoded form to merge into the url
*
* @param (char *) the merged url
*/
char *http_join(char *url, size_t url_size, char *query, size_t query_size) {
	//size_t url_size = strlen(url);
	//size_t query_size = strlen(query);
	size_t buff_size = 2 + url_size + query_size;

	char *buff = (char *)malloc(buff_size);

	strncpy_s(buff, buff_size, url, url_size);
	strncat_s(buff, buff_size, "?", 1);
	strncat_s(buff, buff_size, query, query_size);

	return buff;
}