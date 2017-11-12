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

#include "Avaline.h"

void test_get() {
	HTTPResponse response;

	char url[] = "http://postman-echo.com/get";
	char query[] = "action=test";

	response = http_get(url, query, NULL);

	if (response.buff) {
		printf("%s\n", response.buff);
	} else {
		printf("ERROR %d\n", response.status);
	}

	http_delete(&response, DELETECOOKIES);
}

void test_post() {
	HTTPResponse response;

	char url[] = "http://postman-echo.com/post";
	char query[] = "action=test";

	response = http_post(url, query, NULL);

	if (response.buff) {
		printf("%s\n", response.buff);
	} else {
		printf("ERROR %d\n", response.status);
	}
	
	http_delete(&response, DELETECOOKIES);
}

int main (int argc, char *argv[]) {
	char *username;
	char *password;

	size_t username_size;
	size_t password_size;

	if (argc < 3) {
		printf("Please supply a username and password argument!\n");
		return 1;
	}

	username_size = strlen(argv[1]) + 1;
	password_size = strlen(argv[2]) + 1;

	username = (char *)malloc(username_size);
	password = (char *)malloc(password_size);

	strncpy_s(username, username_size, argv[1], username_size);
	strncpy_s(password, password_size, argv[2], password_size);

	printf("Username: %s\nPassword: %s\n", username, password);

	free(username);
	free(password);

	// pause execution
	getchar();

	return 0;
}