/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * debug.c
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
 * Debug toolkits
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/20/2015
 * @changelog
 *      [03/20/2015] - Creation
 */

#include "../bspd.h"

BSP_SPINLOCK log_lock = BSP_SPINLOCK_INITIALIZER;
FILE *log_fp = NULL;

static void _dump_value(BSP_VALUE *val, int layer);
static void _dump_object(BSP_OBJECT *obj, int layer);

static void _dump_value(BSP_VALUE *val, int layer)
{
    if (!val)
    {
        fprintf(stderr, "\033[1;35m### NO VALUE ###\033[0m\n");

        return;
    }

    BSP_STRING *str = NULL;
    BSP_OBJECT *sub_obj = NULL;
    ssize_t ret;
    switch (val->type)
    {
        case BSP_VALUE_NULL : 
            fprintf(stderr, "\033[1;31m(NULL)\033[0m\n");
            break;
        case BSP_VALUE_INT8 : 
        case BSP_VALUE_INT16 : 
        case BSP_VALUE_INT32 : 
        case BSP_VALUE_INT64 : 
        case BSP_VALUE_INT29 : 
        case BSP_VALUE_INT : 
            fprintf(stderr, "\033[1;33m(INTEGER)\033[0m : %lld\n", (long long int) (V_GET_INT(val)));
            break;
        case BSP_VALUE_UINT8 : 
        case BSP_VALUE_UINT16 : 
        case BSP_VALUE_UINT32 : 
        case BSP_VALUE_UINT64 : 
            fprintf(stderr, "\033[1;33m(UNSIGNED INTEGER)\033[0m : %llu\n", (unsigned long long int) (V_GET_INT(val)));
            break;
        case BSP_VALUE_FLOAT : 
        case BSP_VALUE_DOUBLE : 
            fprintf(stderr, "\033[1;33m(FLOAT)\033[0m : %g\n", (double) V_GET_FLOAT(val));
            break;
        case BSP_VALUE_BOOLEAN : 
            fprintf(stderr, "\033[1;33m(BOOLEAN)\033[0m : %s\n", (BSP_TRUE == V_GET_BOOLEAN(val)) ? "true" : "false");
            break;
        case BSP_VALUE_STRING : 
            fprintf(stderr, "\033[1;32m(STRING)\033[0m : ");
            str = V_GET_STRING(val);
            if (str && STR_STR(str))
            {
                ret = write(STDERR_FILENO, STR_STR(str), STR_LEN(str));
                if (ret <= 0)
                {
                    fprintf(stderr, "\033[1;37m### STRING ###\033[0m");
                }
            }
            else
            {
                fprintf(stderr, "\033[1;31m### NULL_STRING ###\033[0m");
            }

            fprintf(stderr, "\n");
            break;
        case BSP_VALUE_OBJECT : 
            fprintf(stderr, "\033[1;36m(OBJECT)\033[0m : ");
            sub_obj = V_GET_OBJECT(val);
            _dump_object(sub_obj, layer + 1);
            break;
        case BSP_VALUE_POINTER : 
            fprintf(stderr, "\033[1;36m(POINTER)\033[0m : %p\n", V_GET_POINTER(val));
            break;
        case BSP_VALUE_UNKNOWN : 
        default : 
            fprintf(stderr, "\033[1;31m(NOTHING)\033[0m\n");
            break;
    }

    return;
}

static void _dump_object(BSP_OBJECT *obj, int layer)
{
    if (!obj)
    {
        return;
    }

    int i;
    size_t idx, total;
    BSP_VALUE *val;
    BSP_STRING *key;
    bsp_object_reset(obj);
    ssize_t ret;
    switch (obj->type)
    {
        case BSP_OBJECT_SINGLE : 
            // Single
            fprintf(stderr, "\033[1;37mObject type : [SINGLE]\033[0m\n");
            val = bsp_object_value_single(obj);
            _dump_value(val, layer);
            break;
        case BSP_OBJECT_ARRAY : 
            // Array
            fprintf(stderr, "\033[1;37mObject type : [ARRAY]\033[0m\n");
            total = bsp_object_size(obj);
            for (idx = 0; idx < total; idx ++)
            {
                for (i = 0; i <= layer; i ++)
                {
                    fprintf(stderr, "    ");
                }

                fprintf(stderr, "\033[1;35m%lld\033[0m\t=> ", (long long int) idx);
                val = bsp_object_value_array(obj, idx);
                _dump_value(val, layer);
            }

            //fprintf(stderr, "\n");
            break;
        case BSP_OBJECT_HASH : 
            // Hash
            fprintf(stderr, "\033[1;37mObject type : [HASH]\033[0m\n");
            val = bsp_object_curr(obj, (void **) &key);
            while (val && key)
            {
                for (i = 0; i <= layer; i ++)
                {
                    fprintf(stderr, "    ");
                }

                fprintf(stderr, "\033[1;33m");
                ret = write(STDERR_FILENO, STR_STR(key), STR_LEN(key));
                if (ret <= 0)
                {
                    fprintf(stderr, "KEY");
                }

                fprintf(stderr, "\033[0m => ");
                _dump_value(val, layer);
                bsp_object_next(obj);
                val = bsp_object_curr(obj, (void **) &key);
            }

            //fprintf(stderr, "\n");
            break;
        case BSP_OBJECT_UNDETERMINED : 
        default : 
            // NullS
            fprintf(stderr, "\033[1;36mObject type : [UNKNOWN]\033[0m\n");
            break;
    }

    return;
}

