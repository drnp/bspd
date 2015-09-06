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
 * Transmission protocol : Http
 *
 * @package bspd::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/30/2015
 * @changelog
 *      [03/30/2015] - Creation
 */

#include "../bspd.h"

static BSP_MEMPOOL *mp_http_request = NULL;

static void _clean_req(void *ptr)
{
    BSPD_HTTP_REQUEST *req = (BSPD_HTTP_REQUEST *) ptr;
    if (req)
    {
        if (req->params)
        {
            bsp_del_object(req->params);
            req->params = NULL;
        }
        if (req->raw_post_data)
        {
            bsp_del_string(req->raw_post_data);
            req->raw_post_data = NULL;
        }

        bsp_free(req);
    }

    return;
}

int proto_http_init()
{
    if (!mp_http_request)
    {
        mp_http_request = bsp_new_mempool(sizeof(BSPD_HTTP_REQUEST), NULL, _clean_req);
        if (!mp_http_request)
        {
            return BSP_RTN_ERR_MEMORY;
        }
    }

    return BSP_RTN_SUCCESS;
}

static void _parse_http_request_line(BSPD_HTTP_REQUEST *req, const char *input, size_t len)
{
    if (!req || !input)
    {
        return;
    }

    size_t ml = 0;
    // Check Request-line
    // : METHOD<SP>REQUEST_URI[<CP>VERSION]<CRLF>
    // Request-line must be the first line of HTTP request
    // Request method, case sensitive
    if (0 == strncmp(input, "OPTIONS ", 8))
    {
        ml = 9;
        req->method = BSPD_HTTP_METHOD_OPTIONS;
    }
    else if (0 == strncmp(input, "GET ", 4))
    {
        ml = 4;
        req->method = BSPD_HTTP_METHOD_GET;
    }
    else if (0 == strncmp(input, "HEAD ", 5))
    {
        ml = 5;
        req->method = BSPD_HTTP_METHOD_HEAD;
    }
    else if (0 == strncmp(input, "POST ", 5))
    {
        ml = 5;
        req->method = BSPD_HTTP_METHOD_POST;
    }
    else if (0 == strncmp(input, "PUT ", 4))
    {
        ml = 4;
        req->method = BSPD_HTTP_METHOD_PUT;
    }
    else if (0 == strncmp(input, "DELETE ", 7))
    {
        ml = 7;
        req->method = BSPD_HTTP_METHOD_DELETE;
    }
    else if (0 == strncmp(input, "TRACE ", 6))
    {
        ml = 6;
        req->method = BSPD_HTTP_METHOD_TRACE;
    }
    else if (0 == strncmp(input, "CONNECT ", 8))
    {
        ml = 8;
        req->method = BSPD_HTTP_METHOD_CONNECT;
    }
    else
    {
        req->method = BSPD_HTTP_METHOD_UNKNOWN;

        return;
    }

    const char *uri_starting = NULL;
    const char *uri_endding = NULL;
    for (; ml < len; ml ++)
    {
        if (!uri_starting && input[ml] > 32)
        {
            uri_starting = input + ml;
        }

        if (uri_starting && input[ml] <= 32)
        {
            uri_endding = input + ml;
            break;
        }
    }

    if (uri_endding)
    {
        req->request_uri = bsp_new_string(uri_starting, (uri_endding - uri_starting));
    }

    const char *version_starting = NULL;
    for (; ml < len; ml ++)
    {
        if (!version_starting && input[ml] > 32)
        {
            version_starting = input + ml;
            if (0 == strncmp(version_starting, "HTTP/0.9", 8))
            {
                req->version = HTTP_VERSION_09;
            }
            else if (0 == strncmp(version_starting, "HTTP/1.0", 8))
            {
                req->version = HTTP_VERSION_10;
            }
            else if (0 == strncmp(version_starting, "HTTP/1.1", 8))
            {
                req->version = HTTP_VERSION_11;
            }
            else if (0 == strncmp(version_starting, "HTTP/2.0", 8))
            {
                req->version = HTTP_VERSION_20;
            }
            else
            {
                req->version = HTTP_VERSION_UNKNOWN;
            }

            break;
        }
    }

    return;
}

static void _parse_http_request_header(BSPD_HTTP_REQUEST *req, const char *input, size_t len)
{
    const char *v_starting = NULL;
    const char *v_endding = NULL;
    BSP_STRING *key = NULL;
    BSP_STRING *value = NULL;
    size_t ml;
    for (ml = 0; ml < len; ml ++)
    {
        if (!v_starting && input[ml] > 32)
        {
            v_starting = input + ml;
        }

        if (v_starting && (input[ml] <= 32 || input[ml] == 58))
        {
            v_endding = input + ml;
            break;
        }
    }

    if (v_endding)
    {
        key = bsp_new_string(v_starting, (v_endding - v_starting));
    }

    v_starting = NULL;
    v_endding = NULL;
    for (ml ++; ml < len; ml ++)
    {
        if (!v_starting && input[ml] > 32)
        {
            v_starting = input + ml;
        }

        if (v_starting && input[ml] < 32)
        {
            v_endding = input + ml;
            break;
        }
    }

    if (key)
    {
        if (!v_endding)
        {
            v_endding = input + ml;
        }

        value = bsp_new_string(v_starting, (v_endding - v_starting));
        bsp_str_toupper(key);
        STR_STDERR(key);
        fprintf(stderr, " => ");
        STR_STDERR(value);
        fprintf(stderr, "\n");
    }

    return;
}

static BSPD_HTTP_REQUEST * _parse_http_request(const char *data, size_t len)
{
    BSPD_HTTP_REQUEST *req = NULL;
    char *header_endding = (char *) memmem(data, len, "\r\n\r\n", 4);
    if (header_endding)
    {
        // Valid HTTP header
        req = bsp_mempool_alloc(mp_http_request);
        if (!req)
        {
            return NULL;
        }

        _parse_http_request_line(req, data, len);

        const char *line_beginning = strstr(data, "\r\n") + 2;
        const char *line_endding = header_endding;
        while (req)
        {
            line_endding = strstr(line_beginning, "\r\n");
            if (!line_endding)
            {
                break;
            }

            // Parse line
            _parse_http_request_header(req, line_beginning, (line_endding - line_beginning));

            line_beginning = line_endding + 2;
            if (line_beginning >= header_endding)
            {
                break;
            }
        }
    }

    return req;
}

size_t http_bare_data(BSPD_BARED *bared, const char *data, size_t len)
{
    if (bared)
    {
        if (!bared->proto)
        {
            // New HTTP request
            BSPD_HTTP_REQUEST *req = _parse_http_request(data, len);
            bared->proto = req;
        }
    }

    return len;
}

size_t http_pack_data(BSPD_BARED *packed, const char *data, size_t len)
{
    return 0;
}
