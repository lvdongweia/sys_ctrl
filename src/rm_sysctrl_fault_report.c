/*************************************************************************
FileName: rm_sysctrl_fault_report.c
Copy Right: Copyright (C), 2014-2016, Avatarmind. Co., Ltd.
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

#define LOG_TAG "RM_SYSCTRL_FLR"
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
#include "rm_sysctrl_conf.h"

static unsigned int
fault_subsys_dead_id[SUB_SYSTEM_MAX] = {
    FAULT_SUBSYS_DEAD_ID_PER_SUBSYSTEM
};

static unsigned int
fault_expect_state_expired_id[ROBOT_STATUS_MAX][SUB_SYSTEM_MAX] = {
    FAULT_EXPECT_STATE_EXPIRE_ID_PER_STATUS_SUBSYS
};

static unsigned int
fault_init_subsys_expire_id[SUB_SYSTEM_MAX] = {
    FAULT_INIT_SUBSYS_EXPIRE_ID_PER_SUBSYSTEM
};

static unsigned int
fault_debug_set_expire_id[SUB_SYSTEM_MAX] = {
    FAULT_DEBUG_SET_EXPIRE_ID_PER_SUBSYSTEM
};

static unsigned int
fault_mode_set_expire_id[SUB_SYSTEM_MAX] = {
    FAULT_MODE_SET_EXPIRE_ID_PER_SUBSYSTEM
};

static unsigned int
fault_subsys_fault_id[SUB_SYSTEM_MAX] = {
    FAULT_SUBSYS_FAULT_ID_PER_SUBSYSTEM
};

extern char *invalid_str;

static char *rm_sysctrl_fault_report_desc_str[] =
{
    FAULT_REPORT_DESCIPTION_STR
};

static bool RmSysctrlIsValidFaultReport(unsigned int fault)
{
    bool is_valid = false;

    if (fault < FAULT_REPORT_MAX)
        is_valid = true;
    else
        is_valid = false;

    return is_valid;
}

static char* RmSysctrlGetFaultReportStr(unsigned int fault)
{
    if (RmSysctrlIsValidFaultReport(fault))
        return rm_sysctrl_fault_report_desc_str[fault];
    else
        return invalid_str;
}

static int RmSysctrlFaultReportFault(char src_id, char fault_id, char level)
{
    char len = 0;
    char data[3] = {0};
    unsigned int priority = LOW;

    data[0] = level;
    data[1] = len = 3;
    data[2] = fault_id;

    if (level == ROBOT_FAULT_WARN)
        priority = LOW;
    else if (level == ROBOT_FAULT_ERROR)
        priority = MIDDLE;
    else if (level == ROBOT_FAULT_FATAL)
        priority = HIGH;

    return RmSysctrlCanDataSend(priority,
                                src_id,
                                RM_FM,
                                data,
                                len);
}


int RmSysctrlFaultReportFaultGeneral(char src_id, char fault_id, char level)
{
    return RmSysctrlFaultReportFault(src_id, fault_id, level);
}

int RmSysctrlFaultReportFaultDetail(char src_id, char fault_id, char level)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    unsigned int fault_detail_id;

    if (!rm_sysctrl) return -1;

    if (fault_id == FAULT_SUBSYS_DEAD) {
        unsigned int subsys_idx;
        if (!rm_sysctrl->fault) return -1;
        RmSysctrlFaultGetFaultSubsys(rm_sysctrl->fault, &subsys_idx);
        
        fault_detail_id = fault_subsys_dead_id[subsys_idx];
        RLOGI("Report detail fault that '%s'",
            RmSysctrlGetFaultReportStr(fault_detail_id));
        RmSysctrlFaultReportFault(src_id, fault_detail_id, level);
    } else if (fault_id == FAULT_EXPECT_STATE_EXPIRE) {
        int i = 0;
        unsigned int expect_state = rm_sysctrl->expect_state;
        for (i = 0; i < SUB_SYSTEM_MAX; i++)
        {
            if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
            {
                if ((&rm_sysctrl->sub_system[i])->state != expect_state)
                {
                    fault_detail_id =
                        fault_expect_state_expired_id[expect_state][i];
                    RLOGI("Report detail fault that '%s'",
                        RmSysctrlGetFaultReportStr(fault_detail_id));
                    RmSysctrlFaultReportFault(src_id, fault_detail_id, level);
                }                    
            }
        }
    } else if (fault_id == FAULT_INIT_SUBSYS_EXPIRE) {
        int i = 0;
        for (i = 0; i < SUB_SYSTEM_MAX; i++)
        {
            if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
            {
                if (!IS_SUBSYS_INITALIZED(&rm_sysctrl->sub_system[i]))
                {
                    fault_detail_id = fault_init_subsys_expire_id[i];
                    RLOGI("Report detail fault that '%s'",
                        RmSysctrlGetFaultReportStr(fault_detail_id));
                    RmSysctrlFaultReportFault(src_id, fault_detail_id, level);
                }                    
            }
        }
    } else if (fault_id == FAULT_DEBUG_SET_EXPIRE) {
        int i = 0;
        bool debug_en = false;
        if (!rm_sysctrl->config) return -1;
        RmSysctrlGetDebug(rm_sysctrl->config, &debug_en);
        for (i = 0; i < SUB_SYSTEM_MAX; i++)
        {
            if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
            {
                if ((&rm_sysctrl->sub_system[i])->debug_en != debug_en)
                {
                    fault_detail_id = fault_debug_set_expire_id[i];
                    RLOGI("Report detail fault that '%s'",
                        RmSysctrlGetFaultReportStr(fault_detail_id));
                    RmSysctrlFaultReportFault(src_id, fault_detail_id, level);
                }                    
            }
        }
    } else if (fault_id == FAULT_MODE_SET_EXPIRE) {
        int i = 0;
        int sys_mode = -1;
        if (!rm_sysctrl->config) return -1;
        RmSysctrlGetMode(rm_sysctrl->config, &sys_mode);
        for (i = 0; i < SUB_SYSTEM_MAX; i++)
        {
            if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
            {
                if ((&rm_sysctrl->sub_system[i])->mode != (unsigned int)sys_mode)
                {
                    fault_detail_id = fault_mode_set_expire_id[i];
                    RLOGI("Report detail fault that '%s'",
                        RmSysctrlGetFaultReportStr(fault_detail_id));
                    RmSysctrlFaultReportFault(src_id, fault_detail_id, level);
                }
            }
        }
    } else if (fault_id == FAULT_SUBSYS_FAULT) {
        unsigned int falt_subsys_id = 0;
        struct RmSysctrlFault *fault = rm_sysctrl->fault;
        if (!fault) return -1;
        falt_subsys_id = fault->fault_subsys;
        fault_detail_id = fault_subsys_fault_id[falt_subsys_id];
        RLOGI("Report detail fault that '%s'",
            RmSysctrlGetFaultReportStr(fault_detail_id));
        RmSysctrlFaultReportFault(src_id, fault_detail_id, level);
    }

    return 0;
}



