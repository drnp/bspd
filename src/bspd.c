/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bspd.c
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
 * 3. Neither the name ``Dr.NP'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * bspd IS PROVIDED BY Dr.NP ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Dr.NP OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * BSPd binary main
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/27/2015
 * @changelog
 *      [02/27/2015] - Creation
 */

#include "bspd.h"

BSPD_CONFIG global_config;

// Get global configuration
BSPD_CONFIG * get_global_config()
{
    return &global_config;
}

static void _usage()
{
    fprintf(stderr, "\n\033[1;37mBSPD (\033[0m\033[1;32mgPVP\033[0m\033[1;37m) server. libbsp version :\033[0m \033[1;34m%s\033[0m\n", BSP_LIB_VERSION);
    fprintf(stderr, "========================================================================================\n");
    fprintf(stderr, "\t\033[1;33m-c\033[0m : Configuration file.\n");
    fprintf(stderr, "\t\033[1;33m-p\033[0m : PID file.\n");
    fprintf(stderr, "\t\033[1;33m-v\033[0m : Verbose mode (Set trace level to ALL).\n");
    fprintf(stderr, "\t\033[1;33m-h\033[0m : Display this topic.\n");
    fprintf(stderr, "\n\n");

    return;
}

// Portal
int main(int argc, char *const *argv)
{
    bzero(&global_config, sizeof(BSPD_CONFIG));
    global_config.verbose = BSP_FALSE;
    global_config.daemonize = BSP_FALSE;
    int c;
    char config_file[_POSIX_PATH_MAX];
    snprintf(config_file, _POSIX_PATH_MAX - 1, "%s/%s", BSPD_PREFIX_DIR, BSPD_CONF_FILE);
    config_file[_POSIX_PATH_MAX - 1] = 0x0;
    char pid_file[_POSIX_PATH_MAX];
    snprintf(pid_file, _POSIX_PATH_MAX - 1, "%s/%s", BSPD_PREFIX_DIR, BSPD_PID_FILE);
    pid_file[_POSIX_PATH_MAX - 1] = 0x0;
    char log_file[_POSIX_PATH_MAX];
    snprintf(log_file, _POSIX_PATH_MAX - 1, "%s/%s/%s", BSPD_PREFIX_DIR, BSPD_LOG_DIR, LOG_FILENAME);
    log_file[_POSIX_PATH_MAX - 1] = 0x0;

    while (-1 != (c = getopt(argc, argv, "c:p:vdh")))
    {
        switch (c)
        {
            case 'h' : 
                // Help topic
                _usage();
                return BSP_RTN_SUCCESS;
                break;
            case 'v' : 
                // Set verbose
                global_config.verbose = BSP_TRUE;
                break;
            case 'c' : 
                // Configuration file
                snprintf(config_file, _POSIX_PATH_MAX - 1, "%s", optarg);
                config_file[_POSIX_PATH_MAX - 1] = 0x0;
                break;
            case 'p' : 
                // PID file
                snprintf(pid_file, _POSIX_PATH_MAX - 1, "%s", optarg);
                pid_file[_POSIX_PATH_MAX - 1] = 0x0;
                break;
            case 'd' : 
                global_config.daemonize = BSP_TRUE;
                break;
            default : 
                // Do nothing
                break;
        }
    }

    // Maybe leak here, hehehe~~~
    global_config.config_file = bsp_strdup(config_file);
    global_config.pid_file = bsp_strdup(pid_file);
    global_config.log_file = bsp_strdup(log_file);
    bspd_startup();
    bsp_shutdown();

    return BSP_RTN_SUCCESS;
}
