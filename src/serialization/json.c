/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * json.c
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
 * 3. Neither the name of Unknown nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Unknown AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Unknown OR ANY OTHER
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * JSON serializator
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/18/2015
 * @changelog
 *      [03/18/2015] - Creation
 */

#include "../bspd.h"

/** Serialize **/
static void _append_string_to_json(BSP_STRING *json, BSP_STRING *key);
static void _append_value_to_json(BSP_STRING *json, BSP_VALUE *val);
static void _append_object_to_json(BSP_STRING *json, BSP_OBJECT *obj);

static void _append_string_to_json(BSP_STRING *json, BSP_STRING *str)
{
    if (json && str)
    {
        size_t i, len = STR_LEN(str);
        ssize_t normal_start = -1, normal_end = -1;
        unsigned char c;
        const char *data = (const char *) STR_STR(str), *esc;
        int32_t utf_value;
        int utf_len;
        for (i = 0; i < len; i ++)
        {
            c = data[i];
            if (c < 0x80)
            {
                esc = bsp_escape_char(c);
                if (esc)
                {
                    // Escaped
                    normal_end = i;
                    bsp_string_append(json, esc, -1);
                }
                else
                {
                    // Normal data
                    if (normal_start < 0)
                    {
                        normal_start = i;
                    }
                }
            }
            else
            {
                // UTF
                normal_end = i;
                utf_value = bsp_utf8_value(data + i, len - i, &utf_len);
                bsp_string_printf(json, "\\u%04x", utf_value);
                i += (utf_len - 1);
            }

            if (i == (len - 1))
            {
                normal_end = i;
            }

            // Append normal data
            if (normal_start >= 0 && normal_end > normal_start) 
            {
                bsp_string_append(json, data + normal_start, (normal_end - normal_start + 1));
                normal_start = -1;
                normal_end = -1;
            }
        }
    }

    return;
}

static void _append_value_to_json(BSP_STRING *json, BSP_VALUE *val)
{
    if (!json)
    {
        return;
    }

    if (!val)
    {
        bsp_string_append(json, "null", 4);
        return;
    }

    switch (val->type)
    {
        case BSP_VALUE_NULL : 
            bsp_string_append(json, "null", 4);
            break;
        case BSP_VALUE_UINT8 : 
        case BSP_VALUE_UINT16 : 
        case BSP_VALUE_UINT32 : 
        case BSP_VALUE_UINT64 : 
            bsp_string_printf(json, "%llu", (long long unsigned int) (V_GET_INT(val)));
            break;
        case BSP_VALUE_INT8 : 
        case BSP_VALUE_INT16 : 
        case BSP_VALUE_INT32 : 
        case BSP_VALUE_INT64 : 
        case BSP_VALUE_INT29 : 
        case BSP_VALUE_INT : 
            bsp_string_printf(json, "%lld", (long long int) (V_GET_INT(val)));
            break;
        case BSP_VALUE_FLOAT : 
        case BSP_VALUE_DOUBLE : 
            bsp_string_printf(json, "%.14g", V_GET_FLOAT(val));
            break;
        case BSP_VALUE_BOOLEAN : 
            if (BSP_TRUE == V_GET_BOOLEAN(val))
            {
                bsp_string_append(json, "true", 4);
            }
            else
            {
                bsp_string_append(json, "false", 5);
            }

            break;
        case BSP_VALUE_STRING : 
            bsp_string_append(json, "\"", 1);
            _append_string_to_json(json, V_GET_STRING(val));
            bsp_string_append(json, "\"", 1);
            break;
        case BSP_VALUE_OBJECT : 
            _append_object_to_json(json, V_GET_OBJECT(val));
            break;
        default : 
            // Nothing to append
            break;
    }

    return;
}

