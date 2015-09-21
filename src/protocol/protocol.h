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
/* HTTP */
#define HTTP_VERSION_09                 9
#define HTTP_VERSION_10                 10000
#define HTTP_VERSION_11                 10001
#define HTTP_VERSION_20                 20000
#define HTTP_VERSION_UNKNOWN            0

typedef enum bspd_http_request_method_e
{
    BSPD_HTTP_METHOD_UNKNOWN
                        = 0, 
#define BSPD_HTTP_METHOD_UNKNOWN        BSPD_HTTP_METHOD_UNKNOWN
    BSPD_HTTP_METHOD_OPTIONS
                        = 1, 
#define BSPD_HTTP_METHOD_OPTIONS        BSPD_HTTP_METHOD_OPTIONS
    BSPD_HTTP_METHOD_GET
                        = 2, 
#define BSPD_HTTP_METHOD_GET            BSPD_HTTP_METHOD_GET
    BSPD_HTTP_METHOD_HEAD
                        = 4, 
#define BSPD_HTTP_METHOD_HEAD           BSPD_HTTP_METHOD_HEAD
    BSPD_HTTP_METHOD_POST
                        = 8, 
#define BSPD_HTTP_METHOD_POST           BSPD_HTTP_METHOD_POST
    BSPD_HTTP_METHOD_PUT
                        = 16, 
#define BSPD_HTTP_METHOD_PUT            BSPD_HTTP_METHOD_PUT
    BSPD_HTTP_METHOD_DELETE
                        = 32, 
#define BSPD_HTTP_METHOD_DELETE         BSPD_HTTP_METHOD_DELETE
    BSPD_HTTP_METHOD_TRACE
                        = 64, 
#define BSPD_HTTP_METHOD_TRACE          BSPD_HTTP_METHOD_TRACE
    BSPD_HTTP_METHOD_CONNECT
                        = 128
#define BSPD_HTTP_METHOD_CONNECT        BSPD_HTTP_METHOD_CONNECT
} BSPD_HTTP_REQUEST_METHOD;

typedef enum bspd_http_connection_type_e
{
    BSPD_HTTP_CONNECTION_CLOSE
                        = 0, 
#define BSPD_HTTP_CONNECTION_CLOSE      BSPD_HTTP_CONNECTION_CLOSE
    BSPD_HTTP_CONNECTION_KEEP_ALIVE
                        = 1, 
#define BSPD_HTTP_CONNECTION_KEEP_ALIVE BSPD_HTTP_CONNECTION_KEEP_ALIVE
    BSPD_HTTP_CONNECTION_UPGRADE
                        = 255
#define BSPD_HTTP_CONNECTION_UPGRADE    BSPD_HTTP_CONNECTION_UPGRADE
} BSPD_HTTP_CONNECTION_TYPE;

