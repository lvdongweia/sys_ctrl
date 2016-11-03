/*************************************************************************
FileName: rm_sysctrl.c
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2014-9-4
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
#include <semaphore.h>

#define LOG_TAG "RM_SYSCTRL_OWN"
#include "cutils/log.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#ifdef DEEP_SLEEP_SUPPORT
#include "rm_sysctrl_gpio.h"
#endif
#include "rm_sysctrl_can.h"
#include "rm_sysctrl_conf.h"
#include "rm_sysctrl_service.h"
#include "rm_sysctrl_uevent.h"

struct RmSysctrl *rm_sysctrl = NULL;

extern char *invalid_str;

char *rm_sysctrl_Timer_str[] =
{
    "Heart Beat",			//TIMER_HEART_BEAT	0
    "Init",				//TIMER_INIT_SUBSYS	1
    "Register",				//TIMER_REGISTER	2
    "Relogin",				//TIMER_RELOGIN		3
    "Expect state",			//TIMER_EXPECT_STATE	4
    "Deep sleep",			//TIMER_DEEP_SLEEP	5
    "Deep wake",			//TIMER_DEEP_WAKE	6
    "Debug set",			//TIMER_DEBUG_SET	7
    "Mode set",				//TIMER_MODE_SET	8
    "Auto sleep",			//TIMER_AUTO_SLEEP	9
};

bool RmSysctrlIsValidTimer(int timer)
{
    bool is_valid = false;

    switch (timer)
    {
    case TIMER_HEART_BEAT:
    case TIMER_INIT_SUBSYS:
    case TIMER_REGISTER:
    case TIMER_RELOGIN:
    case TIMER_EXPECT_STATE:
#ifdef DEEP_SLEEP_SUPPORT
    case TIMER_DEEP_SLEEP:
    case TIMER_DEEP_WAKE:
#endif
    case TIMER_DEBUG_SET:
    case TIMER_MODE_SET:
#ifdef AUTO_SLEEP_SUPPORT
    case TIMER_AUTO_SLEEP:
#endif
        is_valid = true;
        break;
    default:
        is_valid = false;
    }

    return is_valid;
}

char* RmSysctrlGetTimerStr(int timer)
{
    if (RmSysctrlIsValidTimer(timer))
        return rm_sysctrl_Timer_str[timer];
    else
        return invalid_str;
}

struct RmSysctrl *RmSysctrlSelf()
{
    return rm_sysctrl;
}

int RmSysctrlCreate(struct RmSysctrl **rm_sysctrl_addr)
{
    struct RmSysctrl *rm_sysctrl_l;
    rm_sysctrl_l = (struct RmSysctrl *)malloc(sizeof(struct RmSysctrl));
    if (!rm_sysctrl_l)
    {
        RLOGE("RmSysctrl create failed!\n");
        return -1;
    }
    memset(rm_sysctrl_l, 0, sizeof(struct RmSysctrl));
    *rm_sysctrl_addr = rm_sysctrl_l;
    return 0;
}

static int InitConfig(struct RmSysctrl *rm_sysctrl)
{
    int ret = -1;

    RLOGI("Init Config");
    ret = RmSysctrlInitConfig(&rm_sysctrl->config);
    if (ret < 0)
    {
        RLOGE("Init RmSysctrlInitConfig failed!");
        return -1;
    }

    return 0;
}

static int InitCommunication(struct RmSysctrl *rm_sysctrl)
{
    int ret = -1;

    RLOGI("Init Communication");
    ret = RmSysctrlInitComm(&rm_sysctrl->comm);
    if (ret < 0)
    {
        RLOGE("Init RmSysctrlInitComm failed!");
        return -1;
    }

    return 0;
}

static int InitFsm(struct RmSysctrl *rm_sysctrl)
{
    int ret = -1;

    RLOGI("Init FSM");
    ret = RmSysctrlFsmInit(rm_sysctrl);
    if (ret < 0)
    {
        RLOGE("Init RmSysctrlFsmInit failed!");
        return -1;
    }

    return 0;
}

static int InitSubSystem(struct RmSysctrl *rm_sysctrl)
{
    int ret = -1;
    int i = 0;

    RLOGI("Init Sub-system");
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        ret = RmSysctrlInitSubSystem(&rm_sysctrl->sub_system[i]);
        if (ret < 0)
        {
            RLOGE("Init Sub-system failed!");
            return -1;
        }
    }
#if 0 /* For test */
    RmSysctrlDisableSubSystem(&rm_sysctrl->sub_system[RP_SYSTEM_ID]);
    RmSysctrlDisableSubSystem(&rm_sysctrl->sub_system[RF_SYSTEM_ID]);
    RmSysctrlDisableSubSystem(&rm_sysctrl->sub_system[RC_SYSTEM_ID]);
    RmSysctrlDisableSubSystem(&rm_sysctrl->sub_system[RB_R_ARM_ID]);
    RmSysctrlDisableSubSystem(&rm_sysctrl->sub_system[RB_L_ARM_ID]);
    RmSysctrlDisableSubSystem(&rm_sysctrl->sub_system[RB_BODY_ID]);
    RmSysctrlDisableSubSystem(&rm_sysctrl->sub_system[RM_SYSTEM_ID]);
