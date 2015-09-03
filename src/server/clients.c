/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * clients.c
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
 * Client list and channel defination
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 04/29/2015
 * @changelog
 *      [04/29/2015] - Creation
 */

#include "../bspd.h"

static BSP_MEMPOOL *mp_session = NULL;
static BSP_MEMPOOL *mp_channel = NULL;
static BSPD_CHANNEL *global_channel = NULL;
static BSP_OBJECT *static_channel_list = NULL;
static BSPD_CHANNEL **dynamic_channel_list = NULL;
static size_t dynamic_channel_list_size = 0;
static size_t next_dynamic_channel_id = 0;

int clients_init()
{
    // Session pool
    mp_session = bsp_new_mempool(sizeof(BSPD_SESSION), NULL, NULL);
    if (!mp_session)
    {
        return BSP_RTN_ERR_MEMORY;
    }

    // Channel pool
    mp_channel = bsp_new_mempool(sizeof(BSPD_CHANNEL), NULL, NULL);
    if (!mp_channel)
    {
        return BSP_RTN_ERR_MEMORY;
    }

    // Global channel
    global_channel = bsp_mempool_alloc(mp_channel);
    if (!global_channel)
    {
        bsp_trace_message(I_EMERG, "bspd", "Create global channel error");
        _exit(BSP_RTN_FATAL);
    }

    // Static channel list
    static_channel_list = bsp_new_object(BSP_OBJECT_HASH);
    if (!static_channel_list)
    {
        bsp_trace_message(I_EMERG, "bspd", "Create static channel list error");
        _exit(BSP_RTN_FATAL);
    }
    
    // Dynamic channel list
    dynamic_channel_list = bsp_calloc(DYNAMIC_CHANNEL_LIST_INITIAL, sizeof(BSPD_CHANNEL *));
    if (!dynamic_channel_list)
    {
        bsp_trace_message(I_EMERG, "bspd", "Create dynamic channel list error");
        _exit(BSP_RTN_ERR_MEMORY);
    }

    dynamic_channel_list_size = DYNAMIC_CHANNEL_LIST_INITIAL;
    bzero(global_channel, sizeof(BSPD_CHANNEL));
    global_channel->type = BSPD_CHANNEL_GLOBAL;

    return BSP_RTN_SUCCESS;
}

/* Clients */
int reg_client(BSP_SOCKET_CLIENT *clt)
{
    if (!clt)
    {
        return BSP_RTN_INVALID;
    }

    /*
    int fd = clt->sck.fd;
    BSP_FD *f = bsp_get_fd(fd, BSP_FD_SOCKET_CLIENT);
    if (f)
    {
        FD_ADD_SET(f, BSPD_FD_ADD_CLT, (void *) clt);
    }
    */
    BSPD_SESSION *session = new_session();
    bind_session(clt, session);

    return BSP_RTN_SUCCESS;
}

int unreg_client(BSP_SOCKET_CLIENT *clt)
{
    if (!clt)
    {
        return BSP_RTN_INVALID;
    }

    /*
    int fd = clt->sck.fd;
    BSP_FD *f = bsp_get_fd(fd, BSP_FD_SOCKET_CLIENT);
    if (f)
    {
        FD_ADD_SET(f, BSPD_FD_ADD_CLT, NULL);
    }
    */

    BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
    if (session && BSP_FALSE == session->logged)
    {
        // Delete session
        del_session(session);
    }

    return BSP_RTN_SUCCESS;
}

BSP_SOCKET_CLIENT * check_client(int fd)
{
    BSP_FD *f = bsp_get_fd(fd, BSP_FD_SOCKET_CLIENT);
    if (f)
    {
        return (BSP_SOCKET_CLIENT *) (FD_PTR(f));
    }

    return NULL;
}

/* User session */
BSPD_SESSION * new_session()
{
    BSPD_SESSION *ret = bsp_mempool_alloc(mp_session);
    if (ret)
    {
        bzero(ret, sizeof(BSPD_SESSION));
        ret->connect_time = time(NULL);
        ret->serialize_type = BSPD_SERIALIZE_NATIVE;
        ret->compress_type = BSPD_COMPRESS_NONE;
        ret->logged = BSP_FALSE;
        ret->reported = BSP_FALSE;
    }

    return ret;
}

