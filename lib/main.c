/*
simplified version of love2d/lua-https
Copyright (c) 2023 Chunhui He

Distributed under the same terms as the license of the original software

Modification:
- be compatible with luasocket.http
- add timeout support
- use libcurl as the only backend
- handle multiple headers (e.g. Set-Cookie)
- keep header order

Original copyright notice:
----
Copyright (c) 2019-2022 LOVE Development Team

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#ifndef _WIN32
#include <dlfcn.h>
#else
#define CURL_STATICLIB
#endif
#include <curl/curl.h>

typedef struct {
	char *str;
	size_t len;
	size_t pos;
} String;

static size_t string_reader(char *ptr, size_t size, size_t nb, void *ud)
{
	String *reader = ud;
	size_t maxCount = (reader->len - reader->pos) / size;
	size_t desiredCount = maxCount < nb ? maxCount : nb;
	size_t desiredBytes = desiredCount * size;
	memcpy(ptr, reader->str + reader->pos, desiredBytes);
	reader->pos += desiredBytes;
	return desiredCount;
}

static size_t string_writer0(const char *ptr, size_t size, size_t nb, void *ud)
{
	String *writer = ud;
	size_t count = size * nb;
	writer->str = realloc(writer->str, writer->len + count + 1);
	memcpy(writer->str + writer->len, ptr, count);
	writer->len += count;
	writer->str[writer->len++] = 0;
	return count;
}

static size_t string_writer(const char *ptr, size_t size, size_t nb, void *ud)
{
	String *writer = ud;
	size_t count = size * nb;
	writer->str = realloc(writer->str, writer->len + count);
	memcpy(writer->str + writer->len, ptr, count);
	writer->len += count;
	return count;
}

typedef struct {
	size_t urllen;
	size_t postdatalen;
	const char *url;
	const char *method;
	const char *postdata;
	String header;
	long timeout;
} Request;

typedef struct {
	const char *error;
	String header;
	String body;
	int code;
} Reply;

typedef struct {
	bool loaded;
#ifndef _WIN32
	void *handle;
	CURLcode (*global_init)(long flags);
	void (*global_cleanup)(void);
	CURL *(*easy_init)(void);
	void (*easy_cleanup)(CURL *curl);
	CURLcode (*easy_setopt)(CURL *curl, CURLoption option, ...);
	CURLcode (*easy_perform)(CURL *curl);
	CURLcode (*easy_getinfo)(CURL *curl, CURLINFO info, ...);
	struct curl_slist *(*slist_append)(struct curl_slist *list,
					   const char *data);
	void (*slist_free_all)(struct curl_slist *list);
#endif
} Context;

#ifndef _WIN32
#define curl_(x) curl->x
#else
#define curl_(x) curl_ ## x
#endif

static bool init(Context *curl)
{
	if (curl->loaded)
		return true;
#ifndef _WIN32
#define LOAD(x) if (!(curl->x = dlsym(curl->handle, "curl_" # x))) return false
#ifdef __APPLE__
	curl->handle = dlopen("/usr/lib/libcurl.4.dylib", RTLD_LAZY);
#else
	curl->handle = dlopen("libcurl.so.4", RTLD_LAZY);
	if (!curl->handle)
		curl->handle = dlopen("libcurl-gnutls.so.4", RTLD_LAZY);
#endif
	if (!curl->handle)
		return false;
	LOAD(global_init);
	LOAD(global_cleanup);
	LOAD(easy_init);
	LOAD(easy_cleanup);
	LOAD(easy_setopt);
	LOAD(easy_perform);
	LOAD(easy_getinfo);
	LOAD(slist_append);
	LOAD(slist_free_all);
#undef LOAD
#endif
	curl_(global_init)(CURL_GLOBAL_DEFAULT);
	curl->loaded = true;
	return true;
}

static void cleanup(Context *curl)
{
	if (curl->loaded)
		curl_(global_cleanup)();
#ifndef _WIN32
	if (curl->handle)
		dlclose(curl->handle);
#endif
	curl->loaded = false;
}

static bool request(Context *curl, const Request *req, Reply *reply)
{
	CURL *ch = curl_(easy_init)();
	if (!ch) {
		reply->error = "Could not create curl request";
		return false;
	}

	curl_(easy_setopt)(ch, CURLOPT_URL, req->url);
	curl_(easy_setopt)(ch, CURLOPT_FOLLOWLOCATION, 1L);
	curl_(easy_setopt)(ch, CURLOPT_CUSTOMREQUEST, req->method);
	if (req->timeout)
		curl_(easy_setopt)(ch, CURLOPT_TIMEOUT_MS, req->timeout);

	if (req->postdatalen > 0 &&
	    strcmp(req->method, "GET") && strcmp(req->method, "HEAD")) {
		String reader;
		reader.str = (char *) req->postdata;
		reader.len = req->postdatalen;
		reader.pos = 0;
		curl_(easy_setopt)(ch, CURLOPT_UPLOAD, 1L);
		curl_(easy_setopt)(ch, CURLOPT_READFUNCTION, string_reader);
		curl_(easy_setopt)(ch, CURLOPT_READDATA, &reader);
		curl_(easy_setopt)(ch, CURLOPT_INFILESIZE_LARGE,
				 (curl_off_t) reader.len);
	}
	if (strcmp(req->method, "HEAD") == 0)
		curl_(easy_setopt)(ch, CURLOPT_NOBODY, 1L);

	struct curl_slist *headers = NULL;
	const char *p = req->header.str;
	const char *e = req->header.str + req->header.len;
	while (p < e) {
		headers = curl_(slist_append)(headers, p);
		p += strlen(p) + 1;
	}
	if (headers)
		curl_(easy_setopt)(ch, CURLOPT_HTTPHEADER, headers);

	curl_(easy_setopt)(ch, CURLOPT_WRITEFUNCTION, string_writer);
	curl_(easy_setopt)(ch, CURLOPT_WRITEDATA, &reply->body);
	curl_(easy_setopt)(ch, CURLOPT_HEADERFUNCTION, string_writer0);
	curl_(easy_setopt)(ch, CURLOPT_HEADERDATA, &reply->header);

	curl_(easy_perform)(ch);
	long code;
	curl_(easy_getinfo)(ch, CURLINFO_RESPONSE_CODE, &code);
	reply->code = (int) code;

	if (headers)
		curl_(slist_free_all)(headers);
	curl_(easy_cleanup)(ch);
	return true;
}

static int string_gc(lua_State *L)
{
	String *string = lua_touserdata(L, 1);
	free(string->str);
	return 0;
}

static int readonly_error(lua_State *L)
{
	luaL_error(L, "attempt to update a read-only table");
	return 0;
}

static int ordered_next(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(2));
	lua_rawget(L, 1);
	if (lua_isnil(L, -1))
		return 1;

	lua_pushvalue(L, -1);
	lua_rawget(L, lua_upvalueindex(1));

	int i = lua_tointeger(L, lua_upvalueindex(2));
	lua_pushinteger(L, i + 1);
	lua_replace(L, lua_upvalueindex(2));
	return 2;
}

static int ordered_pairs(lua_State *L)
{
	lua_pushvalue(L, 1);
	lua_pushinteger(L, 1);
	lua_pushcclosure(L, ordered_next, 2);
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_pushnil(L);
	return 3;
}

static int w_request(lua_State *L)
{
	Context *curl = lua_touserdata(L, lua_upvalueindex(2));
	Request req;
	Reply reply;
	bool advanced = false;
	const char *post_defhdr = "Content-Type: "
		"application/x-www-form-urlencoded";
	memset(&req, 0, sizeof(req));
	memset(&reply, 0, sizeof(reply));

	req.method = "GET";
	lua_getfield(L, lua_upvalueindex(1), "TIMEOUT");
	if (!lua_isnoneornil(L, -1)) {
		double timeout = luaL_checknumber(L, -1);
		req.timeout = timeout * 1000;
	}
	lua_pop(L, 1);

	int table_idx = 0;
	if (lua_istable(L, 1)) {
		table_idx = 1;
	} else {
		req.url = luaL_checklstring(L, 1, &req.urllen);
		if (lua_isstring(L, 2)) {
			req.postdata = (char *)
				luaL_checklstring(L, -1, &req.postdatalen);
			req.method = "POST";
			string_writer(post_defhdr, 1,
				      strlen(post_defhdr) + 1, &req.header);
		}
	}

	if (table_idx && lua_istable(L, table_idx)) {
		const char *defhdr = NULL;
		advanced = true;

		if (req.url == NULL) {
			lua_getfield(L, table_idx, "url");
			req.url = luaL_checklstring(L, -1, &req.urllen);
			lua_pop(L, 1);
		}

		lua_getfield(L, table_idx, "data");
		if (!lua_isnoneornil(L, -1)) {
			req.postdata = (char *)
				luaL_checklstring(L, -1, &req.postdatalen);
			req.method = "POST";
			defhdr = post_defhdr;
		}
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "method");
		if (!lua_isnoneornil(L, -1)) {
			req.method = luaL_checkstring(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "headers");
		bool flag = false;
		if (!lua_isnoneornil(L, -1)) {
			String *buffer = lua_newuserdata(L, sizeof(String));
			memset(buffer, 0, sizeof(String));
			lua_createtable(L, 0, 1);
			lua_pushcfunction(L, string_gc);
			lua_setfield(L, -2, "__gc");
			lua_setmetatable(L, -2);

			int idx = lua_gettop(L) - 1;
			lua_getglobal(L, "pairs");
			lua_pushvalue(L, idx);
			lua_call(L, 1, 3);
			while (true) {
				/* _f _s _var */
				lua_pushvalue(L, -3);
				lua_pushvalue(L, -3);
				lua_rotate(L, -3, -1);
				/* _f _s _f _s _var */
				lua_call(L, 2, 2);
				/* _f _s _var val */
				if (lua_isnil(L, -2)) {
					lua_pop(L, 4);
					break;
				}
				size_t hl, cl;
				const char *h = luaL_checklstring(L, -2, &hl);
				const char *c = luaL_checklstring(L, -1, &cl);
				if (strcmp(h, "Content-Type") == 0)
					flag = true;
				string_writer(h, 1, hl, buffer);
				string_writer(": ", 1, 2, buffer);
				string_writer(c, 1, cl, buffer);
				string_writer("", 1, 1, buffer);
				lua_pop(L, 1);
			}
			req.header = *buffer;
			buffer->str = NULL;
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
		if (!flag && defhdr) {
			string_writer(defhdr, 1,
				      strlen(defhdr) + 1, &req.header);
		}
	}

	bool ok = request(curl, &req, &reply);
	free(req.header.str);

	if (!ok) {
		lua_pushnil(L);
		lua_pushstring(L, reply.error);
		return 2;
	}

	lua_pushlstring(L, reply.body.str, reply.body.len);
	lua_pushinteger(L, reply.code);
	free(reply.body.str);

	if (advanced) {
		int fid = 1;
		lua_newtable(L); // field array, for keeping order

		lua_newtable(L); // header map
		char *p = reply.header.str;
		char *e = reply.header.str + reply.header.len;
		while (p < e) {
			char *q = strchr(p, ':');
			size_t plen = strlen(p);
			if (q) {
				lua_pushlstring(L, p, q - p);
				char *r = strchr(p + 1, '\r');
				if (!r)
					r = p + plen;
				char *s = q + 1;
				while (*s == ' ') s++;
				size_t xlen = r - s;
				lua_pushvalue(L, -1);
				lua_gettable(L, -3);
				if (lua_isnil(L, -1)) {
					lua_pop(L, 1);

					lua_pushvalue(L, -1);
					lua_rawseti(L, -4, fid++);

					lua_pushlstring(L, s, xlen);
				} else {
					size_t olen;
					const char *ostr =
						lua_tolstring(L, -1, &olen);
					char *nstr = malloc(olen + 2 + xlen);
					memcpy(nstr, ostr, olen);
					memcpy(nstr + olen, ", ", 2);
					memcpy(nstr + olen + 2, s, xlen);
					lua_pop(L, 1);
					lua_pushlstring(L, nstr,
							olen + 2 + xlen);
					free(nstr);
				}
				lua_settable(L, -3);
			}
			p += plen + 1;
		}
		free(reply.header.str);

		lua_createtable(L, 0, 3);
		lua_pushvalue(L, -2);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, readonly_error);
		lua_setfield(L, -2, "__newindex");
		lua_pushvalue(L, -3);
		lua_pushcclosure(L, ordered_pairs, 1);
		lua_setfield(L, -2, "__pairs");
		lua_setmetatable(L, -2);
		lua_remove(L, -2);
		return 3;
	}
	return 2;
}

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

static Context curl;

extern int DLLEXPORT luaopen_simplehttp(lua_State *L)
{
	if (!init(&curl))
		luaL_error(L, "unable to initialize libcurl");

	lua_newtable(L);

	lua_pushvalue(L, -1);
	lua_pushlightuserdata(L, &curl);
	lua_pushcclosure(L, w_request, 2);
	lua_setfield(L, -2, "request");
	return 1;
}

__attribute__((destructor))
static int close_simplehttp(void)
{
	cleanup(&curl);
	return 0;
}
