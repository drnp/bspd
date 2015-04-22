/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * http.c
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
 * Server protocol definations
 *
 * @package bspd::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/27/2015
 * @changelog
 *      [03/27/2015] - Creation
 */

#ifndef _PROTOCOL_H

#define _PROTOCOL_H
typedef struct bspd_http_request_t
{
    
} BSPD_HTTP_REQUEST;

// Internal server
size_t internal_bare_data(BSPD_BARED *bared, const char *data, size_t len);
size_t internal_pack_data(const char *data, size_t len);

// Normal server
size_t normal_bare_data(BSPD_BARED *bared, const char *data, size_t len);
size_t normal_pack_data(const char *data, size_t len);
void normal_on_event(BSPD_SERVER_EVENT ev, BSP_OBJECT *protp, void *data);

// Http server
size_t http_bare_data(BSPD_BARED *bared, const char *data, size_t len);
size_t http_pack_data(const char *data, size_t len);

// Websocket server
size_t websocket_bare_data(BSPD_BARED *bared, const char *data, size_t len);
size_t websocket_pack_data(const char *data, size_t len);

#endif  /* _PROTOCOL_H */
