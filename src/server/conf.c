/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * conf.c
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
 * Configuration paraser
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/23/2015
 * @changelog
 *      [03/23/2015] - Creation
 */

#include "../bspd.h"

BSP_OBJECT * get_conf_from_file(const char *filename)
{
    BSP_STRING *str = bsp_new_string_from_file(filename);
    if (!str)
    {
        fprintf(stderr, "\033[1;37mCannot open configuration file : \033[0m\033[1;34m%s\033[0m\n\n", filename);

        return NULL;
    }

    BSP_OBJECT *conf = json_nd_decode(str);

    return conf;
}

static struct bspd_server_t * _parse_conf_server(BSP_OBJECT *desc)
{
    if (!desc || BSP_OBJECT_HASH != desc->type)
    {
        return NULL;
    }

    struct bspd_server_t *ret = bsp_calloc(1, sizeof(struct bspd_server_t));
    if (!ret)
    {
        return NULL;
    }

    ret->inet_type = BSP_INET_ANY;
    ret->sock_type = BSP_SOCK_ANY;
    ret->addr = NULL;
    ret->port = 0;
    ret->prop.type = BSPD_SERVER_NORMAL;
    ret->prop.data_type = BSPD_DATA_RAW;
    ret->prop.debug_input = BSP_FALSE;
    ret->prop.debug_output = BSP_FALSE;

    BSP_STRING *str = NULL;
    BSP_VALUE *node = NULL;

    node = bsp_object_value_hash_original(desc, "name");
    if (node)
    {
        str = V_GET_STRING(node);
        if (str)
        {
            ret->name = bsp_strndup(STR_STR(str), STR_LEN(str));
        }
    }

    // Inet
    node = bsp_object_value_hash_original(desc, "inet");
    if (node)
    {
        str = V_GET_STRING(node);
        if (str)
        {
            if (0 == strncasecmp(STR_STR(str), "ipv4", 4))
            {
                ret->inet_type = BSP_INET_IPV4;
            }
            else if (0 == strncasecmp(STR_STR(str), "ipv6", 4))
            {
                ret->inet_type = BSP_INET_IPV6;
            }
            else if (0 == strncasecmp(STR_STR(str), "local", 5))
            {
                ret->inet_type = BSP_INET_LOCAL;
            }
        }
    }

    // Sock
    node = bsp_object_value_hash_original(desc, "sock");
    if (node)
    {
        str = V_GET_STRING(node);
        if (str)
        {
            if (0 == strncasecmp(STR_STR(str), "tcp", 3))
            {
                ret->sock_type = BSP_SOCK_TCP;
            }
            else if (0 == strncasecmp(STR_STR(str), "udp", 3))
            {
                ret->sock_type = BSP_SOCK_UDP;
            }
            else if (0 == strncasecmp(STR_STR(str), "sctp", 4))
            {
                ret->sock_type = BSP_SOCK_SCTP;
            }
        }
    }

    // Addr
    node = bsp_object_value_hash_original(desc, "addr");
    if (node)
    {
        str = V_GET_STRING(node);
        if (str)
        {
            ret->addr = bsp_strndup(STR_STR(str), STR_LEN(str));
        }
    }

    // Port
    node = bsp_object_value_hash_original(desc, "port");
    if (node)
    {
        ret->port = (uint16_t) V_GET_INT(node);
    }

    // Type
    node = bsp_object_value_hash_original(desc, "type");
    if (node)
    {
        str = V_GET_STRING(node);
        if (str)
        {
            if (0 == strncasecmp(STR_STR(str), "internal", 8))
            {
                ret->prop.type = BSPD_SERVER_INTERNAL;
            }
            else if (0 == strncasecmp(STR_STR(str), "normal", 6))
            {
                ret->prop.type = BSPD_SERVER_NORMAL;
            }
            else if (0 == strncasecmp(STR_STR(str), "http", 0))
            {
                ret->prop.type = BSPD_SERVER_HTTP;
            }
            else if (0 == strncasecmp(STR_STR(str), "websocket", 9))
            {
                ret->prop.type = BSPD_SERVER_WEBSOCKET;
            }
            else
            {
                // Unsupported
            }
        }
    }

    // Data type
    node = bsp_object_value_hash_original(desc, "data_type");
    if (node)
    {
        str = V_GET_STRING(node);
        if (str)
        {
            if (0 == strncasecmp(STR_STR(str), "packet", 6))
            {
                ret->prop.data_type = BSPD_DATA_PACKET;
            }
        }
    }

