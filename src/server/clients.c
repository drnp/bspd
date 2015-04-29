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

BSP_SOCKET_CLIENT **client_list = NULL;
size_t client_list_size = 0;
static BSP_OBJECT *logged = NULL;
static BSP_MEMPOOL *mp_session = NULL;
static BSP_MEMPOOL *mp_channel = NULL;

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

    // Login group
    if (!logged)
    {
        logged = bsp_new_object();
        if (!logged)
        {
            return BSP_RTN_ERR_GENERAL;
        }

        logged->type = BSP_OBJECT_HASH;
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

    return BSP_RTN_SUCCESS;
}

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

BSPD_SESSION * new_session(BSP_SOCKET_CLIENT *clt)
{
    BSPD_SESSION *ret = bsp_mempool_alloc(mp_session);
    if (ret)
    {
        bzero(ret, sizeof(BSPD_SESSION));
        ret->connect_time = time(NULL);
        ret->serialize_type = BSPD_SERIALIZE_NATIVE;
        ret->compress_type = BSPD_COMPRESS_NONE;
        ret->logged = BSP_FALSE;
        if (clt)
        {
            ret->bind = clt;
            clt->additional = (void *) ret;
        }
    }

    return ret;
}

int del_session(BSPD_SESSION *session)
{
    if (!session)
    {
        return BSP_RTN_INVALID;
    }

    // Try log off
    session_logoff(session);

    bsp_mempool_free(mp_session, session);

    return BSP_RTN_SUCCESS;
}

// TODO : Lock here
int session_login(BSPD_SESSION *session)
{
    if (!session || BSP_TRUE == session->logged || !logged)
    {
        return BSP_RTN_INVALID;
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
    bsp_object_set_hash(logged, key, val);
    session->logged = BSP_TRUE;

    return BSP_RTN_SUCCESS;
}

// TODO : Lock here
int session_logoff(BSPD_SESSION *session)
{
    if (!session || BSP_FALSE == session->logged || !logged)
    {
        return BSP_RTN_INVALID;
    }

    if (0 == strnlen(session->session_id, MAX_SESSION_ID_LENGTH))
    {
        // No id
        return BSP_RTN_ERR_GENERAL;
    }

    BSP_STRING *key = bsp_new_const_string(session->session_id, -1);
    bsp_object_set_hash(logged, key, NULL);
    session->logged = BSP_FALSE;

    return BSP_RTN_SUCCESS;
}

BSPD_SESSION * check_session(const char *session_id)
{
    return NULL;
}
