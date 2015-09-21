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

/*
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
*/
int proto_http_init()
{
    /*
    if (!mp_http_request)
    {
        mp_http_request = bsp_new_mempool(sizeof(BSPD_HTTP_REQUEST), NULL, _clean_req);
        if (!mp_http_request)
        {
            return BSP_RTN_ERR_MEMORY;
        }
    }
    */

    return BSP_RTN_SUCCESS;
}

static void _parse_http_request_line(BSP_OBJECT *req, const char *input, size_t len)
{
    if (!req || !input)
    {
        return;
    }

    BSP_VALUE *val = NULL;
    BSP_STRING *str = NULL;
    size_t ml = 0;
    // Check Request-line
    // : METHOD<SP>REQUEST_URI[<CP>VERSION]<CRLF>
    // Request-line must be the first line of HTTP request
    // Request method, case sensitive
    val = bsp_new_value();
    if (!val)
    {
        return;
    }

    if (0 == strncmp(input, "OPTIONS ", 8))
    {
        ml = 9;
        str = bsp_new_string("OPTIONS", 7);
    }
    else if (0 == strncmp(input, "GET ", 4))
    {
        ml = 4;
        str = bsp_new_string("GET", 3);
    }
    else if (0 == strncmp(input, "HEAD ", 5))
    {
        ml = 5;
        str = bsp_new_string("HEAD", 4);
    }
    else if (0 == strncmp(input, "POST ", 5))
    {
        ml = 5;
        str = bsp_new_string("POST", 4);
    }
    else if (0 == strncmp(input, "PUT ", 4))
    {
        ml = 4;
        str = bsp_new_string("PUT", 3);
    }
    else if (0 == strncmp(input, "DELETE ", 7))
    {
        ml = 7;
        str = bsp_new_string("DELETE", 6);
    }
    else if (0 == strncmp(input, "TRACE ", 6))
    {
        ml = 6;
        str = bsp_new_string("TRACE", 5);
    }
    else if (0 == strncmp(input, "CONNECT ", 8))
    {
        ml = 8;
        str = bsp_new_string("CONNECT", 7);
    }
    else
    {
        str = bsp_new_string("UNKNOWN", 7);

        return;
    }

    V_SET_STRING(val, str);
    str = bsp_new_string("METHOD", 6);
    bsp_object_set_hash(req, str, val);

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

    val = bsp_new_value();
    if (!val)
    {
        return;
    }

    if (uri_endding)
    {
        str = bsp_new_string(uri_starting, (uri_endding - uri_starting));
        V_SET_STRING(val, str);
        str = bsp_new_string("REQUEST_URI", 11);
        bsp_object_set_hash(req, str, val);
    }

    val = bsp_new_value();
    if (!val)
    {
        return;
    }

    const char *version_starting = NULL;
    for (; ml < len; ml ++)
    {
        if (!version_starting && input[ml] > 32)
        {
            version_starting = input + ml;
            if (0 == strncmp(version_starting, "HTTP/0.9", 8))
            {
                str = bsp_new_string("0.9", 3);
            }
            else if (0 == strncmp(version_starting, "HTTP/1.0", 8))
            {
                str = bsp_new_string("1.0", 3);
            }
            else if (0 == strncmp(version_starting, "HTTP/1.1", 8))
            {
                str = bsp_new_string("1.1", 3);
            }
            else if (0 == strncmp(version_starting, "HTTP/2.0", 8))
            {
                str = bsp_new_string("2.0", 3);
            }
            else
            {
                str = bsp_new_string("UNKNOWN", 7);
            }

            break;
        }
    }

    V_SET_STRING(val, str);
    str = bsp_new_string("VERSION", 7);
    bsp_object_set_hash(req, str, val);

    return;
}

static void _parse_http_request_header(BSP_OBJECT *req, const char *input, size_t len)
{
    if (!req)
    {
        return;
    }

    const char *v_starting = NULL;
    const char *v_endding = NULL;
    BSP_STRING *key = NULL;
    BSP_STRING *str = NULL;
    BSP_VALUE *val = NULL;
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

        str = bsp_new_string(v_starting, (v_endding - v_starting));
        bsp_str_toupper(key);
        val = bsp_new_value();
        V_SET_STRING(val, str);
        bsp_object_set_hash(req, key, val);
    }

    return;
}

static void _parse_http_request(BSPD_BARED *bared, const char *data, size_t len)
{
    BSP_OBJECT *req = NULL;
    char *header_endding = (char *) memmem(data, len, "\r\n\r\n", 4);
    if (header_endding)
    {
        // Valid HTTP header
        req = bsp_new_object(BSP_OBJECT_HASH);
        if (!req)
        {
            return NULL;
        }

        _parse_http_request_line(req, data, len);

        const char *line_beginning = strstr(data, "\r\n") + 2;
        const char *line_endding = header_endding;
        while (BSP_TRUE)
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

        bared->proced = (header_endding - data) + 4;
        bared->proto = req;
    }
}

size_t http_bare_data(BSPD_BARED *bared, const char *data, size_t len)
{
    if (bared)
    {
        if (!bared->proto)
        {
            // New HTTP request
            bared->expectation = 0;
            bared->proced = 0;
            _parse_http_request(bared, data, len);
        }

        // Subsequent data
        if (bared->expectation > 0)
        {
            // Query data
            if (len >= (bared->proced + bared->expectation))
            {
                bared->data = bsp_new_const_string(data + bared->proced, bared->expectation);
                bared->proced += bared->expectation;
                bared->bared = BSP_TRUE;
            }
        }
        else
        {
            bared->bared = BSP_TRUE;
        }
    }

    return bared->proced;
}

/* * * Response * * */
static void _build_http_response_header(BSPD_BARED *packed, BSPD_HTTP_RESPONSE *resp)
{
    if (!packed || !resp)
    {
        return;
    }

    if (packed->data)
    {
        bsp_del_string(packed->data);
    }

    packed->data = bsp_new_string(NULL, 0);
    if (!packed->data)
    {
        return;
    }

    switch (resp->version)
    {
        case HTTP_VERSION_09 : 
            bsp_string_printf(packed->data, "HTTP/0.9 %d STATUS\r\n", (int) resp->status);
            break;
        case HTTP_VERSION_10 : 
            bsp_string_printf(packed->data, "HTTP/1.0 %d STATUS\r\n", (int) resp->status);
            break;
        case HTTP_VERSION_11 : 
            bsp_string_printf(packed->data, "HTTP/1.1 %d STATUS\r\n", (int) resp->status);
            break;
        case HTTP_VERSION_20 : 
            bsp_string_printf(packed->data, "HTTP/2.0 %d STATUS\r\n", (int) resp->status);
            break;
        default :
            break;
    }

    bsp_string_append(packed->data, "Server: BSPD HTTP service\r\n", -1);
    bsp_string_printf(packed->data, "Content-Length: %llu\r\n", (unsigned long long) resp->content_length);
    bsp_string_append(packed->data, "Connection: Close\r\n", -1);

    bsp_string_append(packed->data, "\r\n", -1);

    return;
}

size_t http_pack_data(BSPD_BARED *packed, const char *data, size_t len)
{
    if (packed)
    {
        BSPD_HTTP_RESPONSE resp;
        resp.version = HTTP_VERSION_11;
        resp.connection = BSPD_HTTP_CONNECTION_CLOSE;
        resp.content_length = len;
        resp.status = BSPD_HTTP_STATUS_OK;

        _build_http_response_header(packed, &resp);
        if (packed->data)
        {
            bsp_string_append(packed->data, data, len);
        }
    }

    return len;
}
