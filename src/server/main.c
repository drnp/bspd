/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
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
 * Server circuit
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/23/2015
 * @changelog
 *      [03/23/2015] - Creation
 */

#include "../bspd.h"

static BSP_MEMPOOL *mp_bared = NULL;

int bared_init()
{
    if (!mp_bared)
    {
        mp_bared = bsp_new_mempool(sizeof(BSPD_BARED), NULL, NULL);
        if (!mp_bared)
        {
            return BSP_RTN_ERR_MEMORY;
        }
    }

    return BSP_RTN_SUCCESS;
}

/*
// Get location of process
static char * _get_dir()
{
    char self_name[_POSIX_PATH_MAX];
    char *ret = NULL;
    char *curr;
    ssize_t nbytes = readlink("/proc/self/exe", self_name, _POSIX_PATH_MAX - 1);

    if (-1 == nbytes)
    {
        ret = "./";
    }
    else
    {
        self_name[nbytes] = 0x0;
        ret = realpath(self_name, NULL);
        curr = strrchr(ret, '/');

        if (curr)
        {
            curr[0] = 0x0;
            curr = strrchr(ret, '/');
            // Prev layer
            if (curr)
            {
                curr[0] = 0x0;
            }
        }
    }

    return ret;
}
*/

static void _set_dir(const char *dir)
{
    if (0 == chdir(dir))
    {
        bsp_trace_message(BSP_TRACE_NOTICE, "bspd", "Change current working directory to %s", dir);
    }
    else
    {
        bsp_trace_message(BSP_TRACE_ERROR, "bspd", "Change current working directory failed");
    }

    return;
}

static void _worker_on_poke(BSP_THREAD *t)
{
    if (!t || !t->additional)
    {
        return;
    }

    BSPD_SCRIPT_TASK *task = pop_script_task();
    if (task)
    {
        BSPD_SCRIPT *scrt = (BSPD_SCRIPT *) t->additional;
        call_script(scrt, task);
        del_script_task(task);
    }

    return;
}

static ssize_t _proc_raw(struct bspd_bare_data_t *bared, BSP_SOCKET_CLIENT *clt, const char *hook)
{
    BSPD_SCRIPT_TASK *task = new_script_task(BSPD_TASK_RAW);
    if (task)
    {
        BSP_STRING *str = bsp_new_string(STR_STR(bared->data), STR_LEN(bared->data));
        task->clt = clt->sck.fd;
        task->ptr = (void *) str;
        task->func = hook;
        push_script_task(task);
    }

    return STR_LEN(bared->data);
}

