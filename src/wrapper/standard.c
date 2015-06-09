/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * standard.c
 * Copyright (C) 2015 Dr.NP <conan.np@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Dr.NP nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Dr.NP AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Dr.NP OR ANY OTHER
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Standard apis for LUA
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 04/29/2015
 * @changelog
 *      [04/19/1015] - Creation
 */

#include "../bspd.h"

/* Network */
static int standard_net_close(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    int client_fd = 0;
    const char *session_id = NULL;
    BSP_SOCKET_CLIENT *clt = NULL;
    BSPD_SESSION *session = NULL;

    if (lua_isnumber(s, -1))
    {
        // Close fd
        client_fd = lua_tointeger(s, -1);
        clt = check_client(client_fd);
    }
    else if (lua_isstring(s, -1));
    {
        // Close session
        session_id = lua_tostring(s, -1);
        session = check_session(session_id);
        if (session)
        {
            clt = session->bind;
        }
    }

    if (clt)
    {
        bsp_socket_close(&clt->sck);
    }

    return 0;
}

static int standard_net_send(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    int client_fd = 0;
    const char *session_id = NULL;
    BSP_SOCKET_CLIENT *clt = NULL;
    BSPD_SESSION *session = NULL;
    BSP_OBJECT *object = NULL;
    size_t sent = 0;

    if (lua_isnumber(s, 1))
    {
        // Send to fd
        client_fd = lua_tointeger(s, 1);
        clt = check_client(client_fd);
    }
    else if (lua_isstring(s, 1))
    {
        // Send to session
        session_id = lua_tostring(s, 1);
        session = check_session(session_id);
        if (session)
        {
            clt = session->bind;
        }
    }

    if (!clt)
    {
        // No client
        lua_pushinteger(s, 0);

        return 1;
    }

    // Check params
    if (lua_isnumber(s, 2) && lua_istable(s, 3))
    {
        // Command
        int cmd = lua_tointeger(s, 2);
        object = lua_to_object(s, 3);
        sent = send_command(clt, cmd, object);
        bsp_del_object(object);
        lua_pushinteger(s, (int) sent);
    }
    else if (lua_istable(s, 2))
    {
        // Object
        object = lua_to_object(s, 2);
        sent = send_object(clt, object);
        bsp_del_object(object);
        lua_pushinteger(s, (int) sent);
    }
    else if (lua_isstring(s, 2))
    {
        // Raw or stream
        size_t str_len = 0;
        const char *str = lua_tolstring(s, 2, &str_len);
        BSP_STRING *data = bsp_new_const_string(str, str_len);
        sent = send_string(clt, data);
        bsp_del_string(data);
        lua_pushinteger(s, (int) sent);
    }
    else
    {
        lua_pushinteger(s, 0);

        return 1;
    }

    return 1;
}

static int standard_net_send_channel(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    if (!lua_isnumber(s, 1))
    {
        lua_pushnil(s);

        return 1;
    }

    int channel_id = lua_tointeger(s, 1);
    BSPD_CHANNEL *channel = check_channel(channel_id);
    if (!channel || !channel->list)
    {
        lua_pushnil(s);

        return 1;
    }

    BSP_SOCKET_CLIENT *clt = NULL;
    BSPD_SESSION *session = NULL;
    BSP_OBJECT *object = NULL;
    BSP_VALUE *val = NULL;
    size_t sent = 0;

    // Check params
    if (lua_isnumber(s, 2) && lua_istable(s, 3))
    {
        // Command
        int cmd = lua_tointeger(s, 2);
        object = lua_to_object(s, 3);
        bsp_object_reset(channel->list);
        val = bsp_object_curr(channel->list, NULL);
        while (val)
        {
            session = (BSPD_SESSION *) (V_GET_POINTER(val));
            if (session && session->bind)
            {
                clt = (BSP_SOCKET_CLIENT *) session->bind;
                send_command(clt, cmd, object);
                sent ++;
            }

            bsp_object_next(channel->list);
            val = bsp_object_curr(channel->list, NULL);
        }

        bsp_del_object(object);
        lua_pushinteger(s, (int) sent);
    }
    else if (lua_istable(s, 2))
    {
        // Object
        object = lua_to_object(s, 2);
        bsp_object_reset(channel->list);
        val = bsp_object_curr(channel->list, NULL);
        while (val)
        {
            session = (BSPD_SESSION *) (V_GET_POINTER(val));
            if (session && session->bind)
            {
                clt = (BSP_SOCKET_CLIENT *) session->bind;
                send_object(clt, object);
                sent ++;
            }

            bsp_object_next(channel->list);
            val = bsp_object_curr(channel->list, NULL);
        }

        bsp_del_object(object);
        lua_pushinteger(s, (int) sent);
    }
    else if (lua_isstring(s, 2))
    {
        // Raw or stream
        size_t str_len = 0;
        const char *str = lua_tolstring(s, 2, &str_len);
        BSP_STRING *data = bsp_new_const_string(str, str_len);
        bsp_object_reset(channel->list);
        val = bsp_object_curr(channel->list, NULL);
        while (val)
        {
            session = (BSPD_SESSION *) (V_GET_POINTER(val));
            if (session && session->bind)
            {
                clt = (BSP_SOCKET_CLIENT *) session->bind;
                send_string(clt, data);
                sent ++;
            }

            bsp_object_next(channel->list);
            val = bsp_object_curr(channel->list, NULL);
        }

        bsp_del_string(data);
        lua_pushinteger(s, (int) sent);
    }
    else
    {
        lua_pushinteger(s, 0);

        return 1;
    }

    return 1;
}