#else
    RmSysctrlDisableSubSystem(&rm_sysctrl->sub_system[R_RC_ID]);
#endif

    return 0;
}

static int InitFault(struct RmSysctrl *rm_sysctrl)
{
    int ret = -1;

    RLOGI("Init Fault");
    ret = RmSysctrlInitFault(&rm_sysctrl->fault);
    if (ret < 0)
    {
        RLOGE("Init RmSysctrlInitFault failed!");
        return -1;
    }

    return 0;
}

static int InitCan(struct RmSysctrl *rm_sysctrl)
{
    int ret = -1;

    RLOGI("Init Can");
    ret = RmSysctrlInitCan(&rm_sysctrl->can);
    if (ret < 0)
    {
        RLOGE("Init RmSysctrlCan failed!");
        return -1;
    }

    return 0;
}

static int InitTimer(struct RmSysctrl *rm_sysctrl)
{
    RLOGI("Init Timer");
    RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_HEART_BEAT,
        HEART_BEAT_EXPIRE_TIME);
    RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_INIT_SUBSYS,
        INIT_SUBSYS_EXPIRE_TIME);
    RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_REGISTER,
        REGISTER_EXPIRE_TIME);
    RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_RELOGIN,
        RELOGIN_EXPIRE_TIME);
    RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_EXPECT_STATE,
        EXPECT_STATE_EXPIRE_TIME);
#ifdef DEEP_SLEEP_SUPPORT
    RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_DEEP_SLEEP,
        DEEP_SLEEP_EXPIRE_TIME);
    RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_DEEP_WAKE,
        DEEP_WAKE_EXPIRE_TIME);
#endif
    RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_DEBUG_SET,
        DEBUG_SET_EXPIRE_TIME);
    RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_MODE_SET,
        MODE_SET_EXPIRE_TIME);
#ifdef AUTO_SLEEP_SUPPORT
    RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_AUTO_SLEEP,
        AUTO_SLEEP_EXPIRE_TIME);
#endif

    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_HEART_BEAT);
    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_EXPECT_STATE);
#ifdef DEEP_SLEEP_SUPPORT
    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_SLEEP);
    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_WAKE);
#endif
    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEBUG_SET);
    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_MODE_SET);
#ifdef AUTO_SLEEP_SUPPORT
    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_AUTO_SLEEP);
#endif

    return 0;
}

#ifdef DEEP_SLEEP_SUPPORT
static int InitGpio(struct RmSysctrl *rm_sysctrl)
{
    int ret = -1;

    RLOGI("Init GPIO");
    ret = RmSysctrlGpioInit(&rm_sysctrl->gpio);
    if (ret < 0)
    {
        RLOGE("Init RmSysctrlGpioInit failed!");
        return -1;
    }

    return 0;
}
#endif