static ssize_t _proc_packet(struct bspd_bare_data_t *bared, BSP_SOCKET_CLIENT *clt, const char *hook)
{
    // Parse packet
    size_t len = STR_LEN(bared->data);
    ssize_t ret = len;
    if (len < 1)
    {
        // Not enough data
        return 0;
    }

    BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
/*
    if (!session)
    {
        return len;
    }
*/
    const char *input = STR_STR(bared->data);
    uint8_t hdr = (uint8_t) input[0];

    /* A header looks like : 
        | * * | * * * | * * | * |
        0-1 : Packet type (Control / Stream / Object / Command)
        2-4 : Serialization type (Native / Json ...)
        5-6 : Compress type (None / deflate / lz4 / snappy)
        7 : Encrypt data (Boolean)
    */
    BSPD_SCRIPT_TASK *task = NULL;
    BSP_VALUE value;
    BSPD_PACKET_TYPE p_type = (hdr >> 6) & 3;
    if (BSPD_PACKET_CTL == p_type)
    {
        // Control packet
        BSPD_CTL_TYPE ctl = (hdr >> 3) & 7;
        int ctl_data = hdr & 7;
        switch (ctl)
        {
            case BSPD_CTL_REP : 
                break;
            case BSPD_CTL_HANDSHAKE :
                break;
            case BSPD_CTL_HEARTBEAT : 
                // Send back
                bsp_socket_append(&clt->sck, input, 1);
                bsp_socket_flush(&clt->sck);
                break;
            case BSPD_CTL_SPEC : 
                break;
            case BSPD_CTL_SERIALIZE : 
                if (session)
                {
                    session->serialize_type = (BSPD_SERIALIZE_TYPE) ctl_data;
                }

                break;
            case BSPD_CTL_COMPRESS : 
                if (session)
                {
                    session->compress_type = (BSPD_COMPRESS_TYPE) ctl_data;
                }

                break;
        }

        return 1;
    }
    else
    {
        // Data
        BSPD_SERIALIZE_TYPE s_type = (hdr >> 3) & 7;
        BSPD_COMPRESS_TYPE c_type = (hdr >> 1) & 3;
        BSP_BOOLEAN encrypted = hdr & 1;

        size_t remaining = len - 1;
        if (remaining < 4)
        {
            // Not enough data for <LENGTH>
            return 0;
        }

        value.type = BSP_VALUE_INT32;
        bsp_get_value(input + 1, &value, BSP_BIG_ENDIAN);
        int plen = V_GET_INT((&value));
        // TODO : Drop huge packet
        remaining -= 4;
        if (remaining < plen)
        {
            // Half-baked packet
            return 0;
        }

        BSP_STRING *data = NULL;
        BSP_OBJECT *obj = NULL;

        if (encrypted)
        {
            // Decrypt
        }

        switch (c_type)
        {
            case BSPD_COMPRESS_DEFLATE : 
                break;
            case BSPD_COMPRESS_LZ4 : 
                break;
            case BSPD_COMPRESS_SNAPPY : 
                break;
            case BSPD_COMPRESS_NONE : 
            default : 
                break;
        }

        switch (p_type)
        {
            case BSPD_PACKET_STR : 
                task = new_script_task(BSPD_TASK_STREAM);
                if (task)
                {
                    data = bsp_new_string(input + 5, plen);
                    task->clt = clt->sck.fd;
                    task->ptr = (void *) data;
                    task->func = hook;
                    push_script_task(task);
                }

                ret = 5 + plen;
                break;
            case BSPD_PACKET_OBJ : 
                task = new_script_task(BSPD_TASK_OBJECT);
                if (task)
                {
                    data = bsp_new_const_string(input + 5, plen);
                    switch (s_type)
                    {
                        case BSPD_SERIALIZE_NATIVE : 
                            break;
                        case BSPD_SERIALIZE_JSON : 
                            obj = json_nd_decode(data);
                            break;
                        case BSPD_SERIALIZE_MSGPACK : 
                            break;
                        case BSPD_SERIALIZE_AMF3 : 
                            break;
                        case BSPD_SERIALIZE_PROTOBUF : 
                            break;
                        case BSPD_SERIALIZE_THRIFT : 
                            break;
                        case BSPD_SERIALIZE_AVRO : 
                            break;
                        case BSPD_SERIALIZE_HESSIAN2 : 
                            break;
                        default : 
                            break;
                    }

                    bsp_del_string(data);
                    task->clt = clt->sck.fd;
                    task->ptr = (void *) obj;
                    task->func = hook;
                    push_script_task(task);
                }

                ret = 5 + plen;
                break;
            case BSPD_PACKET_CMD : 
                if (plen < 4)
                {
                    // No command
                    return len;
                }

                value.type = BSP_VALUE_INT32;
                bsp_get_value(input + 5, &value, BSP_BIG_ENDIAN);
                task = new_script_task(BSPD_TASK_COMMAND);
                if (task)
                {
                    data = bsp_new_const_string(input + 9, plen - 4);
                    switch (s_type)
                    {
                        case BSPD_SERIALIZE_NATIVE : 
                            break;
                        case BSPD_SERIALIZE_JSON : 
                            obj = json_nd_decode(data);
                            break;
                        case BSPD_SERIALIZE_MSGPACK : 
                            break;
                        case BSPD_SERIALIZE_AMF3 : 
                            break;
                        case BSPD_SERIALIZE_PROTOBUF : 
                            break;
                        case BSPD_SERIALIZE_THRIFT : 
                            break;
                        case BSPD_SERIALIZE_AVRO : 
                            break;
                        case BSPD_SERIALIZE_HESSIAN2 : 
                            break;
                        default : 
                            break;
                    }

                    bsp_del_string(data);
                    task->clt = clt->sck.fd;
                    task->cmd = V_GET_INT((&value));
                    task->ptr = (void *) obj;
                    task->func = hook;
                    push_script_task(task);
                }

                ret = 5 + plen;
                break;
            default : 
                ret = len;
                break;
        }
    }

    return ret;
}

