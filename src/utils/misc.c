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
 * Misc functions
 *
 * @package bsp::duang
 * @author Dr.NP <np@bsgroup.org>
 * @update 06/15/2015
 * @changelog
 *      [06/15/2015] - Creation
 */

#include "../bspd.h"

// Get current directory
char * get_dir()
{
    char self_name[_POSIX_PATH_MAX];
    char *ret = NULL;
    char *curr;
    ssize_t nbytes = readlink("/proc/self/exe", self_name, _POSIX_PATH_MAX - 1);

    if (-1 == nbytes)
    {
        ret = "./";
    }
    else
    {
        self_name[nbytes] = 0x0;
        ret = realpath(self_name, NULL);
        curr = strrchr(ret, '/');

        if (curr)
        {
            curr[0] = 0x0;
            curr = strrchr(ret, '/');
            // Prev layer
            if (curr)
            {
                curr[0] = 0x0;
            }
        }
    }

    return ret;
}

// Set working directory
int set_dir(const char *dir)
{
    if (0 == chdir(dir))
    {
        bsp_trace_message(BSP_TRACE_NOTICE, "bspd", "Change current working directory to %s", dir);

        return BSP_RTN_SUCCESS;
    }
    else
    {
        bsp_trace_message(BSP_TRACE_ERROR, "bspd", "Change current working directory failed");

        return BSP_RTN_ERR_IO_ROUGH;
    }

    return BSP_RTN_INVALID;
}

// Write PID to file
int save_pid()
{
    BSPD_CONFIG *c = get_global_config();
    FILE *fp = fopen(c->pid_file, "w");
    if (!fp)
    {
        bsp_trace_message(BSP_TRACE_ERROR, "bspd", "Cannot write PID file : %s", c->pid_file);

        return BSP_RTN_ERR_IO_OPEN;
    }

    pid_t pid = getpid();
    fprintf(fp, "%d", (int) pid);
    fclose(fp);
    bsp_trace_message(BSP_TRACE_INFORMATIONAL, "bspd", "Save pid %d to PID file %s", (int) pid, c->pid_file);

    return BSP_RTN_SUCCESS;
}

// Process daemonization
pid_t proc_daemonize()
{
    pid_t pid = fork();
    switch (pid)
    {
        case -1 : 
            bsp_trace_message(BSP_TRACE_ERROR, "bspd", "Fork child proces failed");

            return BSP_RTN_ERR_PROCESS;
            break;
        case 0 : 
            // Child returned
            break;
        default : 
            // Parent process exit
            exit(BSP_RTN_SUCCESS);
            break;
    }

    // I am child
    if (-1 == setsid())
    {
        bsp_trace_message(BSP_TRACE_ERROR, "bspd", "Setsid failed");

        return BSP_RTN_ERR_PROCESS;
    }

    // Redirect dtandard IO
    int fd = open("/dev/null", O_RDWR, 0);
    if (fd)
    {
        (void) dup2(fd, STDIN_FILENO);
        (void) dup2(fd, STDOUT_FILENO);
        (void) dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO)
        {
            close(fd);
        }
    }
    else
    {
        bsp_trace_message(BSP_TRACE_ERROR, "bspd", "Open null device failed");
    }

    return pid;
}
