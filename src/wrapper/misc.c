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

int module_misc(lua_State *s)
{
    if (!s || !lua_checkstack(s, 1))
    {
        return BSP_RTN_INVALID;
    }

    lua_pushcfunction(s, misc_random_int);
    lua_setglobal(s, "bsp_random_int");

    return BSP_RTN_SUCCESS;
}