int RmSysctrlInit(struct RmSysctrl **rm_sysctrl_addr)
{
    int ret = -1;
    struct RmSysctrl *rm_sysctrl;

    RLOGI("Init RmSysctrl");

    /* Create RmSysctrl */
    ret = RmSysctrlCreate(rm_sysctrl_addr);
    if (ret < 0)
    {
        RLOGE("RmSysctrl create failed!");
        return -1;
    }

    rm_sysctrl = *rm_sysctrl_addr;

    /* Init Config */
    ret = InitConfig(rm_sysctrl);
    if (ret < 0) goto init_fail;

    /* Init Communication */
    ret = InitCommunication(rm_sysctrl);
    if (ret < 0) goto init_fail;
    
    /* Init FSM */
    ret = InitFsm(rm_sysctrl);
    if (ret < 0) goto init_fail;

    /* Init sub-system */
    ret = InitSubSystem(rm_sysctrl);
    if (ret < 0) goto init_fail;

    /* Init Fault */
    ret = InitFault(rm_sysctrl);
    if (ret < 0) goto init_fail;

    /* Init Can */
    ret = InitCan(rm_sysctrl);
    if (ret < 0) goto init_fail;

    /* Init Timer */
    ret = InitTimer(rm_sysctrl);
    if (ret < 0) goto init_fail;

#ifdef DEEP_SLEEP_SUPPORT
    /* Init GPIO */
    ret = InitGpio(rm_sysctrl);
    if (ret < 0) goto init_fail;
#endif

    return 0;

init_fail:
    return -1;
}

static void UpdateChangedData(struct RmSysctrl *rm_sysctrl)
{
    /* Broadcast query sub-system state message into can */
    RmSysctrlBroadcastSubsysStateQueryMsg(rm_sysctrl);

    /* Uptime*/
    RLOGD("To RP: query the uptime by sending message onto can.");
    RmSysctrlCommSendUptimeQueryToCan(rm_sysctrl->comm,
         MIDDLE, RP_SYSCTRL);
}

static bool all_subsys_version_updated(struct RmSysctrl *rm_sysctrl)
{
    int i;
    bool is_all_subsys_version_updated = true;

    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        struct RmSysctrlSubSystem *subsys =
            &rm_sysctrl->sub_system[i];

        if (IS_SUBSYSTEM_ENABLE(subsys))
        {
            if ((subsys->ver_str_used && strlen(subsys->u.ver_str) == 0) ||
                (!subsys->ver_str_used && subsys->u.ver.building_year == 0))
            {
                is_all_subsys_version_updated = false;
                break;
            }
        }
    }

    return is_all_subsys_version_updated;
}

#ifdef GET_RM_VERSION_FROM_PROPERTY
#include "cutils/properties.h"
#endif

UNICAST_MSG_TO_SUBSYS(VersionQuery)

#ifdef UNICAST_ALL_INSTEAD_OF_MULTICAST_SUPPORT
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(VersionQuery)
#else
BROADCAST_MSG_TO_SUBSYS(VersionQuery)
#endif

static void UpdateUnchangedData(struct RmSysctrl *rm_sysctrl)
{
    if (rm_sysctrl->unchanged_data_updated_times
        >= MAX_UNCHANGED_DATA_UPDATE_TIMES)
    {
        return;
    }

    if (!all_subsys_version_updated(rm_sysctrl))
    {
        /* Broadcast query sub-system version message into can */
        RmSysctrlBroadcastVersionQueryMsg(rm_sysctrl);

#ifdef GET_RM_VERSION_FROM_PROPERTY
        char prop[PROPERTY_VALUE_MAX] = {0};
        if(property_get("ro.build.version.incremental", prop, NULL) != 0) {
            struct RmSysctrlSubSystem *subsys =
                &rm_sysctrl->sub_system[RM_SYSTEM_ID];
            memcpy(subsys->u.ver_str, prop, MAX_VER_LEN);
            subsys->ver_str_used = true;
        }
#endif
    }

    /* Update cpuid
     * Send Get CPUID Command to Rp
     */
    if (strlen(rm_sysctrl->cpuid) == 0)
    {
        RLOGI("ToCan: Send Get CPUID Command to Rp.");
        RmSysctrlCommSendGetCPUIDCmdToCan(rm_sysctrl->comm,
            MIDDLE, RP_SYSCTRL);
    }

    /* Update SN
     * Send Get SN Command to Rp
     */
    if (strlen(rm_sysctrl->sn) == 0)
    {
        RLOGI("ToCan: Send Get SN Command to Rp.");
        RmSysctrlCommSendReadSNCmdToCan(rm_sysctrl->comm,
             MIDDLE, RP_SYSCTRL);
    }

    /* Update Datetime
     * Send Datetime Query to Rp
     */
    if (rm_sysctrl->datetime.year == 0)
    {
        RLOGI("ToCan: Send Datetime Query to Rp.");
        RmSysctrlCommSendDatetimeQueryToCan(rm_sysctrl->comm,
             MIDDLE, RP_SYSCTRL);
    }

    /* Update sex
     * Send sex Query to Rp
     */
    if (rm_sysctrl->sex == INVALID_SEX)
    {
        RLOGI("ToCan: Send Sex Query to Rp.");
        RmSysctrlCommSendSexQueryToCan(rm_sysctrl->comm,
             MIDDLE, RP_SYSCTRL);
    }

    /* Update uptime
     * Send uptime Query to Rp
     */
    if (rm_sysctrl->uptime == 0)
    {
        RLOGI("ToCan: Send uptime Query to Rp.");
        RmSysctrlCommSendUptimeQueryToCan(rm_sysctrl->comm,
             MIDDLE, RP_SYSCTRL);
    }

    rm_sysctrl->unchanged_data_updated_times++;
}