    // Debug
    node = bsp_object_value_hash_original(desc, "debug_input");
    if (node)
    {
        if (BSP_FALSE == ret->prop.debug_input)
        {
            ret->prop.debug_input = V_GET_BOOLEAN(node);
        }
    }

    node = bsp_object_value_hash_original(desc, "debug_output");
    if (node)
    {
        if (BSP_FALSE == ret->prop.debug_output)
        {
            ret->prop.debug_output = V_GET_BOOLEAN(node);
        }
    }

    // LUA hooks
    node = bsp_object_value_hash_original(desc, "lua_hook_connect");
    if (node)
    {
        str = V_GET_STRING(node);
        if (str)
        {
            ret->prop.lua_hook_connect = bsp_strndup(STR_STR(str), STR_LEN(str));
        }
    }

    node = bsp_object_value_hash_original(desc, "lua_hook_disconnect");
    if (node)
    {
        str = V_GET_STRING(node);
        if (str)
        {
            ret->prop.lua_hook_disconnect = bsp_strndup(STR_STR(str), STR_LEN(str));
        }
    }

    node = bsp_object_value_hash_original(desc, "lua_hook_data");
    if (node)
    {
        str = V_GET_STRING(node);
        if (str)
        {
            ret->prop.lua_hook_data = bsp_strndup(STR_STR(str), STR_LEN(str));
        }
    }

    return ret;
}