static int _bspd_on_connect(BSP_SOCKET_CLIENT *clt)
{
    if (!clt)
    {
        return BSP_RTN_INVALID;
    }

    BSP_SOCKET_SERVER *srv = clt->connected_server;
    if (srv)
    {
        BSPD_SERVER_PROP *prop = (BSPD_SERVER_PROP *) srv->additional;
        BSPD_SCRIPT_TASK *task = new_script_task(BSPD_TASK_CTL);
        task->clt = clt->sck.fd;
        task->func = prop->lua_hook_connect;
        push_script_task(task);
    }

    reg_client(clt);

    return BSP_RTN_SUCCESS;
}

static int _bspd_on_disconnect(BSP_SOCKET_CLIENT *clt)
{
    if (!clt)
    {
        return BSP_RTN_INVALID;
    }

    BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
    del_session(session);

    BSP_SOCKET_SERVER *srv = clt->connected_server;
    if (srv)
    {
        BSPD_SERVER_PROP *prop = (BSPD_SERVER_PROP *) srv->additional;
        BSPD_SCRIPT_TASK *task = new_script_task(BSPD_TASK_CTL);
        task->clt = clt->sck.fd;
        task->func = prop->lua_hook_disconnect;
        push_script_task(task);
    }

    unreg_client(clt);

    return BSP_RTN_SUCCESS;
}

static size_t _bspd_on_data(BSP_SOCKET_CLIENT *clt, const char *data, size_t len)
{
    if (!clt || !data || !len)
    {
        return 0;
    }

    BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
    if (session)
    {
        session->last_active = time(NULL);
    }

    size_t data_len = 0;
    BSP_SOCKET_SERVER *srv = clt->connected_server;
    if (!srv)
    {
        // Dissociative connection?
        return len;
    }

    BSPD_SERVER_PROP *prop = (BSPD_SERVER_PROP *) srv->additional;
    if (!prop)
    {
        return len;
    }

    size_t (*barer)(BSPD_BARED *bared, const char *data, size_t len) = NULL;
    switch (prop->type)
    {
        case BSPD_SERVER_INTERNAL : 
            barer = internal_bare_data;
            break;
        case BSPD_SERVER_NORMAL : 
            barer = normal_bare_data;
            break;
        case BSPD_SERVER_HTTP : 
            barer = http_bare_data;
            break;
        case BSPD_SERVER_WEBSOCKET : 
            barer = websocket_bare_data;
            break;
        default : 
            break;
    }

    size_t offset = 0;
    ssize_t proced = 0;
    BSP_BOOLEAN end = BSP_FALSE;
    BSPD_BARED *bared = NULL;
    while (len > offset)
    {
        bared = bsp_mempool_alloc(mp_bared);
        if (bared)
        {
            barer(bared, data + offset, len - offset);
            if (bared->data)
            {
                if (BSPD_DATA_PACKET == prop->data_type)
                {
                    proced = _proc_packet(bared, clt, prop->lua_hook_data);
                }
                else
                {
                    proced = _proc_raw(bared, clt, prop->lua_hook_data);
                }

                if (proced < 0)
                {
                    end = BSP_TRUE;
                }
                else
                {
                    data_len += proced;
                }

                offset += STR_LEN(bared->data);
            }
            else
            {
                end = BSP_TRUE;
            }

            bsp_del_string(bared->data);
            bsp_del_object(bared->proto);
            bsp_mempool_free(mp_bared, bared);
        }
        else
        {
            // Bare error
            data_len = len;
            end = BSP_TRUE;
        }

        if (end)
        {
            break;
        }
    }

    return data_len;
}

