/*************************************************************************
FileName: rm_sysctrl_can.c
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2014-9-11
**************************************************************************/
/*************************************************************************
Editor: wangkun
Version: v1.0 init version
Edit Date: 2014-9-4
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
#include <sys/ipc.h>
#include <malloc.h>
#include <semaphore.h>

#define LOG_TAG "RM_SYSCTRL_FLT"
#include "cutils/log.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#include "rm_sysctrl_can.h"
#include "rm_sysctrl_fault_report.h"

extern char *invalid_str;

char *rm_sysctrl_fault_desc_str[] =
{
    "the sub-system is dead.",					//FAULT_SUBSYS_DEAD		0
    "the synchronization of state to sub-system is timeout.",	//FAULT_EXPECT_STATE_EXPIRE	1
    "the init subsys is timeout",				//FAULT_INIT_SUBSYS_EXPIRE	2
    "the deep sleep is timeout",				//FAULT_DEEP_SLEEP_EXPIRE	3
    "the deep wake is timeout",					//FAULT_DEEP_WAKE_EXPIRE	4
    "the debug set is timeout",					//FAULT_DEBUG_SET_EXPIRE	5
    "the mode set is timeout",					//FAULT_MODE_SET_EXPIRE		6
    "the subsys fault occurs",					//FAULT_SUBSYS_FAULT		7
};

bool RmSysctrlIsValidFault(int fault)
{
    bool is_valid = false;

    switch (fault)
    {
    case FAULT_SUBSYS_DEAD:
    case FAULT_EXPECT_STATE_EXPIRE:
    case FAULT_INIT_SUBSYS_EXPIRE:
#ifdef DEEP_SLEEP_SUPPORT
    case FAULT_DEEP_SLEEP_EXPIRE:
    case FAULT_DEEP_WAKE_EXPIRE:
#endif
    case FAULT_DEBUG_SET_EXPIRE:
    case FAULT_MODE_SET_EXPIRE:
    case FAULT_SUBSYS_FAULT:
        is_valid = true;
        break;
    default:
        is_valid = false;
    }

    return is_valid;
}

char* RmSysctrlGetFaultStr(int fault)
{
    if (RmSysctrlIsValidFault(fault))
        return rm_sysctrl_fault_desc_str[fault];
    else
        return invalid_str;
}

int RmSysctrlCreateFault(struct RmSysctrlFault **rscf_addr)
{
    struct RmSysctrlFault *rscf_l;
    rscf_l = (struct RmSysctrlFault *)malloc(sizeof(struct RmSysctrlFault));
    if (!rscf_l)
    {
        RLOGE("RmSysctrlFault create failed!\n");
        return -1;
    }
    memset(rscf_l, 0, sizeof(struct RmSysctrlFault));
    *rscf_addr = rscf_l;
    return 0;
}

int RmSysctrlInitFault(struct RmSysctrlFault **rscf_addr)
{
    int ret = -1;
    struct RmSysctrlFault *rscf;

    /* Create RmSysctrlFault */    
    ret = RmSysctrlCreateFault(rscf_addr);
    if (ret < 0)
    {
        RLOGE("RmSysctrlFault create failed!");
        return -1;
    }

    rscf = *rscf_addr;

    rscf->last_fault_type = FAULT_NONE;
    rscf->level[FAULT_SUBSYS_DEAD] = ROBOT_FAULT_FATAL;
    rscf->level[FAULT_EXPECT_STATE_EXPIRE] = ROBOT_FAULT_ERROR;
    rscf->level[FAULT_INIT_SUBSYS_EXPIRE] = ROBOT_FAULT_ERROR;
#ifdef DEEP_SLEEP_SUPPORT
    rscf->level[FAULT_DEEP_SLEEP_EXPIRE] = ROBOT_FAULT_ERROR;
    rscf->level[FAULT_DEEP_WAKE_EXPIRE] = ROBOT_FAULT_ERROR;
#endif
    rscf->level[FAULT_DEBUG_SET_EXPIRE] = ROBOT_FAULT_ERROR;
    rscf->level[FAULT_MODE_SET_EXPIRE] = ROBOT_FAULT_ERROR;
    rscf->level[FAULT_SUBSYS_FAULT] = ROBOT_FAULT_ERROR;

    return 0;
}

int RmSysctrlFaultSetLastFaultType(struct RmSysctrlFault *rscf,
                                    int fault_type)
{
    if (!rscf || fault_type > FAULT_MAX)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    rscf->last_fault_type = fault_type;

    /* Report fault */
    RLOGI("Report fault that %s", RmSysctrlGetFaultStr(fault_type));
    RmSysctrlFaultReportFaultGeneral(RM_SYSCTRL, fault_type, rscf->level[fault_type]);
    RmSysctrlFaultReportFaultDetail(RM_SYSCTRL, fault_type, rscf->level[fault_type]);

    return 0;
}

int RmSysctrlFaultGetLastFaultType(struct RmSysctrlFault *rscf,
                                   int *fault_type)
{
    if (!rscf || !fault_type)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    *fault_type = rscf->last_fault_type;
    return 0;
}

int RmSysctrlFaultSetSubsysFaultType(struct RmSysctrlFault *rscf,
                                   unsigned int fault_type)
{
    if (!rscf)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    rscf->subsys_fault_type = fault_type;

    return 0;
}

int RmSysctrlFaultGetSubsysFaultType(struct RmSysctrlFault *rscf,
                                   unsigned int *fault_type)
{
    if (!rscf || !fault_type)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    *fault_type = rscf->subsys_fault_type;
    return 0;
}

int RmSysctrlFaultSetFaultSubsys(struct RmSysctrlFault *rscf,
                                   unsigned int fault_subsys)
{
    if (!rscf)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    rscf->fault_subsys = fault_subsys;

    return 0;
}

int RmSysctrlFaultGetFaultSubsys(struct RmSysctrlFault *rscf,
                                   unsigned int *fault_subsys)
{
    if (!rscf || !fault_subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    *fault_subsys = rscf->fault_subsys;
    return 0;
}