static void UpdateHeartBeatTimer(struct RmSysctrl *rm_sysctrl)
{
    /* Heart Beat Timer */
    RM_SYSCTRL_INC_TIMER(rm_sysctrl, TIMER_HEART_BEAT);
    if (RM_SYSCTRL_IS_TIMER_EXPIRE(rm_sysctrl, TIMER_HEART_BEAT))
    {
        int i = 0;

        /* Heart Beat timer expire */
	RM_SYSCTRL_INC_EXPIRE_TIMES(rm_sysctrl, TIMER_HEART_BEAT);

        UpdateChangedData(rm_sysctrl);
        UpdateUnchangedData(rm_sysctrl);

        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_HEART_BEAT);
    }
}

static void UpdateSubSystemTimer(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;

    /* Sub-system Timer */
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        struct RmSysctrlSubSystem *subsys =
            &rm_sysctrl->sub_system[i];

        if (IS_SUBSYSTEM_ENABLE(subsys))
        {
            INC_SUBSYSTEM_TIMER(&rm_sysctrl->sub_system[i]);

            if (RmSysctrlIsSubSystemDead(&rm_sysctrl->sub_system[i]))
            {
                 /* Sub-system dead */
                 /* Send sub-system dead event */
                 RmSysctrlCommSendSubsysDead(rm_sysctrl->comm, i);

                 RESET_SUBSYSTEM_TIMER(&rm_sysctrl->sub_system[i]);
             }
             else if (IS_SUBSYSTEM_NEARLY_EXPIRE(subsys))
             {
                 /* If the sub-system is nearly expired, a unicast
                  * query state message will be sent separatly.
                  */
                 unsigned int subsys_module_id;

                 if (i == 0)
                     subsys_module_id = RM_SCHEDULER;
                 else
                     subsys_module_id = i << 4;
                 RLOGW("ToCan: unicast state query to nearly dead"
                       " sub-system.");
                 RmSysctrlCommSendSubsysStateQueryToCan(
                     rm_sysctrl->comm, HIGH, subsys_module_id);
                 RLOGW("To sleep 100ms to wait the nearly dead"
                       " sub-system response query.");
                 usleep(100000);
             }
        }
    }
}

static void UpdateInitSubSystemTimer(struct RmSysctrl *rm_sysctrl)
{
    RM_SYSCTRL_INC_TIMER(rm_sysctrl, TIMER_INIT_SUBSYS);
    if (RM_SYSCTRL_IS_TIMER_EXPIRE(rm_sysctrl, TIMER_INIT_SUBSYS))
    {
        /* Init sub-system expire */
	RM_SYSCTRL_INC_EXPIRE_TIMES(rm_sysctrl, TIMER_INIT_SUBSYS);
        /* Send Init sub-system timer expire event */
        RmSysctrlCommSendTimerExpire(rm_sysctrl->comm, TIMER_INIT_SUBSYS);
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_INIT_SUBSYS);
    }
}

static void UpdateRegisterTimer(struct RmSysctrl *rm_sysctrl)
{
    RM_SYSCTRL_INC_TIMER(rm_sysctrl, TIMER_REGISTER);
    if (RM_SYSCTRL_IS_TIMER_EXPIRE(rm_sysctrl, TIMER_REGISTER))
    {
        /* Register Timer expire */
	RM_SYSCTRL_INC_EXPIRE_TIMES(rm_sysctrl, TIMER_REGISTER);
        /* Send Register timer expire event */
        RmSysctrlCommSendTimerExpire(rm_sysctrl->comm, TIMER_REGISTER);
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_REGISTER);
    }
}

