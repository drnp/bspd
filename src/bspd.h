/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bspd.n
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
 * BSPd binary main header
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/27/2015
 * @changelog
 *      [02/27/2015] - Creation
 */

#ifndef _BSPD_H

#define _BSPD_H
// Load libbsp
#include "../config.h"
#include "bsp.h"

#define MAX_SESSION_ID_LENGTH           64
#define MAX_CHANNEL_NAME_LENGTH         64
#define DYNAMIC_CHANNEL_LIST_INITIAL    4096
#define SESSION_JOINED_LIST_INITIAL     64

#define LOG_FILENAME                    "bspd.log"

#define BSPD_FD_ADD_CLT                 0
#define BSPD_FD_ADD_SESSION             1

#include "serialization/serialization.h"
#include "server/server.h"
#include "protocol/protocol.h"
#include "utils/utils.h"
#include "wrapper/wrapper.h"

typedef struct bspd_config_t
{
    char                *config_file;
    char                *pid_file;
    char                *log_file;
    BSP_BOOTSTRAP_OPTIONS
                        opt;
    BSP_BOOLEAN         verbose;
    BSP_BOOLEAN         daemonize;
    const char          *script;
    const char          *lua_hook_load;

    // Servers
    struct bspd_server_t
                        *servers;
} BSPD_CONFIG;

// Definations
#define lua_lock(L)                     ((void) 0)

// Functions
BSPD_CONFIG * get_global_config();

#endif  /* _BSPD_H */
