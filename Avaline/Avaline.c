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

int main (void) {
	test_get();
	test_post();
	getchar();

    return 0;
}