static void _append_object_to_json(BSP_STRING *json, BSP_OBJECT *obj)
{
    if (!json || !obj)
    {
        return;
    }

    BSP_STRING *key = NULL;
    BSP_VALUE *val = NULL;
    bsp_object_reset(obj);
    size_t idx, total;
    switch (obj->type)
    {
        case BSP_OBJECT_SINGLE : 
            val = bsp_object_curr(obj, NULL);
            _append_value_to_json(json, val);
            break;
        case BSP_OBJECT_ARRAY : 
            bsp_string_append(json, "[", 1);
            bsp_spin_lock(&obj->lock);
            total = bsp_object_size(obj);
            for (idx = 0; idx < total; idx ++)
            {
                val = bsp_object_curr(obj, NULL);
                _append_value_to_json(json, val);
                if (idx < total - 1)
                {
                    bsp_string_append(json, ",", 1);
                }

                bsp_object_next(obj);
            }
            bsp_spin_unlock(&obj->lock);
            bsp_string_append(json, "]", 1);
            break;
        case BSP_OBJECT_HASH : 
            bsp_string_append(json, "{", 1);
            debug_hex(STR_STR(json), STR_LEN(json));
            bsp_spin_lock(&obj->lock);
            debug_hex(STR_STR(json), STR_LEN(json));
            val = bsp_object_curr(obj, (void **) &key);
            while (key && val)
            {
                bsp_string_append(json, "\"", 1);
                _append_string_to_json(json, key);
                bsp_string_append(json, "\"", 1);
                bsp_string_append(json, ":", 1);
                _append_value_to_json(json, val);
                bsp_object_next(obj);
                val = bsp_object_curr(obj, (void **) &key);
                if (val)
                {
                    bsp_string_append(json, ",", 1);
                }
            }
            bsp_spin_unlock(&obj->lock);
            bsp_string_append(json, "}", 1);
            break;
        case BSP_OBJECT_UNDETERMINED : 
        default : 
            break;
    }

    return;
}

BSP_STRING * json_nd_encode(BSP_OBJECT *obj, BSP_STRING *json)
{
    if (obj)
    {
        if (!json)
        {
            json = bsp_new_string(NULL, 0);
        }

        _append_object_to_json(json, obj);
    }

    return json;
}

/** Unserialize **/
enum json_decode_status
{
    JSON_DECODE_STATUS_STR
                        = 0b00000001, 
#define JSON_DECODE_STATUS_STR          JSON_DECODE_STATUS_STR
    JSON_DECODE_STATUS_STRIP
                        = 0b00000010, 
#define JSON_DECODE_STATUS_STRIP        JSON_DECODE_STATUS_STRIP
    JSON_DECODE_STATUS_UTF
                        = 0b00000100, 
#define JSON_DECODE_STATUS_UTF          JSON_DECODE_STATUS_UTF
    JSON_DECODE_STATUS_DIGIT
                        = 0b00001000, 
#define JSON_DECODE_STATUS_DIGIT        JSON_DECODE_STATUS_DIGIT
    JSON_DECODE_STATUS_BOOLEAN
                        = 0b00010000
#define JSON_DECODE_STATUS_BOOLEAN      JSON_DECODE_STATUS_BOOLEAN
};

static BSP_VALUE * _get_value_from_json(BSP_STRING *json);
static void _traverse_json_array(BSP_STRING *json, BSP_OBJECT *obj);
static void _traverse_json_hash(BSP_STRING *json, BSP_OBJECT *obj);