// Session
static int standard_reg_session(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    if ((!lua_isstring(s, -1) && !lua_isnumber(s, -1)) || !lua_isnumber(s, -2))
    {
        lua_pushnil(s);

        return 1;
    }

    const char *session_id = lua_tostring(s, -1);
    int client_fd = lua_tointeger(s, -2);
    BSP_SOCKET_CLIENT *clt = check_client(client_fd);
    if (!clt)
    {
        // Client not exists
        lua_pushnil(s);

        return 1;
    }

    BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
    // Check previous logged session
    BSPD_SESSION *old_session = check_session(session_id);
    if (old_session)
    {
        if (session != old_session)
        {
            // Bind to client
            del_session(session);
            bind_session(clt, old_session);
            lua_pushlightuserdata(s, (void *) old_session);
        }
    }
    else
    {
        if (!session)
        {
            session = new_session();
            bind_session(clt, session);
        }

        set_session(session, session_id);
        logon_session(session);
        lua_pushlightuserdata(s, (void *) session);
    }

    return 1;
}

static int standard_unreg_session(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    BSPD_SESSION *session = NULL;
    if (lua_isstring(s, -1))
    {
        const char *session_id = lua_tostring(s, -1);
        session = check_session(session_id);
    }
    else if (lua_isnumber(s, -1))
    {
        int client_fd = lua_tointeger(s, -1);
        BSP_SOCKET_CLIENT *clt = check_client(client_fd);
        if (clt)
        {
            session = (BSPD_SESSION *) clt->additional;
        }
    }

    del_session(session);

    return 0;
}

static int standard_check_session(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    int client_fd = lua_tointeger(s, -1);
    BSP_SOCKET_CLIENT *clt = check_client(client_fd);
    if (clt)
    {
        BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
        if (session)
        {
            lua_pushstring(s, session->session_id);
        }
        else
        {
            lua_pushnil(s);
        }
    }
    else
    {
        lua_pushnil(s);
    }

    return 1;
}

// Channel
static int standard_new_channel(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    int channel_id = new_channel(BSPD_CHANNEL_DYNAMIC);
    if (channel_id > 0)
    {
        lua_pushnumber(s, channel_id);
    }
    else
    {
        lua_pushnil(s);
    }

    return 1;
}

static int standard_del_channel(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_isnumber(s, -1))
    {
        return 0;
    }

    int channel_id = lua_tointeger(s, -1);
    del_channel(channel_id);

    return 0;
}

static int standard_join_channel(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    if (!lua_isstring(s, -1) || !lua_isnumber(s, -2))
    {
        lua_pushboolean(s, BSP_FALSE);

        return 1;
    }

    const char *session_id = lua_tostring(s, -1);
    int channel_id = lua_tointeger(s, -2);
    BSPD_SESSION *session = check_session(session_id);
    BSPD_CHANNEL *channel = check_channel(channel_id);

    if (BSP_RTN_SUCCESS == add_session_to_channel(channel, session))
    {
        lua_pushboolean(s, BSP_TRUE);
    }
    else
    {
        lua_pushboolean(s, BSP_FALSE);
    }

    return 1;
}

static int standard_leave_channel(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    if (!lua_isstring(s, -1) || !lua_isnumber(s, -2))
    {
        lua_pushboolean(s, BSP_FALSE);

        return 1;
    }

    const char *session_id = lua_tostring(s, -1);
    int channel_id = lua_tointeger(s, -2);
    BSPD_SESSION *session = check_session(session_id);
    BSPD_CHANNEL *channel = check_channel(channel_id);

    if (BSP_RTN_SUCCESS == remove_session_from_channel(channel, session))
    {
        lua_pushboolean(s, BSP_TRUE);
    }
    else
    {
        lua_pushboolean(s, BSP_FALSE);
    }

    return 1;
}

static int standard_list_channel(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    int channel_id = lua_tointeger(s, -1);
    BSPD_CHANNEL *channel = check_channel(channel_id);

    if (channel && channel->list && BSP_OBJECT_HASH == channel->list->type)
    {
        BSP_STRING *key;
        BSP_VALUE *val;
        int idx = 1;
        lua_createtable(s, bsp_object_size(channel->list), 0);
        bsp_spin_lock(&channel->list->lock);
        bsp_object_reset(channel->list);
        val = bsp_object_curr(channel->list, (void **) &key);
        while (val)
        {
            if (!key)
            {
                break;
            }

            lua_pushinteger(s, idx ++);
            lua_pushlstring(s, STR_STR(key), STR_LEN(key));
            lua_settable(s, -3);
            bsp_object_next(channel->list);
            val = bsp_object_curr(channel->list, (void **) &key);
        }

        bsp_spin_unlock(&channel->list->lock);
    }
    else
    {
        lua_pushnil(s);
    }

    return 1;
}

