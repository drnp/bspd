/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * script.c
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
 * LUA script container
 *
 * @package bspd::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 04/01/2015
 * @changelog
 *      [04/01/2015] - Creation
 */

#include "../bspd.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

static BSP_MEMPOOL *mp_task = NULL;
static BSPD_SCRIPT_TASK *task_queue_head = NULL;
static BSPD_SCRIPT_TASK *task_queue_tail = NULL;
static BSP_SPINLOCK task_queue_lock;

inline size_t lua_table_size(lua_State *s, int idx);

int script_init()
{
    if (!mp_task)
    {
        mp_task = bsp_new_mempool(sizeof(BSPD_SCRIPT_TASK), NULL, NULL);
        if (!mp_task)
        {
            return BSP_RTN_ERR_MEMORY;
        }
    }

    bsp_spin_init(&task_queue_lock);

    return BSP_RTN_SUCCESS;
}

static void * _default_allocator(void *ud, void *ptr, size_t osize, size_t nsize)
{
    (void) ud;

    if (0 == nsize)
    {
        bsp_free(ptr);

        return NULL;
    }
    else
    {
        return bsp_realloc(ptr, nsize);
    }
}

// New lua script container
BSPD_SCRIPT * new_script_container()
{
    BSPD_SCRIPT *ret = bsp_calloc(1, sizeof(BSPD_SCRIPT));
    if (ret)
    {
        ret->state = lua_newstate(_default_allocator, NULL);
        if (!ret->state)
        {
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Create lua state failed");
            bsp_free(ret);

            return NULL;
        }

        luaL_openlibs(ret->state);
    }

    return ret;
}

// Delete script container
int del_script_container(BSPD_SCRIPT *scrt)
{
    if (!scrt)
    {
        return BSP_RTN_INVALID;
    }

    if (scrt->state)
    {
        lua_close(scrt->state);
    }

    bsp_free(scrt);

    return BSP_RTN_SUCCESS;
}

// Reload script state
int restart_script_container(BSPD_SCRIPT *scrt)
{
    if (!scrt)
    {
        return BSP_RTN_INVALID;
    }

    if (scrt->state)
    {
        lua_close(scrt->state);
    }

    scrt->state = lua_newstate(_default_allocator, NULL);
    if (!scrt->state)
    {
        bsp_trace_message(BSP_TRACE_ERROR, "lua", "Create lua state failed");

        return BSP_RTN_ERR_GENERAL;
    }

    luaL_openlibs(scrt->state);

    return BSP_RTN_SUCCESS;
}

// Load script from file
int load_script_file(BSPD_SCRIPT *scrt, const char *script_filename)
{
    if (!scrt || !scrt->state || !script_filename)
    {
        return BSP_RTN_INVALID;
    }

    int ret = BSP_RTN_INVALID;
    int status = luaL_loadfile(scrt->state, script_filename);
    switch (status)
    {
        case LUA_OK : 
            bsp_trace_message(BSP_TRACE_DEBUG, "lua", "Script file %s loaded", script_filename);
            // Call chunk after load
            if (LUA_OK != lua_pcall(scrt->state, 0, 0, 0))
            {
                bsp_trace_message(BSP_TRACE_ERROR, "lua", "Run script chunk error : %s", lua_tostring(scrt->state, -1));
            }
            ret = BSP_RTN_SUCCESS;
            break;
        case LUA_ERRFILE : 
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Load script file %s failed : Cannot open file", script_filename);
            ret = BSP_RTN_ERR_IO_READ;
            break;
        case LUA_ERRSYNTAX : 
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Load script file %s failed : Syntax error : ", script_filename, lua_tostring(scrt->state, -1));
            ret = BSP_RTN_ERR_GENERAL;
            break;
        case LUA_ERRMEM : 
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Load script file %s failed : Memory allocation error", script_filename);
            ret = BSP_RTN_ERR_MEMORY;
            break;
        case LUA_ERRGCMM : 
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Load script file %s failed : GC error", script_filename);
            ret = BSP_RTN_ERR_GENERAL;
            break;
        default : 
            break;
    }

    return ret;
}