void debug_object(BSP_OBJECT *obj)
{
    if (!obj)
    {
        fprintf(stderr, "\n\033[1;37m === [NO OBJECT INPUT] ===\033[0m\n\n");

        return;
    }

    fprintf(stderr, "\n\033[1;37m=== [Debug Object] === < START > ===\033[0m\n");
    _dump_object(obj, 0);
    fprintf(stderr, "\033[1;37m=== [Debug Object] === < END > ===\033[0m\n\n");

    return;
}

void debug_value(BSP_VALUE *val)
{
    if (!val)
    {
        fprintf(stderr, "\n\033[1;37m === [NOTHING TO DEBUG] ===\033[0m\n\n");

        return;
    }

    fprintf(stderr, "\n\033[1;37m=== [Debug Value] === < START > ===\033[0m\n");
    _dump_value(val, 0);
    fprintf(stderr, "\033[1;37m=== [Debug Value] === < END > ===\033[0m\n\n");

    return;
}

void debug_hex(const char *data, size_t len)
{
    if (!data)
    {
        fprintf(stderr, "\n\033[1;36m=== [NOTHING TO OUTPUT] ===\033[0m\n");

        return;
    }

    int i;
    time_t now = time((time_t *) NULL);
    struct tm loctime;
    char tgdate[64];
    localtime_r(&now, &loctime);
    strftime(tgdate, 64, "%m/%d/%Y %H:%M:%S", &loctime);
    fprintf(stderr, "\n\033[1;37m=== [Debug Hex %d bytes] === <%s ORIGIN > ===\033[0m\n", (int) len, tgdate);
    for (i = 0; i < len; i ++)
    {
        fprintf(stderr, "\033[1;33m%02X\033[0m ", (unsigned char) data[i]);
        if (i % 32 == 31)
        {
            fprintf(stderr, "\n");
        }
        else if (i % 8 == 7)
        {
            fprintf(stderr, "  ");
        }
    }

    fprintf(stderr, "\n\033[1;37m=== [Debug Hex %d bytes] === <%s DATA   > ===\033[0m\n", (int) len, tgdate);
    for (i = 0; i < len; i ++)
    {
        if (data[i] >= 32 && data[i] <= 127)
        {
            fprintf(stderr, "\033[1;35m %c \033[0m", data[i]);
        }
        else
        {
            fprintf(stderr, "\033[0;34m . \033[0m");
        }

        if (i % 32 == 31)
        {
            fprintf(stderr, "\n");
        }
        else if (i % 8 == 7)
        {
            fprintf(stderr, "  ");
        }
    }

    fprintf(stderr, "\n\033[1;37m=== [Debug Hex %d bytes] === <%s END    > ===\033[0m\n\n", (int) len, tgdate);

    return;
}

void debug_lua_stack(lua_State *s)
{
    if (!s)
    {
        return;
    }

    int i;
    fprintf(stderr, "\n\033[1;37m=== [Debug LUA stack %p] ===\033[0m\n", (void *) s);
    for (i = 1; i <= lua_gettop(s); i ++)
    {
        fprintf(stderr, " \033[1;35m%d\033[0m => \033[1;34m%s\033[0m\n", i, lua_typename(s, lua_type(s, i)));
    }

    fprintf(stderr, "\n\033[1;37m=== [Debug LUA stack END] ===\033[0m\n");

    return;
}

void show_trace(BSP_TRACE *bt)
{
    struct tm loctime;
    char tgdata[64];
    char *lstr[] = {"\033[1;30m EMERG\033[0m", 
                    "\033[1;31m ALERT\033[0m", 
                    "\033[0;33m  CRIT\033[0m", 
                    "\033[0;31m   ERR\033[0m", 
                    "\033[0;34m  WARN\033[0m", 
                    "\033[1;33mNOTICE\033[0m", 
                    "\033[1;32m  INFO\033[0m", 
                    "\033[1;34m DEBUG\033[0m"};
    int idx = 0;
    int level = bt->level;
    while (level > 0)
    {
        if (level & 1)
        {
            break;
        }

        level = level >> 1;
        idx ++;
    }

    localtime_r(&bt->localtime, &loctime);
    strftime(tgdata, 64, "%m/%d/%Y %H:%M:%S", &loctime);
    fprintf(stderr, "\033[1;37m[\033[0m"
                    "\033[0;36m%s\033[0m"
                    "\033[1;37m]\033[0m"
                    "\033[1;37m[\033[0m"
                    "%s"
                    "\033[1;37m]\033[0m"
                    " - "
                    "\033[1;37m[\033[0m"
                    "\033[1;35m%s\033[0m"
                    "\033[1;37m]\033[0m"
                    " : %s\n", tgdata, lstr[idx], bt->tag, bt->msg);

    return;
}

void append_log(BSP_TRACE *bt)
{
    struct tm loctime;
    char tgdata[64];
    char *lstr[] = {" ERERG", 
                    " ALERT", 
                    "  CRIT", 
                    "   ERR", 
                    "  WARN", 
                    "NOTICE", 
                    "  INFO", 
                    " DEBUG"};
    int idx = 0;
    int level = bt->level;
    while (level > 0)
    {
        if (level & 1)
        {
            break;
        }

        level = level >> 1;
        idx ++;
    }

    localtime_r(&bt->localtime, &loctime);
    strftime(tgdata, 64, "%m/%d%Y %H:%M:%S", &loctime);

    if (!log_fp)
    {
        BSPD_CONFIG *c = get_global_config();
        log_fp = fopen(c->log_file, "w+");
    }

    if (log_fp)
    {
        bsp_spin_lock(&log_lock);
        fprintf(log_fp, "[%s][%s]-[%s] : %s\n", tgdata, lstr[idx], bt->tag, bt->msg);
        bsp_spin_unlock(&log_lock);
    }

    return;
}

void append_binary_log(BSP_TRACE *bt)
{
    return;
}
