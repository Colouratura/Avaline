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

#ifndef MEDIAWIKI_H
#define MEDIAWIKI_H

#include <jansson.h>
#include "Network.h"

typedef struct MediaWikiCred {
	char   *username;
	char   *password;
	size_t  username_size;
	size_t  password_size;
} MediaWikiCred;

typedef struct MediaWikiToken {
	char   *token;
	size_t  size;
} MediaWikiToken;

typedef enum MediaWikiState {
	MEDIAWIKI_OK,
	MEDIAWIKI_WRONGTOKEN,
	MEDIAWIKI_EMPTYPASS,
	MEDIAWIKI_WRONGPASS,
	MEDIAWIKI_WRONGPLUGINPASS,
	MEDIAWIKI_NONAME,
	MEDIAWIKI_NOTEXISTS,
	MEDIAWIKI_BLOCKED,
	MEDIAWIKI_THROTTLED,
	MEDIAWIKI_CREATEBLOCKED,
	MEDIAWIKI_FATAL
} MediaWikiState;

typedef struct MediaWiki {
	MediaWikiToken    *edit_token;
	MediaWikiToken    *login_token;
	MediaWikiState     status;
	struct curl_slist *cookies;
} MediaWiki;

MediaWikiState mw_login(MediaWiki *mediawiki, MediaWikiCred *credentials);
void delete_credentials(MediaWikiCred *credentials);
void delete_token(MediaWikiToken *token);
void delete_mediawiki(MediaWiki *mediawiki);

// Private functions
char *sanitize_username(char *username);

#endif