// Load script content
int load_script_content(BSPD_SCRIPT *scrt, BSP_STRING *script)
{
    if (!scrt || !scrt->state || !script)
    {
        return BSP_RTN_INVALID;
    }

    int ret = BSP_RTN_INVALID;
    int status = luaL_loadbuffer(scrt->state, STR_STR(script), STR_LEN(script), NULL);
    switch (status)
    {
        case LUA_OK : 
            bsp_trace_message(BSP_TRACE_DEBUG, "lua", "Script loaded from content.");
            if (LUA_OK != lua_pcall(scrt->state, 0, 0, 0))
            {
                bsp_trace_message(BSP_TRACE_ERROR, "lua", "Run script chunk error : %s", lua_tostring(scrt->state, -1));
            }

            ret = BSP_RTN_SUCCESS;
            break;
        case LUA_ERRSYNTAX : 
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Load script failed : Syntax error.");
            ret = BSP_RTN_ERR_GENERAL;
            break;
        case LUA_ERRMEM : 
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Load script failed : Memory allocation error.");
            ret = BSP_RTN_ERR_MEMORY;
            break;
        case LUA_ERRGCMM : 
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Load script failed : GC error.");
            ret = BSP_RTN_ERR_GENERAL;
            break;
        default : 
            break;
    }

    return ret;
}

// Real length of LUA table
inline size_t lua_table_size(lua_State *s, int idx)
{
    size_t ret = 0;
    idx = lua_absindex(s, idx);
    if (s && lua_istable(s, idx))
    {
        lua_checkstack(s, 2);
        lua_pushnil(s);
        while (0 != lua_next(s, idx))
        {
            ret ++;
            lua_pop(s, 1);
        }
    }

    return ret;
}

// BSP object and lua table
static void _push_value_to_lua(lua_State *s, BSP_VALUE *val);
static void _push_object_to_lua(lua_State *s, BSP_OBJECT *obj);

static void _push_value_to_lua(lua_State *s, BSP_VALUE *val)
{
    if (!s)
    {
        return;
    }

    if (!val)
    {
        lua_pushnil(s);

        return;
    }

    BSP_STRING *str = NULL;
    BSP_OBJECT *sub_obj = NULL;
    switch (val->type)
    {
        case BSP_VALUE_INT8 : 
        case BSP_VALUE_INT16 : 
        case BSP_VALUE_INT32 : 
        case BSP_VALUE_INT64 : 
        case BSP_VALUE_INT29 : 
        case BSP_VALUE_INT : 
        case BSP_VALUE_UINT8 : 
        case BSP_VALUE_UINT16 : 
        case BSP_VALUE_UINT32 : 
        case BSP_VALUE_UINT64 : 
            lua_pushinteger(s, (lua_Integer) V_GET_INT(val));
            break;
        case BSP_VALUE_FLOAT : 
        case BSP_VALUE_DOUBLE : 
            lua_pushnumber(s, (lua_Number) V_GET_FLOAT(val));
            break;
        case BSP_VALUE_BOOLEAN : 
            lua_pushboolean(s, (int) V_GET_BOOLEAN(val));
            break;
        case BSP_VALUE_STRING : 
            str = V_GET_STRING(val);
            if (str && STR_STR(str))
            {
                lua_pushlstring(s, STR_STR(str), STR_LEN(str));
            }
            else
            {
                lua_pushnil(s);
            }
            break;
        case BSP_VALUE_OBJECT : 
            sub_obj = V_GET_OBJECT(val);
            if (sub_obj)
            {
                _push_object_to_lua(s, sub_obj);
            }
            else
            {
                lua_pushnil(s);
            }
            break;
        case BSP_VALUE_POINTER : 
            lua_pushlightuserdata(s, V_GET_POINTER(val));
            break;
        case BSP_VALUE_NULL : 
        case BSP_VALUE_UNKNOWN : 
        default : 
            lua_pushnil(s);
            break;
    }

    return;
}