static void UpdateReloginTimer(struct RmSysctrl *rm_sysctrl)
{
    RM_SYSCTRL_INC_TIMER(rm_sysctrl, TIMER_RELOGIN);
    if (RM_SYSCTRL_IS_TIMER_EXPIRE(rm_sysctrl, TIMER_RELOGIN))
    {
        /* Relogin Timer expire */
	RM_SYSCTRL_INC_EXPIRE_TIMES(rm_sysctrl, TIMER_RELOGIN);
        /* Send Relogin timer expire event */
        RmSysctrlCommSendTimerExpire(rm_sysctrl->comm, TIMER_RELOGIN);
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_RELOGIN);
    }
}

static void UpdateExpectStatusTimer(struct RmSysctrl *rm_sysctrl)
{
    RM_SYSCTRL_INC_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);
    if (RM_SYSCTRL_IS_TIMER_EXPIRE(rm_sysctrl, TIMER_EXPECT_STATE))
    {
        /* Expect state expire */
	RM_SYSCTRL_INC_EXPIRE_TIMES(rm_sysctrl, TIMER_EXPECT_STATE);
        /* Send Expect Status timer expire event */
        RmSysctrlCommSendTimerExpire(rm_sysctrl->comm,
            TIMER_EXPECT_STATE);
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);
    }
    else if (RM_SYSCTRL_IS_TIMER_NEARLY_EXPIRE(rm_sysctrl,
             TIMER_EXPECT_STATE))
    {
        RLOGW("The expect state timer nearly expired.");
        RmSysctrlUnicastLastMsgForExpectState(rm_sysctrl);
    }
    else if (RM_SYSCTRL_IS_TIMER_TOO_LONG(rm_sysctrl,
             TIMER_EXPECT_STATE))
    {
        RLOGW("The expect state is not updated too long time.");
        RmSysctrlUnicastLastMsgForExpectState(rm_sysctrl);
    }
    else if (RM_SYSCTRL_IS_TIMER_NOT_IMMEDIATELY(rm_sysctrl,
             TIMER_EXPECT_STATE))
    {
        RLOGW("The expect state is not updated immediately.");
        RmSysctrlUnicastLastMsgForExpectState(rm_sysctrl);
    }

}

#ifdef DEEP_SLEEP_SUPPORT
static void UpdateDeepSleepTimer(struct RmSysctrl *rm_sysctrl)
{
    RM_SYSCTRL_INC_TIMER(rm_sysctrl, TIMER_DEEP_SLEEP);
    if (RM_SYSCTRL_IS_TIMER_EXPIRE(rm_sysctrl, TIMER_DEEP_SLEEP))
    {
        /* Deep sleep expire */
	RM_SYSCTRL_INC_EXPIRE_TIMES(rm_sysctrl, TIMER_DEEP_SLEEP);
        /* Send Deep sleep timer expire event */
        RmSysctrlCommSendTimerExpire(rm_sysctrl->comm, TIMER_DEEP_SLEEP);
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_DEEP_SLEEP);
    }
}

static void UpdateDeepWakeTimer(struct RmSysctrl *rm_sysctrl)
{
    RM_SYSCTRL_INC_TIMER(rm_sysctrl, TIMER_DEEP_WAKE);
    if (RM_SYSCTRL_IS_TIMER_EXPIRE(rm_sysctrl, TIMER_DEEP_WAKE))
    {
        /* Deep wake expire */
	RM_SYSCTRL_INC_EXPIRE_TIMES(rm_sysctrl, TIMER_DEEP_WAKE);
        /* Send Deep wake timer expire event */
        RmSysctrlCommSendTimerExpire(rm_sysctrl->comm, TIMER_DEEP_WAKE);
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_DEEP_WAKE);
    }
}
#endif