// Parser
int parse_conf(BSP_OBJECT *conf)
{
    if (!conf)
    {
        return BSP_RTN_INVALID;
    }

    BSP_VALUE *node = NULL, *sub;
    BSP_STRING *str = NULL;
    BSPD_CONFIG *c = get_global_config();

    // Global
    node = bsp_object_value_hash_original(conf, "global");
    if (!node)
    {
        // No global segment?
        return BSP_RTN_ERR_GENERAL;
    }

    BSP_OBJECT *global = V_GET_OBJECT(node);
    if (global && BSP_OBJECT_HASH == global->type)
    {
        if (!c->verbose)
        {
            sub = bsp_object_value_hash_original(global, "trace_level");
            if (sub)
            {
                str = V_GET_STRING(sub);
                if (str)
                {
                    if (0 == strncasecmp(STR_STR(str), "none", 4))
                    {
                        c->opt.trace_level = I_NONE;
                    }
                    else if (0 == strncasecmp(STR_STR(str), "emerg", 5))
                    {
                        c->opt.trace_level = I_EMERG;
                    }
                    else if (0 == strncasecmp(STR_STR(str), "alert", 5))
                    {
                        c->opt.trace_level = I_EMERG | I_ALERT;
                    }
                    else if (0 == strncasecmp(STR_STR(str), "crit", 4))
                    {
                        c->opt.trace_level = I_EMERG | I_ALERT | I_CRIT;
                    }
                    else if (0 == strncasecmp(STR_STR(str), "err", 3))
                    {
                        c->opt.trace_level = I_EMERG | I_ALERT | I_CRIT | I_ERR;
                    }
                    else if (0 == strncasecmp(STR_STR(str), "warn", 4))
                    {
                        c->opt.trace_level = I_EMERG | I_ALERT | I_CRIT | I_ERR | I_WARN;
                    }
                    else if (0 == strncasecmp(STR_STR(str), "notice", 6))
                    {
                        c->opt.trace_level = I_EMERG | I_ALERT | I_CRIT | I_ERR | I_WARN | I_NOTICE;
                    }
                    else if (0 == strncasecmp(STR_STR(str), "info", 4))
                    {
                        c->opt.trace_level = I_EMERG | I_ALERT | I_CRIT | I_ERR | I_WARN | I_NOTICE | I_INFO;
                    }
                    else if (0 == strncasecmp(STR_STR(str), "debug", 5))
                    {
                        c->opt.trace_level = I_EMERG | I_ALERT | I_CRIT | I_ERR | I_WARN | I_NOTICE | I_INFO | I_DEBUG;
                    }
                    else if (0 == strncasecmp(STR_STR(str), "all", 3))
                    {
                        c->opt.trace_level = I_ALL;
                    }
                }
            }
        }
        else
        {
            c->opt.trace_level = I_ALL;
        }

        sub = bsp_object_value_hash_original(global, "log_level");
        if (sub)
        {
            str = V_GET_STRING(sub);
            if (str)
            {
                if (0 == strncasecmp(STR_STR(str), "none", 4))
                {
                    c->opt.log_level = I_NONE;
                }
                else if (0 == strncasecmp(STR_STR(str), "emerg", 5))
                {
                    c->opt.log_level = I_EMERG;
                }
                else if (0 == strncasecmp(STR_STR(str), "alert", 5))
                {
                    c->opt.log_level = I_EMERG | I_ALERT;
                }
                else if (0 == strncasecmp(STR_STR(str), "crit", 4))
                {
                    c->opt.log_level = I_EMERG | I_ALERT | I_CRIT;
                }
                else if (0 == strncasecmp(STR_STR(str), "err", 3))
                {
                    c->opt.log_level = I_EMERG | I_ALERT | I_CRIT | I_ERR;
                }
                else if (0 == strncasecmp(STR_STR(str), "warn", 4))
                {
                    c->opt.log_level = I_EMERG | I_ALERT | I_CRIT | I_ERR | I_WARN;
                }
                else if (0 == strncasecmp(STR_STR(str), "notice", 6))
                {
                    c->opt.log_level = I_EMERG | I_ALERT | I_CRIT | I_ERR | I_WARN | I_NOTICE;
                }
                else if (0 == strncasecmp(STR_STR(str), "info", 4))
                {
                    c->opt.log_level = I_EMERG | I_ALERT | I_CRIT | I_ERR | I_WARN | I_NOTICE | I_INFO;
                }
                else if (0 == strncasecmp(STR_STR(str), "debug", 5))
                {
                    c->opt.log_level = I_EMERG | I_ALERT | I_CRIT | I_ERR | I_WARN | I_NOTICE | I_INFO | I_DEBUG;
                }
                else if (0 == strncasecmp(STR_STR(str), "all", 3))
                {
                    c->opt.log_level = I_ALL;
                }
            }
        }

        sub = bsp_object_value_hash_original(global, "daemonize");
        if (sub)
        {
            if (BSP_FALSE == c->daemonize)
            {
                c->daemonize = V_GET_BOOLEAN(sub);
            }
        }

        sub = bsp_object_value_hash_original(global, "boss_threads");
        if (sub)
        {
            c->opt.boss_threads = (int) V_GET_INT(sub);
        }

        sub = bsp_object_value_hash_original(global, "acceptor_threads");
        if (sub)
        {
            c->opt.acceptor_threads = (int) V_GET_INT(sub);
        }

        sub = bsp_object_value_hash_original(global, "io_threads");
        if (sub)
        {
            c->opt.io_threads = (int) V_GET_INT(sub);
        }

        sub = bsp_object_value_hash_original(global, "worker_threads");
        if (sub)
        {
            c->opt.worker_threads = (int) V_GET_INT(sub);
        }

        sub = bsp_object_value_hash_original(global, "script");
        if (sub)
        {
            str = V_GET_STRING(sub);
            if (str)
            {
                c->script = bsp_strndup(STR_STR(str), STR_LEN(str));
            }
        }

        sub = bsp_object_value_hash_original(global, "lua_hook_load");
        if (sub)
        {
            str = V_GET_STRING(sub);
            if (str)
            {
                c->lua_hook_load = bsp_strndup(STR_STR(str), STR_LEN(str));
            }
        }
    }
    else
    {
        // Type error
        return BSP_RTN_ERR_GENERAL;
    }

    // Servers
    node = bsp_object_value_hash_original(conf, "servers");
    if (!node)
    {
        // No server
    }
    else
    {
        BSP_OBJECT *servers = V_GET_OBJECT(node);
        BSP_OBJECT *server_obj = NULL;
        BSP_VALUE *server_val = NULL;
        struct bspd_server_t *server = NULL;
        if (servers && BSP_OBJECT_ARRAY == servers->type)
        {
            bsp_object_reset(servers);
            server_val = bsp_object_curr(servers, NULL);
            while (server_val)
            {
                server_obj = V_GET_OBJECT(server_val);
                if (server_obj && BSP_OBJECT_HASH == server_obj->type)
                {
                    server = _parse_conf_server(server_obj);
                    if (server)
                    {
                        server->next = c->servers;
                        c->servers = server;
                    }
                }

                bsp_object_next(servers);
                server_val = bsp_object_curr(servers, NULL);
            }
        }
    }

    return BSP_RTN_SUCCESS;
}