// Send data to client
static size_t _real_send(BSP_SOCKET_CLIENT *clt, unsigned char hdr, BSP_STRING *data)
{
    if (!clt || !data)
    {
        return 0;
    }

    BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
    if (!session)
    {
        return 0;
    }

    BSP_SOCKET_SERVER *srv = clt->connected_server;
    if (!srv)
    {
        return 0;
    }

    BSPD_SERVER_PROP *prop = (BSPD_SERVER_PROP *) srv->additional;
    if (!prop)
    {
        return 0;
    }

    size_t (*packer)(BSPD_BARED *packed, const char *data, size_t len) = NULL;
    switch (prop->type)
    {
        case BSPD_SERVER_INTERNAL : 
            break;
        case BSPD_SERVER_NORMAL : 
            break;
        case BSPD_SERVER_HTTP : 
            packer = http_pack_data;
            break;
        case BSPD_SERVER_WEBSOCKET : 
            packer = websocket_pack_data;
            break;
        default : 
            break;
    }

    size_t ret = 0;
    if (BSPD_DATA_PACKET == prop->data_type)
    {
        // Stream packet
/* Compress & crypt
        switch (session->compress_type)
        {
            case BSPD_COMPRESS_DEFLATE : 
                break;
            case BSPD_COMPRESS_LZ4 : 
                break;
            case BSPD_COMPRESS_SNAPPY : 
                break;
            case BSPD_COMPRESS_NONE : 
            default : 
                break;
        }
*/
        int len = (int) STR_LEN(data);
        char len_str[4] = {
                            (char) ((len >> 24) & 255), 
                            (char) ((len >> 16) & 255), 
                            (char) ((len >> 8) & 255), 
                            (char) (len & 255)
        };

        if (!packer)
        {
            ret = bsp_socket_append(&clt->sck, (const char *) &hdr, sizeof(unsigned char));
            ret += bsp_socket_append(&clt->sck, (const char *) len_str, 4);
            ret += bsp_socket_append(&clt->sck, STR_STR(data), STR_LEN(data));
        }
        else
        {
            BSP_STRING *src = bsp_new_string(NULL, 1);
            if (src)
            {
                BSPD_BARED *packed = bsp_mempool_alloc(mp_bared);
                if (packed)
                {
                    bsp_string_append(src, (const char *) &hdr, sizeof(unsigned char));
                    bsp_string_append(src, (const char *) len_str, 4);
                    bsp_string_append(src, STR_STR(data), STR_LEN(data));

                    packer(packed, STR_STR(src), STR_LEN(src));
                    if (packed->data)
                    {
                        ret = bsp_socket_append(&clt->sck, STR_STR(packed->data), STR_LEN(packed->data));
                        bsp_del_string(packed->data);
                    }

                    bsp_mempool_free(mp_bared, packed);
                }

                bsp_del_string(src);
            }
        }
    }
    else
    {
        // Raw
        if (!packer)
        {
            ret = bsp_socket_append(&clt->sck, STR_STR(data), STR_LEN(data));
        }
        else
        {
            BSPD_BARED *packed = bsp_mempool_alloc(mp_bared);
            if (packed)
            {
                packer(packed, STR_STR(data), STR_LEN(data));
                if (packed->data)
                {
                    ret = bsp_socket_append(&clt->sck, STR_STR(packed->data), STR_LEN(packed->data));
                    bsp_del_string(packed->data);
                }

                bsp_mempool_free(mp_bared, packed);
            }
        }
    }

    bsp_socket_flush(&clt->sck);

    return ret;
}

