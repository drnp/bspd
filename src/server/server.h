/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * server.n
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
 * BSPD server header
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/23/2015
 * @changelog
 *      [03/23/2015] - Creation
 */

#ifndef _SERVER_H

#define _SERVER_H

// 2bits
typedef enum bspd_server_type_e
{
    BSPD_SERVER_INTERNAL
                        = 0, 
#define BSPD_SERVER_INTERNAL            BSPD_SERVER_INTERNAL
    BSPD_SERVER_NORMAL  = 1, 
#define BSPD_SERVER_NORMAL              BSPD_SERVER_NORMAL
    BSPD_SERVER_HTTP    = 2, 
#define BSPD_SERVER_HTTP                BSPD_SERVER_HTTP
    BSPD_SERVER_WEBSOCKET
                        = 3
#define BSPD_SERVER_WEBSOCKET           BSPD_SERVER_WEBSOCKET
} BSPD_SERVER_TYPE;

// 1bit
typedef enum bspd_data_type_e
{
    BSPD_DATA_RAW       = 0, 
#define BSPD_DATA_RAW                   BSPD_DATA_RAW
    BSPD_DATA_PACKET    = 1
#define BSPD_DATA_PACKET                BSPD_DATA_PACKET
} BSPD_DATA_TYPE;

// 2bits
typedef enum bspd_packet_type_e
{
    BSPD_PACKET_CTL     = 0, 
#define BSPD_PACKET_CTL                 BSPD_PACKET_CTL
    BSPD_PACKET_STR     = 1, 
#define BSPD_PACKET_STR                 BSPD_PACKET_STR
    BSPD_PACKET_OBJ     = 2, 
#define BSPD_PACKET_OBJ                 BSPD_PACKET_OBJ
    BSPD_PACKET_CMD     = 3
#define BSPD_PACKET_CMD                 BSPD_PACKET_CMD
} BSPD_PACKET_TYPE;

typedef enum bspd_control_packet_type_e
{
    BSPD_CTL_REP        = 0, 
#define BSPD_CTL_REP                    BSPD_CTL_REP
    BSPD_CTL_HANDSHAKE  = 1, 
#define BSPD_CTL_HANDSHAKE              BSPD_CTL_HANDSHAKE
    BSPD_CTL_HEARTBEAT  = 2, 
#define BSPD_CTL_HEARTBEAT              BSPD_CTL_HEARTBEAT
    BSPD_CTL_SPEC       = 3
#define BSPD_CTL_SPEC                   BSPD_CTL_SPEC
} BSPD_CTL_TYPE;

// 3bits
typedef enum bspd_serialize_type_e
{
    BSPD_SERIALIZE_NATIVE
                        = 0, 
#define BSPD_SERIALIZE_NATIVE           BSPD_SERIALIZE_NATIVE
    BSPD_SERIALIZE_JSON = 1, 
#define BSPD_SERIALIZE_JSON             BSPD_SERIALIZE_JSON
    BSPD_SERIALIZE_MSGPACK
                        = 2, 
#define BSPD_SERIALIZE_MSGPACK          BSPD_SERIALIZE_MSGPACK
    BSPD_SERIALIZE_AMF3 = 3, 
#define BSPD_SERIALIZE_AMF3             BSPD_SERIALIZE_AMF3
    BSPD_SERIALIZE_PROTOBUF
                        = 4, 
#define BSPD_SERIALIZE_PROTOBUF         BSPD_SERIALIZE_PROTOBOF
    BSPD_SERIALIZE_THRIFT
                        = 5, 
#define BSPD_SERIALIZE_THRIFT           BSPD_SERIALIZE_THRIFT
    BSPD_SERIALIZE_AVRO
                        = 6, 
#define BSPD_SERIALIZE_AVRO             BSPD_SERIALIZE_AVRO
    BSPD_SERIALIZE_HESSIAN2
                        = 7
#define BSPD_SERIALIZE_HESSIAN2         BSPD_SERIALIZE_HESSIAN2
} BSPD_SERIALIZE_TYPE;

// 2bits
typedef enum bspd_compress_type_e
{
    BSPD_COMPRESS_NONE  = 0, 
#define BSPD_COMPRESS_NONE              BSPD_COMPRESS_NONE
    BSPD_COMPRESS_DEFLATE
                        = 1, 
#define BSPD_COMPRESS_DEFLATE           BSPD_COMPRESS_DEFLATE
    BSPD_COMPRESS_LZ4   = 2, 
#define BSPD_COMPRESS_LZ4               BSPD_COMPRESS_LZ4
    BSPD_COMPRESS_SNAPPY
                        = 3, 
#define BSPD_COMPRESS_SNAPPY            BSPD_COMPRESS_SNAPPY
} BSPD_COMPRES_TYPE;