static void _push_object_to_lua(lua_State *s, BSP_OBJECT *obj)
{
    if (!s)
    {
        return;
    }

    if (!obj)
    {
        lua_pushnil(s);

        return;
    }

    BSP_VALUE *val = NULL;
    BSP_STRING *key = NULL;
    size_t idx, total;
    bsp_spin_lock(&obj->lock);
    bsp_object_reset(obj);
    lua_checkstack(s, 1);
    switch (obj->type)
    {
        case BSP_OBJECT_SINGLE : 
            // Single value
            val = bsp_object_value_single(obj);
            _push_value_to_lua(s, val);
            break;
        case BSP_OBJECT_ARRAY : 
            // Array
            total = bsp_object_size(obj);
            lua_newtable(s);
            for (idx = 0; idx < total; idx ++)
            {
                lua_checkstack(s, 2);
                lua_pushinteger(s, (lua_Integer) idx + 1);
                val = bsp_object_value_array(obj, idx);
                if (val)
                {
                    _push_value_to_lua(s, val);
                }
                else
                {
                    lua_pushnil(s);
                }

                lua_settable(s, -3);
            }

            break;
        case BSP_OBJECT_HASH : 
            // Hash
            lua_newtable(s);
            val = bsp_object_curr(obj, (void **) &key);
            while (val)
            {
                if (key)
                {
                    lua_checkstack(s, 2);
                    lua_pushlstring(s, STR_STR(key), STR_LEN(key));
                    _push_value_to_lua(s, val);
                    lua_settable(s, -3);
                }

                bsp_object_next(obj);
                val = bsp_object_curr(obj, (void **) &key);
            }

            break;
        case BSP_OBJECT_UNDETERMINED : 
        default : 
            break;
    }

    bsp_spin_unlock(&obj->lock);

    return;
}

void object_to_lua(lua_State *s, BSP_OBJECT *obj)
{
    if (!s || !obj)
    {
        return;
    }

    _push_object_to_lua(s, obj);

    return;
}

static BSP_VALUE * _lua_value_to_value(lua_State *s, int idx);
static BSP_OBJECT * _lua_table_to_object(lua_State *s, int idx);

static BSP_VALUE * _lua_value_to_value(lua_State *s, int idx)
{
    if (!s)
    {
        return NULL;
    }

    idx = lua_absindex(s, idx);
    BSP_VALUE *ret = bsp_new_value();
    if (!ret)
    {
        return NULL;
    }

    lua_Number v_number = 0;
    int v_boolean = 0;
    size_t str_len = 0;
    const char *str = NULL;
    void *v_ptr = NULL;
    BSP_STRING *v_str = NULL;
    BSP_OBJECT *v_obj = NULL;
    switch (lua_type(s, idx))
    {
        case LUA_TNIL : 
            V_SET_NULL(ret);
            break;
        case LUA_TNUMBER : 
            v_number = lua_tonumber(s, idx);
            if (v_number == (lua_Number)(int64_t) v_number)
            {
                // Integer
                V_SET_INT(ret, v_number);
            }
            else
            {
                // Double
                V_SET_DOUBLE(ret, v_number);
            }

            break;
        case LUA_TBOOLEAN : 
            v_boolean = lua_toboolean(s, idx);
            if (!v_boolean)
            {
                V_SET_BOOLEAN(ret, BSP_FALSE);
            }
            else
            {
                V_SET_BOOLEAN(ret, BSP_TRUE);
            }

            break;
        case LUA_TSTRING : 
            str = lua_tolstring(s, idx, &str_len);
            v_str = bsp_new_string(str, str_len);
            V_SET_STRING(ret, v_str);
            break;
        case LUA_TUSERDATA : 
        case LUA_TLIGHTUSERDATA : 
            v_ptr = lua_touserdata(s, idx);
            V_SET_POINTER(ret, v_ptr);
            break;
        case LUA_TTABLE : 
            v_obj = _lua_table_to_object(s, idx);
            V_SET_OBJECT(ret, v_obj);
            break;
        default : 
            V_SET_NULL(ret);
            break;
    }

    return ret;
}

