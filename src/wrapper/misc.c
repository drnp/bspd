/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * misc.c
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
 * Misc apis for LUA
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 05/19/2015
 * @changelog
 *      [04/19/1015] - Creation
 */

#include "../bspd.h"

// Generate a randomize value
static int misc_random_int(lua_State *s)
{
    if (!s || !lua_checkstack(s, 1))
    {
        return 0;
    }

    int min = 0x80000000;
    int max = 0x7FFFFFFF;
    if (lua_isnumber(s, 2))
    {
        max = lua_tointeger(s, 2);
    }

    if (lua_isnumber(s, 1))
    {
        min = lua_tointeger(s, 1);
    }

    uint32_t value, range = (max - min);
    bsp_rand((char *) &value, sizeof(uint32_t));
    int rand = min + (value % range);
    lua_pushinteger(s, rand);

    return 1;
}

// Dump table
static void _output_var(lua_State *s, int layer)
{
    if (!s || !lua_checkstack(s, 3))
    {
        return;
    }

    int i;
    switch (lua_type(s, -1))
    {
        case LUA_TNIL : 
            fprintf(stderr, "(\033[1;34mNIL\033[0m)");
            break;
        case LUA_TNUMBER : 
            fprintf(stderr, "(\033[1;34mNUMBER\033[0m) - %g", lua_tonumber(s, -1));
            break;
        case LUA_TBOOLEAN : 
            fprintf(stderr, "(\033[1;34mBOOLEAN\033[0m) - %s", (lua_toboolean(s, -1)) ? "TRUE" : "FALSE");
            break;
        case LUA_TSTRING : 
            fprintf(stderr, "(\033[1;34mSTRING\033[0m) - %s", lua_tostring(s, -1));
            break;
        case LUA_TUSERDATA : 
        case LUA_TLIGHTUSERDATA : 
            fprintf(stderr, "(\033[1;34mUSERDATA\033[0m) - %p", lua_touserdata(s, -1));
            break;
        case LUA_TFUNCTION : 
            fprintf(stderr, "(\033[1;34mFUNCTION\033[0m)");
            break;
        case LUA_TTABLE : 
            fprintf(stderr, "(\033[1;34mTABLE\033[0m) =>\n");
            lua_pushnil(s);
            while (0 != lua_next(s, -2))
            {
                for (i = 0; i <= layer; i ++)
                {
                    fprintf(stderr, "\t");
                }

                lua_pushvalue(s, -2);
                fprintf(stderr, "\033[1;32m%s\033[0m : ", lua_tostring(s, -1));
                lua_pop(s, 1);
                _output_var(s, layer + 1);
                fprintf(stderr, "\n");
                lua_pop(s, 1);
            }

            lua_pop(s, 1);
            break;
        default : 
            fprintf(stderr, "(\033[1;33mNULL\033[0m)");
            break;
    }

    return;
}

static int misc_var_dump(lua_State *s)
{
    if (!s)
    {
        return 0;
    }

    fprintf(stderr, "\033[1;35m* * * Variable Dump * * *\033[0m\n");
    _output_var(s, 0);
    fprintf(stderr, "\n\n");

    return 0;
}

int module_misc(lua_State *s)
{
    if (!s || !lua_checkstack(s, 1))
    {
        return BSP_RTN_INVALID;
    }

    lua_pushcfunction(s, misc_random_int);
    lua_setglobal(s, "bsp_random_int");

    lua_pushcfunction(s, misc_var_dump);
    lua_setglobal(s, "bsp_var_dump");

    return BSP_RTN_SUCCESS;
}