static void UpdateDebugSetTimer(struct RmSysctrl *rm_sysctrl)
{
    RM_SYSCTRL_INC_TIMER(rm_sysctrl, TIMER_DEBUG_SET);
    if (RM_SYSCTRL_IS_TIMER_EXPIRE(rm_sysctrl, TIMER_DEBUG_SET))
    {
        /* Debug set expire */
	RM_SYSCTRL_INC_EXPIRE_TIMES(rm_sysctrl, TIMER_DEBUG_SET);
        /* Send debug set timer expire event */
        RmSysctrlCommSendTimerExpire(rm_sysctrl->comm, TIMER_DEBUG_SET);
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_DEBUG_SET);
    }
    else if (RM_SYSCTRL_IS_TIMER_NEARLY_EXPIRE(rm_sysctrl,
             TIMER_DEBUG_SET))
    {
        ALOGW("The debug set timer nearly expired.");
        RmSysctrlUnicastLastMsgForDebugSet(rm_sysctrl);
    }
    else if (RM_SYSCTRL_IS_TIMER_TOO_LONG(rm_sysctrl,
             TIMER_DEBUG_SET))
    {
        RLOGW("The debug set is not updated too long time.");
        RmSysctrlUnicastLastMsgForDebugSet(rm_sysctrl);
    }
    else if (RM_SYSCTRL_IS_TIMER_NOT_IMMEDIATELY(rm_sysctrl,
             TIMER_DEBUG_SET))
    {
        RLOGW("The debug set is not updated immediately.");
        RmSysctrlUnicastLastMsgForDebugSet(rm_sysctrl);
    }
}

static void UpdateModeSetTimer(struct RmSysctrl *rm_sysctrl)
{
    RM_SYSCTRL_INC_TIMER(rm_sysctrl, TIMER_MODE_SET);
    if (RM_SYSCTRL_IS_TIMER_EXPIRE(rm_sysctrl, TIMER_MODE_SET))
    {
        /* Mode set expire */
	RM_SYSCTRL_INC_EXPIRE_TIMES(rm_sysctrl, TIMER_MODE_SET);
        /* Send mode set timer expire event */
        RmSysctrlCommSendTimerExpire(rm_sysctrl->comm, TIMER_MODE_SET);
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_MODE_SET);
    }
    else if (RM_SYSCTRL_IS_TIMER_NEARLY_EXPIRE(rm_sysctrl,
             TIMER_MODE_SET))
    {
        ALOGW("The mode set timer nearly expired.");
        RmSysctrlUnicastLastMsgForModeSet(rm_sysctrl);
    }
    else if (RM_SYSCTRL_IS_TIMER_TOO_LONG(rm_sysctrl,
             TIMER_MODE_SET))
    {
        RLOGW("The mode set is not updated too long time.");
        RmSysctrlUnicastLastMsgForModeSet(rm_sysctrl);
    }
    else if (RM_SYSCTRL_IS_TIMER_NOT_IMMEDIATELY(rm_sysctrl,
             TIMER_MODE_SET))
    {
        RLOGW("The mode set is not updated immediately.");
        RmSysctrlUnicastLastMsgForModeSet(rm_sysctrl);
    }
}

#ifdef AUTO_SLEEP_SUPPORT
static void UpdateAutoSleepTimer(struct RmSysctrl *rm_sysctrl)
{
    RM_SYSCTRL_INC_TIMER(rm_sysctrl, TIMER_AUTO_SLEEP);
    if (RM_SYSCTRL_IS_TIMER_EXPIRE(rm_sysctrl, TIMER_AUTO_SLEEP))
    {
        /* Auto sleep expire */
	RM_SYSCTRL_INC_EXPIRE_TIMES(rm_sysctrl, TIMER_AUTO_SLEEP);
        /* Send auto sleep timer expire event */
        RmSysctrlCommSendTimerExpire(rm_sysctrl->comm, TIMER_AUTO_SLEEP);
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_AUTO_SLEEP);
    }
}
#endif

void *RmSysctrlTimerThread(void *arg)
{
    bool rm_sysctrl_timer_expired = false;
    struct RmSysctrl *rm_sysctrl = arg;
    unsigned long expire_timer_ids = 0;

    while(1)
    {
        /* Heart Beat and Sub-system Timer */
        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_HEART_BEAT))
        {
            UpdateHeartBeatTimer(rm_sysctrl);
            UpdateSubSystemTimer(rm_sysctrl);
        }

        /* Init Sub-system Timer */
        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_INIT_SUBSYS))
        {
            UpdateInitSubSystemTimer(rm_sysctrl);
        }

        /* Register Timer */
        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_REGISTER))
        {
            UpdateRegisterTimer(rm_sysctrl);
        }

        /* Relogin Timer */
        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_RELOGIN))
        {
            UpdateReloginTimer(rm_sysctrl);
        }

        /* Expect Status Timer */
        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_EXPECT_STATE))
        {
            UpdateExpectStatusTimer(rm_sysctrl);
        }