int set_session_id(BSPD_SESSION *session, const char *session_id)
{
    if (!session || !session_id)
    {
        return BSP_RTN_INVALID;
    }

    strncpy(session->session_id, session_id, MAX_SESSION_ID_LENGTH - 1);
    session->session_id[MAX_SESSION_ID_LENGTH - 1] = 0x0;

    return BSP_RTN_SUCCESS;
}

int del_session(BSPD_SESSION *session)
{
    if (!session)
    {
        return BSP_RTN_INVALID;
    }

    // Try log off
    quit_channel(global_channel, session);
    size_t i;
    for (i = 0; i < session->total_joined; i ++)
    {
        quit_channel(session->joined_list[i], session);
    }

    bsp_free(session->joined_list);
    bsp_mempool_free(mp_session, session);

    return BSP_RTN_SUCCESS;
}

BSPD_SESSION * check_logged_session(const char *session_id)
{
    if (!session_id)
    {
        return NULL;
    }

    BSP_VALUE *val = bsp_object_value_hash_original(global_channel->list, session_id);
    BSPD_SESSION *ret = (val) ? ((BSPD_SESSION *) (V_GET_POINTER(val))) : NULL;

    return ret;
}

int bind_session(BSP_SOCKET_CLIENT *clt, BSPD_SESSION *session)
{
    if (!clt || !session)
    {
        return BSP_RTN_INVALID;
    }

    session->bind = clt;
    clt->additional = (void *) session;

    return BSP_RTN_SUCCESS;
}

int logon_session(BSPD_SESSION *session)
{
    int ret = join_channel(global_channel, session);
    if (BSP_RTN_SUCCESS == ret)
    {
        session->logged = BSP_TRUE;
    }

    return ret;
}

int logoff_session(BSPD_SESSION *session)
{
    int ret = quit_channel(global_channel, session);
    if (BSP_RTN_SUCCESS == ret)
    {
        session->logged = BSP_FALSE;
    }

    return ret;
}

/* Channels */
BSPD_CHANNEL * new_channel(const char *name)
{
    BSPD_CHANNEL *channel = bsp_mempool_alloc(mp_channel);
    if (channel)
    {
        bzero(channel, sizeof(BSPD_CHANNEL));
        if (name)
        {
            // Static (named) channel
            BSP_VALUE *vc = bsp_object_value_hash_original(static_channel_list, name);
            if (vc)
            {
                BSPD_CHANNEL *old = (BSPD_CHANNEL *) (V_GET_POINTER(vc));
                if (old)
                {
                    // Delete old channel
                    bsp_trace_message(BSP_TRACE_DEBUG, "channel", "New static <%s> channel created instead the old one", name);
                    del_channel(old);
                }
            }

            vc = bsp_new_value();
            if (!vc)
            {
                bsp_mempool_free(mp_channel, channel);

                return NULL;
            }

            strncpy(channel->name, name, MAX_CHANNEL_NAME_LENGTH - 1);
            channel->name[MAX_CHANNEL_NAME_LENGTH - 1] = 0x0;
            BSP_STRING *name_str = bsp_new_const_string(channel->name, -1);
            if (!name_str)
            {
                bsp_del_value(vc);

                return NULL;
            }

            V_SET_POINTER(vc, channel);
            bsp_object_set_hash(static_channel_list, name_str, vc);

            channel->type = BSPD_CHANNEL_STATIC;
        }
        else
        {
            // Dynamic (anonymous) channel
            dynamic_channel_list[next_dynamic_channel_id] = channel;
            channel->id = next_dynamic_channel_id;
            channel->type = BSPD_CHANNEL_DYNAMIC;

            // Find next
            next_dynamic_channel_id = 0;
            size_t i;
            for (i = next_dynamic_channel_id + 1; i < dynamic_channel_list_size; i ++)
            {
                if (!dynamic_channel_list[i])
                {
                    next_dynamic_channel_id = i;
                    break;
                }
            }

            if (0 == next_dynamic_channel_id)
            {
                // List full
                BSPD_CHANNEL **new_list = bsp_realloc(dynamic_channel_list, 2 * dynamic_channel_list_size * sizeof(BSPD_CHANNEL *));
                if (!new_list)
                {
                    bsp_mempool_free(mp_channel, channel);

                    return NULL;
                }

                dynamic_channel_list = new_list;
                next_dynamic_channel_id = dynamic_channel_list_size;
                dynamic_channel_list_size *= 2;
            }
        }

        return channel;
    }

    return NULL;
}

