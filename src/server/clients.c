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

static BSP_SOCKET_CLIENT **client_list = NULL;
static size_t client_list_size = 0;
static BSPD_CHANNEL **channel_list = NULL;
static size_t channel_list_size = 0;
static size_t next_channel_id = 0;
static BSP_MEMPOOL *mp_session = NULL;
static BSP_MEMPOOL *mp_channel = NULL;
static BSPD_CHANNEL *global_channel = NULL;

int clients_init()
{
    // FD list
    if (!client_list)
    {
        client_list = bsp_calloc(CLIENT_LIST_INITIAL, sizeof(BSP_SOCKET_CLIENT *));
        if (!client_list)
        {
            return BSP_RTN_ERR_MEMORY;
        }

        client_list_size = CLIENT_LIST_INITIAL;
    }

    // Channel list
    if (!channel_list)
    {
        channel_list = bsp_calloc(CHANNEL_LIST_INITIAL, sizeof(BSPD_CHANNEL *));
        if (!channel_list)
        {
            return BSP_RTN_ERR_MEMORY;
        }

        channel_list_size = CHANNEL_LIST_INITIAL;
    }

    // Session pool
    if (!mp_session)
    {
        mp_session = bsp_new_mempool(sizeof(BSPD_SESSION), NULL, NULL);
        if (!mp_session)
        {
            return BSP_RTN_ERR_MEMORY;
        }
    }

    // Channel pool
    if (!mp_channel)
    {
        mp_channel = bsp_new_mempool(sizeof(BSPD_CHANNEL), NULL, NULL);
        if (!mp_channel)
        {
            return BSP_RTN_ERR_MEMORY;
        }
    }

    // Global channel (All session)
    int global_channel_id = new_channel(BSPD_CHANNEL_ALL);
    global_channel = check_channel(global_channel_id);
    if (!global_channel)
    {
        return BSP_RTN_ERR_MEMORY;
    }

    return BSP_RTN_SUCCESS;
}

/* Clients */
int reg_client(BSP_SOCKET_CLIENT *clt)
{
    if (!clt)
    {
        return BSP_RTN_INVALID;
    }

    int fd = clt->sck.fd;
    if (fd >= client_list_size)
    {
        // Enlarge
        size_t new_size = 2 << bsp_log2(fd);
        BSP_SOCKET_CLIENT **new_list = bsp_realloc(client_list, sizeof(BSP_SOCKET_CLIENT *) * new_size);
        if (!new_list)
        {
            return BSP_RTN_ERR_MEMORY;
        }

        client_list = new_list;
        client_list_size = new_size;
    }

    BSPD_SESSION *session = new_session();
    bind_session(clt, session);
    client_list[fd] = clt;

    return BSP_RTN_SUCCESS;
}

int unreg_client(BSP_SOCKET_CLIENT *clt)
{
    if (!clt)
    {
        return BSP_RTN_INVALID;
    }

    int fd = clt->sck.fd;
    if (fd < client_list_size)
    {
        if (clt == client_list[fd])
        {
            client_list[fd] = NULL;
        }
    }

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
    if (fd < client_list_size)
    {
        return client_list[fd];
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

int set_session(BSPD_SESSION *session, const char *session_id)
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
    remove_session_from_channel(global_channel, session);
    // TODO : Remove from all channels
    bsp_mempool_free(mp_session, session);

    return BSP_RTN_SUCCESS;
}

BSPD_SESSION * check_session(const char *session_id)
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
    int ret = add_session_to_channel(global_channel, session);
    if (BSP_RTN_SUCCESS == ret)
    {
        session->logged = BSP_TRUE;
    }

    return ret;
}

int logoff_session(BSPD_SESSION *session)
{
    int ret = remove_session_from_channel(global_channel, session);
    if (BSP_RTN_SUCCESS == ret)
    {
        session->logged = BSP_FALSE;
    }

    return ret;
}

/* Channels */
int new_channel(BSPD_CHANNEL_TYPE type)
{
    BSPD_CHANNEL *channel = bsp_mempool_alloc(mp_channel);
    if (channel)
    {
        bzero(channel, sizeof(BSPD_CHANNEL));
        channel->type = type;

        // Add into list+
        channel_list[next_channel_id] = channel;
        channel->id = next_channel_id;

        // Find next
        next_channel_id = 0;
        size_t i;
        for (i = next_channel_id + 1; i < channel_list_size; i ++)
        {
            if (!channel_list[i])
            {
                next_channel_id = i;
                break;
            }
        }

        if (!next_channel_id)
        {
            // List full
            BSPD_CHANNEL **new_list = bsp_realloc(channel_list, 2 * client_list_size * sizeof(BSPD_CHANNEL *));
            if (!new_list)
            {
                bsp_mempool_free(mp_channel, channel);

                return -1;
            }

            channel_list = new_list;
            next_channel_id = client_list_size;
            client_list_size *= 2;
        }
    }

    return channel->id;
}

int del_channel(int channel_id)
{
    if (channel_id >= channel_list_size)
    {
        return BSP_RTN_INVALID;
    }

    BSPD_CHANNEL *channel = channel_list[channel_id];
    if (!channel)
    {
        return BSP_RTN_INVALID;
    }

    if (BSPD_CHANNEL_ALL == channel->type)
    {
        // Cannot delete global channel
        return BSP_RTN_INVALID;
    }

    bsp_del_object(channel->list);
    bsp_mempool_free(mp_channel, channel);
    channel_list[channel_id] = NULL;
    if (channel_id < next_channel_id)
    {
        next_channel_id = channel_id;
    }

    return BSP_RTN_SUCCESS;
}

BSPD_CHANNEL * check_channel(int channel_id)
{
    if (channel_id >= channel_list_size)
    {
        return NULL;
    }

    BSPD_CHANNEL *channel = channel_list[channel_id];

    return channel;
}

int add_session_to_channel(BSPD_CHANNEL *channel, BSPD_SESSION *session)
{
    if (!channel || !session)
    {
        return BSP_RTN_INVALID;
    }

    if (!channel->list)
    {
        channel->list = bsp_new_object();
        if (channel->list)
        {
            channel->list->type = BSP_OBJECT_HASH;
        }
        else
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

    BSP_STRING *key = bsp_new_string(session->session_id, -1);
    if (!key)
    {
        bsp_del_value(val);

        return BSP_RTN_ERR_MEMORY;
    }

    V_SET_POINTER(val, session);
    bsp_object_set_hash(channel->list, key, val);

    return BSP_RTN_SUCCESS;
}

int remove_session_from_channel(BSPD_CHANNEL *channel, BSPD_SESSION *session)
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
    if (0 == bsp_object_size(channel->list))
    {
        // TODO del_channel
    }

    return BSP_RTN_SUCCESS;
}