size_t send_string(BSP_SOCKET_CLIENT *clt, BSP_STRING *string)
{
    if (!clt || !string)
    {
        return 0;
    }

    BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
    if (!session)
    {
        return 0;
    }

    BSP_SOCKET_SERVER *srv = clt->connected_server;
    if (!srv)
    {
        return 0;
    }

    BSPD_SERVER_PROP *prop = (BSPD_SERVER_PROP *) srv->additional;
    if (!prop)
    {
        return 0;
    }

    size_t ret = 0;
    if (BSPD_DATA_PACKET == prop->data_type)
    {
        // Stream packet
/* Compress & crypt
        switch (session->compress_type)
        {
            case BSPD_COMPRESS_DEFLATE : 
                break;
            case BSPD_COMPRESS_LZ4 : 
                break;
            case BSPD_COMPRESS_SNAPPY : 
                break;
            case BSPD_COMPRESS_NONE : 
            default : 
                break;
        }
*/
        unsigned char hdr = BSPD_PACKET_STR << 6 | (session->compress_type) << 1;
        ret = _real_send(clt, hdr, string);
    }
    else
    {
        // Raw
        ret = _real_send(clt, 0, string);
    }

    return ret;
}

size_t send_object(BSP_SOCKET_CLIENT *clt, BSP_OBJECT *object)
{
    if (!clt || !object)
    {
        return 0;
    }

    BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
    if (!session)
    {
        return 0;
    }

    BSP_SOCKET_SERVER *srv = clt->connected_server;
    if (!srv)
    {
        return 0;
    }

    BSPD_SERVER_PROP *prop = (BSPD_SERVER_PROP *) srv->additional;
    if (!prop)
    {
        return 0;
    }

    if (BSPD_DATA_PACKET != prop->data_type)
    {
        return 0;
    }

    size_t ret = 0;
    BSP_STRING *data = NULL;
    switch (session->serialize_type)
    {
        case BSPD_SERIALIZE_JSON : 
            data = json_nd_encode(object, data);
            break;
        case BSPD_SERIALIZE_MSGPACK : 
            break;
        case BSPD_SERIALIZE_AMF3 : 
            break;
        case BSPD_SERIALIZE_PROTOBUF : 
            break;
        case BSPD_SERIALIZE_THRIFT : 
            break;
        case BSPD_SERIALIZE_AVRO : 
            break;
        case BSPD_SERIALIZE_HESSIAN2 : 
            break;
        case BSPD_SERIALIZE_NATIVE : 
        default : 
            break;
    }

/*
    switch (session->compress_type)
    {
        case BSPD_COMPRESS_DEFLATE : 
            break;
        case BSPD_COMPRESS_LZ4 : 
            break;
        case BSPD_COMPRESS_SNAPPY : 
            break;
        case BSPD_COMPRESS_NONE : 
        default : 
            break;
    }
*/
    unsigned char hdr = BSPD_PACKET_OBJ << 6 | (session->compress_type) << 1;
    ret = _real_send(clt, hdr, data);
    bsp_del_string(data);

    return ret;
}

