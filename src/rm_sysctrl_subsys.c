/*************************************************************************
FileName: rm_sysctrl_subsys.c
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2014-9-12
**************************************************************************/
/*************************************************************************
Editor: wangkun
Version: v1.0 init version
Edit Date: 2014-9-12
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
#include <malloc.h>
#include <semaphore.h>

#define LOG_TAG "RM_SYSCTRL_SUBSYS"
#include "cutils/log.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#include "rm_sysctrl_conf.h"

int RmSysctrlInitSubSystem(struct RmSysctrlSubSystem *subsys)
{
    int def_mode;
    bool def_debug;
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();

    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }
    memset(subsys, 0, sizeof(struct RmSysctrlSubSystem));
    SET_STATE(subsys, ROBOT_STATUS_NONE);
    ENABLE_SUBSYSTEM(subsys);
    CONFIG_SUBSYS_EXPIRE_TIME(subsys, SUBSYSTEM_EXPIRE_TIME);
    RESET_SUBSYSTEM_TIMER(subsys);
    RESET_SUBSYS_INITALIZED(subsys);
    RmSysctrlGetDefaultDebug(rm_sysctrl->config, &def_debug);
    SET_SUBSYS_DEBUG(subsys, def_debug);
    RmSysctrlGetDefaultMode(rm_sysctrl->config, &def_mode);
    SET_SUBSYS_MODE(subsys, def_mode);
    subsys->ver_str_used = false;
    return 0;
}

int RmSysctrlSetSubSystemStatus(struct RmSysctrlSubSystem *subsys,
                                unsigned int state,
                                unsigned int sub_state)
{
    if (!subsys || state > ROBOT_STATUS_MAX)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    SET_STATE(subsys, state);
    SET_SUBSTATE(subsys, sub_state);
    return 0;
}

int RmSysctrlEnableSubSystem(struct RmSysctrlSubSystem *subsys)
{
    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    ENABLE_SUBSYSTEM(subsys);
    return 0;
}

int RmSysctrlDisableSubSystem(struct RmSysctrlSubSystem *subsys)
{
    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    DISABLE_SUBSYSTEM(subsys);
    return 0;
}

int RmSysctrlSetPowerOnSubSystem(struct RmSysctrlSubSystem *subsys)
{
    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    SET_POWERON_SUBSYSTEM(subsys);
    return 0;
}

int RmSysctrlUnsetPowerOnSubSystem(struct RmSysctrlSubSystem *subsys)
{
    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    UNSET_POWERON_SUBSYSTEM(subsys);
    return 0;
}

bool RmSysctrlIsSubSystemPowerOn(struct RmSysctrlSubSystem *subsys)
{
    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }
    
    return IS_SUBSYSTEM_POWERON(subsys);
}

int RmSysctrlActiveSubSystem(struct RmSysctrlSubSystem *subsys)
{
    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    ACTIVE_SUBSYSTEM(subsys);
    return 0;
}

int RmSysctrlInactiveSubSystem(struct RmSysctrlSubSystem *subsys)
{
    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    INACTIVE_SUBSYSTEM(subsys);
    INC_SUBSYSTEM_DEAD_TIMES(subsys);
    return 0;
}

bool RmSysctrlIsSubSystemDead(struct RmSysctrlSubSystem *subsys)
{
    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return true;
    }

    return IS_SUBSYSTEM_EXPIRE(subsys);
}

int RmSysctrlFreshSubSystem(struct RmSysctrlSubSystem *subsys)
{
    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    ACTIVE_SUBSYSTEM(subsys);
    RESET_SUBSYSTEM_TIMER(subsys);
    return 0;
}

int RmSysctrlConfSubSystemExpire(struct RmSysctrlSubSystem *subsys,
                                 int expire_conf)
{
    if (!subsys)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    CONFIG_SUBSYS_EXPIRE_TIME(subsys, expire_conf);
    return 0;
}

int RmSysctrlUpdateSubSystemLastFault(struct RmSysctrlSubSystem *subsys,
    struct RmSysctrlFault *last_fault)
{
    return 0;
}

int RmSysctrlSetVersion(struct RmSysctrlSubSystem *subsys,
    char sub_sys_id,
    char major,
    char minor,
    char revision,
    unsigned short building_year,
    char building_month,
    char building_day,
    char rom_id)
{
    struct RmSysctrlVersion *ver = &(subsys->u.ver);

    if (!subsys || sub_sys_id > SUB_SYSTEM_ID_MAX ||
        building_month > 12 || building_day > 31)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }

    ver->sub_sys_id = sub_sys_id;
    ver->major = major;
    ver->minor = minor;
    ver->revision = revision;
    ver->building_year = building_year;
    ver->building_month = building_month;
    ver->building_day = building_day;
    ver->rom_id = rom_id;

    return 0;
}



