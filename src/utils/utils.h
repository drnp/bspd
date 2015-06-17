/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * utils.h
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
 * BSPD utils' header
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/23/2015
 * @changelog
 *      [03/23/2015] - Creation
 */

#ifndef _UTILS_H

#define _UTILS_H
// Compressor
BSP_STRING * zlib_compress(BSP_STRING *input);
BSP_STRING * zlib_decompress(BSP_STRING *input);
BSP_STRING * snappy_compress(BSP_STRING *input);
BSP_STRING * snappy_decompress(BSP_STRING *input);
BSP_STRING * lz4_compress(BSP_STRING *input);
BSP_STRING * lz4_decompress(BSP_STRING *input);

// Debugger
void debug_object(BSP_OBJECT *obj);
void debug_value(BSP_VALUE *val);
void debug_hex(const char *data, size_t len);
void debug_lua_stack(lua_State *s);
void show_trace(BSP_TRACE *bt);
void append_log(BSP_TRACE *bt);
void close_log();
void append_binary_log(BSP_TRACE *bt);
void close_binary_log();

// Misc
char * get_dir();
int set_dir(const char *dir);
int save_pid();
pid_t proc_daemonize();

#endif  /* _UTILS_H */