size_t send_command(BSP_SOCKET_CLIENT *clt, int command, BSP_OBJECT *params)
{
    if (!clt || !params)
    {
        return 0;
    }

    BSPD_SESSION *session = (BSPD_SESSION *) clt->additional;
    if (!session)
    {
        return 0;
    }

    BSP_SOCKET_SERVER *srv = clt->connected_server;
    if (!srv)
    {
        return 0;
    }

    BSPD_SERVER_PROP *prop = (BSPD_SERVER_PROP *) srv->additional;
    if (!prop)
    {
        return 0;
    }

    if (BSPD_DATA_PACKET != prop->data_type)
    {
        return 0;
    }

    size_t ret = 0;
    char cmd_str[4] = {
                        (char) ((command >> 24) & 255), 
                        (char) ((command >> 16) & 255), 
                        (char) ((command >> 8) & 255), 
                        (char) (command & 255)
    };

    BSP_STRING *data = bsp_new_string(cmd_str, 4);
    switch (session->serialize_type)
    {
        case BSPD_SERIALIZE_JSON : 
            data = json_nd_encode(params, data);
            debug_hex(STR_STR(data), STR_LEN(data));
            break;
        case BSPD_SERIALIZE_MSGPACK : 
            break;
        case BSPD_SERIALIZE_AMF3 : 
            break;
        case BSPD_SERIALIZE_PROTOBUF : 
            break;
        case BSPD_SERIALIZE_THRIFT : 
            break;
        case BSPD_SERIALIZE_AVRO : 
            break;
        case BSPD_SERIALIZE_HESSIAN2 : 
            break;
        case BSPD_SERIALIZE_NATIVE : 
        default : 
            break;
    }

/*
    switch (session->compress_type)
    {
        case BSPD_COMPRESS_DEFLATE : 
            break;
        case BSPD_COMPRESS_LZ4 : 
            break;
        case BSPD_COMPRESS_SNAPPY : 
            break;
        case BSPD_COMPRESS_NONE : 
        default : 
            break;
    }
*/
    unsigned char hdr = BSPD_PACKET_CMD << 6 | (session->compress_type) << 1;
    ret = _real_send(clt, hdr, data);
    bsp_del_string(data);

    return ret;
}

/* Signals */
// USR1 : Reload LUA script (Append chunk)
static void _callback_reload_script()
{
    BSPD_CONFIG *c = get_global_config();
    if (!c)
    {
        fprintf(stderr, "Panic : application error!\n");

        exit(BSP_RTN_FATAL);
    }

    bsp_trace_message(BSP_TRACE_NOTICE, "bspd", "Try to reload logic script");
    int i;
    BSP_THREAD *t;
    BSPD_SCRIPT *scrt;
    for (i = 0; i < c->opt.worker_threads; i ++)
    {
        t = bsp_get_thread(BSP_THREAD_WORKER, i);
        scrt = (BSPD_SCRIPT *) t->additional;
        if (scrt)
        {
            load_script_file(scrt, c->script);
        }
    }

    return;
}

// USR2 : Reload LUA state (Clear all state and rebuild)
static void _callback_reload_state()
{
    BSPD_CONFIG *c = get_global_config();
    if (!c)
    {
        fprintf(stderr, "Panic : application error!\n");

        exit(BSP_RTN_FATAL);
    }

    bsp_trace_message(BSP_TRACE_NOTICE, "bspd", "Try to restart script state");
    int i;
    BSP_THREAD *t;
    BSPD_SCRIPT *scrt;
    for (i = 0; i < c->opt.worker_threads; i ++)
    {
        t = bsp_get_thread(BSP_THREAD_WORKER, i);
        scrt = (BSPD_SCRIPT *) t->additional;
        if (scrt)
        {
            restart_script_container(scrt);
            LOAD_WRAPPERS(scrt->state)
            load_script_file(scrt, c->script);
        }
    }

    return;
}

// TSTP : Reload configuration
static void _callback_reload_conf()
{
    return;
}

