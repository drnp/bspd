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
    if (!channel)
    {
        lua_pushnil(s);

        return 1;
    }

    int client_fd = 0;
    const char *session_id = NULL;
    BSP_SOCKET_CLIENT *clt = NULL;
    BSPD_SESSION *session = NULL;
    BSP_OBJECT *object = NULL;
    size_t sent = 0;

    // Check params
    if (lua_isnumber(s, 2) && lua_istable(s, 3))
    {
        // Command
        int cmd = lua_tointeger(s, 2);
        object = lua_to_object(s, 3);
        //sent = send_command(clt, cmd, object);
        bsp_del_object(object);
        lua_pushinteger(s, (int) sent);
    }
    else if (lua_istable(s, 2))
    {
        // Object
        object = lua_to_object(s, 2);
        //sent = send_object(clt, object);
        bsp_del_object(object);
        lua_pushinteger(s, (int) sent);
    }
    else if (lua_isstring(s, 2))
    {
        // Raw or stream
        size_t str_len = 0;
        const char *str = lua_tolstring(s, 2, &str_len);
        BSP_STRING *data = bsp_new_const_string(str, str_len);
        //sent = send_string(clt, data);
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
        lua_pushboolean(s, BSP_FALSE);

        return 1;
    }

    BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
    if (session)
    {
        logoff_session(session);
    }

    session = new_session(session_id);
    if (session)
    {
        bind_session(clt, session);
        logon_session(session);
        lua_pushlightuserdata(s, (void *) session);
    }
    else
    {
        lua_pushnil(s);
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

    if (!lua_isstring(s, -1) && !lua_isnumber(s, -1))
    {
        lua_pushnil(s);
    }

    const char *session_id = lua_tostring(s, -1);
    BSPD_SESSION *session = check_session(session_id);
    del_session(session);

    return 0;
}

/* Module */
int module_standard(lua_State *s)
{
    if (!s || !lua_checkstack(s, 1))
    {
        return 0;
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

    return 0;
}