typedef enum bspd_http_status_code_t
{
    BSPD_HTTP_STATUS_CONTINUE
                        = 100, 
#define BSPD_HTTP_STATUS_CONTINUE       BSPD_HTTP_STATUS_CONTINUE
    BSPD_HTTP_STATUS_SWITCHING
                        = 101, 
#define BSPD_HTTP_STATUS_SWITCHING      BSPD_HTTP_STATUS_SWITCHING
    BSPD_HTTP_STATUS_OK
                        = 200, 
#define BSPD_HTTP_STATUS_OK             BSPD_HTTP_STATUS_OK
    BSPD_HTTP_STATUS_CREATED
                        = 201, 
#define BSPD_HTTP_STATUS_CREATED        BSPD_HTTP_STATUS_CREATED
    BSPD_HTTP_STATUS_ACCEPTED
                        = 202, 
#define BSPD_HTTP_STATUS_ACCEPTED       BSPD_HTTP_STATUS_ACCEPTED
    BSPD_HTTP_STATUS_NON_AUTHORITATIVE
                        = 203, 
#define BSPD_HTTP_STATUS_NON_AUTHORITATIVE
                                        BSPD_HTTP_STATUS_NON_AUTHORITATIVE
    BSPD_HTTP_STATUS_NO_CONTENT
                        = 204, 
#define BSPD_HTTP_STATUS_NO_CONTENT     BSPD_HTTP_STATUS_NO_CONTENT
    BSPD_HTTP_STATUS_RESET_CONTENT
                        = 205, 
#define BSPD_HTTP_STATUS_RESET_CONTENT  BSPD_HTTP_STATUS_RESET_CONTENT
    BSPD_HTTP_STATUS_PARTIAL_CONTENT
                        = 206, 
#define BSPD_HTTP_STATUS_PARTIAL_CONTENT
                                        BSPD_HTTP_STATUS_PARTIAL_CONTENT
    BSPD_HTTP_STATUS_MULTIPLE_CHOICES
                        = 300, 
#define BSPD_HTTP_STATUS_MULTIPLE_CHOICES
                                        BSPD_HTTP_STATUS_MULTIPLE_CHOICES
    BSPD_HTTP_STATUS_MOVED_PERMANENTLY
                        = 301, 
#define BSPD_HTTP_STATUS_MOVED_PERMANENTLY
                                        BSPD_HTTP_STATUS_MOVED_PERMANENTLY
    BSPD_HTTP_STATUS_FOUND
                        = 302, 
#define BSPD_HTTP_STATUS_FOUND          BSPD_HTTP_STATUS_FOUND
    BSPD_HTTP_STATUS_SEE_OTHER
                        = 303, 
#define BSPD_HTTP_STATUS_SEE_OTHER      BSPD_HTTP_STATUS_SEE_OTHER
    BSPD_HTTP_STATUS_NOT_MODIFIED
                        = 304, 
#define BSPD_HTTP_STATUS_NOT_MODIFIED   BSPD_HTTP_STATUS_NOT_MODIFIED
    BSPD_HTTP_STATUS_USE_PROXY
                        = 305, 
#define BSPD_HTTP_STATUS_USE_PROXY      BSPD_HTTP_STATUS_USE_PROXY
    BSPD_HTTP_STATUS_UNUSED
                        = 306, 
#define BSPD_HTTP_STATUS_UNUSED         BSPD_HTTP_STATUS_UNUSED
    BSPD_HTTP_STATUS_TEMPORARY_REDIRECT
                        = 307, 
#define BSPD_HTTP_STATUS_TEMPORARY_REDIRECT
                                        BSPD_HTTP_STATUS_TEMPORARY_REDIRECT
    BSPD_HTTP_STATUS_BAD_REQUEST
                        = 400, 
#define BSPD_HTTP_STATUS_BAD_REQUEST    BSPD_HTTP_STATUS_BAD_REQUEST
    BSPD_HTTP_STATUS_UNAUTHORIZED
                        = 401, 
#define BSPD_HTTP_STATUS_UNAUTHORIZED   BSPD_HTTP_STATUS_UNAUTHORIZED
    BSPD_HTTP_STATUS_PAYMENT_REQUEST
                        = 402, 
#define BSPD_HTTP_STATUS_PAYMENT_REQUEST
                                        BSPD_HTTP_STATUS_PAYMENT_REQUEST
    BSPD_HTTP_STATUS_FORBIDDEN
                        = 403, 
#define BSPD_HTTP_STATUS_FORBIDDEN      BSPD_HTTP_STATUS_FORBIDDEN
    BSPD_HTTP_STATUS_NOT_FOUND
                        = 404, 
#define BSPD_HTTP_STATUS_NOT_FOUND      BSPD_HTTP_STATUS_NOT_FOUND
    BSPD_HTTP_STATUS_METHOD_NOT_ALLOWED
                        = 405, 
#define BSPD_HTTP_STATUS_METHOD_NOT_ALLOWED
                                        BSPD_HTTP_STATUS_METHOD_NOT_ALLOWED
    BSPD_HTTP_STATUS_NOT_ACCEPTABLE
                        = 406, 
#define BSPD_HTTP_STATUS_NOT_ACCEPTABLE BSPD_HTTP_STATUS_NOT_ACCEPTABLE
    BSPD_HTTP_STATUS_PROXY_AUTHONTICATION
                        = 407, 
#define BSPD_HTTP_STATUS_PROXY_AUTHONTICATION
                                        BSPD_HTTP_STATUS_PROXY_AUTHONTICATION
    BSPD_HTTP_STATUS_REQUEST_TIMEOUT
                        = 408, 
#define BSPD_HTTP_STATUS_REQUEST_TIMEOUT
                                        BSPD_HTTP_STATUS_REQUEST_TIMEOUT
    BSPD_HTTP_STATUS_CONFLICT
                        = 409, 
#define BSPD_HTTP_STATUS_CONFLICT       BSPD_HTTP_STATUS_CONFLICT
    BSPD_HTTP_STATUS_GONE
                        = 410, 
#define BSPD_HTTP_STATUS_GONE           BSPD_HTTP_STATUS_GONE
    BSPD_HTTP_STATUS_LENGTH_REQUIRED
                        = 411, 
#define BSPD_HTTP_STATUS_LENGTH_REQUIRED
                                        BSPD_HTTP_STATUS_LENGTH_REQUIRED
    BSPD_HTTP_STATUS_PRECONDITON_FAILED
                        = 412, 
#define BSPD_HTTP_STATUS_PRECONDITON_FAILED
                                        BSPD_HTTP_STATUS_PRECONDITON_FAILED
    BSPD_HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE
                        = 413, 
#define BSPD_HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE
                                        BSPD_HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE
    BSPD_HTTP_STATUS_REQUEST_URI_TOO_LONG
                        = 414, 
#define BSPD_HTTP_STATUS_REQUEST_URI_TOO_LONG
                                        BSPD_HTTP_STATUS_REQUEST_URI_TOO_LONG
    BSPD_HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE
                        = 415, 
#define BSPD_HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE
                                        BSPD_HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE
    BSPD_HTTP_STATUS_EXPECTATION_FAILED
                        = 415, 
#define BSPD_HTTP_STATUS_EXPECTATION_FAILED
                                        BSPD_HTTP_STATUS_EXPECTATION_FAILED
    BSPD_HTTP_STATUS_INTERNAL_SERVER_ERROR
                        = 500, 
#define BSPD_HTTP_STATUS_INTERNAL_SERVER_ERROR
                                        BSPD_HTTP_STATUS_INTERNAL_SERVER_ERROR
    BSPD_HTTP_STATUS_NOT_IMPLEMENTED
                        = 501, 
#define BSPD_HTTP_STATUS_NOT_IMPLEMENTED
                                        BSPD_HTTP_STATUS_NOT_IMPLEMENTED
    BSPD_HTTP_STATUS_BAD_GATEWAY
                        = 502, 
#define BSPD_HTTP_STATUS_BAD_GATEWAY    BSPD_HTTP_STATUS_BAD_GATEWAY
    BSPD_HTTP_STATUS_SERVICE_UNAVAILABLE
                        = 503, 
#define BSPD_HTTP_STATUS_SERVICE_UNAVAILABLE
                                        BSPD_HTTP_STATUS_SERVICE_UNAVAILABLE
    BSPD_HTTP_STATUS_GATEWAY_TIMEOUT
                        = 504, 
#define BSPD_HTTP_STATUS_GATEWAY_TIMEOUT
                                        BSPD_HTTP_STATUS_GATEWAY_TIMEOUT
    BSPD_HTTP_STATUS_VERSION_NOT_SUPPORTED
                        = 505
#define BSPD_HTTP_STATUS_VERSION_NOT_SUPPORTED
                                        BSPD_HTTP_STATUS_VERSION_NOT_SUPPORTED
} BSPD_HTTP_STATUS_CODE;

typedef struct bspd_http_response_t
{
    BSPD_HTTP_STATUS_CODE
                        status;
    int                 version;
    size_t              content_length;
    BSPD_HTTP_CONNECTION_TYPE
                        connection;
} BSPD_HTTP_RESPONSE;

// Internal server
int proto_internal_init();
size_t internal_bare_data(BSPD_BARED *bared, const char *data, size_t len);
size_t internal_pack_data(BSPD_BARED *packed, const char *data, size_t len);

// Normal server
int proto_normal_init();
size_t normal_bare_data(BSPD_BARED *bared, const char *data, size_t len);
size_t normal_pack_data(BSPD_BARED *packed, const char *data, size_t len);
void normal_on_event(BSPD_SERVER_EVENT ev, BSP_OBJECT *protp, void *data);

// Http server
int proto_http_init();
size_t http_bare_data(BSPD_BARED *bared, const char *data, size_t len);
size_t http_pack_data(BSPD_BARED *packed, const char *data, size_t len);

// Websocket server
int proto_websocket_init();
size_t websocket_bare_data(BSPD_BARED *bared, const char *data, size_t len);
size_t websocket_pack_data(BSPD_BARED *packed, const char *data, size_t len);

#endif  /* _PROTOCOL_H */