static BSP_OBJECT * _lua_table_to_object(lua_State *s, int idx)
{
    if (!s || !lua_istable(s, idx))
    {
        return NULL;
    }

    idx = lua_absindex(s, idx);
    BSP_OBJECT *ret = bsp_new_object(BSP_OBJECT_UNDETERMINED);
    BSP_VALUE *val = NULL;
    if (!ret)
    {
        return NULL;
    }

    // Check type, array or hash
    size_t total = luaL_len(s, idx);
    if (total == lua_table_size(s, idx))
    {
        // Array
        ret->type = BSP_OBJECT_ARRAY;
        size_t i;
        for (i = 1; i <= total; i ++)
        {
            lua_rawgeti(s, idx, i);
            val = _lua_value_to_value(s, -1);
            bsp_object_set_array(ret, i - 1, val);
            lua_pop(s, 1);
        }
    }
    else
    {
        // Hash
        ret->type = BSP_OBJECT_HASH;
        const char *key_str = NULL;
        size_t key_len = 0;
        BSP_STRING *key = NULL;
        lua_checkstack(s, 2);
        lua_pushnil(s);
        while (0 != lua_next(s, idx))
        {
            // Key
            key_str = lua_tolstring(s, -2, &key_len);
            key = bsp_new_string(key_str, key_len);

            // Value
            val = _lua_value_to_value(s, -1);

            bsp_object_set_hash(ret, key, val);
            lua_pop(s, 1);
        }
    }

    return ret;
}

BSP_OBJECT * lua_to_object(lua_State *s, int idx)
{
    if (!s)
    {
        return NULL;
    }

    BSP_OBJECT *ret = NULL;
    if (lua_istable(s, idx))
    {
        // Array or hash
        ret = _lua_table_to_object(s, idx);
    }
    else
    {
        ret = bsp_new_object(BSP_OBJECT_SINGLE);
        ret->type = BSP_OBJECT_SINGLE;
        bsp_object_set_single(ret, _lua_value_to_value(s, idx));
    }

    return ret;
}

// Call lua function
static int _cont()
{
    return 0;
}

int call_script(BSPD_SCRIPT *scrt, BSPD_SCRIPT_TASK *task)
{
    if (!scrt || !scrt->state || !task)
    {
        return BSP_RTN_INVALID;
    }

    if (task->func)
    {
        // Call a named function
        lua_getglobal(scrt->state, task->func);
    }
    else if (task->ref)
    {
        // Call a referenced function
        lua_rawgeti(scrt->state, LUA_REGISTRYINDEX, task->ref);
    }
    else
    {
        // Call global
    }

    if (!lua_isfunction(scrt->state, -1))
    {
        // Not a function
        bsp_trace_message(BSP_TRACE_ERROR, "lua", "Cannot call a non-callable chunk.");

        return BSP_RTN_ERR_GENERAL;
    }

    // Insert params
    int nargs = 0;
    BSP_STRING *str = NULL;
    BSP_OBJECT *obj = NULL;

    switch (task->type)
    {
        case BSPD_SCRIPT_TASK_CTL : 
            lua_pushinteger(scrt->state, (lua_Integer) task->clt);
            //lua_pushstring(scrt->state, (const char *) task->ptr);
            nargs = 1;
            break;
        case BSPD_SCRIPT_TASK_RAW : 
        case BSPD_SCRIPT_TASK_STREAM : 
            lua_pushinteger(scrt->state, (lua_Integer) task->clt);
            str = (BSP_STRING *) task->ptr;
            if (str)
            {
                lua_pushlstring(scrt->state, STR_STR(str), STR_LEN(str));
            }
            else
            {
                lua_pushnil(scrt->state);
            }

            nargs = 2;
            break;
        case BSPD_SCRIPT_TASK_OBJECT : 
            lua_pushinteger(scrt->state, (lua_Integer) task->clt);
            obj = (BSP_OBJECT *) task->ptr;
            if (obj)
            {
                object_to_lua(scrt->state, obj);
            }
            else
            {
                lua_pushnil(scrt->state);
            }

            nargs = 2;
            break;
        case BSPD_SCRIPT_TASK_COMMAND : 
            lua_pushinteger(scrt->state, (lua_Integer) task->clt);
            lua_pushinteger(scrt->state, task->cmd);
            obj = (BSP_OBJECT *) task->ptr;
            if (obj)
            {
                object_to_lua(scrt->state, obj);
            }
            else
            {
                lua_pushnil(scrt->state);
            }

            nargs = 3;
            break;
        case BSPD_SCRIPT_TASK_LOAD : 
        default : 
            break;
    }

    // We do not need any result returned from LUA
    int status = lua_status(scrt->state);
    int ret = 0;
    if (LUA_OK == status)
    {
        ret = lua_pcallk(scrt->state, nargs, 0, 0, 0, _cont);
    }
    else if (LUA_YIELD == status)
    {
        ret = lua_resume(scrt->state, NULL, nargs);
    }
    else
    {
        bsp_trace_message(BSP_TRACE_ERROR, "lua", "Stack status error, cannot execute.");
        ret = LUA_OK;
    }

    switch (ret)
    {
        case LUA_OK : 
            // Successed
            bsp_trace_message(BSP_TRACE_DEBUG, "lua", "Call chunk successfully");
            break;
        case LUA_YIELD : 
            // Yield
            break;
        case LUA_ERRRUN : 
            // RUntime error
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Call chunk runtime error : %s", lua_tostring(scrt->state, -1));
            break;
        case LUA_ERRMEM : 
            // Allocatation error
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Call chunk memory allcatation error.");
            break;
        case LUA_ERRERR : 
            // Handler error
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Call chunk general error : %s", lua_tostring(scrt->state, -1));
            break;
        case LUA_ERRGCMM : 
            // GC error
            bsp_trace_message(BSP_TRACE_ERROR, "lua", "Call chunk error : GC.");
            break;
    }

    // Coutinue
    _cont();

    // Clear stack
    lua_settop(scrt->state, 0);

    return BSP_RTN_SUCCESS;
}