int del_channel(BSPD_CHANNEL *channel)
{
    if (!channel || channel == global_channel)
    {
        return BSP_RTN_INVALID;
    }

    BSP_STRING *name_str = NULL;
    switch (channel->type)
    {
        case BSPD_CHANNEL_DYNAMIC : 
            // Remove from list
            if (dynamic_channel_list[channel->id] == channel)
            {
                dynamic_channel_list[channel->id] = NULL;
                if (channel->id < next_dynamic_channel_id)
                {
                    next_dynamic_channel_id = channel->id;
                }
            }

            break;
        case BSPD_CHANNEL_STATIC : 
            // Remove from list
            name_str = bsp_new_const_string(channel->name, -1);
            bsp_object_set_hash(static_channel_list, name_str, NULL);
            bsp_del_string(name_str);

            break;
        default : 
            break;
    }

    // Make removal
    //bsp_object_reset(channel->list)
    bsp_del_object(channel->list);
    bsp_mempool_free(mp_channel, channel);

    return BSP_RTN_SUCCESS;
}

BSPD_CHANNEL * check_static_channel(const char *name)
{
    if (!name)
    {
        return NULL;
    }

    BSP_VALUE *vc = bsp_object_value_hash_original(static_channel_list, name);
    if (vc)
    {
        return (BSPD_CHANNEL *) (V_GET_POINTER(vc));
    }

    return NULL;
}

BSPD_CHANNEL * check_dynamic_channel(int id)
{
    if (id >= dynamic_channel_list_size)
    {
        return NULL;
    }

    BSPD_CHANNEL *channel = dynamic_channel_list[id];

    return channel;
}

int join_channel(BSPD_CHANNEL *channel, BSPD_SESSION *session)
{
    if (!channel || !session)
    {
        return BSP_RTN_INVALID;
    }

    if (!channel->list)
    {
        channel->list = bsp_new_object(BSP_OBJECT_HASH);
        if (!channel->list)
        {
            return BSP_RTN_ERR_MEMORY;
        }
    }

    if (0 == strnlen(session->session_id, MAX_SESSION_ID_LENGTH))
    {
        if (session->session_id_int > 0)
        {
            // Internal id set
            sprintf(session->session_id, "%llu", (unsigned long long int) session->session_id_int);
        }
        else
        {
            // No id
            return BSP_RTN_ERR_GENERAL;
        }
    }

    BSP_VALUE *val = bsp_new_value();
    if (!val)
    {
        return BSP_RTN_ERR_MEMORY;
    }

    BSP_STRING *key = bsp_new_const_string(session->session_id, -1);
    if (!key)
    {
        bsp_del_value(val);

        return BSP_RTN_ERR_MEMORY;
    }

    V_SET_POINTER(val, session);
    bsp_object_set_hash(channel->list, key, val);
    while (session->total_joined >= session->joined_list_size)
    {
        if (!session->joined_list)
        {
            session->joined_list = bsp_calloc(SESSION_JOINED_LIST_INITIAL, sizeof(BSPD_CHANNEL *));
            session->joined_list_size = SESSION_JOINED_LIST_INITIAL;
            session->total_joined = 0;
        }
        else
        {
            session->joined_list = bsp_realloc(session->joined_list, 2 * session->joined_list_size * sizeof(BSPD_CHANNEL *));
            session->joined_list_size *= 2;
        }

        if (!session->joined_list)
        {
            return BSP_RTN_ERR_MEMORY;
        }
    }

    session->joined_list[session->total_joined ++] = channel;

    return BSP_RTN_SUCCESS;
}

int quit_channel(BSPD_CHANNEL *channel, BSPD_SESSION *session)
{
    if (!channel || !channel->list || BSP_OBJECT_HASH != channel->list->type || !session)
    {
        return BSP_RTN_INVALID;
    }

    if (0 == strnlen(session->session_id, MAX_SESSION_ID_LENGTH))
    {
        // No id
        return BSP_RTN_ERR_GENERAL;
    }

    BSP_STRING *key = bsp_new_const_string(session->session_id, -1);
    bsp_object_set_hash(channel->list, key, NULL);
    bsp_del_string(key);

    // Del dynamic channel if no member in it
    if (BSPD_CHANNEL_DYNAMIC == channel->type && 0 == bsp_object_size(channel->list))
    {
        del_channel(channel);
    }

    return BSP_RTN_SUCCESS;
}
