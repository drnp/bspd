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

static int _cont()
{
    return 0;
}

// Call lua function
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
        case BSPD_TASK_CTL : 
            break;
        case BSPD_TASK_RAW : 
        case BSPD_TASK_STREAM : 
            str = (BSP_STRING *) task->ptr;
            if (str)
            {
                lua_pushlstring(scrt->state, STR_STR(str), STR_LEN(str));
            }
            else
            {
                lua_pushnil(scrt->state);
            }

            nargs = 1;
            break;
        case BSPD_TASK_OBJECT : 
            obj = (BSP_OBJECT *) task->ptr;
            if (obj)
            {
                // object_to_lua_table(scrt->state, obj);
            }
            else
            {
                lua_pushnil(scrt->state);
            }

            nargs = 1;
            break;
        case BSPD_TASK_COMMAND : 
            lua_pushinteger(scrt->state, task->cmd);
            obj = (BSP_OBJECT *) task->ptr;
            if (obj)
            {
                // object_to_lua_table(scrt->state, obj);
            }
            else
            {
                lua_pushnil(scrt->state);
            }

            nargs = 2;
            break;
        case BSPD_TASK_LOAD : 
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
BSPD_SCRIPT_TASK * script_new_task(BSPD_SCRIPT_TASK_TYPE type)
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
void script_del_task(BSPD_SCRIPT_TASK *task)
{
    BSP_STRING *str = NULL;
    BSP_OBJECT *obj = NULL;
    switch (task->type)
    {
        case BSPD_TASK_RAW : 
        case BSPD_TASK_STREAM : 
            str = (BSP_STRING *) task->ptr;
            bsp_del_string(str);
            break;
        case BSPD_TASK_OBJECT : 
        case BSPD_TASK_COMMAND : 
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
    bsp_spin_unlock(&task_queue_lock);

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

    // Tell worker
    if (task_queue_head)
    {
    }

    bsp_spin_unlock(&task_queue_lock);

    return task;
}
