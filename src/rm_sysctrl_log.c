/*************************************************************************
FileName: rm_sysctrl_log.c
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2014-9-25
**************************************************************************/
/*************************************************************************
Editor: wangkun
Version: v1.0 init version
Edit Date: 2014-9-25
**************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <semaphore.h>

#define LOG_TAG "RM_SYSCTRL_LOG"
#include "cutils/log.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#include "rm_sysctrl_conf.h"

#define DEBUG_LEVEL		ANDROID_LOG_DEBUG
#define MAX_LOG_LEN 		2048
int RmSysctrlLog(int priv, const char * tag, const char * fmt, ...)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    char        buf[MAX_LOG_LEN] = {0};
    va_list     args;
    bool debug_en = false;

    if (rm_sysctrl)
    {
        struct RmSysctrlCnf *config = rm_sysctrl->config;
        if (config)
            RmSysctrlGetDebug(config, &debug_en);
    }

    if (!debug_en)
    {
        return 0;
    }

    if (priv < DEBUG_LEVEL)
    {
        return 0;
    }

    va_start(args,fmt);
    vsprintf(buf,fmt,args);
    va_end(args);

    return __android_log_print(priv, tag, "%s", buf);
}

