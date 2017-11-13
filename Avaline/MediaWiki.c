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

#include "MediaWiki.h"

/**
 * sanitize_username
 *     this function takes a username and replaces all of the spaces in it
 *     with the plus sign that mediawiki is, for whatever reason, looking for.
 *
 * @visibility private
 *
 * @param (char *) username      - a pointer to the username to sanitize
 *
 * @return (char *) a pointer to the sanitized username
 */
char *sanitize_username(char *username) {
	int i = 0;

	size_t username_size = strlen(username) + 1;

	char search = ' ';
	char esc = '+';
	char *nusername;
	
	nusername = (char *)malloc(username_size);
	strncpy_s(nusername, username_size, username, username_size);

	while (i < username_size) {
		if (nusername[i] == search) {
			nusername[i] = esc;
		}

		i++;
	}

	return nusername;
}

/**
 * mw_login
 *     this function attempts to perform a mediawiki login.
 *
 * @visibility public
 *
 * @param (MediaWiki *)     mediawiki   - persistent configuration
 * @param (MediaWikiCred *) credentials - login information
 *
 * @return (MediaWikiState) the state of the request
 */
MediaWikiState mw_login(MediaWiki *mediawiki, MediaWikiCred *credentials) {
	HTTPResponse response;
	MediaWikiState state;

	json_t *root;
	json_t *search;
	json_error_t error;

	char url[] = "http://community.wikia.com/api.php";
	char query[] = "action=login&lgname=%s&lgpassword=%s&format=json";
	char *query_string;
	char *sanitized_username;
	char *token_string;

	// this takes the size of our query string, removes 4 bytes for the formatting, and adds
	// the lengths of both the password and username
	size_t query_size = strlen(query) + credentials->username_size + credentials->password_size - 4;
	size_t url_size = strlen(url);

	query_string = (char *)malloc(query_size);
	sanitized_username = sanitize_username(credentials->username);
	snprintf(query_string, query_size, query, sanitized_username, credentials->password);

	// make the post
	response = http_post(url, query_string, mediawiki->cookies);

	// check response status
	if (response.status != HTTP_OK) {
		state = MEDIAWIKI_FATAL;
		goto cleanup;
	}

	// set the cookies
	mediawiki->cookies = response.cookies;

	// process the JSON
	root = json_loads(response.buff, 0, &error);

	if (!root) {
		state = MEDIAWIKI_FATAL;
		goto cleanup;
	}

	if (!json_is_object(root)) {
		state = MEDIAWIKI_FATAL;
		goto cleanup;
	}

	// find the login token
	search = json_object_get(root, "login");
	search = json_object_get(search, "token");

	// allocate and assign the token
	const char *token = json_string_value(search);
	size_t token_size = strlen(token) + 1;

	mediawiki->login_token = (MediaWikiToken *)malloc(sizeof(MediaWikiToken));
	mediawiki->login_token->size = token_size;
	mediawiki->login_token->token = (char *)malloc(token_size);

	strncpy_s(mediawiki->login_token->token, token_size, token, token_size);

	// append the token to the query string
	token_size += 9;
	token_string = malloc(token_size);

	strncpy_s(token_string, token_size, "&lgtoken=", token_size);
	strncat_s(token_string, token_size, token, token_size);

	query_size += token_size;
	query_string = realloc(query_string, query_size);

	strncat_s(query_string, query_size, token_string, query_size);

	// cleanup old json and request
	json_decref(root);
	json_decref(search);
	http_delete(&response, KEEP_COOKIES);

	// make the post
	response = http_post(url, query_string, mediawiki->cookies);

	// check response status
	if (response.status != HTTP_OK) {
		state = MEDIAWIKI_FATAL;
		goto cleanup;
	}

	// set the cookies
	mediawiki->cookies = response.cookies;

	// process the JSON
	root = json_loads(response.buff, 0, &error);

	if (!root) {
		state = MEDIAWIKI_FATAL;
		goto cleanup;
	}

	if (!json_is_object(root)) {
		state = MEDIAWIKI_FATAL;
		goto cleanup;
	}

	// find the login token
	search = json_object_get(root, "login");
	search = json_object_get(search, "result");
	const char *status = json_string_value(search);

	state = get_status(status);
	goto cleanup;

cleanup:
	free(sanitized_username);
	http_delete(&response, KEEP_COOKIES);

	if (root) {
		json_decref(root);
	}

	if (search) {
		json_decref(search);
	}

	return state;
}

/**
 * get_status
 *     this function returns enum values for api responses
 * 
 * @visibility private
 *
 * @param (char *) status - status to check
 *
 * @return (MediaWikiState) the enumerated status
 */
MediaWikiState get_status(char *status) {
	if(strcmp(status, "Success") == 0) {
		return MEDIAWIKI_OK;
	}

	if (strcmp(status, "NoName") == 0) {
		return MEDIAWIKI_NONAME;
	}

	if (strcmp(status, "NotExists") == 0) {
		return MEDIAWIKI_NOTEXISTS;
	}

	if (strcmp(status, "EmptyPass") == 0) { 
		return MEDIAWIKI_EMPTYPASS;
	}

	if (strcmp(status, "WrongPass") == 0) {
		return MEDIAWIKI_WRONGPASS;
	}

	if (strcmp(status, "WrongPluginPass") == 0) {
		return MEDIAWIKI_WRONGPLUGINPASS;
	}

	if (strcmp(status, "CreateBlocked") == 0) {
		return MEDIAWIKI_CREATEBLOCKED;
	}

	if (strcmp(status, "Throttled") == 0) {
		return MEDIAWIKI_THROTTLED;
	}

	if (strcmp(status, "Blocked") == 0) {
		return MEDIAWIKI_BLOCKED;
	}

	if (strcmp(status, "mustbeposted") == 0) {
		return MEDIAWIKI_FATAL;
	}

	if (strcmp(status, "NeedToken") == 0) {
		return MEDIAWIKI_FATAL;
	}
}

/**
 * delete_credentials
 *     this function performs cleanup on credentials
 *
 * @visibility public
 *
 * @param (MediaWikiCred *) credentials - pointer to struct to delete
 *
 * @return (void)
 */
void delete_credentials(MediaWikiCred *credentials) {
	if (credentials->username) {
		free(credentials->username);
		credentials->username_size = 0;
	}

	if (credentials->password) {
		// remove password from memory
		memset(credentials->password, 0, credentials->password_size);

		free(credentials->password);
		credentials->password_size = 0;
	}
}

/**
* delete_token
*     this function performs cleanup on a token
*
* @visibility public
*
* @param (MediaWikiToken *) token - pointer to struct to delete
*
* @return (void)
*/
void delete_token(MediaWikiToken *token) {
	if (token) {
		if (token->token) {
			free(token->token);
			token->size = 0;
		}
	}
}

/**
* delete_mediawiki
*     this function performs cleanup on mediawiki
*
* @visibility public
*
* @param (MediaWiki *) mediawiki - pointer to struct to delete
*
* @return (void)
*/
void delete_mediawiki(MediaWiki *mediawiki) {
	if (mediawiki) {
		if (mediawiki->edit_token) {
			delete_token(mediawiki->edit_token);
			free(mediawiki->edit_token);
		}

		if (mediawiki->login_token) {
			delete_token(mediawiki->login_token);
			free(mediawiki->login_token);
		}

		if (mediawiki->cookies) {
			curl_slist_free_all(mediawiki->cookies);
		}
	}
}