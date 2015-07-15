/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * global.c
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
 * Shared global stroage between LUA states
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 05/22/2015
 * @changelog
 *      [05/22/2015] - Creation
 */

#include "../bspd.h"

static BSP_OBJECT *base_object = NULL;
static BSP_SPINLOCK base_lock = BSP_SPINLOCK_INITIALIZER;

static int _global_init()
{
    bsp_spin_lock(&base_lock);
    if (!base_object)
    {
        base_object = bsp_new_object(BSP_OBJECT_HASH);
        if (!base_object)
        {
            bsp_spin_unlock(&base_lock);

            return BSP_RTN_ERR_MEMORY;
        }
    }

    bsp_spin_unlock(&base_lock);

    return BSP_RTN_SUCCESS;
}

static int global_set(lua_State *s)
{
    if (!s || !lua_checkstack(s, 1) || !base_object)
    {
        return 0;
    }

    size_t len, i, start;
    const char *path = lua_tolstring(s, -2, &len);
    if (!path)
    {
        lua_pushboolean(s, BSP_FALSE);

        return 1;
    }

    // Parse path for set
    start = 0;
    BSP_STRING *key = NULL;
    BSP_VALUE *val = NULL;
    bsp_spin_lock(&base_lock);
    BSP_OBJECT *obj = base_object;
    BSP_OBJECT *new_obj = NULL;
    for (i = 0; i < len; i ++)
    {
        if ('.' == path[i])
        {
            key = bsp_new_string(path + start, (i - start));
            val = bsp_object_value_hash(obj, key);
            if (val)
            {
                if (BSP_VALUE_OBJECT != val->type)
                {
                    bsp_object_set_hash(obj, key, NULL);
                    val = NULL;
                }
            }

            if (!val)
            {
                new_obj = bsp_new_object(BSP_OBJECT_HASH);
                if (!new_obj)
                {
                    bsp_spin_unlock(&base_lock);
                    lua_pushboolean(s, BSP_FALSE);
                    bsp_del_string(key);

                    return 1; 
                }

                val = bsp_new_value();
                if (!val)
                {
                    bsp_del_object(new_obj);
                    bsp_spin_unlock(&base_lock);
                    lua_pushboolean(s, BSP_FALSE);
                    bsp_del_string(key);

                    return 1;
                }

                V_SET_OBJECT(val, new_obj);
                bsp_object_set_hash(obj, key, val);
                obj = new_obj;
            }
            else
            {
                obj = V_GET_OBJECT(val);
                bsp_del_string(key);
            }

            start = i + 1;
        }
    }

    if (start < len)
    {
        key = bsp_new_string(path + start, (len - start));
        val = bsp_object_value_hash(obj, key);
        if (val)
        {
            bsp_object_set_hash(obj, key, NULL);
        }

        new_obj = lua_to_object(s, -1);
        if (new_obj)
        {
            if (BSP_OBJECT_SINGLE == new_obj->type)
            {
                val = bsp_object_value_single(new_obj);
                if (!val)
                {
                    bsp_spin_unlock(&base_lock);
                    lua_pushboolean(s, BSP_FALSE);
                    bsp_del_string(key);

                    return 1;
                }

                new_obj->type = BSP_OBJECT_UNDETERMINED;
                bsp_del_object(new_obj);
            }
            else
            {
                val = bsp_new_value();
                if (!val)
                {
                    bsp_spin_unlock(&base_lock);
                    lua_pushboolean(s, BSP_FALSE);
                    bsp_del_string(key);

                    return 1;
                }

                V_SET_OBJECT(val, new_obj);
            }

            bsp_object_set_hash(obj, key, val);
        }
        else
        {
            bsp_spin_unlock(&base_lock);
            lua_pushboolean(s, BSP_FALSE);
            bsp_del_string(key);

            return 1;
        }
    }

    bsp_spin_unlock(&base_lock);
    lua_pushboolean(s, BSP_TRUE);

    return 1;
}

static int global_get(lua_State *s)
{
    if (!s || !lua_checkstack(s, 1) || !base_object)
    {
        return 0;
    }

    size_t len, i, start;
    const char *path = lua_tolstring(s, -1, &len);
    if (!path)
    {
        lua_pushnil(s);

        return 1;
    }

    // Parse path for get
    start = 0;
    BSP_STRING *key = NULL;
    BSP_VALUE *val = NULL;
    bsp_spin_lock(&base_lock);
    BSP_OBJECT *obj = base_object;
    for (i = 0; i <= len; i ++)
    {
        if ('.' == path[i])
        {
            key = bsp_new_const_string(path + start, (i - start));
            val = bsp_object_value_hash(obj, key);
            if (!val || BSP_VALUE_OBJECT != val->type)
            {
                bsp_spin_unlock(&base_lock);
                lua_pushnil(s);
                bsp_del_string(key);

                return 1;
            }

            obj = V_GET_OBJECT(val);
            bsp_del_string(key);
            start = i + 1;
        }

        if (i == len)
        {
            key = bsp_new_const_string(path + start, len - start);
            val = bsp_object_value_hash(obj, key);
            if (!val)
            {
                bsp_spin_unlock(&base_lock);
                lua_pushnil(s);
                bsp_del_string(key);

                return 1;
            }

            obj = bsp_new_object(BSP_OBJECT_SINGLE);
            bsp_object_set_single(obj, val);
            object_to_lua(s, obj);
            obj->type = BSP_OBJECT_UNDETERMINED;
            bsp_del_object(obj);
            bsp_del_string(key);
        }
    }

    bsp_spin_unlock(&base_lock);

    return 1;
}

static int global_flush(lua_State *s)
{
    if (!s || !lua_checkstack(s, 1) || !base_object)
    {
        return 0;
    }

    return 1;
}

int module_global(lua_State *s)
{
    _global_init();

    lua_pushcfunction(s, global_set);
    lua_setglobal(s, "bsp_global_set");

    lua_pushcfunction(s, global_get);
    lua_setglobal(s, "bsp_global_get");

    lua_pushcfunction(s, global_flush);
    lua_setglobal(s, "bsp_global_flush");

    return 0;
}