// Portal
int bspd_startup()
{
    if (BSP_RTN_SUCCESS != bsp_init() || 
        BSP_RTN_SUCCESS != script_init() || 
        BSP_RTN_SUCCESS != bared_init() || 
        BSP_RTN_SUCCESS != clients_init())
    {
        fprintf(stderr, "Application error\n");

        exit(BSP_RTN_ERR_MEMORY);
    }

    // Set current working directory to $prefix
    _set_dir(BSPD_PREFIX_DIR);
    BSPD_CONFIG *c = get_global_config();
    if (!c)
    {
        fprintf(stderr, "Panic : application error!\n");

        exit(BSP_RTN_FATAL);
    }

    c->opt.mode = BSP_BOOTSTRAP_SERVER;
    c->opt.trace_level = I_ERR;
    c->opt.trace_recipient = show_trace;
    c->opt.worker_hook_notify = _worker_on_poke;

    c->opt.signal_on_usr1 = _callback_reload_script;
    c->opt.signal_on_usr2 = _callback_reload_state;
    c->opt.signal_on_tstp = _callback_reload_conf;

    // Load config
    BSP_OBJECT *conf = get_conf_from_file(c->config_file);
    if (!conf || BSP_OBJECT_HASH != conf->type)
    {
        return BSP_RTN_ERR_GENERAL;
    }

    parse_conf(conf);
    bsp_prepare(&c->opt);

    int i = 0;
    BSP_THREAD *t = NULL;

    // Create servers
    struct bspd_server_t *curr = c->servers;
    while (curr)
    {
        if (BSP_INET_LOCAL == curr->inet_type)
        {
            if (curr->addr)
            {
                curr->srv = bsp_new_unix_server(curr->addr, 0644);
            }
        }
        else
        {
            curr->srv = bsp_new_net_server(curr->addr, curr->port, curr->inet_type, curr->sock_type);
        }

        if (curr->srv)
        {
            // Dispatch to thread pool
            BSP_EVENT ev;
            for (i = 0; i < curr->srv->nscks; i ++)
            {
                switch (curr->srv->scks[i].sock_type)
                {
                    case BSP_SOCK_TCP : 
                        t = bsp_select_thread(BSP_THREAD_ACCEPTOR);
                        ev.data.fd_type = BSP_FD_SOCKET_SERVER_TCP;
                        ev.events = BSP_EVENT_ACCEPT;
                        break;
                    case BSP_SOCK_SCTP_TO_ONE : 
                    case BSP_SOCK_SCTP_TO_MANY : 
                        t = bsp_select_thread(BSP_THREAD_ACCEPTOR);
                        ev.data.fd_type = BSP_FD_SOCKET_SERVER_SCTP;
                        ev.events = BSP_EVENT_ACCEPT;
                        break;
                    case BSP_SOCK_UDP : 
                    default : 
                        t = bsp_select_thread(BSP_THREAD_IO);
                        ev.data.fd_type = BSP_FD_SOCKET_SERVER_UDP;
                        ev.events = BSP_EVENT_READ;
                        break;
                }

                ev.data.fd = curr->srv->scks[i].fd;
                ev.data.associate.ptr = (void *) &curr->srv->scks[i];
                if (t)
                {
                    bsp_add_event(t->event_container, &ev);
                }
            }

            curr->srv->on_connect = _bspd_on_connect;
            curr->srv->on_disconnect = _bspd_on_disconnect;
            curr->srv->on_data = _bspd_on_data;

            switch (curr->prop.type)
            {
                case BSPD_SERVER_INTERNAL : 
                    //curr->prop.event_hook = internal_on_event;
                    break;
                case BSPD_SERVER_NORMAL : 
                    curr->prop.event_hook = normal_on_event;
                    break;
                case BSPD_SERVER_HTTP : 
                    //curr->prop.event_hook = http_on_event;
                    break;
                case BSPD_SERVER_WEBSOCKET : 
                    //curr->prop.event_hook = websocket_on_event;
                    break;
                default : 
                    break;
            }

            curr->srv->additional = (void *) &curr->prop;
        }

        curr = curr->next;
    }

    // Create script container
    BSPD_SCRIPT *scrt = NULL;
    for (i = 0; i < c->opt.worker_threads; i ++)
    {
        t = bsp_get_thread(BSP_THREAD_WORKER, i);
        scrt = new_script_container();
        if (scrt)
        {
            t->additional = (void *) scrt;
            LOAD_WRAPPERS(scrt->state)
            load_script_file(scrt, c->script);
        }
    }

    fprintf(stderr, "\n\033[1;37mBSPD (\033[0m\033[1;32mgPVP\033[0m\033[1;37m) server started\033[0m\n\n");

    return bsp_startup();
}