#ifdef DEEP_SLEEP_SUPPORT
        /* Deep sleep Timer */
        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_DEEP_SLEEP))
        {
            UpdateDeepSleepTimer(rm_sysctrl);
        }

        /* Deep wake Timer */
        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_DEEP_WAKE))
        {
            UpdateDeepWakeTimer(rm_sysctrl);
        }
#endif

        /* Debug set Timer */
        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_DEBUG_SET))
        {
            UpdateDebugSetTimer(rm_sysctrl);
        }

        /* Mode set Timer */
        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_MODE_SET))
        {
            UpdateModeSetTimer(rm_sysctrl);
        }

#ifdef AUTO_SLEEP_SUPPORT
        /* Auto sleep Timer */
        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_AUTO_SLEEP))
        {
            UpdateAutoSleepTimer(rm_sysctrl);
        }
#endif

        sleep(1);
    }
}

void RmSysctrlStartTimerThread(struct RmSysctrl *rm_sysctrl)
{
    int ret = 0;

    ret = pthread_create(&rm_sysctrl->ntid_timer,
                         NULL,
                         RmSysctrlTimerThread,
                         rm_sysctrl);
    if(ret != 0){
        RLOGE("can't create thread: %s\n",strerror(ret));
        return;
    }
}

void RmSysctrlStartCliThread(struct RmSysctrl *rm_sysctrl)
{
    int ret = 0;

    ret = pthread_create(&rm_sysctrl->ntid_cli,
                         NULL,
                         RmSysctrlCliThread,
                         rm_sysctrl);
    if(ret != 0){
        RLOGE("can't create thread: %s\n",strerror(ret));
        return;
    }
}

#ifdef DEEP_SLEEP_SUPPORT
void RmSysctrlStartGpioThread(struct RmSysctrl *rm_sysctrl)
{
    int ret = 0;
    struct RmSysctrlGpio *gpio = rm_sysctrl->gpio;

    ret = pthread_create(&gpio->ntid_gpio,
                         NULL,
                         RmSysctrlGpioThread,
                         rm_sysctrl);
    if(ret != 0){
        RLOGE("can't create thread: %s\n",strerror(ret));
        return;
    }
}
#endif

void RmSysctrlStartSrvThread(struct RmSysctrl *rm_sysctrl)
{
    int ret = 0;

    ret = pthread_create(&rm_sysctrl->ntid_srv,
                         NULL,
                         RmSysctrlSrvThread,
                         rm_sysctrl);
    if(ret != 0){
        RLOGE("can't create thread: %s\n",strerror(ret));
        return;
    }
}

void RmSysctrlStartUeventThread(struct RmSysctrl *rm_sysctrl)
{
    int ret = 0;

    ret = pthread_create(&rm_sysctrl->ntid_uevent,
                         NULL,
                         RmSysctrlUeventThread,
                         rm_sysctrl);
    if(ret != 0){
        RLOGE("can't create thread: %s\n",strerror(ret));
        return;
    }
}

int main(void)
{
    int ret = -1;

    /* Init RmSysctrl */
    ret = RmSysctrlInit(&rm_sysctrl);
    if (ret < 0)
    {
        RLOGE("RmSysctrl init failed!");
        return -1;
    }

    /* Start timer thread */
    RmSysctrlStartTimerThread(rm_sysctrl);

    /* Start cli thread */
    RmSysctrlStartCliThread(rm_sysctrl);

#ifdef DEEP_SLEEP_SUPPORT
    /* Start Gpio thread */
    RmSysctrlStartGpioThread(rm_sysctrl);
#endif

    /* Start Srv thread */
    RmSysctrlStartSrvThread(rm_sysctrl);

    /* Start Uevent thread */
    RmSysctrlStartUeventThread(rm_sysctrl);

    /* Power on all sub-systems */
    RLOGI("ToCan: Send Power on all systems to Rp.");
    RmSysctrlCommSendPoweronCmdToCan(rm_sysctrl->comm,
        MIDDLE, RP_SYSCTRL, 0x80, 0x00);

    while(1)
    {
        RmSysctrlCommRecvMsg(rm_sysctrl->comm,
                             rm_sysctrl->ibuf,
                             BUF_MAX_SIZE);
        struct RmSysctrlMessage *rscm =
            (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
        RmSysctrlFsmEvent(rm_sysctrl, rscm->mtype);
    }
    return 0;
}