static int standard_check_channel(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    const char *session_id = lua_tostring(s, -1);
    if (!session_id)
    {
        lua_pushnil(s);

        return 1;
    }

    BSPD_SESSION *session = check_session(session_id);
    if (!session)
    {
        lua_pushnil(s);

        return 1;
    }

    lua_newtable(s);
    lua_pushstring(s, "global");
    lua_pushinteger(s, 0);
    lua_settable(s, -3);

    lua_pushstring(s, "static");
    lua_pushinteger(s, session->static_channel);
    lua_settable(s, -3);

    lua_pushstring(s, "dynamic");
    lua_pushinteger(s, session->dynamic_channel);
    lua_settable(s, -3);

    return 1;
}

// Timer
static void _trigger_timer(BSP_TIMER *tmr)
{
    fprintf(stderr, "Timer\n");

    return;
}

static void _trigger_complete(BSP_TIMER *tmr)
{
    fprintf(stderr, "Timer complete\n");

    return;
}

static int standard_new_timer(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_checkstack(s, 1))
    {
        return 0;
    }

    if (!lua_isnumber(s, 1))
    {
        lua_pushnil(s);

        return 1;
    }

    struct timespec initial = {.tv_sec = 0, .tv_nsec = 0};
    struct timespec interval = {.tv_sec = 0, .tv_nsec = 0};
    ssize_t loop = 1;

    double vinitial = lua_tonumber(s, 1);
    initial.tv_sec = (time_t) vinitial;
    initial.tv_nsec = (long) ((vinitial - (double) initial.tv_sec) * 1000000000);
    if (lua_isnumber(s, 2))
    {
        // Has interval
        double vinterval = lua_tonumber(s, 2);
        interval.tv_sec = (time_t) vinterval;
        interval.tv_nsec = (long) ((vinterval - (double) interval.tv_nsec) * 1000000000);
    }

    if (lua_isnumber(s, 3))
    {
        // Hash loop
        loop = lua_tointeger(s, 3);
    }

    BSP_THREAD *t = bsp_self_thread();
    if (!t)
    {
        // Cannot add timer to main thread
        lua_pushnil(s);

        return 1;
    }

    BSP_TIMER *tmr = bsp_new_timer(t->event_container, &initial, &interval, loop);

    if (tmr)
    {
        tmr->on_timer = _trigger_timer;
        tmr->on_complete = _trigger_complete;
        lua_pushlightuserdata(s, (void *) tmr);
    }
    else
    {
        lua_pushnil(s);
    }

    return 1;
}

static int standard_stop_timer(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    if (!lua_islightuserdata(s, -1))
    {
        lua_pushnil(s);

        return 1;
    }

    BSP_TIMER *tmr = (BSP_TIMER *) lua_touserdata(s, -1);
    lua_pushboolean(s, (BSP_RTN_SUCCESS == bsp_del_timer(tmr)) ? 1 : 0);

    return 1;
}

/* Module */
int module_standard(lua_State *s)
{
    if (!s || !lua_checkstack(s, 1))
    {
        return BSP_RTN_INVALID;
    }

    lua_pushcfunction(s, standard_net_close);
    lua_setglobal(s, "bsp_net_close");

    lua_pushcfunction(s, standard_net_send);
    lua_setglobal(s, "bsp_net_send");

    lua_pushcfunction(s, standard_net_send_channel);
    lua_setglobal(s, "bsp_net_send_channel");

    lua_pushcfunction(s, standard_reg_session);
    lua_setglobal(s, "bsp_reg_session");

    lua_pushcfunction(s, standard_unreg_session);
    lua_setglobal(s, "bsp_unreg_session");

    lua_pushcfunction(s, standard_check_session);
    lua_setglobal(s, "bsp_check_session");

    lua_pushcfunction(s, standard_new_channel);
    lua_setglobal(s, "bsp_new_channel");

    lua_pushcfunction(s, standard_del_channel);
    lua_setglobal(s, "bsp_del_channel");

    lua_pushcfunction(s, standard_join_channel);
    lua_setglobal(s, "bsp_join_channel");

    lua_pushcfunction(s, standard_leave_channel);
    lua_setglobal(s, "bsp_leave_channel");

    lua_pushcfunction(s, standard_list_channel);
    lua_setglobal(s, "bsp_list_channel");

    lua_pushcfunction(s, standard_check_channel);
    lua_setglobal(s, "bsp_check_channel");

    lua_pushcfunction(s, standard_new_timer);
    lua_setglobal(s, "bsp_new_timer");

    lua_pushcfunction(s, standard_stop_timer);
    lua_setglobal(s, "bsp_stop_timer");

    return BSP_RTN_SUCCESS;
}
