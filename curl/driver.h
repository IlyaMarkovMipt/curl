/*
 * Copyright (C) 2016 Tarantool AUTHORS: please see AUTHORS file.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef DRIVER_H
#define DRIVER_H 1

#include <assert.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <tarantool/module.h>

#include <curl/curl.h>

#include "utils.h"

/**
 * Unique name for userdata metatables
 */
#define DRIVER_LUA_UDATA_NAME	"__tnt_curl"
#define WORK_TIMEOUT 0.3

typedef struct {
  int sfd;
  int type;
} event_t;

typedef struct {
	lua_State *L;
  CURL *easy;
  CURLM *multi;
  struct fiber *fiber;
  struct fiber *io_fiber;
  int still_running;
  bool need_work;

  event_t events[64];
  size_t events_tail;

  struct curl_slist *headers;
} curl_t;


static inline
void
push_event(curl_t *ctx, event_t event)
{
  if (ctx->events_tail < 64) {
    ctx->events[ctx->events_tail] = event;
    ++ctx->events_tail;
  }
}

static inline
bool
pop_event(curl_t *ctx, event_t *event)
{
  assert(event);

  if (ctx->events_tail != 0) {
    *event = ctx->events[ctx->events_tail - 1];
    --ctx->events_tail;
    return true;
  }
  return false;
}


static inline
curl_t*
curl_get(lua_State *L)
{
	return (curl_t *) luaL_checkudata(L, 1, DRIVER_LUA_UDATA_NAME);
}


static inline
int
curl_make_result(lua_State *L, CURLcode code, CURLMcode mcode)
{
  const char *emsg = NULL;
  if (code != CURL_LAST)
    emsg = curl_easy_strerror(code);
  else if (mcode != CURLM_LAST)
    emsg = curl_multi_strerror(mcode);
  return make_str_result(L,
        code != CURLE_OK,
        (emsg != NULL ? emsg : "ok"));
}

#endif