typedef enum bspd_server_event_hook_e
{
    BSPD_SERVER_EVENT_ON_CONNECT
                        = 0x1, 
#define BSPD_SERVER_EVENT_ON_CONNECT    BSPD_SERVER_EVENT_ON_CONNECT
    BSPD_SERVER_EVENT_ON_DISCONNECT
                        = 0x2, 
#define BSPD_SERVER_EVENT_ON_DISCONNECT BSPD_SERVER_EVENT_ON_DISCONNECT
    BSPD_SERVER_EVENT_ON_ERROR
                        = 0xF, 
#define BSPD_SERVER_EVENT_ON_ERROR      BSPD_SERVER_EVENT_ON_ERROR
    BSPD_SERVER_EVENT_ON_HEARTBEAT
                        = 0x10, 
#define BSPD_SERVER_EVENT_ON_HEARTBEAT  BSPD_SERVER_EVENT_ON_HEARTBEAT
    BSPD_SERVER_EVENT_ON_RAW
                        = 0x11, 
#define BSPD_SERVER_EVENT_ON_RAW        BSPD_SERVER_EVENT_ON_RAW
    BSPD_SERVER_EVENT_ON_PACKET_CTL
                        = 0x21, 
#define BSPD_SERVER_EVENT_ON_PACKET_CTL BSPD_SERVER_EVENT_ON_PACKET_CTL
    BSPD_SERVER_EVENT_ON_PACKET_STR
                        = 0x22, 
#define BSPD_SERVER_EVENT_ON_PACKET_STR BSPD_SERVER_EVENT_ON_PACKET_STR
    BSPD_SERVER_EVENT_ON_PACKET_OBJ
                        = 0x23, 
#define BSPD_SERVER_EVENT_ON_PACKET_OBJ BSPD_SERVER_EVENT_ON_PACKET_OBJ
    BSPD_SERVER_EVENT_ON_PACKET_CMD
                        = 0x24
#define BSPD_SERVER_EVENT_ON_PACKET_CMD BSPD_SERVER_EVENT_ON_PACKET_CMD
} BSPD_SERVER_EVENT;

// Session struct
typedef struct bspd_session_t
{
    char                session_id[MAX_SESSION_ID_LENGTH];
    uint64_t            session_id_int;
    time_t              connect_time;
    time_t              login_time;
} BSPD_SESSION;

typedef struct bspd_server_prop_t
{
    BSPD_SERVER_TYPE    type;
    BSPD_DATA_TYPE      data_type;
    void                (*event_hook)(BSPD_SERVER_EVENT event, BSP_OBJECT *proto, void *data);
    const char          *lua_hook_connect;
    const char          *lua_hook_disconnect;
    const char          *lua_hook_data;
} BSPD_SERVER_PROP;

struct bspd_server_t
{
    const char          *name;
    const char          *addr;
    uint16_t            port;
    BSP_INET_TYPE       inet_type;
    BSP_SOCK_TYPE       sock_type;
    BSP_SOCKET_SERVER   *srv;
    BSPD_SERVER_PROP    prop;
    struct bspd_server_t
                        *next;
};

// Input data
struct bspd_data_raw
{
    BSP_STRING          *raw;
};

struct bspd_data_stream
{
    BSP_STRING          *stream;
};

struct bspd_data_object
{
    BSP_OBJECT          *obj;
};

struct bspd_data_command
{
    int32_t             cmd;
    BSP_OBJECT          *params;
};

typedef struct bspd_bare_data_t
{
    BSP_OBJECT          *proto;
    BSP_STRING          *data;
    size_t              proced;
} BSPD_BARED;

// Script
#include "lua.h"

typedef enum bspd_script_task_type_e
{
    BSPD_TASK_LOAD      = 0, 
#define BSPD_TASK_LOAD                  BSPD_TASK_LOAD
    BSPD_TASK_RAW       = 1, 
#define BSPD_TASK_RAW                   BSPD_TASK_RAW
    BSPD_TASK_CTL       = 2, 
#define BSPD_TASK_CTL                   BSPD_TASK_CTL
    BSPD_TASK_STREAM    = 3, 
#define BSPD_TASK_STREAM                BSPD_TASK_STREAM
    BSPD_TASK_OBJECT    = 4, 
#define BSPD_TASK_OBJECT                BSPD_TASK_OBJECT
    BSPD_TASK_COMMAND   = 5
#define BSPD_TASK_COMMAND               BSPD_TASK_COMMAND
} BSPD_SCRIPT_TASK_TYPE;

typedef struct bspd_script_task_t
{
    BSPD_SCRIPT_TASK_TYPE
                        type;
    const char          *func;
    int                 ref;
    int                 cmd;
    void                *ptr;
    struct bspd_script_task_t
                        *next;
} BSPD_SCRIPT_TASK;

typedef struct bspd_script_container_t
{
    lua_State *state;
} BSPD_SCRIPT;

typedef struct bspd_script_call_t
{
    const char          *func;
    int                 ref;
    BSPD_SCRIPT_TASK    *task;
} BSPD_SCRIPT_CALL;

int script_init();
int bspd_startup();
BSP_OBJECT * get_conf_from_file(const char *file);
int parse_conf(BSP_OBJECT *conf);
BSPD_SCRIPT * new_script_container();
int del_script_container(BSPD_SCRIPT *scrt);
int load_script_file(BSPD_SCRIPT *scrt, const char *script_filename);
int load_script_content(BSPD_SCRIPT *scrt, BSP_STRING *script);
int call_script(BSPD_SCRIPT *scrt, BSPD_SCRIPT_TASK *task);
BSPD_SCRIPT_TASK * script_new_task(BSPD_SCRIPT_TASK_TYPE type);
void script_del_task(BSPD_SCRIPT_TASK *task);
int push_script_task(BSPD_SCRIPT_TASK *task);
BSPD_SCRIPT_TASK * pop_script_task();

#endif  /* _SERVER_H */
