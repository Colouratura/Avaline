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

int main (int argc, char *argv[]) {
	MediaWiki mediawiki;
	MediaWikiCred credentials;
	MediaWikiState status;

	// init mediawiki
	mediawiki.edit_token = NULL;
	mediawiki.login_token = NULL;
	mediawiki.cookies = (struct curl_slist *)0;

#ifndef _DEBUG
	if (argc < 3) {
		printf("Please supply a username and password argument!\n");
		return 1;
	}
#endif

#ifdef _DEBUG
	argv = malloc(3);
	argv[1] = calloc(32, 1);
	argv[2] = calloc(32, 1);

	gets(argv[1]);
	gets(argv[2]);
#endif

	credentials.username_size = strlen(argv[1]) + 1;
	credentials.password_size = strlen(argv[2]) + 1;

	credentials.username = (char *)malloc(credentials.username_size);
	credentials.password = (char *)malloc(credentials.password_size);

	strncpy_s(credentials.username, credentials.username_size, argv[1], credentials.username_size);
	strncpy_s(credentials.password, credentials.password_size, argv[2], credentials.password_size);

	status = mw_login(&mediawiki, &credentials);

	if (status == MEDIAWIKI_OK) {
		printf("We logged into the wiki as %s!\n", credentials.username);
	} else {
		printf("MediaWiki reported error number %d!\n", status);
	}

	delete_credentials(&credentials);
	delete_mediawiki(&mediawiki);

	// pause execution
	getchar();

	return 0;
}