static BSP_VALUE * _get_value_from_json(BSP_STRING *json)
{
    if (!json)
    {
        return NULL;
    }

    unsigned char c;
    char status = 0;
    size_t normal = 0;
    BSP_VALUE *ret = NULL;
    BSP_STRING *v_str = NULL;
    BSP_OBJECT *v_obj = NULL;
    BSP_BOOLEAN end = BSP_FALSE;
    char utf[5], utf_data[4], *digit_end = NULL;
    long int utf_value;
    double intpart, v_digit = 0.0f;
    while (STR_REMAIN(json) > 0)
    {
        c = STR_CHAR(json);
        if (status & JSON_DECODE_STATUS_STR)
        {
            // In string
            if (('\\' == c) && 0 == (status & JSON_DECODE_STATUS_STRIP))
            {
                // Strip
                status |= JSON_DECODE_STATUS_STRIP;
                if (STR_NOW(json) > normal && v_str)
                {
                    bsp_string_append(v_str, STR_STR(json) + normal, (STR_NOW(json) - normal));
                }

                STR_NEXT(json);
            }
            else if (status & JSON_DECODE_STATUS_STRIP)
            {
                switch (c)
                {
                    case '\\' : 
                        bsp_string_append(v_str, "\\", 1);
                        break;
                    case '/' : 
                        bsp_string_append(v_str, "/", 1);
                        break;
                    case '"' : 
                        bsp_string_append(v_str, "\"", 1);
                        break;
                    case 'b' : 
                        bsp_string_append(v_str, "\b", 1);
                        break;
                    case 't' : 
                        bsp_string_append(v_str, "\t", 1);
                        break;
                    case 'n' : 
                        bsp_string_append(v_str, "\n", 1);
                        break;
                    case 'f' : 
                        bsp_string_append(v_str, "\f", 1);
                        break;
                    case 'r' : 
                        bsp_string_append(v_str, "\r", 1);
                        break;
                    case 'u' : 
                        // Unicode
                        if (STR_REMAIN(json) > 4)
                        {
                            memcpy(utf, STR_CURR(json) + 1, 4);
                            utf[4] = 0x0;
                            utf_value = strtol(utf, NULL, 16);
                            if (utf_value < 0x80)
                            {
                                // Single character
                                utf_data[0] = utf_value;
                                bsp_string_append(v_str, utf_data, 1);
                            }
                            else if (utf_value < 0x800)
                            {
                                // Two bytes
                                utf_data[0] = ((utf_value >> 6) & 0x1F) | 0xC0;
                                utf_data[1] = (utf_value & 0x3F) | 0x80;
                                bsp_string_append(v_str, utf_data, 2);
                            }
                            else
                            {
                                // Three bytes
                                utf_data[0] = ((utf_value >> 12) & 0x0F) | 0xE0;
                                utf_data[1] = ((utf_value >> 6) & 0x3F) | 0x80;
                                utf_data[2] = (utf_value & 0x3F) | 0x80;
                                bsp_string_append(v_str, utf_data, 3);
                            }

                            STR_NOW(json) += 4;
                        }
                        else
                        {
                            bsp_string_append(v_str, "u", 1);
                        }

                        break;
                }

                status &= ~JSON_DECODE_STATUS_STRIP;
                STR_NEXT(json);
                normal = STR_NOW(json);
            }
            else
            {
                if ('"' == c && v_str)
                {
                    // String endding
                    status &= ~JSON_DECODE_STATUS_STR;
                    if (STR_NOW(json) > normal)
                    {
                        bsp_string_append(v_str, STR_STR(json) + normal, (STR_NOW(json) - normal));
                    }

                    ret = bsp_new_value();
                    V_SET_STRING(ret, v_str);
                    end = BSP_TRUE;
                }
                else
                {
                    // Normal
                }

                STR_NEXT(json);
            }
        }
        else
        {
            if ('"' == c)
            {
                // String
                status |= JSON_DECODE_STATUS_STR;
                v_str = bsp_new_string(NULL, 0);
                STR_NEXT(json);
                normal = STR_NOW(json);
            }
            else if ('{' == c)
            {
                // A new hash
                v_obj = bsp_new_object();
                v_obj->type = BSP_OBJECT_HASH;
                STR_NEXT(json);
                _traverse_json_hash(json, v_obj);
                ret = bsp_new_value();
                V_SET_OBJECT(ret, v_obj);
                end = BSP_TRUE;
            }
            else if ('}' == c)
            {
                // Hash endding
                ret = bsp_new_value();
                ret->type = BSP_VALUE_OBJECT_HASH_END;
                STR_NEXT(json);
                end = BSP_TRUE;
            }
            else if ('[' == c)
            {
                // A new array
                v_obj = bsp_new_object();
                v_obj->type = BSP_OBJECT_ARRAY;
                STR_NEXT(json);
                _traverse_json_array(json, v_obj);
                ret = bsp_new_value();
                V_SET_OBJECT(ret, v_obj);
                end = BSP_TRUE;
            }
            else if (']' == c)
            {
                // Array endding
                ret = bsp_new_value();
                ret->type = BSP_VALUE_OBJECT_ARRAY_END;
                STR_NEXT(json);
                end = BSP_TRUE;
            }
            else if (STR_REMAIN(json) >= 4 && 0 == strncmp("null", STR_CURR(json), 4))
            {
                // null
                ret = bsp_new_value();
                V_SET_NULL(ret);
                STR_NOW(json) += 4;
                end = BSP_TRUE;
            }
            else if (STR_REMAIN(json) >= 4 && 0 == strncmp("true", STR_CURR(json), 4))
            {
                // Boolean true
                ret = bsp_new_value();
                V_SET_BOOLEAN(ret, BSP_TRUE);
                STR_NOW(json) += 4;
                end = BSP_TRUE;
            }
            else if (STR_REMAIN(json) >= 5 && 0 == strncmp("false", STR_CURR(json), 5))
            {
                // Boolean false
                ret = bsp_new_value();
                V_SET_BOOLEAN(ret, BSP_FALSE);
                STR_NOW(json) += 5;
                end = BSP_TRUE;
            }
            else if (c > 0x20)
            {
                // Digit
                errno = 0;
                v_digit = strtod(STR_CURR(json), &digit_end);
                if (digit_end && digit_end > STR_CURR(json) && !errno)
                {
                    // Has value
                    ret = bsp_new_value();
                    if (0.0f == modf(v_digit, &intpart))
                    {
                        // Integer
                        V_SET_INT(ret, (int64_t) v_digit);
                    }
                    else
                    {
                        // Double
                        V_SET_DOUBLE(ret, (double) v_digit);
                    }

                    STR_NOW(json) += (digit_end - STR_CURR(json));
                    end = BSP_TRUE;
                }
                else
                {
                    // Other
                    STR_NEXT(json);
                }
            }
            else
            {
                // Ignore
                STR_NEXT(json);
            }
        }

        if (end)
        {
            break;
        }
    }

    // Non-closed object
    if (status & JSON_DECODE_STATUS_STR && v_str)
    {
        if (STR_NOW(json) > normal)
        {
            bsp_string_append(v_str, STR_STR(json) + normal, (STR_NOW(json) - normal));
        }

        ret = bsp_new_value();
        V_SET_STRING(ret, v_str);
    }

    return ret;
}