// Create a new task
BSPD_SCRIPT_TASK * new_script_task(BSPD_SCRIPT_TASK_TYPE type)
{
    BSPD_SCRIPT_TASK *task = bsp_mempool_alloc(mp_task);
    if (task)
    {
        task->type = type;
        task->next = NULL;
    }

    return task;
}

// Free task
void del_script_task(BSPD_SCRIPT_TASK *task)
{
    if (!task)
    {
        return;
    }

    BSP_STRING *str = NULL;
    BSP_OBJECT *obj = NULL;
    switch (task->type)
    {
        case BSPD_SCRIPT_TASK_RAW : 
        case BSPD_SCRIPT_TASK_STREAM : 
            str = (BSP_STRING *) task->ptr;
            bsp_del_string(str);
            break;
        case BSPD_SCRIPT_TASK_OBJECT : 
        case BSPD_SCRIPT_TASK_COMMAND : 
            obj = (BSP_OBJECT *) task->ptr;
            bsp_del_object(obj);
            break;
        default : 
            break;
    }

    bsp_mempool_free(mp_task, task);

    return;
}

// Push task to queue
int push_script_task(BSPD_SCRIPT_TASK *task)
{
    if (!task)
    {
        return BSP_RTN_INVALID;
    }

    BSP_THREAD *t = bsp_select_thread(BSP_THREAD_WORKER);
    if (!t)
    {
        return BSP_RTN_ERR_THREAD;
    }

    bsp_spin_lock(&task_queue_lock);
    if (!task_queue_head)
    {
        // Queue empty
        task_queue_head = task;
    }

    if (task_queue_tail)
    {
        task_queue_tail->next = task;
    }
    else
    {
        task_queue_tail = task;
    }

    // Tell worker
    bsp_poke_event_container(t->event_container);
    bsp_spin_unlock(&task_queue_lock);
    bsp_trace_message(BSP_TRACE_DEBUG, "lua", "Push task %p to queue", task);

    return BSP_RTN_SUCCESS;
}

// Pop task from queue
BSPD_SCRIPT_TASK * pop_script_task()
{
    BSPD_SCRIPT_TASK *task = NULL;
    bsp_spin_lock(&task_queue_lock);
    task = task_queue_head;
    if (task)
    {
        task_queue_head = task->next;
        if (!task_queue_head)
        {
            // Queue empty
            task_queue_tail = NULL;
        }
    }

    bsp_spin_unlock(&task_queue_lock);
    bsp_trace_message(BSP_TRACE_DEBUG, "lua", "Pop task %p from queue", task);

    return task;
}