static void _traverse_json_array(BSP_STRING *json, BSP_OBJECT *obj)
{
    if (!obj || BSP_OBJECT_ARRAY != obj->type || !json)
    {
        return;
    }

    BSP_VALUE *val = NULL;
    while (BSP_TRUE)
    {
        val = _get_value_from_json(json);
        if (!val)
        {
            break;
        }

        if (BSP_VALUE_OBJECT_ARRAY_END == val->type)
        {
            bsp_del_value(val);
            break;
        }

        bsp_object_set_array(obj, -1, val);
    }

    return;
}

static void _traverse_json_hash(BSP_STRING *json, BSP_OBJECT *obj)
{
    if (!obj || BSP_OBJECT_HASH != obj->type || !json)
    {
        return;
    }

    BSP_VALUE *key_v = NULL;
    BSP_VALUE *val = NULL;
    BSP_STRING *key = NULL;
    while (BSP_TRUE)
    {
        key_v = _get_value_from_json(json);
        if (!key_v)
        {
            // Unwilling break
            break;
        }

        if (BSP_VALUE_OBJECT_HASH_END == key_v->type)
        {
            // Succeed
            bsp_del_value(key_v);
            break;
        }

        val = _get_value_from_json(json);
        if (!val)
        {
            // No value
            bsp_del_value(key_v);
            break;
        }

        if (BSP_VALUE_OBJECT_HASH_END == val->type)
        {
            // Endding
            bsp_del_value(key_v);
            bsp_del_value(val);
            break;
        }

        if (BSP_VALUE_STRING == key_v->type)
        {
            // Pair
            key = V_GET_STRING(key_v);
            bsp_object_set_hash(obj, key, val);
        }
        else
        {
            // Unavailable key
            bsp_del_value(key_v);
            bsp_del_value(val);
        }
    }

    return;
}

BSP_OBJECT * json_nd_decode(BSP_STRING *json)
{
    BSP_OBJECT *obj = NULL;
    if (json)
    {
        STR_RESET(json);
        BSP_VALUE *first = _get_value_from_json(json);
        if (first)
        {
            if (BSP_VALUE_OBJECT == first->type)
            {
                // Structed
                obj = V_GET_OBJECT(first);
            }
            else
            {
                // Single
                obj = bsp_new_object();
                obj->type = BSP_OBJECT_SINGLE;
                bsp_object_set_single(obj, first);
            }
        }
    }

    return obj;
}
