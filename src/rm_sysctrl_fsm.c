/*************************************************************************
FileName: rm_sysctrl_fsm.c
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
#include <malloc.h>
#include <semaphore.h>

#define LOG_TAG "RM_SYSCTRL_FSM"
#include "cutils/log.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#include "rm_sysctrl_can.h"
#ifdef DEEP_SLEEP_SUPPORT
#include "rm_sysctrl_gpio.h"
#endif
#include "rm_sysctrl_vbdev.h"
#include "rm_sysctrl_conf.h"
#include "rm_sysctrl_service.h"

extern bool isvalidSN(char * i_cpuid, char * i_sn);

char *invalid_str = "Invalid";

char *rm_sysctrl_fsm_state_str[] =
{
    "Compute",				//ROBOT_STATUS_COMPUTE
    "Init",				//ROBOT_STATUS_INIT
    "Register",				//ROBOT_STATUS_REGISTER
    "Loginning",			//ROBOT_STATUS_LOGINNING
    "Not Login",			//ROBOT_STATUS_NOT_LOGIN
    "Work",				//ROBOT_STATUS_WORK
    "Sleep",				//ROBOT_STATUS_SLEEP
    "Charge",				//ROBOT_STATUS_CHARGE
    "Abnormal",				//ROBOT_STATUS_ABNORMAL
    "Halt",				//ROBOT_STATUS_HALT
    "Security",				//ROBOT_STATUS_SECURITY
    "Upgrade",				//ROBOT_STATUS_UPGRADE
};

bool RmSysctrlIsValidState(int state)
{
    bool is_valid = false;

    switch (state)
    {
    case ROBOT_STATUS_COMPUTE:
    case ROBOT_STATUS_INIT:
    case ROBOT_STATUS_REGISTER:
    case ROBOT_STATUS_LOGINNING:
    case ROBOT_STATUS_NOT_LOGIN:
    case ROBOT_STATUS_WORK:
    case ROBOT_STATUS_SLEEP:
    case ROBOT_STATUS_CHARGE:
    case ROBOT_STATUS_ABNORMAL:
    case ROBOT_STATUS_HALT:
    case ROBOT_STATUS_SECURITY:
    case ROBOT_STATUS_UPGRADE:
        is_valid = true;
        break;
    default:
        is_valid = false;
    }

    return is_valid;
}

char* RmSysctrlGetStateStr(int state)
{
    if (RmSysctrlIsValidState(state))
        return rm_sysctrl_fsm_state_str[state];
    else
        return invalid_str;
}

char *rm_sysctrl_fsm_work_substate_str[] =
{
    "Idle",				//WORK_IDLE
    "Busy",				//WORK_BUSY
};

bool RmSysctrlIsValidWorkSubState(int sub_state)
{
    bool is_valid = false;

    switch (sub_state)
    {
    case WORK_IDLE:
    case WORK_BUSY:
        is_valid = true;
        break;
    default:
        is_valid = false;
    }

    return is_valid;
}

char* RmSysctrlGetWorkSubStateStr(int sub_state)
{
    if (RmSysctrlIsValidWorkSubState(sub_state))
        return rm_sysctrl_fsm_work_substate_str[sub_state];
    else
        return invalid_str;
}

char *rm_sysctrl_fsm_sleep_substate_str[] =
{
    "Shallow",				//SLEEP_SHALLOW
#ifdef DEEP_SLEEP_SUPPORT
    "Deep",				//SLEEP_DEEP
#endif
};

bool RmSysctrlIsValidSleepSubState(int sub_state)
{
    bool is_valid = false;

    switch (sub_state)
    {
    case SLEEP_SHALLOW:
#ifdef DEEP_SLEEP_SUPPORT
    case SLEEP_DEEP:
#endif
        is_valid = true;
        break;
    default:
        is_valid = false;
    }

    return is_valid;
}

char* RmSysctrlGetSleepSubStateStr(int sub_state)
{
    if (RmSysctrlIsValidSleepSubState(sub_state))
        return rm_sysctrl_fsm_sleep_substate_str[sub_state];
    else
        return invalid_str;
}

char *rm_sysctrl_fsm_subsystem_str[] =
{
    "Rm",				//RM_SYSTEM_ID
    "Rc",				//RC_SYSTEM_ID
    "Rp",				//RP_SYSTEM_ID
    "Rf",				//RF_SYSTEM_ID
    "Rb right arm",			//RB_R_ARM_ID
    "Rb left arm",			//RB_L_ARM_ID
    "Rb body",				//RB_BODY_ID
    "No",				//R_RC_ID
};

bool RmSysctrlIsValidSubSystem(int sys_id)
{
    bool is_valid = false;

    switch (sys_id)
    {
    case RM_SYSTEM_ID:
    case RC_SYSTEM_ID:
    case RP_SYSTEM_ID:
    case RF_SYSTEM_ID:
    case RB_R_ARM_ID:
    case RB_L_ARM_ID:
    case RB_BODY_ID:
    case R_RC_ID:
        is_valid = true;
        break;
    default:
        is_valid = false;
    }

    return is_valid;
}

char* RmSysctrlGetSubSystemStr(int sys_id)
{
    if (RmSysctrlIsValidSubSystem(sys_id))
        return rm_sysctrl_fsm_subsystem_str[sys_id];
    else
        return invalid_str;
}

char *rm_sysctrl_fsm_mode_str[] =
{
    "Developer mode",			//DEVELOPER_MODE
    "Children mode",			//CHILDREN_MODE
    "Guarder mode",			//GUARDER_MODE
};

bool RmSysctrlIsValidMode(int mode)
{
    bool is_valid = false;

    switch (mode)
    {
    case DEVELOPER_MODE:
    case CHILDREN_MODE:
    case GUARDER_MODE:
        is_valid = true;
        break;
    default:
        is_valid = false;
    }

    return is_valid;
}

char* RmSysctrlGetModeStr(int mode)
{
    if (RmSysctrlIsValidMode(mode))
        return rm_sysctrl_fsm_mode_str[mode];
    else
        return invalid_str;
}

static char *rm_sysctrl_fsm_event_str[] =
{
    "Sub-system dead",			//INTERNAL_SYSCTRL_SUBSYS_DEAD_MSG
    "Timer expire",			//INTERNAL_SYSCTRL_TIMER_EXPIRE_MSG
#ifdef DEEP_SLEEP_SUPPORT
    "S_poweron change",			//INTERNAL_SYSCTRL_S_POWERON_CHANGE_MSG
#endif
    "Init command response",		//COMMON_SYSCTRL_INIT_CMD_REP_MSG
    "Sleep command",			//COMMON_SYSCTRL_SLEEP_CMD_MSG
    "Sleep command response",		//COMMON_SYSCTRL_SLEEP_CMD_REP_MSG
    "Wakeup command",			//COMMON_SYSCTRL_WAKE_CMD_MSG
    "Wakeup command response",		//COMMON_SYSCTRL_WAKE_CMD_REP_MSG
    "Security command",			//COMMON_SYSCTRL_SECURITY_CMD_MSG
    "Security command response",	//COMMON_SYSCTRL_SECURITY_CMD_REP_MSG
    "Exit security command",		//COMMON_SYSCTRL_EXIT_SECURITY_CMD_MSG
    "Exit security command response",	//COMMON_SYSCTRL_EXIT_SECURITY_CMD_REP_MSG
    "Halt command",			//COMMON_SYSCTRL_HALT_CMD_MSG
    "Halt command response",		//COMMON_SYSCTRL_HALT_CMD_REP_MSG
    "Debug command",			//COMMON_SYSCTRL_DEBUG_CMD_MSG
    "Debug command response",		//COMMON_SYSCTRL_DEBUG_CMD_REP_MSG
    "Mode command",			//COMMON_SYSCTRL_MODE_CMD_MSG
    "Mode command response",		//COMMON_SYSCTRL_MODE_CMD_REP_MSG
    "Abnormal events occur",		//COMMON_SYSCTRL_ABNORMAL_OCCUR_NOTI_MSG
    "Abnormal events resolved",		//COMMON_SYSCTRL_ABNORMAL_RESOLVE_NOTI_MSG
    "Low power",			//COMMON_SYSCTRL_LOW_POWER_NOTI_MSG
    "Power charge begin",		//COMMON_SYSCTRL_POWER_CHARGE_NOTI_MSG
    "Power charge end",			//COMMON_SYSCTRL_POWER_CHARGE_END_NOTI_MSG
    "Sub-system state change",		//COMMON_SYSCTRL_SUBSYS_STATE_CHANGE_NOTI_MSG
    "Sub-system state query response",	//COMMON_SYSCTRL_SUBSYS_STATE_QUERY_REP_MSG
    "version query response",		//COMMON_SYSCTRL_VERSION_QUERY_REP_MSG
    "Upgrade command",			//COMMON_SYSCTRL_UPGRADE_CMD_MSG
    "Upgrade command response",		//COMMON_SYSCTRL_UPGRADE_CMD_REP_MSG
    "Exit upgrade command",		//COMMON_SYSCTRL_EXIT_UPGRADE_CMD_MSG
    "Exit upgrade command response",	//COMMON_SYSCTRL_EXIT_UPGRADE_CMD_REP_MSG
    "Not low power",			//COMMON_SYSCTRL_NOT_LOW_POWER_NOTI_MSG
    "Register command response",	//RM_SYSCTRL_REGISTER_CMD_REP_MSG
    "Login command response",		//RM_SYSCTRL_LOGIN_CMD_REP_MSG
    "Restore factory command",		//RM_SYSCTRL_RESTORE_FACTORY_CMD_MSG
    "Registered notification",		//RM_SYSCTRL_REGISTERED_NOTI_MSG
    "Login notification",		//RM_SYSCTRL_LOGIN_NOTI_MSG
    "Battery query command",		//RM_SYSCTRL_BATTERY_QUERY_MSG
    "Register query response",		//RM_SYSCTRL_REGISTERED_QUERY_REP_MSG
    "Login query response",		//RM_SYSCTRL_LOGIN_QUERY_REP_MSG
    "Power off command response",	//RP_SYSCTRL_POWER_OFF_CMD_REP_MSG
    "Power on cammand response",	//RP_SYSCTRL_POWER_ON_CMD_REP_MSG
    "Battery notification",		//RP_SYSCTRL_BATTERY_NOTI_MSG
    "Power off notification",		//RP_SYSCTRL_POWER_OFF_NOTI_MSG
    "Power on notification",		//RP_SYSCTRL_POWER_ON_NOTI_MSG
    "Battery status query response",	//RP_SYSCTRL_BATTERY_STATUS_QUERY_REP_MSG
    "Get CPU ID command response",	//RP_SYSCTRL_GET_CPU_ID_CMD_REP_MSG
    "Save Robot SN command response",	//RP_SYSCTRL_SAVE_ROBOT_SN_CMD_REP_MSG
    "Read Robot SN command response",	//RP_SYSCTRL_READ_ROBOT_SN_CMD_REP_MSG
    "Datetime query response",		//RP_SYSCTRL_DATETIME_QUERY_REP_MSG
    "Set Datetime command response",	//RP_SYSCTRL_SET_DATETIME_CMD_REP_MSG
    "Sex query response",		//RP_SYSCTRL_SEX_QUERY_REP_MSG
    "Set Sex command response",		//RP_SYSCTRL_SET_SEX_CMD_REP_MSG
    "Uptime query response",		//RP_SYSCTRL_UPTIME_QUERY_REP_MSG
    "Wake notification",		//RF_SYSCTRL_WAKE_NOTI_MSG
};


int RmSysctrlFsmInit(struct RmSysctrl *rm_sysctrl)
{
    if (!rm_sysctrl)
    {
        RLOGD("Invalid prameter!");
        return -1;
    }

    SET_STATE(rm_sysctrl, ROBOT_STATUS_INIT);
    return 0;
}


bool is_all_subsys_init(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    bool is_all_init = true;
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if (!IS_SUBSYS_INITALIZED(&rm_sysctrl->sub_system[i]))
            {
                is_all_init = false;
                break;
            }                    
        }
    }
    return is_all_init;
}

bool is_all_subsys_uninit(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    bool is_all_uninit = true;
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if (IS_SUBSYS_INITALIZED(&rm_sysctrl->sub_system[i]))
            {
                is_all_uninit = false;
                break;
            }
        }
    }
    return is_all_uninit;
}

void clear_all_subsys_init(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;

    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            RESET_SUBSYS_INITALIZED(&rm_sysctrl->sub_system[i]);
        }
    }
}

bool is_all_subsys_work_idle(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    bool is_all_work_idle = true;
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if (!CHECK_SUBSTATE(&rm_sysctrl->sub_system[i], WORK_IDLE))
            {
                is_all_work_idle = false;
                break;
            }
        }
    }
    return is_all_work_idle;
}

bool is_all_subsys_active(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    bool is_all_active = true;
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if (!IS_SUBSYSTEM_ACTIVE(&rm_sysctrl->sub_system[i]))
            {
                is_all_active = false;
                break;
            }                    
        }
    }
    return is_all_active;
}

bool is_all_subsys_at_state(struct RmSysctrl *rm_sysctrl, unsigned int state)
{
    int i = 0;
    bool is_all_at_this_state = true;
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if ((&rm_sysctrl->sub_system[i])->state != state)
            {
                is_all_at_this_state = false;
                break;
            }                    
        }
    }
    return is_all_at_this_state;
}

bool is_all_subsys_work(struct RmSysctrl *rm_sysctrl)
{
    return is_all_subsys_at_state(rm_sysctrl, ROBOT_STATUS_WORK);
}

bool is_all_subsys_charge(struct RmSysctrl *rm_sysctrl)
{
    return is_all_subsys_at_state(rm_sysctrl, ROBOT_STATUS_CHARGE);
}

bool is_all_subsys_security(struct RmSysctrl *rm_sysctrl)
{
    return is_all_subsys_at_state(rm_sysctrl, ROBOT_STATUS_SECURITY);
}

bool is_all_subsys_upgrade(struct RmSysctrl *rm_sysctrl)
{
    return is_all_subsys_at_state(rm_sysctrl, ROBOT_STATUS_UPGRADE);
}

bool is_all_subsys_sleep(struct RmSysctrl *rm_sysctrl)
{
    return is_all_subsys_at_state(rm_sysctrl, ROBOT_STATUS_SLEEP);
}

bool is_all_subsys_halt(struct RmSysctrl *rm_sysctrl)
{
    return is_all_subsys_at_state(rm_sysctrl, ROBOT_STATUS_HALT);
}

bool is_all_subsys_into_expect_state(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    bool is_all_into_expect_state = true;
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if ((&rm_sysctrl->sub_system[i])->state != rm_sysctrl->expect_state)
            {
                is_all_into_expect_state = false;
                break;
            }                    
        }
    }
    return is_all_into_expect_state;
}

bool is_all_subsys_work_except_rp_rm(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    bool is_all_subsys_at_work_except_rp_rm = true;
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (i == RP_SYSTEM_ID || i == RM_SYSTEM_ID) continue;

        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if ((&rm_sysctrl->sub_system[i])->state != ROBOT_STATUS_WORK)
            {
                is_all_subsys_at_work_except_rp_rm = false;
                break;
            }
        }
    }
    return is_all_subsys_at_work_except_rp_rm;
}

bool is_rp_subsys_enable(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlSubSystem *rp_subsys =
        &rm_sysctrl->sub_system[RP_SYSTEM_ID];

    return IS_SUBSYSTEM_ENABLE(rp_subsys);
}

bool is_rp_subsys_charge(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlSubSystem *rp_subsys =
        &rm_sysctrl->sub_system[RP_SYSTEM_ID];

    if (rp_subsys->state != ROBOT_STATUS_CHARGE)
        return false;
    else
        return true;
}

bool is_rp_subsys_work(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlSubSystem *rp_subsys =
        &rm_sysctrl->sub_system[RP_SYSTEM_ID];

    if (rp_subsys->state != ROBOT_STATUS_WORK)
        return false;
    else
        return true;
}

bool is_rm_subsys_enable(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlSubSystem *rm_subsys =
        &rm_sysctrl->sub_system[RM_SYSTEM_ID];

    return IS_SUBSYSTEM_ENABLE(rm_subsys);
}

bool is_rm_subsys_charge(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlSubSystem *rm_subsys =
        &rm_sysctrl->sub_system[RM_SYSTEM_ID];

    if (rm_subsys->state != ROBOT_STATUS_CHARGE)
        return false;
    else
        return true;
}

bool is_rm_subsys_work(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlSubSystem *rm_subsys =
        &rm_sysctrl->sub_system[RM_SYSTEM_ID];

    if (rm_subsys->state != ROBOT_STATUS_WORK)
        return false;
    else
        return true;
}

/* This function consider the power on system with charging */
bool may_tmp_into_work_state(struct RmSysctrl *rm_sysctrl)
{
    if (is_all_subsys_work_except_rp_rm(rm_sysctrl))
    {
        if (is_rp_subsys_enable(rm_sysctrl) &&
            is_rm_subsys_enable(rm_sysctrl))
        {
            if ((is_rp_subsys_charge(rm_sysctrl) ||
                 is_rp_subsys_work(rm_sysctrl)) &&
                (is_rm_subsys_charge(rm_sysctrl) ||
                 is_rm_subsys_work(rm_sysctrl)))
            {
                return true;
            }
        }
        else if (is_rp_subsys_enable(rm_sysctrl) &&
                !is_rm_subsys_enable(rm_sysctrl))
        {
            if (is_rp_subsys_charge(rm_sysctrl) ||
                is_rp_subsys_work(rm_sysctrl))
            {
                return true;
            }
        }
        else if (!is_rp_subsys_enable(rm_sysctrl) &&
                 is_rm_subsys_enable(rm_sysctrl))
        {
            if (is_rm_subsys_charge(rm_sysctrl) ||
                is_rm_subsys_work(rm_sysctrl))
            {
                return true;
            }
        }
        else if (!is_rp_subsys_enable(rm_sysctrl) &&
                 !is_rm_subsys_enable(rm_sysctrl))
        {
            return true;
        }
    }

    return false;
}

bool is_all_subsys_into_debug_set(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    bool is_all_into_debug_set = true;
    bool debug_en = false;

    RmSysctrlGetDebug(rm_sysctrl->config, &debug_en);
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if ((&rm_sysctrl->sub_system[i])->debug_en != debug_en)
            {
                is_all_into_debug_set = false;
                break;
            }
        }
    }
    return is_all_into_debug_set;
}

bool is_all_subsys_into_mode_set(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    bool is_all_into_mode_set = true;
    int sys_mode;

    RmSysctrlGetMode(rm_sysctrl->config, &sys_mode);
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if ((&rm_sysctrl->sub_system[i])->mode != (unsigned int)sys_mode)
            {
                is_all_into_mode_set = false;
                break;
            }
        }
    }
    return is_all_into_mode_set;
}

void set_all_subsys_poweron(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            RmSysctrlSetPowerOnSubSystem(&rm_sysctrl->sub_system[i]);
        }
    }
}

void set_part_subsys_poweron(struct RmSysctrl *rm_sysctrl, char poweron_subsys)
{
    int i = 0;
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]) &&
            (poweron_subsys & (1 << i)))
        {
            RmSysctrlSetPowerOnSubSystem(&rm_sysctrl->sub_system[i]);
        }
    }
}

#ifdef DEEP_SLEEP_SUPPORT
void set_all_subsys_deep_sleep(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        struct RmSysctrlSubSystem *subsys =
            &rm_sysctrl->sub_system[i];

        if (IS_SUBSYSTEM_ENABLE(subsys))
        {
            RmSysctrlSetSubSystemStatus(subsys, ROBOT_STATUS_SLEEP,
                SLEEP_DEEP);
        }
    }
}
#endif

bool can_not_sleep(struct RmSysctrl *rm_sysctrl)
{
    int i;

    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        struct RmSysctrlSubSystem *subsys =
            &rm_sysctrl->sub_system[i];

        if (i == RP_SYSTEM_ID)
            continue;

        if (IS_SUBSYSTEM_ENABLE(subsys))
        {
            if (!(subsys->state == ROBOT_STATUS_WORK) ||
                (subsys->state == ROBOT_STATUS_WORK &&
                subsys->sub_state == WORK_BUSY))
            {
                return true;
            }
        }
    }

    return false;
}

#ifdef AUTO_SLEEP_SUPPORT
void reset_auto_sleep_timer(struct RmSysctrl *rm_sysctrl)
{
    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_AUTO_SLEEP);
}
#endif


UNICAST_MSG_TO_SUBSYS(InitCmd)
UNICAST_MSG_TO_SUBSYS2(SleepCmd, int)
UNICAST_MSG_TO_SUBSYS(WakeCmd)
UNICAST_MSG_TO_SUBSYS(ChargeNoti)
UNICAST_MSG_TO_SUBSYS(ExitChargeNoti)
UNICAST_MSG_TO_SUBSYS(SecurityCmd)
UNICAST_MSG_TO_SUBSYS(ExitSecurityCmd)
UNICAST_MSG_TO_SUBSYS(UpgradeCmd)
UNICAST_MSG_TO_SUBSYS(ExitUpgradeCmd)
UNICAST_MSG_TO_SUBSYS(LowPowerNoti)
UNICAST_MSG_TO_SUBSYS(NotLowPowerNoti)
UNICAST_MSG_TO_SUBSYS(SubsysStateQuery)
UNICAST_MSG_TO_SUBSYS(HaltCmd)
UNICAST_MSG_TO_SUBSYS2(DebugCmd, unsigned int)
UNICAST_MSG_TO_SUBSYS2(ModeCmd, unsigned int)
UNICAST_MSG_TO_SUBSYS2(SysStateChangeNoti, unsigned int)
UNICAST_MSG_TO_SUBSYS2(HDMIStateChangeNoti, bool)

#ifdef UNICAST_ALL_INSTEAD_OF_MULTICAST_SUPPORT
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(InitCmd)
BROADCAST_MSG_TO_SUBSYS2_VIA_UNICAST_ALL(SleepCmd, char)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(WakeCmd)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(ChargeNoti)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(ExitChargeNoti)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(SecurityCmd)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(ExitSecurityCmd)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(UpgradeCmd)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(ExitUpgradeCmd)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(LowPowerNoti)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(NotLowPowerNoti)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(SubsysStateQuery)
BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(HaltCmd)
BROADCAST_MSG_TO_SUBSYS2_VIA_UNICAST_ALL(DebugCmd, unsigned int)
BROADCAST_MSG_TO_SUBSYS2_VIA_UNICAST_ALL(ModeCmd, unsigned int)
BROADCAST_MSG_TO_SUBSYS2_VIA_UNICAST_ALL(SysStateChangeNoti, unsigned int)
BROADCAST_MSG_TO_SUBSYS2_VIA_UNICAST_ALL(HDMIStateChangeNoti, bool)
#else
BROADCAST_MSG_TO_SUBSYS(InitCmd)
BROADCAST_MSG_TO_SUBSYS2(SleepCmd, char)
BROADCAST_MSG_TO_SUBSYS(WakeCmd)
BROADCAST_MSG_TO_SUBSYS(ChargeNoti)
BROADCAST_MSG_TO_SUBSYS(ExitChargeNoti)
BROADCAST_MSG_TO_SUBSYS(SecurityCmd)
BROADCAST_MSG_TO_SUBSYS(ExitSecurityCmd)
BROADCAST_MSG_TO_SUBSYS(UpgradeCmd)
BROADCAST_MSG_TO_SUBSYS(ExitUpgradeCmd)
BROADCAST_MSG_TO_SUBSYS(LowPowerNoti)
BROADCAST_MSG_TO_SUBSYS(NotLowPowerNoti)
BROADCAST_MSG_TO_SUBSYS(SubsysStateQuery)
BROADCAST_MSG_TO_SUBSYS(HaltCmd)
BROADCAST_MSG_TO_SUBSYS2(DebugCmd, unsigned int)
BROADCAST_MSG_TO_SUBSYS2(ModeCmd, unsigned int)
BROADCAST_MSG_TO_SUBSYS2(SysStateChangeNoti, unsigned int)
BROADCAST_MSG_TO_SUBSYS2(HDMIStateChangeNoti, bool)
#endif /* UNICAST_ALL_INSTEAD_OF_MULTICAST_SUPPORT */


void RestoreAllSubsysIntoWorkState(struct RmSysctrl *rm_sysctrl)
{
    if (rm_sysctrl->expect_state == ROBOT_STATUS_SLEEP)
    {
        /* Broadcast wakeup message into can */
        RmSysctrlBroadcastWakeCmdMsg(rm_sysctrl);

        /* Unicast wakeup message into sub-systems to avoid that
         * any sub-sustems donot receive the wakeup message by broadcasting.
         */
        usleep(200000);
        RmSysctrlUnicastWakeCmdMsg(rm_sysctrl);

        rm_sysctrl->is_sleeping = false;
    }
    else if (rm_sysctrl->expect_state == ROBOT_STATUS_CHARGE)
    {
        /* Broadcast exit power charge message into can */
        RmSysctrlBroadcastExitChargeNotiMsg(rm_sysctrl);

        /* Unicast exit charge message into sub-systems to avoid that
         * any sub-sustems donot receive the exit charge message by broadcasting.
         */
        usleep(200000);
        RmSysctrlUnicastExitChargeNotiMsg(rm_sysctrl);

        rm_sysctrl->is_charging = false;
    }
    else if (rm_sysctrl->expect_state == ROBOT_STATUS_SECURITY)
    {
        /* Broadcast exit security message into can */
        RmSysctrlBroadcastExitSecurityCmdMsg(rm_sysctrl);

        /* Unicast exit security message into sub-systems to avoid that
         * any sub-sustems donot receive the exit security message by broadcasting.
         */
        usleep(200000);
        RmSysctrlUnicastExitSecurityCmdMsg(rm_sysctrl);

        rm_sysctrl->is_security_ongoing = false;
    }
    else if (rm_sysctrl->expect_state == ROBOT_STATUS_UPGRADE)
    {
        /* Broadcast exit upgrade message into can */
        RmSysctrlBroadcastExitUpgradeCmdMsg(rm_sysctrl);

        /* Unicast exit upgrade message into sub-systems to avoid that
         * any sub-sustems donot receive the exit upgrade message by broadcasting.
         */
        usleep(200000);
        RmSysctrlUnicastExitUpgradeCmdMsg(rm_sysctrl);

        rm_sysctrl->is_upgrade_ongoing = false;
    }

    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);
    rm_sysctrl->expect_state = ROBOT_STATUS_WORK;
}

#define BUF_SIZE 1024

int GetPidByName(char* i_task_name, int *o_pid)
{
    DIR *dir;
    struct dirent *ptr;
    FILE *fp;
    char filepath[50];
    char cur_task_name[100];
    char buf[BUF_SIZE];

    dir = opendir("/proc");
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL)
        {
            /* skip "." and ".." */
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (DT_DIR != ptr->d_type)
                continue;

            /* Generate the file path */
            sprintf(filepath, "/proc/%s/status", ptr->d_name);
            fp = fopen(filepath, "r");
            if (NULL != fp)
            {
                if( fgets(buf, BUF_SIZE-1, fp)== NULL ) {
                    fclose(fp);
                    continue;
                }
                sscanf(buf, "%*s %s", cur_task_name);

                if (!strcmp(i_task_name, cur_task_name))
                    *o_pid = atoi(ptr->d_name);

                fclose(fp);
            }
        }

        closedir(dir);
    }

    return 0;
}

static bool is_state_switch_ongoing(struct RmSysctrl *rm_sysctrl)
{
    if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, TIMER_EXPECT_STATE))
        return true;
    else
        return false;
}

static void FsmReHandleMsg(int sig)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    struct RmSysctrlComm *comm = rm_sysctrl->comm;
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->obuf;
    int src_module_id = rscm->src_module_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;

    RLOGD("Add the message into queue.");
    RmSysctrlCommEnQueueMsgXX(comm, src_module_id, ptlv, ptlv->len);
    return;
}

static void FsmPostPoneHandleMsg(struct RmSysctrl *rm_sysctrl)
{
    memcpy(rm_sysctrl->obuf, rm_sysctrl->ibuf, BUF_MAX_SIZE);
    signal(SIGALRM, FsmReHandleMsg);
    alarm(1);
    RLOGD("Alarm 1 seconds.");
}

#define CHECK_POSTPONE_MESSAGE \
    do \
    {\
        if (is_state_switch_ongoing(rm_sysctrl))\
        {\
            RLOGD("The system is busy, postpone to handle this message!");\
            FsmPostPoneHandleMsg(rm_sysctrl);\
            return 0;\
        }\
    }\
    while (0)

/*Init*/
static void resend_init_message_to_subsys(struct RmSysctrl *rm_sysctrl,
                                          int sub_sys)
{
    if (sub_sys > SUB_SYSTEM_ID_MAX || sub_sys < 0)
    {
        RLOGE("Invalid sub-system ID 0x%02x", sub_sys);
        return;
    }

    if (sub_sys == 0)
    {
        RLOGI("ToCan: Resend the init message to the"
              " sub-system RmScheduler %02x", RM_SCHEDULER);
        RmSysctrlCommSendInitCmdToCan(rm_sysctrl->comm,
            MIDDLE, RM_SCHEDULER);
    }
    else
    {
        RLOGI("ToCan: Resend the init message to the"
               " sub-system %02x", sub_sys << 4);
        RmSysctrlCommSendInitCmdToCan(rm_sysctrl->comm,
            MIDDLE, sub_sys << 4);
    }
}

static void resend_init_message_to_uninit_subsys(struct RmSysctrl *rm_sysctrl)
{
    /* Resend the init message to the sub-system */
    int i = 0;

    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if (!IS_SUBSYS_INITALIZED(&rm_sysctrl->sub_system[i]))
            {
                resend_init_message_to_subsys(rm_sysctrl, i);
            }
        }
    }
}

static void restart_uninit_subsys(struct RmSysctrl *rm_sysctrl)
{
    /* Restart the uninit sub-system */
    int i = 0;

    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            if (!IS_SUBSYS_INITALIZED(&rm_sysctrl->sub_system[i]))
            {
                /*
                 *Maybe these sub-systems is not in the init state, we will
                 *restart these sub-systems.
                 */
                if (i == RM_SYSTEM_ID)
                {
                    int pid = 0;

                    GetPidByName("system_server", &pid);
                    RLOGD("The pid of system_server process is %d", pid);
                    if (pid)
                    {
                        ALOGE("Kill system_server to restart rm sub-system.");
                        kill(pid, SIGKILL);
                    }

                    /* To restore all sub-system into work state */
                    RLOGI("The sub-system is dead, so to restore all sub-systems");
                    RLOGI(" into work state for restarting the sub-system.");
                    RestoreAllSubsysIntoWorkState(rm_sysctrl);
                }
                else
                {
                    /* Send Power off to not in init state sub-system to Rp */
                    RLOGI("ToCan: Send Power off to not in init state"
                          " sub-system(0x%02x) to Rp.", i);
                    RmSysctrlCommSendPoweroffCmdToCan(rm_sysctrl->comm,
                            MIDDLE, RP_SYSCTRL, 0x00, 1 << i);
                    RLOGI("To sleep 10s to wait the not in init state"
                          " sub-system power off.");
                    sleep(10);
                    /* Send Power on to not in init state sub-system to Rp */
                    RLOGI("ToCan: Send Power on to not in init state"
                          " sub-system(0x%02x) to Rp.", i);
                    RmSysctrlCommSendPoweronCmdToCan(rm_sysctrl->comm,
                            MIDDLE, RP_SYSCTRL, 0x00, 1 << i);
                    RLOGI("To sleep 100ms to wait the not in init state"
                          " sub-system power on.");
                    usleep(100000);
                }
                usleep(100000);
            }
        }
    }
}

void RmSysctrlUnicastLastMsgForExpectState(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;

    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        struct RmSysctrlSubSystem *subsys =
            &rm_sysctrl->sub_system[i];

        if (IS_SUBSYSTEM_ENABLE(subsys) &&
            subsys->state != rm_sysctrl->expect_state)
        {
            char dst_module_id;
            if (i == 0)
                dst_module_id = RM_SCHEDULER;
            else
                dst_module_id = i << 4;

            switch (rm_sysctrl->may_retry_msg)
            {
                case COMMON_SYSCTRL_INIT_CMD:
                {
                    /* Unicast init sub-system message into can */
                    RLOGI("ToCan: Unicast init sub-system message.");
                    RmSysctrlCommSendInitCmdToCan(rm_sysctrl->comm,
                        HIGH, dst_module_id);
                    break;
                }
                case COMMON_SYSCTRL_SLEEP_CMD:
                {
                    if (rm_sysctrl->is_sleeping)
                    {
                        /* Unicast sleep command into can */
                        RLOGI("ToCan: Unicast sleep command message.");
                        RmSysctrlCommSendSleepCmdToCan(rm_sysctrl->comm,
                           HIGH, dst_module_id, SLEEP_SHALLOW);
                    }
#ifdef DEEP_SLEEP_SUPPORT
                    else if (rm_sysctrl->is_sleeping_deep)
                    {
                        /* Unicast deep sleep command into can */
                        RLOGI("ToCan: Unicast deep sleep command message.");
                        RmSysctrlCommSendSleepCmdToCan(rm_sysctrl->comm,
                           HIGH, dst_module_id, SLEEP_DEEP);
                    }
#endif
                    break;
                }
                case COMMON_SYSCTRL_WAKE_CMD:
                {
                    /* Unicast wakeup message into can */
                    RLOGI("ToCan: Unicast wakeup message.");
                    RmSysctrlCommSendWakeCmdToCan(rm_sysctrl->comm,
                        HIGH, dst_module_id);
                    break;
                }
                case COMMON_SYSCTRL_SECURITY_CMD:
                {
                    /* Unicast security message into can */
                    RLOGI("ToCan: Unicast security message.");
                    RmSysctrlCommSendSecurityCmdToCan(rm_sysctrl->comm,
                        HIGH, dst_module_id);
                    break;
                }
                case COMMON_SYSCTRL_EXIT_SECURITY_CMD:
                {
                    /* Unicast exit security message into can */
                    RLOGI("ToCan: Unicast exit security message.");
                    RmSysctrlCommSendExitSecurityCmdToCan(rm_sysctrl->comm,
                        HIGH, dst_module_id);
                    break;
                }
                case COMMON_SYSCTRL_UPGRADE_CMD:
                {
                    /* Unicast upgrade message into can */
                    RLOGI("ToCan: Unicast upgrade message.");
                    RmSysctrlCommSendUpgradeCmdToCan(rm_sysctrl->comm,
                        HIGH, dst_module_id);
                    break;
                }
                case COMMON_SYSCTRL_EXIT_UPGRADE_CMD:
                {
                    /* Unicast exit upgrade message into can */
                    RLOGI("ToCan: Unicast exit upgrade message.");
                    RmSysctrlCommSendExitUpgradeCmdToCan(rm_sysctrl->comm,
                        HIGH, dst_module_id);
                    break;
                }
                case COMMON_SYSCTRL_POWER_CHARGE_NOTI:
                {
                    /* Unicast power charge message into can */
                    RLOGI("ToCan: Unicast power charge message.");
                    RmSysctrlCommSendChargeNotiToCan(rm_sysctrl->comm,
                        HIGH, dst_module_id);
                    usleep(100000);
                    /* Unicast query sub-system state message */
                    RLOGI("ToSubsystem: query sub-system state message.");
                    RmSysctrlCommSendSubsysStateQueryToCan(rm_sysctrl->comm,
                        MIDDLE, dst_module_id);
                    break;
                }
                case COMMON_SYSCTRL_POWER_CHARGE_END_NOTI:
                {
                    /* Unicast exit power charge message into can */
                    RLOGI("ToCan: Unicast exit power charge message.");
                    RmSysctrlCommSendExitChargeNotiToCan(rm_sysctrl->comm,
                        HIGH, dst_module_id);
                    usleep(100000);
                    /* Unicast query sub-system state message */
                    RLOGI("ToSubsystem: query sub-system state message.");
                    RmSysctrlCommSendSubsysStateQueryToCan(rm_sysctrl->comm,
                        MIDDLE, dst_module_id);

                    break;
                }
                case COMMON_SYSCTRL_HALT_CMD:
                {
                    /* Unicast halt message into can */
                    RLOGI("ToCan: Unicast halt message.");
                    RmSysctrlCommSendHaltCmdToCan(rm_sysctrl->comm,
                        HIGH, dst_module_id);
                    break;
                }
                case COMMON_SYSCTRL_SUBSYS_STATE_QUERY:
                {
                    /* Unicast query sub-system state message into can */
                    RLOGI("ToCan: Unicast query sub-system state message.");
                    RmSysctrlCommSendSubsysStateQueryToCan(rm_sysctrl->comm,
                        HIGH, dst_module_id);
                    break;
                }
                default:
                {
                    RLOGE("Invalid message!");
                    return;
                }
            }

            usleep(100000);
        }
    }
}

void RmSysctrlUnicastLastMsgForDebugSet(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    bool debug_en = false;

    RmSysctrlGetDebug(rm_sysctrl->config, &debug_en);
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        struct RmSysctrlSubSystem *subsys =
            &rm_sysctrl->sub_system[i];

        if (IS_SUBSYSTEM_ENABLE(subsys) &&
            subsys->debug_en != debug_en)
        {
            char dst_module_id;
            if (i == 0)
                dst_module_id = RM_SCHEDULER;
            else
                dst_module_id = i << 4;

            /* Unicast debug message into can */
            RLOGI("ToCan: Unicast debug message.");
            RmSysctrlCommSendDebugCmdToCan(rm_sysctrl->comm,
                HIGH, dst_module_id, debug_en);

            usleep(100000);
        }
    }
}

void RmSysctrlUnicastLastMsgForModeSet(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;

    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        struct RmSysctrlSubSystem *subsys =
            &rm_sysctrl->sub_system[i];
        int sys_mode;

        RmSysctrlGetMode(rm_sysctrl->config, &sys_mode);

        if (IS_SUBSYSTEM_ENABLE(subsys) &&
            subsys->mode != (unsigned int)sys_mode)
        {
            char dst_module_id;
            if (i == 0)
                dst_module_id = RM_SCHEDULER;
            else
                dst_module_id = i << 4;

            /* Unicast mode message into can */
            RLOGI("ToCan: Unicast mode message.");
            RmSysctrlCommSendModeCmdToCan(rm_sysctrl->comm,
                HIGH, dst_module_id, sys_mode);

            usleep(100000);
        }
    }
}

int FsmInitSubsys(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmInitSubsys called");

    /* Broadcast init sub-system message into can */
    RmSysctrlBroadcastInitCmdMsg(rm_sysctrl);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_INIT_CMD;

    RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_INIT_SUBSYS);

    return 0;
}

int FsmInitCmdResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    char result = ptlv->value[0]; 

    RLOGD("FSM: FsmInitCmdResp called");

    /* Record init result into sub-system structure */
    if (SUCCESS == result)
    {
        SET_SUBSYS_INITALIZED(&rm_sysctrl->sub_system[src_system_id]);
        if (rscm->src_module_id == RM_SCHEDULER)
        {
            /* Send Register Query to RmScheduler */
            RLOGI("ToCan: Send Register Query to RmScheduler.");
            RmSysctrlCommSendRegQueryToCan(rm_sysctrl->comm,
                MIDDLE, RM_SCHEDULER);
        }
        if (src_system_id == RP_SYSTEM_ID)
        {
            /* Update battery status
             * Send Battery Status query to Rp
             */
            RLOGI("ToCan: Send Battery Status Query to Rp.");
            RmSysctrlCommSendBatteryStatusQueryToCan(rm_sysctrl->comm,
                    MIDDLE, RP_SYSCTRL);
        }
    }


    return 0;
}

/*Register*/
int FsmRegisterQueryResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int is_registered = ptlv->value[0];

    RLOGD("FSM: FsmRegisterQueryResp called");

    if (is_registered == 1)
    {
        rm_sysctrl->is_registered = true;
    }
    else
    {
        rm_sysctrl->is_registered = false;
        /* Send register command to RmScheduler */
        RLOGI("ToCan: Send Register Command to RmScheduler.");
        RmSysctrlCommSendRegCmdToCan(rm_sysctrl->comm,
                MIDDLE, RM_SCHEDULER);
    }

    return 0;
}

int FsmRegisterResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmRegisterResp called");

    if (result == SUCCESS)
    {
        rm_sysctrl->is_registered = true;
    }
    else
    {
        rm_sysctrl->is_registered = false;
    }

    return 0;
}

int FsmRecordRegistered(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int is_registered = ptlv->value[0]; 

    RLOGD("FSM: FsmRecordRegistered called");

    if (is_registered == 1)
    {
        rm_sysctrl->is_registered = true;
    }
    else
    {
        rm_sysctrl->is_registered = false;
    }

    return 0;
}

/*Login*/
int FsmLoginQueryResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int is_login = ptlv->value[0];

    RLOGD("FSM: FsmLoginQueryResp called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmLoginQueryResp");
        return 0;
    }

    if (is_login == 1)
    {
        rm_sysctrl->is_login = true;
        rm_sysctrl->expect_state = ROBOT_STATUS_WORK;
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);
    }
    else
    {
        rm_sysctrl->is_login = false;
        /* Send login command to RmScheduler */
        RLOGI("ToCan: Send Login Command to RmScheduler.");
        RmSysctrlCommSendLoginCmdToCan(rm_sysctrl->comm,
                MIDDLE, RM_SCHEDULER);
    }

    return 0;
}

int FsmLoginResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmLoginResp called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmLoginResp");
        return 0;
    }

    if (result == SUCCESS)
    {
        rm_sysctrl->is_login = true;
        rm_sysctrl->expect_state = ROBOT_STATUS_WORK;
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);
    }
    else
    {
        rm_sysctrl->is_login = false;
    }

    return 0;
}

int FsmRecordLogin(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int is_login = ptlv->value[0];

    RLOGD("FSM: FsmRecordLogin called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmRecordLogin");
        return 0;
    }

    if (is_login == 1)
    {
        rm_sysctrl->is_registered = true;
        rm_sysctrl->is_login = true;
        rm_sysctrl->expect_state = ROBOT_STATUS_WORK;
        RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);
    }
    else
    {
        rm_sysctrl->is_login = false;
    }

    return 0;
}

/*Sleep*/
void SleepShallow(struct RmSysctrl *rm_sysctrl)
{
    /* Broadcast sleep command into can */
    RmSysctrlBroadcastSleepCmdMsg(rm_sysctrl, SLEEP_SHALLOW);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_SLEEP_CMD;

    rm_sysctrl->expect_state = ROBOT_STATUS_SLEEP;
    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);

    rm_sysctrl->sub_state = SLEEP_SHALLOW;
    rm_sysctrl->is_sleeping = true;
}

#ifdef DEEP_SLEEP_SUPPORT
void SleepDeep(struct RmSysctrl *rm_sysctrl)
{
#ifndef DEEP_SLEEP_SUPPORT
    RLOGD("Deep sleep doesn't support according to the hardware.");
    return;
#endif
#if 0
    unsigned int value = 0;
    /* Check S_poweron = 1 */
    RmSysctrlGpioGetValue(rm_sysctrl->gpio, &value);
    if (value != 1)
    {
        RLOGE("S_poweron should be 1.");
        return -1;
    }
#endif
    /* Set S_powerdown = 0 */
    RLOGD("Deep sleep from RM, set S_powerdown = 0");
    RmSysctrlGpioSetValue(rm_sysctrl->gpio, 0);

    /* Wait S_poweron = 0 by starting a thread
     * to listen the S_poweron and timer
     */
    rm_sysctrl->is_sleeping_deep = true;
    rm_sysctrl->is_deep_wake_expired = false;
    rm_sysctrl->sub_state = SLEEP_DEEP;

    /* Broadcast deep sleep command into can */
    RmSysctrlBroadcastSleepCmdMsg(rm_sysctrl, SLEEP_DEEP);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_SLEEP_CMD;
}
#endif


int FsmSleep(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int sleep_level = ptlv->value[0];

    RLOGD("FSM: FsmSleep called");

    CHECK_POSTPONE_MESSAGE;

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmRecordLogin");
        return 0;
    }

    /* Fix bug that the heart beat message wakeup the sub-systems.
     * To wait 1ms to make the heart beat off.
     */
    RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_HEART_BEAT);
    usleep(1000);

    if (sleep_level == SLEEP_SHALLOW)
    {
        SleepShallow(rm_sysctrl);
    }
#ifdef DEEP_SLEEP_SUPPORT
    /* sleep_level == SLEEP_DEEP by hardware signal*/
    else if (sleep_level == SLEEP_DEEP)
    {
        SleepDeep(rm_sysctrl);
    }
#endif
    else
    {
        RLOGD("This sleep option doesn't support!");
    }

    return 0;
}

int FsmSleepResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmSleepResp called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmRecordLogin");
        return 0;
    }

    if (result == SUCCESS)
    {
        RLOGD("The sub-system sleep success, we think the sub-system into"
              " sleep state");
        RmSysctrlSetSubSystemStatus(&rm_sysctrl->sub_system[src_system_id],
                                    ROBOT_STATUS_SLEEP,
                                    SLEEP_SHALLOW);
    }

    return 0;
}

void SendWakeupMsgToCan(struct RmSysctrl *rm_sysctrl)
{
    /* Broadcast wakeup message into can */
    RmSysctrlBroadcastWakeCmdMsg(rm_sysctrl);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_WAKE_CMD;
}

void ShallowWakeup(struct RmSysctrl *rm_sysctrl)
{
    SendWakeupMsgToCan(rm_sysctrl);

    rm_sysctrl->expect_state = ROBOT_STATUS_WORK;
    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);

    rm_sysctrl->is_sleeping = false;
}

#ifdef DEEP_SLEEP_SUPPORT
void DeepWakeupFromRm(struct RmSysctrl *rm_sysctrl)
{
    /* Set S_powerdown = 1 */
    RLOGD("Deep wake from RM, set S_powerdown = 1");
    RmSysctrlGpioSetValue(rm_sysctrl->gpio, 1);

    /* Wait S_poweron = 1 by starting a thread
     * to listen the S_poweron and timer
     */
    rm_sysctrl->is_sleeping_deep = false;
    rm_sysctrl->is_deep_sleep_expired = false;
}

void DeepWakeupFromRf(struct RmSysctrl *rm_sysctrl)
{
    unsigned int value = 0;

    /* Check S_poweron = 0 */
    RmSysctrlGpioGetValue(rm_sysctrl->gpio, &value);
    if (value != 0)
    {
        RLOGE("S_poweron should be 0.");
        return;
    }
    /* Set S_powerdown = 1 */
    RLOGD("Deep wake from RF, set S_powerdown = 1");
    RmSysctrlGpioSetValue(rm_sysctrl->gpio, 1);

    /* Wait S_poweron = 1 by starting a thread
     * to listen the S_poweron and timer
     */
    rm_sysctrl->is_sleeping_deep = false;
    rm_sysctrl->is_deep_sleep_expired = false;
}
#endif

int FsmWakeup(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmWakeup called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmWakeup");
        return 0;
    }

    CHECK_POSTPONE_MESSAGE;

    if (rm_sysctrl->is_sleeping && rm_sysctrl->sub_state == SLEEP_SHALLOW)
    {
        ShallowWakeup(rm_sysctrl);
    }
#ifdef DEEP_SLEEP_SUPPORT
    else if (rm_sysctrl->is_sleeping_deep && rm_sysctrl->sub_state == SLEEP_DEEP)
    {
        DeepWakeupFromRm(rm_sysctrl);
    }
#endif

    return 0;
}

int FsmWakeNoti(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmWakeNoti called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmWakeNoti");
        return 0;
    }

#ifdef DEEP_SLEEP_SUPPORT
    if (rm_sysctrl->is_sleeping_deep && rm_sysctrl->sub_state == SLEEP_DEEP)
    {
        DeepWakeupFromRf(rm_sysctrl);
    }
    else
#endif
    {
        ShallowWakeup(rm_sysctrl);
        rm_sysctrl->is_security_ongoing = false;
    }

    return 0;
}

int FsmWakeupResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmWakeupResp called");

    if (result == SUCCESS)
    {
        RLOGD("The sub-system wakeup success, we think the sub-system"
              " into work state");
        RmSysctrlSetSubSystemStatus(&rm_sysctrl->sub_system[src_system_id],
                                    ROBOT_STATUS_WORK,
                                    0);
    }

    return 0;
}

/*Security*/
int FsmSecurity(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmSecurity called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmSecurity");
        return 0;
    }

    CHECK_POSTPONE_MESSAGE;

    /* Broadcast security message into can */
    RmSysctrlBroadcastSecurityCmdMsg(rm_sysctrl);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_SECURITY_CMD;

    rm_sysctrl->expect_state = ROBOT_STATUS_SECURITY;
    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);

    rm_sysctrl->is_security_ongoing = true;

    return 0;
}

int FsmSecurityResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmSecurityResp called");

    if (result == SUCCESS)
    {
        RLOGD("The sub-system security success, we think the sub-system into"
              " security state");
        RmSysctrlSetSubSystemStatus(&rm_sysctrl->sub_system[src_system_id],
                                    ROBOT_STATUS_SECURITY,
                                    0);
    }

    return 0;
}

int FsmExitSecurity(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmExitSecurity called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmExitSecurity");
        return 0;
    }

    CHECK_POSTPONE_MESSAGE;

    /* Broadcast exit security message into can */
    RmSysctrlBroadcastExitSecurityCmdMsg(rm_sysctrl);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_EXIT_SECURITY_CMD;

    rm_sysctrl->expect_state = ROBOT_STATUS_WORK;
    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);

    rm_sysctrl->is_security_ongoing = false;

    return 0;
}

int FsmExitSecurityResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmExitSecurityResp called");

    if (result == SUCCESS)
    {
        RLOGD("The sub-system exit security success, we think the sub-system"
              " into work state");
        RmSysctrlSetSubSystemStatus(&rm_sysctrl->sub_system[src_system_id],
                                    ROBOT_STATUS_WORK,
                                    0);
    }

    return 0;
}

/*Upgrade*/
int FsmUpgrade(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmUpgrade called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmUpgrade");
        return 0;
    }

    CHECK_POSTPONE_MESSAGE;

    /* Broadcast upgrade message into can */
    RmSysctrlBroadcastUpgradeCmdMsg(rm_sysctrl);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_UPGRADE_CMD;

    rm_sysctrl->expect_state = ROBOT_STATUS_UPGRADE;
    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);

    rm_sysctrl->is_upgrade_ongoing = true;

    return 0;
}

int FsmUpgradeResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmUpgradeResp called");

    if (result == SUCCESS)
    {
        RLOGD("The sub-system upgrade success, we think the sub-system into"
              " upgrade state");
        RmSysctrlSetSubSystemStatus(&rm_sysctrl->sub_system[src_system_id],
                                    ROBOT_STATUS_UPGRADE,
                                    0);
    }

    return 0;
}

int FsmExitUpgrade(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmExitUpgrade called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmExitUpgrade");
        return 0;
    }

    CHECK_POSTPONE_MESSAGE;

    /* Broadcast exit upgrade message into can */
    RmSysctrlBroadcastExitUpgradeCmdMsg(rm_sysctrl);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_EXIT_UPGRADE_CMD;

    rm_sysctrl->expect_state = ROBOT_STATUS_WORK;
    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);

    rm_sysctrl->is_upgrade_ongoing = false;

    return 0;
}

int FsmExitUpgradeResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmExitUpgradeResp called");

    if (result == SUCCESS)
    {
        RLOGD("The sub-system exit upgrade success, we think the sub-system"
              " into work state");
        RmSysctrlSetSubSystemStatus(&rm_sysctrl->sub_system[src_system_id],
                                    ROBOT_STATUS_WORK,
                                    0);
    }

    return 0;
}
/*Power*/
int FsmPowerOn(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    char poweron_upper = ptlv->value[0];

    RLOGD("FSM: FsmPowerOn called");

    if (poweron_upper & 0x80)
    {
        set_all_subsys_poweron(rm_sysctrl);
        if (rm_sysctrl->state == ROBOT_STATUS_INIT
            && !is_all_subsys_init(rm_sysctrl))
        {
            FsmInitSubsys(rm_sysctrl);
        }
    }
    else
    {
        char poweron_lower = ptlv->value[1];
        set_part_subsys_poweron(rm_sysctrl, poweron_lower);
    }

    return 0;
}

static int SetLowPower(struct RmSysctrl *rm_sysctrl)
{
    rm_sysctrl->is_low_power = true;

    /* Broadcast low power message into can */
    RmSysctrlBroadcastLowPowerNotiMsg(rm_sysctrl);

    /* Unicast low power message into sub-systems to avoid that
     * any sub-sustems donot receive the low power message by broadcasting.
     */
    usleep(200000);
    RmSysctrlUnicastLowPowerNotiMsg(rm_sysctrl);

    return 0;
}

static int ClearLowPower(struct RmSysctrl *rm_sysctrl)
{
    rm_sysctrl->is_low_power = false;

    /* Broadcast not low power message into can */
    RmSysctrlBroadcastNotLowPowerNotiMsg(rm_sysctrl);

    /* Unicast not low power message into sub-systems to avoid that
     * any sub-sustems donot receive the not low power message by broadcasting.
     */
    usleep(200000);
    RmSysctrlUnicastNotLowPowerNotiMsg(rm_sysctrl);

    return 0;
}

static bool IsLowPower(struct RmSysctrl *rm_sysctrl)
{
    return rm_sysctrl->is_low_power;
}

int FsmLowPower(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmLowPower called");

    SetLowPower(rm_sysctrl);

    return 0;
}

int FsmNotLowPower(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmNotLowPower called");

    ClearLowPower(rm_sysctrl);

    return 0;
}

int FsmPowerCharge(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmPowerCharge called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmPowerCharge");
        return 0;
    }

    /* Broadcast power charge message into can */
    RmSysctrlBroadcastChargeNotiMsg(rm_sysctrl);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_POWER_CHARGE_NOTI;

    rm_sysctrl->expect_state = ROBOT_STATUS_CHARGE;
    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);

    rm_sysctrl->is_charging = true;

    usleep(100000);
    /* Broadcast query sub-system state message into can */
    RmSysctrlBroadcastSubsysStateQueryMsg(rm_sysctrl);

    return 0;
}

int FsmPowerChargeX(struct RmSysctrl *rm_sysctrl)
{
    CHECK_POSTPONE_MESSAGE; 
    return FsmPowerCharge(rm_sysctrl);
}

int FsmPowerExitCharge(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmPowerExitCharge called");

    if (rm_sysctrl->is_halt_received)
    {
        RLOGD("Current the halt progress is ongoing,"
              " Ignore the FsmPowerExitCharge");
        return 0;
    }

    /* Broadcast exit power charge message into can */
    RmSysctrlBroadcastExitChargeNotiMsg(rm_sysctrl);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_POWER_CHARGE_END_NOTI;

    rm_sysctrl->expect_state = ROBOT_STATUS_WORK;
    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);

    rm_sysctrl->is_charging = false;

    usleep(100000);
    /* Broadcast query sub-system state message into can */
    RmSysctrlBroadcastSubsysStateQueryMsg(rm_sysctrl);

    return 0;
}

int FsmPowerExitChargeX(struct RmSysctrl *rm_sysctrl)
{
    CHECK_POSTPONE_MESSAGE;
    return FsmPowerExitCharge(rm_sysctrl);
}

int FsmBatteryQuery(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmBatteryQuery called");

    /* Send battery power quantity to RmPower */
    RLOGI("ToCan: Send battery power quantity to RmPower.");
    RmSysctrlCommSendBatteryQueryRespToCan(rm_sysctrl->comm,
            MIDDLE, RM_POWER, rm_sysctrl->battery_capacity);

    return 0;
}

#define BATTERY_SUPPLY_PATH "/dev/power_supply/battery"
#define AC_SUPPLY_PATH      "/dev/power_supply/ac"

static char *type_text[] = {
    "Unknown", "Battery", "UPS", "Mains", "USB",
    "USB_DCP", "USB_CDP", "USB_ACA", "WIRELESS"
};
static char *status_text[] = {
    "Unknown", "Charging", "Discharging", "Not charging", "Full"
};
static char *charge_type[] = {
    "Unknown", "N/A", "Trickle", "Fast"
};
static char *health_text[] = {
    "Unknown", "Good", "Overheat", "Dead", "Over voltage",
    "Unspecified failure", "Cold",
};
static char *technology_text[] = {
    "Unknown", "NiMH", "Li-ion", "Li-poly", "LiFe", "NiCd",
    "LiMn"
};
static char *capacity_level_text[] = {
    "Unknown", "Critical", "Low", "Normal", "High", "Full"
};

void create_path(char *pathfile)
{
  unsigned int i = 0;
  char path[200] = {0};
  strcpy(path, pathfile);

  while (i < strlen(path))
  {
     if (path[i] == '/' && i != 0)
     {
       path[i] = '\0';
       if(access(path,0)==-1)
       {
         if (mkdir(path,0777))
         {
          ALOGE("creat file bag failed!!!");
          return;
         }
       }
       path[i] = '/';
     }
     i++;
  }
}

int PowerSupplyFileWrite(char *pathfile, char *value)
{
    int fd;
    ssize_t num;

    fd = open((char*)pathfile, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if(fd <= 0)
    {
      RLOGE("Open %s file error: %s", pathfile, strerror(errno));
      return -1;
    }
    num = write(fd, value, strlen(value));
    if(num < 0)
    {
      int retry = 3;
      while (retry--)
      {
         num = write(fd, value, strlen(value));
         if (0 <= num) break;
         usleep(30 * 1000);
      }
    }
    if(num < 0)
    {
       RLOGE("Write value '%s' into '%s' file error: %s",
             value, pathfile, strerror(errno));
    }
    close(fd);

    return 0;
}

int BatteryStatusUpdate(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    unsigned short power_supply_type;
    unsigned short ac_online;
    unsigned short battery_status;
    unsigned short battery_charge_type;
    unsigned short health_status;
    unsigned short battery_present;
    unsigned int battery_capacity;
    unsigned short battery_capacity_level;
    unsigned long battery_voltage;
    unsigned short battery_temp;
    unsigned short battery_technology;
    char pathfile[100] = {0};
    char value[50] = {0};

    power_supply_type = SYS_POWER_SUPPLY_TYPE_BATTERY;

    ac_online = AC_DISCONNECTED;

    battery_status = ptlv->value[0];
    if (battery_status > SYS_BATTERY_STATUS_FULL)
    {
        RLOGD("Invalid battery status: %d.", battery_status);
        return -1;
    }

    if (battery_status == SYS_BATTERY_STATUS_CHARGING ||
        battery_status == SYS_BATTERY_STATUS_FULL)
    {
        ac_online = AC_CONNECTED;
    }

    battery_charge_type = SYS_BATTERY_CHARGE_NA;

    health_status = ptlv->value[1];
    if (health_status > SYS_BATTERY_HEALTH_COLD)
    {
        RLOGD("Invalid battery health: %d.", health_status);
        return -1;
    }

    battery_present = SYS_BATTERY_IN_USE;

    battery_capacity = ptlv->value[2];
    if (battery_capacity > 100)
    {
        RLOGD("Invalid battery capacity: %d.", battery_capacity);
        return -1;
    }

#ifdef MANDATORY_ENABLE_LOW_POWER_MODE_SUPPORT
    if (!IsLowPower(rm_sysctrl) && battery_capacity <= 20)
    {
        RLOGD("Low Power.");
        SetLowPower(rm_sysctrl);
    }
    else if (IsLowPower(rm_sysctrl) && battery_capacity > 20)
    {
        RLOGD("Low Power is gone.");
        ClearLowPower(rm_sysctrl);
    }
#endif

    if (battery_capacity < 10)
        battery_capacity_level = SYS_BATTERY_CAPACITY_LEVEL_CRITICAL;
    else if (battery_capacity >= 10 && battery_capacity < 20)
        battery_capacity_level = SYS_BATTERY_CAPACITY_LEVEL_LOW;
    else if (battery_capacity >= 20 && battery_capacity < 90)
        battery_capacity_level = SYS_BATTERY_CAPACITY_LEVEL_NORMAL;
    else if (battery_capacity >= 90 && battery_capacity < 100)
        battery_capacity_level = SYS_BATTERY_CAPACITY_LEVEL_HIGH;
    else if (battery_capacity == 100 )
        battery_capacity_level = SYS_BATTERY_CAPACITY_LEVEL_FULL;

    battery_voltage =
        ((ptlv->value[3] << 8  & 0xff00) |
        (ptlv->value[4] << 0  & 0x00ff)) * 1000;
    battery_temp =
        (ptlv->value[5] << 8  & 0xff00) |
        (ptlv->value[6] << 0  & 0x00ff);
    battery_technology = ptlv->value[7];
    if (battery_technology > SYS_BATTERY_TECHNOLOGY_LIMN)
    {
        RLOGD("Invalid battery health:%d.", battery_technology);
        return -1;
    }

    RLOGD("The battery infomation to set is:\n"
          "  power_supply_type %d\n"
          "  connectivity %d\n"
          "  status %d\n"
          "  battery charge type %d\n"
          "  health %d\n"
          "  present %d\n"
          "  capacity %d%%\n"
          "  capacity level %d\%\n"
          "  voltage %duV\n"
          "  temparature %foC\n"
          "  technology %d\n",
          power_supply_type,
          ac_online,
          battery_status,
          battery_charge_type,
          health_status,
          battery_present,
          battery_capacity,
          battery_capacity_level,
          battery_voltage,
          battery_temp / 10.0,
          battery_technology);

    create_path(BATTERY_SUPPLY_PATH "/tmp");
    create_path(AC_SUPPLY_PATH "/tmp");

    if (rm_sysctrl->power_supply_type != power_supply_type)
    {
        rm_sysctrl->power_supply_type = power_supply_type;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", BATTERY_SUPPLY_PATH, "type");
        memset(value, 0, sizeof(value));
        sprintf(value, "%s\n", type_text[power_supply_type]);
        PowerSupplyFileWrite(pathfile, value);
    }
#ifdef POWER_CHARGE_SUPPORT
    if (rm_sysctrl->ac_online != ac_online)
    {
        rm_sysctrl->ac_online = ac_online;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", AC_SUPPLY_PATH, "online");
        memset(value, 0, sizeof(value));
        sprintf(value, "%d\n", ac_online);
        PowerSupplyFileWrite(pathfile, value);
        RmSysctrlSetACOnline(rm_sysctrl->ac_online);
        if (rm_sysctrl->state == ROBOT_STATUS_WORK ||
            rm_sysctrl->state == ROBOT_STATUS_CHARGE)
        {
            if (rm_sysctrl->ac_online == AC_DISCONNECTED)
                FsmPowerExitCharge(rm_sysctrl);
            else if (rm_sysctrl->ac_online == AC_CONNECTED)
                FsmPowerCharge(rm_sysctrl);
            else
                RLOGE("Invalid ac online value.");
        }
        else if (rm_sysctrl->state == ROBOT_STATUS_SLEEP)
        {
            if (rm_sysctrl->ac_online == AC_CONNECTED)
                FsmWakeNoti(rm_sysctrl);
        }
        else if (rm_sysctrl->state == ROBOT_STATUS_SECURITY)
        {
            if (rm_sysctrl->ac_online == AC_CONNECTED)
                FsmExitSecurity(rm_sysctrl);
        }
    }
#endif
    if (rm_sysctrl->battery_status != battery_status)
    {
        rm_sysctrl->battery_status = battery_status;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", BATTERY_SUPPLY_PATH, "status");
        memset(value, 0, sizeof(value));
        sprintf(value, "%s\n", status_text[battery_status]);
        PowerSupplyFileWrite(pathfile, value);
        RmSysctrlSetBatteryStatus(rm_sysctrl->battery_status);
    }
    if (rm_sysctrl->battery_charge_type != battery_charge_type)
    {
        rm_sysctrl->battery_charge_type = battery_charge_type;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", BATTERY_SUPPLY_PATH, "charge_type");
        memset(value, 0, sizeof(value));
        sprintf(value, "%s\n", charge_type[battery_charge_type]);
        PowerSupplyFileWrite(pathfile, value);
    }
    if (rm_sysctrl->health_status != health_status)
    {
        rm_sysctrl->health_status = health_status;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", BATTERY_SUPPLY_PATH, "health");
        memset(value, 0, sizeof(value));
        sprintf(value, "%s\n", health_text[health_status]);
        PowerSupplyFileWrite(pathfile, value);
        RmSysctrlSetBatteryHealth(rm_sysctrl->health_status);
    }
    if (rm_sysctrl->battery_present != battery_present)
    {
        rm_sysctrl->battery_present = battery_present;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", BATTERY_SUPPLY_PATH, "present");
        memset(value, 0, sizeof(value));
        sprintf(value, "%d\n", battery_present);
        PowerSupplyFileWrite(pathfile, value);
        RmSysctrlSetBatteryPresent(rm_sysctrl->battery_present);
    }
    if (rm_sysctrl->battery_capacity != battery_capacity)
    {
        rm_sysctrl->battery_capacity = battery_capacity;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", BATTERY_SUPPLY_PATH, "capacity");
        memset(value, 0, sizeof(value));
        sprintf(value, "%d\n", battery_capacity);
        PowerSupplyFileWrite(pathfile, value);
        RmSysctrlSetBatteryCapacity(rm_sysctrl->battery_capacity);
    }
    if (rm_sysctrl->battery_capacity_level != battery_capacity_level)
    {
        rm_sysctrl->battery_capacity_level = battery_capacity_level;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", BATTERY_SUPPLY_PATH, "capacity_level");
        memset(value, 0, sizeof(value));
        sprintf(value, "%s\n", capacity_level_text[battery_capacity_level]);
        PowerSupplyFileWrite(pathfile, value);
    }
    if (rm_sysctrl->battery_voltage != battery_voltage)
    {
        rm_sysctrl->battery_voltage = battery_voltage;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", BATTERY_SUPPLY_PATH, "voltage_now");
        memset(value, 0, sizeof(value));
        sprintf(value, "%ld\n", battery_voltage);
        PowerSupplyFileWrite(pathfile, value);
        RmSysctrlSetVoltageNow(rm_sysctrl->battery_voltage / 1000);
    }
    if (rm_sysctrl->battery_temp != battery_temp)
    {
        rm_sysctrl->battery_temp = battery_temp;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", BATTERY_SUPPLY_PATH, "temp");
        memset(value, 0, sizeof(value));
        sprintf(value, "%d\n", battery_temp);
        PowerSupplyFileWrite(pathfile, value);
    }
    if (rm_sysctrl->battery_technology != battery_technology)
    {
        rm_sysctrl->battery_technology = battery_technology;
        memset(pathfile, 0, sizeof(pathfile));
        sprintf(pathfile, "%s/%s", BATTERY_SUPPLY_PATH, "technology");
        memset(value, 0, sizeof(value));
        sprintf(value, "%s\n", technology_text[battery_technology]);
        PowerSupplyFileWrite(pathfile, value);
        RmSysctrlSetBatteryTechnology(rm_sysctrl->battery_technology);
    }

    return 0;
}

int FsmBatteryNoti(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmBatteryNoti called");
    return BatteryStatusUpdate(rm_sysctrl);
}

int FsmBatteryStatusQueryResp(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmBatteryStatusQueryResp called");
    return BatteryStatusUpdate(rm_sysctrl);
}

int FsmPowerOffResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmPowerOffResp called");

    if (result == SUCCESS)
    {
        if (rm_sysctrl->state == ROBOT_STATUS_ABNORMAL)
        {
            int last_fault;
            unsigned int sub_sys;
            RmSysctrlFaultGetLastFaultType(rm_sysctrl->fault, &last_fault);
            RmSysctrlFaultGetFaultSubsys(rm_sysctrl->fault, &sub_sys);
            RmSysctrlUnsetPowerOnSubSystem(&rm_sysctrl->sub_system[sub_sys]);
            if (last_fault == FAULT_SUBSYS_DEAD)
            {
                /* Send Power on dead sub-system to Rp */
                RLOGI("ToCan: Send Power on dead sub-system(0x%02x) to Rp.",
                      sub_sys);
                RmSysctrlCommSendPoweronCmdToCan(rm_sysctrl->comm,
                    MIDDLE, RP_SYSCTRL, 0x00, 1 << sub_sys);

                /* To restore all sub-system into work state */
                RLOGI("The sub-system is dead, so to restore all sub-systems");
                RLOGI(" into work state for rebooting the sub-system.");
                RestoreAllSubsysIntoWorkState(rm_sysctrl);
            }
        }
    }

    return 0;
}

int FsmPowerOnResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmPowerOnResp called");

    if (result == SUCCESS)
    {
        if (rm_sysctrl->state == ROBOT_STATUS_INIT)
        {
            int last_fault;
            unsigned int sub_sys;
            RmSysctrlFaultGetLastFaultType(rm_sysctrl->fault, &last_fault);
            RmSysctrlFaultGetFaultSubsys(rm_sysctrl->fault, &sub_sys);
            if (last_fault == FAULT_SUBSYS_DEAD)
            {
                RmSysctrlSetPowerOnSubSystem(&rm_sysctrl->sub_system[sub_sys]);
                resend_init_message_to_subsys(rm_sysctrl, sub_sys);
                /* Reconfigure the init timer as 10 seconds */
                RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_INIT_SUBSYS, 10);
            }
            else if (is_all_subsys_uninit(rm_sysctrl))
            {
                /* For pad reboot */
                set_all_subsys_poweron(rm_sysctrl);
                FsmInitSubsys(rm_sysctrl);
            }
        }
    }

    return 0;
}

/*Sub-system state*/
int FsmSubsystemStateQueryResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int state = ptlv->value[0];
    int sub_state = ptlv->value[1];

    RLOGD("FSM: FsmSubsystemStateQueryResp called");
    RLOGD("The %s sub-system state is: %s",
           RmSysctrlGetSubSystemStr((int)src_system_id),
           RmSysctrlGetStateStr(state));

#if !defined(POWER_CHARGE_SUPPORT)
    if (state == ROBOT_STATUS_CHARGE)
    {
        RLOGD("The charge state is not supported, return.");
        return 0;
    }
#endif

    RmSysctrlSetSubSystemStatus(&rm_sysctrl->sub_system[src_system_id],
                                state,
                                sub_state);

    if (rm_sysctrl->state == ROBOT_STATUS_WORK)
    {
        if (!rm_sysctrl->is_sleeping 
#ifdef DEEP_SLEEP_SUPPORT
            && !rm_sysctrl->is_sleeping_deep
#endif
            )
        {
            if (is_all_subsys_work_idle(rm_sysctrl))
                rm_sysctrl->sub_state = WORK_IDLE;
            else
                rm_sysctrl->sub_state = WORK_BUSY;
        }
#ifdef AUTO_SLEEP_SUPPORT
        if (can_not_sleep(rm_sysctrl))
        {
            reset_auto_sleep_timer(rm_sysctrl);
        }
#endif
    }

#ifdef SUB_SYS_STATUS_QUERY_IN_DEMAND_SUPPORT
    if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[src_system_id]))
    {
        RmSysctrlDecreaseNeedUpdateSubSysStatus();
    }
#endif

    return 0;
}

int FsmSubsystemStateChange(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int state = ptlv->value[0];
    int sub_state = ptlv->value[1];

    RLOGD("FSM: FsmSubsystemStateChange called");
    RLOGD("The %s sub-system state is: %s",
           RmSysctrlGetSubSystemStr((int)src_system_id),
           RmSysctrlGetStateStr(state));

#if !defined(POWER_CHARGE_SUPPORT)
    if (state == ROBOT_STATUS_CHARGE)
    {
        RLOGD("The charge state is not supported, return.");
        return 0;
    }
#endif

    RmSysctrlSetSubSystemStatus(&rm_sysctrl->sub_system[src_system_id],
                                state,
                                sub_state);

    if (rm_sysctrl->state == ROBOT_STATUS_WORK)
    {
        if (is_all_subsys_work_idle(rm_sysctrl))
            rm_sysctrl->sub_state = WORK_IDLE;
        else
            rm_sysctrl->sub_state = WORK_BUSY;
    }

    return 0;
}

/*Version*/
int FsmVersionQueryResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    char sub_sys_id = ptlv->value[0];
    char major = ptlv->value[1];
    char minor = ptlv->value[2];
    char revision = ptlv->value[3];
    unsigned short building_year =
        (ptlv->value[4] << 8  & 0xff00) |
        (ptlv->value[5] << 0  & 0x00ff);
    char building_month = ptlv->value[6];
    char building_day = ptlv->value[7];
    char rom_id = ptlv->value[8];

    RLOGD("FSM: FsmVersionQueryResp called");

    #ifdef GET_RM_VERSION_FROM_PROPERTY
    if (RM_SYSTEM_ID == sub_sys_id) return 0;
    #endif

    RLOGD("The version update: %d.%d.%d.%d.%d%02d%02d.%d",
          sub_sys_id,
          major,
          minor,
          revision,
          building_year,
          building_month,
          building_day,
          rom_id);

    RmSysctrlSetVersion(&rm_sysctrl->sub_system[src_system_id],
                        sub_sys_id,
                        major,
                        minor,
                        revision,
                        building_year,
                        building_month,
                        building_day,
                        rom_id);

    return 0;
}

/*Fault*/
int FsmFault(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    unsigned int subsys_fault_type;

    subsys_fault_type  = ptlv->value[0] << 24;
    subsys_fault_type |= ptlv->value[1] << 16;
    subsys_fault_type |= ptlv->value[2] << 8;
    subsys_fault_type |= ptlv->value[3];

    RLOGD("FSM: FsmFault called");

    RLOGD("Fault: %s subsys %d fault.",
        RmSysctrlGetSubSystemStr(src_system_id),
        subsys_fault_type);
    RmSysctrlFaultSetFaultSubsys(rm_sysctrl->fault, src_system_id);
    RmSysctrlFaultSetSubsysFaultType(rm_sysctrl->fault, subsys_fault_type);
    RmSysctrlFaultSetLastFaultType(rm_sysctrl->fault, FAULT_SUBSYS_FAULT);

    /* We think the fault from sub-system should be fatal,
     * the system will into abnormal state.
     */
    return 0;
}

int FsmFaultResolved(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    unsigned int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    unsigned int fault_subsys;

    RLOGD("FSM: FsmFaultResolved called");

    RmSysctrlFaultGetFaultSubsys(rm_sysctrl->fault, &fault_subsys);
    if (src_system_id != fault_subsys)
    {
        RLOGD("The resolved subsys is not the last fault occurs subsys.");
        return 0;
    }

    RLOGD("%s subsys fault resolved.",
        RmSysctrlGetSubSystemStr(src_system_id));
    RmSysctrlFaultSetSubsysFaultType(rm_sysctrl->fault, FAULT_NONE);

    return 0;
}

int FsmSubsysDead(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int sub_sys = ptlv->value[0];

    RLOGD("FSM: FsmSubsysDead called");

    if (!RmSysctrlIsValidSubSystem(sub_sys))
    {
        RLOGD("Invalid sub-sys ID");
        return 0;
    }

    RLOGD("The %s sub-system dead.", RmSysctrlGetSubSystemStr(sub_sys));
    RmSysctrlFaultSetFaultSubsys(rm_sysctrl->fault, sub_sys);
    RmSysctrlFaultSetLastFaultType(rm_sysctrl->fault, FAULT_SUBSYS_DEAD);
    RmSysctrlInactiveSubSystem(&rm_sysctrl->sub_system[sub_sys]);
    SET_STATE(&rm_sysctrl->sub_system[sub_sys], ROBOT_STATUS_NONE);

    if (sub_sys != RP_SYSTEM_ID &&
        !IS_SUBSYS_DEAD_TIMES_REACH_MAX(&rm_sysctrl->sub_system[sub_sys]) &&
        !(rm_sysctrl->expect_state == ROBOT_STATUS_HALT))
    {
        if (sub_sys == RM_SYSTEM_ID)
        {
            int pid = 0;

            GetPidByName("system_server", &pid);
            RLOGD("The pid of system_server process is %d", pid);
            if (pid)
            {
                ALOGE("Kill system_server to restart rm sub-system.");
                kill(pid, SIGKILL);
            }
            rm_sysctrl->is_registered = false;
            rm_sysctrl->is_login = false;

            /* To restore all sub-system into work state */
            RLOGI("The sub-system is dead, so to restore all sub-systems");
            RLOGI(" into work state for restarting the sub-system.");
            RestoreAllSubsysIntoWorkState(rm_sysctrl);
        }
        else
        {
            /* Send Power off dead sub-system to Rp */
            RLOGI("ToCan: Send Power off dead sub-system(0x%02x) to Rp.", sub_sys);
            RmSysctrlCommSendPoweroffCmdToCan(rm_sysctrl->comm,
                    MIDDLE, RP_SYSCTRL, 0x00, 1 << sub_sys);
            RLOGI("To sleep 100ms to wait the dead sub-system power off.");
            usleep(100000);
        }
    }

    return 0;
}

/*Halt*/
int FsmHalt(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmHalt called");

    CHECK_POSTPONE_MESSAGE;

    /* Broadcast halt message into can */
    RmSysctrlBroadcastHaltCmdMsg(rm_sysctrl);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_HALT_CMD;

    rm_sysctrl->expect_state = ROBOT_STATUS_HALT;
    RM_SYSCTRL_RESET_TIMER(rm_sysctrl, TIMER_EXPECT_STATE);
    rm_sysctrl->is_halt_received = true;

    return 0;
}

int FsmHaltResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];

    RLOGD("FSM: FsmHaltResp called");

    if (result == SUCCESS)
    {
        RLOGD("The sub-system halt success, we think the sub-system into halt"
              " state");
        RmSysctrlSetSubSystemStatus(&rm_sysctrl->sub_system[src_system_id],
                                    ROBOT_STATUS_HALT,
                                    0);
    }

    return 0;
}

/*Timer*/
int FsmTimerExpire(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int timer_id = ptlv->value[0]; 

    RLOGD("FSM: FsmTimerExpire called");

    if (!RmSysctrlIsValidTimer(timer_id))
    {
        ALOGD("Invalid timer id");
        return 0;
    }

    if (timer_id == TIMER_INIT_SUBSYS)
    {
        unsigned int expire_times = rm_sysctrl->timer[timer_id].expire_times;

        RLOGD("Init subsys timer expired.");
        if (expire_times >= INIT_SUBSYS_MAX_TIMES)
        {
            RmSysctrlFaultSetLastFaultType(rm_sysctrl->fault,
                FAULT_INIT_SUBSYS_EXPIRE);
        }
        else if (expire_times == INIT_SUBSYS_MAX_TIMES - 3)
        {
            SendWakeupMsgToCan(rm_sysctrl);
            resend_init_message_to_uninit_subsys(rm_sysctrl);
        }
        else if (expire_times == INIT_SUBSYS_MAX_TIMES - 2)
        {
            restart_uninit_subsys(rm_sysctrl);
        }
        else
        {
            resend_init_message_to_uninit_subsys(rm_sysctrl);
        }
    }
    else if (timer_id == TIMER_REGISTER)
    {
        RLOGD("Register timer expired.");
        /* Send Register Query to RmScheduler */
        RLOGI("ToCan: Send Register Query to RmScheduler.");
        RmSysctrlCommSendRegQueryToCan(rm_sysctrl->comm,
            MIDDLE, RM_SCHEDULER);
    }
    else if (timer_id == TIMER_RELOGIN)
    {
        RLOGD("Relogin timer expired.");
        /* Set relogin */
        if (!rm_sysctrl->is_login)
          rm_sysctrl->is_need_relogin = true;
    }
    else if (timer_id == TIMER_EXPECT_STATE)
    {
        RLOGD("Expect state timer expired.");
        RLOGD("Expect all sub-system into \"%s\" state timeout",
            RmSysctrlGetStateStr(rm_sysctrl->expect_state));

        rm_sysctrl->expect_state_expired = true;
        RmSysctrlFaultSetLastFaultType(rm_sysctrl->fault,
            FAULT_EXPECT_STATE_EXPIRE);

       /* If the system state is abnormal and
        * expect state is halt and expired times is more than 3,
        * we will power off the whole system directly.
        */
        if (rm_sysctrl->state == ROBOT_STATUS_ABNORMAL &&
            rm_sysctrl->expect_state == ROBOT_STATUS_HALT &&
            rm_sysctrl->timer[TIMER_EXPECT_STATE].expire_times
                     >= 3)
        {
            /* Send Power off to Rp */
            RLOGI("ToCan: Send Power off to Rp.");
            RmSysctrlCommSendPoweroffCmdToCan(rm_sysctrl->comm,
                MIDDLE, RP_SYSCTRL, 0x80, 0x00);
        }
    }
#ifdef DEEP_SLEEP_SUPPORT
    else if (timer_id == TIMER_DEEP_SLEEP)
    {
        RLOGD("Deep sleep timer expired.");
        rm_sysctrl->is_deep_sleep_expired = true;
        RmSysctrlFaultSetLastFaultType(rm_sysctrl->fault,
            FAULT_DEEP_SLEEP_EXPIRE);
    }
    else if (timer_id == TIMER_DEEP_WAKE)
    {
        RLOGD("Deep wake timer expired.");
        rm_sysctrl->is_deep_wake_expired = true;
        RmSysctrlFaultSetLastFaultType(rm_sysctrl->fault,
            FAULT_DEEP_WAKE_EXPIRE);
    }
#endif
    else if (timer_id == TIMER_DEBUG_SET)
    {
        RLOGD("Debug set timer expired.");
        rm_sysctrl->is_debug_set_expired = true;
        RmSysctrlFaultSetLastFaultType(rm_sysctrl->fault,
            FAULT_DEBUG_SET_EXPIRE);
    }
    else if (timer_id == TIMER_MODE_SET)
    {
        RLOGD("Mode set timer expired.");
        rm_sysctrl->is_mode_set_expired = true;
        RmSysctrlFaultSetLastFaultType(rm_sysctrl->fault,
            FAULT_MODE_SET_EXPIRE);
    }
#ifdef AUTO_SLEEP_SUPPORT
    else if (timer_id == TIMER_AUTO_SLEEP)
    {
        RLOGD("Auto sleep timer expired.");
        SleepShallow(rm_sysctrl);
    }
#endif

    return 0;
}

#ifdef DEEP_SLEEP_SUPPORT
/*S_poweron*/
int FsmSPoweronChange(struct RmSysctrl *rm_sysctrl)
{
    unsigned int s_poweron = 0;
    unsigned int s_powerdown = rm_sysctrl->gpio->s_powerdown;
    RLOGD("FSM: FsmSPoweronChange called");

    RmSysctrlGpioGetValue(rm_sysctrl->gpio, &s_poweron);
    if (rm_sysctrl->is_sleeping_deep &&
        rm_sysctrl->sub_state == SLEEP_DEEP &&
        s_powerdown == 0 &&
        s_poweron == 1)
    {
        /* Set S_powerdown = 1 */
        RLOGD("Deep wake from RF, Set S_powerdown = 1");
        RmSysctrlGpioSetValue(rm_sysctrl->gpio, 1);

        rm_sysctrl->is_sleeping_deep = false;
        rm_sysctrl->is_deep_sleep_expired = false;
    }
    else if (rm_sysctrl->is_sleeping_deep &&
        rm_sysctrl->sub_state == SLEEP_DEEP &&
        s_powerdown == 0 &&
        s_poweron == 0)
    {
        /* Set all sub-system state into deep sleep */
        set_all_subsys_deep_sleep(rm_sysctrl);
    }
    return 0;
}
#endif

/*Debug*/
int SetDebug(struct RmSysctrl *rm_sysctrl, bool debug_en)
{
    int ret = -1;

    ret = RmSysctrlSetDebug(rm_sysctrl->config, debug_en);
    if (ret < 0)
    {
        RLOGE("Set debug failed.");
        return -1;
    }

    RLOGD("Set debug %s", debug_en ? "enable" : "disable");

    /* Broadcast debug message into can */
    RmSysctrlBroadcastDebugCmdMsg(rm_sysctrl, debug_en ? 1 : 0);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_DEBUG_CMD;

    rm_sysctrl->is_debug_set_received = true;
    rm_sysctrl->is_debug_set_expired = false;
    return 0;
}

int FsmDebug(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int debug_en = ptlv->value[0];
    int sys_mode;

    RLOGD("FSM: FsmDebug called");

    if (debug_en != 1 && debug_en != 0)
    {
        RLOGE("Invalid debug message");
        return -1;
    }

    RmSysctrlGetMode(rm_sysctrl->config, &sys_mode);

    if (debug_en == 1 && sys_mode != DEVELOPER_MODE)
    {
        RLOGE("Set the debug enable failed because"
              " current mode is not in DEVELOPER_MODE");
        return -1;
    }

    if (debug_en == 1)
        SetDebug(rm_sysctrl, true);
    else
        SetDebug(rm_sysctrl, false);


    return 0;
}

int FsmDebugResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];
    bool debug_en = false;

    RLOGD("FSM: FsmDebugResp called");

    if (result == SUCCESS)
    {
        RmSysctrlGetDebug(rm_sysctrl->config, &debug_en);
        SET_SUBSYS_DEBUG(&rm_sysctrl->sub_system[src_system_id],
                         debug_en);
        RLOGD("Set debug %s for sub-system %s",
              debug_en ? "enable" : "disable",
              RmSysctrlGetSubSystemStr(src_system_id));

        if (rm_sysctrl->is_debug_set_received &&
            is_all_subsys_into_debug_set(rm_sysctrl))
        {
            rm_sysctrl->is_debug_set_received = false;
            rm_sysctrl->is_debug_set_expired = false;
        }
    }
    return 0;
}

/*Mode*/
int SetMode(struct RmSysctrl *rm_sysctrl, int mode)
{
    int ret = -1;

    ret = RmSysctrlSetMode(rm_sysctrl->config, mode);
    if (ret < 0)
    {
        RLOGE("Set mode failed.");
        return -1;
    }

    RLOGD("Set mode %s",
        mode == DEVELOPER_MODE ?
        "Developer" :
        (mode == CHILDREN_MODE ? "Children" : "Guarder"));

    /* Broadcast mode message into can */
    RmSysctrlBroadcastModeCmdMsg(rm_sysctrl, mode);
    rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_MODE_CMD;

    rm_sysctrl->is_mode_set_received = true;
    rm_sysctrl->is_mode_set_expired = false;

    return 0;
}

int FsmMode(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int mode = ptlv->value[0];
    int ret = -1;

    RLOGD("FSM: FsmMode called");

    if (mode != DEVELOPER_MODE &&
        mode != CHILDREN_MODE &&
        mode != GUARDER_MODE)
    {
        RLOGE("Invalid mode message");
        return -1;
    }
    
    if (mode == CHILDREN_MODE ||
        mode == GUARDER_MODE)
    {
        bool debug_en = false;
        RmSysctrlGetDebug(rm_sysctrl->config, &debug_en);
        if (debug_en)
        {
            RLOGD("Close the debug as the mode to set is not developer mode.");
            SetDebug(rm_sysctrl, false);
            usleep(100000);
        }
    }

    return SetMode(rm_sysctrl, mode);
}

int FsmModeResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    int result = ptlv->value[0];
    int sys_mode;

    RLOGD("FSM: FsmModeResp called");

    RmSysctrlGetMode(rm_sysctrl->config, &sys_mode);
    if (result == SUCCESS)
    {
        SET_SUBSYS_MODE(&rm_sysctrl->sub_system[src_system_id],
                        sys_mode);
        RLOGD("Set mode %s for sub-system %s",
        sys_mode == DEVELOPER_MODE ?
        "Developer" :
        (sys_mode == CHILDREN_MODE ? "Children" : "Guarder"),
        RmSysctrlGetSubSystemStr(src_system_id));

        if (rm_sysctrl->is_mode_set_received &&
            is_all_subsys_into_mode_set(rm_sysctrl))
        {
            rm_sysctrl->is_mode_set_received = false;
            rm_sysctrl->is_mode_set_expired = false;
        }
    }

    return 0;
}

/*Factory Mode*/
int FsmRestoreFactory(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmRestoreFactory called");

    /* Do something to restore factory mode */
    RLOGD("Restore factory mode ...");
    RmSysctrlRestoreFactory(rm_sysctrl->config);

    /* Send restore factory response to RmScheduler */
    RLOGI("ToCan: Send restore factory successfully response to RmScheduler.");
    RmSysctrlCommSendRestoreFactoryCmdRespToCan(rm_sysctrl->comm,
            MIDDLE, RM_SCHEDULER, SUCCESS);

    return 0;
}

/*SN*/
int FsmGetCPUIDResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;

    memcpy(rm_sysctrl->cpuid, ptlv->value, CPUID_LEN);

    return 0;
}

int FsmSaveSNResp(struct RmSysctrl *rm_sysctrl)
{
    return 0;
}

int FsmReadSNResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;

    memcpy(rm_sysctrl->sn, ptlv->value, SN_LEN);
    if( isvalidSN(rm_sysctrl->cpuid, rm_sysctrl->sn))
      rm_sysctrl->is_valid_sn = true;
    else
      rm_sysctrl->is_valid_sn = false;

    return 0;
}

/*Datetime*/
int SetDatetimeIntoSystem(struct RmSysctrlDatetime *dt)
{
    struct tm _tm;
    struct timeval tv;
    time_t timep;

    RLOGD("Set the datetime into system %d-%d-%d %d:%d:%d.",
          dt->year + 2000, dt->month, dt->day, dt->hour,
          dt->minute, dt->second);

    _tm.tm_isdst = 0;
    _tm.tm_sec = dt->second;
    _tm.tm_min = dt->minute;
    _tm.tm_hour = dt->hour;
    _tm.tm_mday = dt->day;
    _tm.tm_mon = dt->month - 1;
    _tm.tm_year = dt->year + 2000 - 1900;
  
    timep = mktime(&_tm);
    tv.tv_sec = timep;
    tv.tv_usec = 0;
    if(settimeofday (&tv, (struct timezone *) 0) < 0)
    {
        RLOGE("Set system datatime error!\n");
        return -1;
    }
    return 0;
}

int FsmDatetimeQueryResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;
    struct RmSysctrlDatetime *dt = &rm_sysctrl->datetime;

    dt->year = ptlv->value[0];
    dt->month = ptlv->value[1];
    dt->day = ptlv->value[2];
    dt->weekday = ptlv->value[3];
    dt->hour = ptlv->value[4];
    dt->minute = ptlv->value[5];
    dt->second = ptlv->value[6];

    if (SetDatetimeIntoSystem(dt) < 0)
    {
        RLOGD("Set the datetime into system failed.");
        return -1;
    }

    return 0;
}

int FsmSetDatetimeCmdResp(struct RmSysctrl *rm_sysctrl)
{
    return 0;
}

/*Sex*/
int FsmSexQueryResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;

    if (ptlv->value[0] != MALE &&
        ptlv->value[0] != FEMALE &&
        ptlv->value[0] != UNKNOWN_SEX)
    {
        RLOGE("Invalid sex queried from RP");
        return -1;
    }

    rm_sysctrl->sex = ptlv->value[0];

    return 0;
}

int FsmSetSexCmdResp(struct RmSysctrl *rm_sysctrl)
{
    return 0;
}

/*Uptime*/
int FsmUptimeQueryResp(struct RmSysctrl *rm_sysctrl)
{
    struct RmSysctrlMessage *rscm =
        (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    int src_system_id = rscm->src_system_id;
    struct RmSysctrlTLV *ptlv = &rscm->tlv;

    rm_sysctrl->uptime=ptlv->value[0];
    rm_sysctrl->uptime<<=8;
    rm_sysctrl->uptime+=ptlv->value[1];
    rm_sysctrl->uptime<<=8;
    rm_sysctrl->uptime+=ptlv->value[2];
    rm_sysctrl->uptime<<=8;
    rm_sysctrl->uptime+=ptlv->value[3];

    RLOGD("Update the uptime as: %d", rm_sysctrl->uptime);

    return 0;
}

/*Ignore*/
int FsmIgnore(struct RmSysctrl *rm_sysctrl)
{
    RLOGD("FSM: FsmIgnore called");

    return 0;
}

/* Finite State Machine structure */
struct {
    int (*func) ();
    int next_state;
} FSM [ROBOT_STATUS_MAX][RM_SYSCTRL_EVENTS_MAX] = 
{
    {
        /* Compute: This state is a temporary state. 
         * The system state is depend on all of sub-system.
         */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Sub-system dead                  */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Init command response            */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Sleep command                    */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Sleep command response           */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Wakeup command                   */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Wakeup command response          */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Security command                 */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Security command response        */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Exit security command            */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Exit security command response   */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Halt command                     */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Mode command response            */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Low power                        */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Power charge begin               */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Power charge end                 */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Sub-system state change          */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Sub-system state query response  */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* version query response           */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Upgrade command                  */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Upgrade command response         */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Exit Upgrade command             */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Register command response        */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Restore factory command          */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Registered notification          */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Battery query command            */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Register query response          */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Login query response             */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Power on command response        */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Battery notification             */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Power on notification            */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Battery status query response    */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Save Robot SN command response   */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Read Robot SN command response   */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Set Datetime command response    */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Set sex command response         */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Uptime query response            */
        {FsmIgnore,  			ROBOT_STATUS_COMPUTE},	/* Wake notification                */
    },
    {
        /* Init */
        {FsmIgnore,   			ROBOT_STATUS_INIT},	/* Sub-system dead                  */
        {FsmTimerExpire,  		ROBOT_STATUS_COMPUTE},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* S_poweron change                 */
#endif
        {FsmInitCmdResp,  		ROBOT_STATUS_COMPUTE},	/* Init command response            */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Sleep command                    */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Sleep command response           */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Wakeup command                   */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Wakeup command response          */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Security command                 */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Security command response        */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Exit security command            */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Exit security command response   */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Halt command                     */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Mode command response            */
        {FsmFault,  			ROBOT_STATUS_ABNORMAL},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Low power                        */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Power charge begin               */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Power charge end                 */
        {FsmSubsystemStateChange,  	ROBOT_STATUS_INIT},	/* Sub-system state change          */
        {FsmSubsystemStateQueryResp, 	ROBOT_STATUS_INIT},	/* Sub-system state query response  */
        {FsmVersionQueryResp,  		ROBOT_STATUS_INIT},	/* version query response           */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Upgrade command                  */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Upgrade command response         */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Exit Upgrade command             */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Register command response        */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Restore factory command          */
        {FsmRecordRegistered,  		ROBOT_STATUS_COMPUTE},	/* Registered notification          */
        {FsmRecordLogin,  		ROBOT_STATUS_COMPUTE},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Battery query command            */
        {FsmRegisterQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Register query response          */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Login query response             */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Power off command response       */
        {FsmPowerOnResp,  		ROBOT_STATUS_INIT},	/* Power on command response        */
        {FsmBatteryNoti,  		ROBOT_STATUS_INIT},	/* Battery notification             */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Halt notification                */
        {FsmPowerOn,  			ROBOT_STATUS_INIT},	/* Power on notification            */
        {FsmBatteryStatusQueryResp,  	ROBOT_STATUS_INIT},	/* Battery status query response    */
        {FsmGetCPUIDResp,  		ROBOT_STATUS_INIT},	/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Save Robot SN command response   */
        {FsmReadSNResp,  		ROBOT_STATUS_INIT},	/* Read Robot SN command response   */
        {FsmDatetimeQueryResp,  	ROBOT_STATUS_INIT},	/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Set Datetime command response    */
        {FsmSexQueryResp,  		ROBOT_STATUS_INIT},	/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Set sex command response         */
        {FsmUptimeQueryResp,  		ROBOT_STATUS_INIT},	/* Uptime query response            */
        {FsmIgnore,  			ROBOT_STATUS_INIT},	/* Wake notification                */
    },
    {
        /* Register */
        {FsmSubsysDead,  		ROBOT_STATUS_ABNORMAL},	/* Sub-system dead                  */
        {FsmTimerExpire,  		ROBOT_STATUS_COMPUTE},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Init command response            */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Sleep command                    */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Sleep command response           */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Wakeup command                   */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Wakeup command response          */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Security command                 */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Security command response        */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Exit security command            */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Exit security command response   */
        {FsmHalt,  			ROBOT_STATUS_REGISTER},	/* Halt command                     */
        {FsmHaltResp,  			ROBOT_STATUS_COMPUTE},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Mode command response            */
        {FsmFault,  			ROBOT_STATUS_ABNORMAL},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Low power                        */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Power charge begin               */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Power charge end                 */
        {FsmSubsystemStateChange,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state change          */
        {FsmSubsystemStateQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state query response  */
        {FsmVersionQueryResp,  		ROBOT_STATUS_REGISTER},	/* version query response           */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Upgrade command                  */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Upgrade command response         */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Exit Upgrade command             */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Not low power                    */
        {FsmRegisterResp,  		ROBOT_STATUS_COMPUTE},	/* Register command response        */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Restore factory command          */
        {FsmRecordRegistered,  		ROBOT_STATUS_COMPUTE},	/* Registered notification          */
        {FsmRecordLogin,  		ROBOT_STATUS_COMPUTE},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Battery query command            */
        {FsmRegisterQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Register query response          */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Login query response             */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Power on command response        */
        {FsmBatteryNoti,  		ROBOT_STATUS_REGISTER},	/* Battery notification             */
        {FsmHalt,  			ROBOT_STATUS_REGISTER},	/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Power on notification            */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Battery status query response    */
        {FsmGetCPUIDResp,  		ROBOT_STATUS_REGISTER},	/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Save Robot SN command response   */
        {FsmReadSNResp,  		ROBOT_STATUS_REGISTER},	/* Read Robot SN command response   */
        {FsmDatetimeQueryResp,  	ROBOT_STATUS_REGISTER},	/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Set Datetime command response    */
        {FsmSexQueryResp,  		ROBOT_STATUS_REGISTER},	/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Set sex command response         */
        {FsmUptimeQueryResp,  		ROBOT_STATUS_REGISTER},	/* Uptime query response            */
        {FsmIgnore,  			ROBOT_STATUS_REGISTER},	/* Wake notification                */
    },
    {
        /* Loginning */
        {FsmSubsysDead,  		ROBOT_STATUS_ABNORMAL},	/* Sub-system dead                  */
        {FsmTimerExpire,  		ROBOT_STATUS_COMPUTE},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Init command response            */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Sleep command                    */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Sleep command response           */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Wakeup command                   */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Wakeup command response          */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Security command                 */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Security command response        */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Exit security command            */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Exit security command response   */
        {FsmHalt,  			ROBOT_STATUS_LOGINNING},/* Halt command                     */
        {FsmHaltResp,  			ROBOT_STATUS_COMPUTE},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Mode command response            */
        {FsmFault,  			ROBOT_STATUS_ABNORMAL},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Low power                        */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Power charge begin               */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Power charge end                 */
        {FsmSubsystemStateChange,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state change          */
        {FsmSubsystemStateQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state query response  */
        {FsmVersionQueryResp,  		ROBOT_STATUS_LOGINNING},/* version query response           */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Upgrade command                  */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Upgrade command response         */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Exit Upgrade command             */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Register command response        */
        {FsmLoginResp,  		ROBOT_STATUS_COMPUTE},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Restore factory command          */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Registered notification          */
        {FsmRecordLogin,  		ROBOT_STATUS_COMPUTE},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Battery query command            */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Register query response          */
        {FsmLoginQueryResp,  		ROBOT_STATUS_COMPUTE},	/* Login query response             */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Power on command response        */
        {FsmBatteryNoti,  		ROBOT_STATUS_LOGINNING},/* Battery notification             */
        {FsmHalt,  			ROBOT_STATUS_LOGINNING},/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Power on notification            */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Battery status query response    */
        {FsmGetCPUIDResp,  		ROBOT_STATUS_LOGINNING},/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Save Robot SN command response   */
        {FsmReadSNResp,  		ROBOT_STATUS_LOGINNING},/* Read Robot SN command response   */
        {FsmDatetimeQueryResp,  	ROBOT_STATUS_LOGINNING},/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Set Datetime command response    */
        {FsmSexQueryResp,  		ROBOT_STATUS_LOGINNING},/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Set sex command response         */
        {FsmUptimeQueryResp,  		ROBOT_STATUS_LOGINNING},/* Uptime query response            */
        {FsmIgnore,  			ROBOT_STATUS_LOGINNING},/* Wake notification                */
    },
    {
        /* Not Login */
        {FsmSubsysDead,  		ROBOT_STATUS_ABNORMAL},	/* Sub-system dead                  */
        {FsmTimerExpire,  		ROBOT_STATUS_COMPUTE},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Init command response            */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Sleep command                    */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Sleep command response           */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Wakeup command                   */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Wakeup command response          */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Security command                 */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Security command response        */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Exit security command            */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Exit security command response   */
        {FsmHalt,  			ROBOT_STATUS_NOT_LOGIN},/* Halt command                     */
        {FsmHaltResp,  			ROBOT_STATUS_COMPUTE},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Mode command response            */
        {FsmFault,  			ROBOT_STATUS_ABNORMAL},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Low power                        */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Power charge begin               */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Power charge end                 */
        {FsmSubsystemStateChange,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state change          */
        {FsmSubsystemStateQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state query response  */
        {FsmVersionQueryResp,  		ROBOT_STATUS_NOT_LOGIN},/* version query response           */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Upgrade command                  */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Upgrade command response         */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Exit Upgrade command             */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Register command response        */
        {FsmLoginResp,  		ROBOT_STATUS_COMPUTE},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Restore factory command          */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Registered notification          */
        {FsmRecordLogin,  		ROBOT_STATUS_COMPUTE},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Battery query command            */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Register query response          */
        {FsmLoginQueryResp,  		ROBOT_STATUS_COMPUTE},	/* Login query response             */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Power on command response        */
        {FsmBatteryNoti,  		ROBOT_STATUS_NOT_LOGIN},/* Battery notification             */
        {FsmHalt,  			ROBOT_STATUS_NOT_LOGIN},/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Power on notification            */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Battery status query response    */
        {FsmGetCPUIDResp,  		ROBOT_STATUS_NOT_LOGIN},/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Save Robot SN command response   */
        {FsmReadSNResp,  		ROBOT_STATUS_NOT_LOGIN},/* Read Robot SN command response   */
        {FsmDatetimeQueryResp,  	ROBOT_STATUS_NOT_LOGIN},/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Set Datetime command response    */
        {FsmSexQueryResp,  		ROBOT_STATUS_NOT_LOGIN},/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Set sex command response         */
        {FsmUptimeQueryResp,  		ROBOT_STATUS_NOT_LOGIN},/* Uptime query response            */
        {FsmIgnore,  			ROBOT_STATUS_NOT_LOGIN},/* Wake notification                */
    },
    {
        /* Work */
        {FsmSubsysDead,  		ROBOT_STATUS_ABNORMAL},	/* Sub-system dead                  */
        {FsmTimerExpire,  		ROBOT_STATUS_COMPUTE},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmSPoweronChange,  		ROBOT_STATUS_COMPUTE},	/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Init command response            */
        {FsmSleep,  			ROBOT_STATUS_WORK},	/* Sleep command                    */
        {FsmSleepResp,  		ROBOT_STATUS_COMPUTE},	/* Sleep command response           */
        {FsmWakeup,  			ROBOT_STATUS_WORK},	/* Wakeup command                   */
        {FsmWakeupResp,  		ROBOT_STATUS_COMPUTE},	/* Wakeup command response          */
        {FsmSecurity,  			ROBOT_STATUS_WORK},	/* Security command                 */
        {FsmSecurityResp,  		ROBOT_STATUS_COMPUTE},	/* Security command response        */
        {FsmExitSecurity,  		ROBOT_STATUS_WORK},	/* Exit security command            */
        {FsmExitSecurityResp,  		ROBOT_STATUS_COMPUTE},	/* Exit security command response   */
        {FsmHalt,  			ROBOT_STATUS_WORK},	/* Halt command                     */
        {FsmHaltResp,  			ROBOT_STATUS_COMPUTE},	/* Halt command response            */
        {FsmDebug,  			ROBOT_STATUS_WORK},	/* Debug command                    */
        {FsmDebugResp,  		ROBOT_STATUS_WORK},	/* Debug command response           */
        {FsmMode,  			ROBOT_STATUS_WORK},	/* Mode command                     */
        {FsmModeResp,  			ROBOT_STATUS_WORK},	/* Mode command response            */
        {FsmFault,  			ROBOT_STATUS_ABNORMAL},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Abnormal events resolved         */
        {FsmLowPower,  			ROBOT_STATUS_WORK},	/* Low power                        */
        {FsmPowerChargeX,  		ROBOT_STATUS_WORK},	/* Power charge begin               */
        {FsmPowerExitChargeX,  		ROBOT_STATUS_WORK},	/* Power charge end                 */
        {FsmSubsystemStateChange,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state change          */
        {FsmSubsystemStateQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state query response  */
        {FsmVersionQueryResp,  		ROBOT_STATUS_WORK},	/* version query response           */
        {FsmUpgrade,  			ROBOT_STATUS_WORK},	/* Upgrade command                  */
        {FsmUpgradeResp,  		ROBOT_STATUS_COMPUTE},	/* Upgrade command response         */
        {FsmExitUpgrade,  		ROBOT_STATUS_WORK},	/* Exit Upgrade command             */
        {FsmExitUpgradeResp,  		ROBOT_STATUS_COMPUTE},	/* Exit Upgrade command response    */
        {FsmNotLowPower,  		ROBOT_STATUS_WORK},	/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Register command response        */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Login command response           */
        {FsmRestoreFactory,  		ROBOT_STATUS_WORK},	/* Restore factory command          */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Registered notification          */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Login notification               */
        {FsmBatteryQuery,  		ROBOT_STATUS_WORK},	/* Battery query command            */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Register query response          */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Login query response             */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Power on command response        */
        {FsmBatteryNoti,  		ROBOT_STATUS_WORK},	/* Battery notification             */
        {FsmHalt,  			ROBOT_STATUS_WORK},	/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Power on notification            */
        {FsmBatteryStatusQueryResp,	ROBOT_STATUS_WORK},	/* Battery status query response    */
        {FsmGetCPUIDResp,  		ROBOT_STATUS_WORK},	/* Get CPU ID command response      */
        {FsmSaveSNResp,  		ROBOT_STATUS_WORK},	/* Save Robot SN command response   */
        {FsmReadSNResp,  		ROBOT_STATUS_WORK},	/* Read Robot SN command response   */
        {FsmDatetimeQueryResp,  	ROBOT_STATUS_WORK},	/* Datetime query response          */
        {FsmSetDatetimeCmdResp,  	ROBOT_STATUS_WORK},	/* Set Datetime command response    */
        {FsmSexQueryResp,  		ROBOT_STATUS_WORK},	/* Sex query response               */
        {FsmSetSexCmdResp,  		ROBOT_STATUS_WORK},	/* Set sex command response         */
        {FsmUptimeQueryResp,  		ROBOT_STATUS_WORK},	/* Uptime query response            */
        {FsmIgnore,  			ROBOT_STATUS_WORK},	/* Wake notification                */
    },
    {
        /* Sleep */
        {FsmSubsysDead,  		ROBOT_STATUS_ABNORMAL},	/* Sub-system dead                  */
        {FsmTimerExpire,  		ROBOT_STATUS_COMPUTE},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmSPoweronChange,  		ROBOT_STATUS_COMPUTE},	/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Init command response            */
        {FsmSleep,  			ROBOT_STATUS_SLEEP},	/* Sleep command                    */
        {FsmSleepResp,  		ROBOT_STATUS_COMPUTE},	/* Sleep command response           */
        {FsmWakeup,  			ROBOT_STATUS_SLEEP},	/* Wakeup command                   */
        {FsmWakeupResp,  		ROBOT_STATUS_COMPUTE},	/* Wakeup command response          */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Security command                 */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Security command response        */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Exit security command            */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Exit security command response   */
        {FsmWakeNoti,  			ROBOT_STATUS_SLEEP},	/* Halt command                     */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Mode command response            */
        {FsmFault,  			ROBOT_STATUS_ABNORMAL},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Low power                        */
        {FsmWakeNoti,  			ROBOT_STATUS_SLEEP},	/* Power charge begin               */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Power charge end                 */
        {FsmSubsystemStateChange,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state change          */
        {FsmSubsystemStateQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state query response  */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* version query response           */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Upgrade command                  */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Upgrade command response         */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Exit Upgrade command             */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Register command response        */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Restore factory command          */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Registered notification          */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Battery query command            */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Register query response          */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Login query response             */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Power on command response        */
        {FsmBatteryNoti,  		ROBOT_STATUS_SLEEP},	/* Battery notification             */
        {FsmWakeNoti,  			ROBOT_STATUS_SLEEP},	/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Power on notification            */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Battery status query response    */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Save Robot SN command response   */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Read Robot SN command response   */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Set Datetime command response    */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Set sex command response         */
        {FsmIgnore,  			ROBOT_STATUS_SLEEP},	/* Uptime query response            */
        {FsmWakeNoti,  			ROBOT_STATUS_COMPUTE},	/* Wake notification                */
    },
    {
        /* Charge */
        {FsmSubsysDead,  		ROBOT_STATUS_ABNORMAL},	/* Sub-system dead                  */
        {FsmTimerExpire,  		ROBOT_STATUS_COMPUTE},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Init command response            */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Sleep command                    */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Sleep command response           */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Wakeup command                   */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Wakeup command response          */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Security command                 */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Security command response        */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Exit security command            */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Exit security command response   */
        {FsmHalt,  			ROBOT_STATUS_CHARGE},	/* Halt command                     */
        {FsmHaltResp,  			ROBOT_STATUS_COMPUTE},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Mode command response            */
        {FsmFault,  			ROBOT_STATUS_ABNORMAL},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Low power                        */
        {FsmPowerChargeX,  		ROBOT_STATUS_CHARGE},	/* Power charge begin               */
        {FsmPowerExitChargeX,  		ROBOT_STATUS_CHARGE},	/* Power charge end                 */
        {FsmSubsystemStateChange,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state change          */
        {FsmSubsystemStateQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state query response  */
        {FsmVersionQueryResp,  		ROBOT_STATUS_CHARGE},	/* version query response           */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Upgrade command                  */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Upgrade command response         */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Exit Upgrade command             */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Register command response        */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Restore factory command          */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Registered notification          */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Battery query command            */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Register query response          */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Login query response             */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Power on command response        */
        {FsmBatteryNoti,  		ROBOT_STATUS_CHARGE},	/* Battery notification             */
        {FsmHalt,  			ROBOT_STATUS_CHARGE},	/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Power on notification            */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Battery status query response    */
        {FsmGetCPUIDResp,  		ROBOT_STATUS_CHARGE},	/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Save Robot SN command response   */
        {FsmReadSNResp,  		ROBOT_STATUS_CHARGE},	/* Read Robot SN command response   */
        {FsmDatetimeQueryResp,  	ROBOT_STATUS_CHARGE},	/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Set Datetime command response    */
        {FsmSexQueryResp,  		ROBOT_STATUS_CHARGE},	/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Set sex command response         */
        {FsmUptimeQueryResp,  		ROBOT_STATUS_CHARGE},	/* Uptime query response            */
        {FsmIgnore,  			ROBOT_STATUS_CHARGE},	/* Wake notification                */
    },
    {
        /* Abnormal */
        {FsmSubsysDead,  		ROBOT_STATUS_ABNORMAL},	/* Sub-system dead                  */
        {FsmTimerExpire,  		ROBOT_STATUS_ABNORMAL},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmSPoweronChange,  		ROBOT_STATUS_COMPUTE},	/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Init command response            */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Sleep command                    */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Sleep command response           */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Wakeup command                   */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Wakeup command response          */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Security command                 */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Security command response        */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Exit security command            */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Exit security command response   */
        {FsmHalt,  			ROBOT_STATUS_ABNORMAL},	/* Halt command                     */
        {FsmHaltResp,  			ROBOT_STATUS_COMPUTE},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Mode command response            */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Abnormal events occur            */
        {FsmFaultResolved,  		ROBOT_STATUS_COMPUTE},	/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Low power                        */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Power charge begin               */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Power charge end                 */
        {FsmSubsystemStateChange,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state change          */
        {FsmSubsystemStateQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state query response  */
        {FsmVersionQueryResp,  		ROBOT_STATUS_ABNORMAL},	/* version query response           */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Upgrade command                  */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Upgrade command response         */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Exit Upgrade command             */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Register command response        */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Restore factory command          */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Registered notification          */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Battery query command            */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Register query response          */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Login query response             */
        {FsmPowerOffResp,  		ROBOT_STATUS_COMPUTE},	/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Power on command response        */
        {FsmBatteryNoti,  		ROBOT_STATUS_ABNORMAL},	/* Battery notification             */
        {FsmHalt,  			ROBOT_STATUS_ABNORMAL},	/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Power on notification            */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Battery status query response    */
        {FsmGetCPUIDResp,  		ROBOT_STATUS_ABNORMAL},	/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Save Robot SN command response   */
        {FsmReadSNResp,  		ROBOT_STATUS_ABNORMAL},	/* Read Robot SN command response   */
        {FsmDatetimeQueryResp,  	ROBOT_STATUS_ABNORMAL},	/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Set Datetime command response    */
        {FsmSexQueryResp,  		ROBOT_STATUS_ABNORMAL},	/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Set sex command response         */
        {FsmUptimeQueryResp,  		ROBOT_STATUS_ABNORMAL},	/* Uptime query response            */
        {FsmIgnore,  			ROBOT_STATUS_ABNORMAL},	/* Wake notification                */
    },
    {
        /* Halt */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Sub-system dead                  */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Init command response            */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Sleep command                    */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Sleep command response           */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Wakeup command                   */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Wakeup command response          */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Security command                 */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Security command response        */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Exit security command            */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Exit security command response   */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Halt command                     */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Mode command response            */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Low power                        */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Power charge begin               */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Power charge end                 */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Sub-system state change          */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Sub-system state query response  */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* version query response           */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Upgrade command                  */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Upgrade command response         */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Exit Upgrade command             */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Register command response        */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Restore factory command          */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Registered notification          */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Battery query command            */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Register query response          */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Login query response             */
        {FsmPowerOffResp,  		ROBOT_STATUS_HALT},	/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Power on command response        */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Battery notification             */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Power on notification            */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Battery status query response    */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Save Robot SN command response   */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Read Robot SN command response   */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Set Datetime command response    */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Set sex command response         */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Uptime query response            */
        {FsmIgnore,  			ROBOT_STATUS_HALT},	/* Wake notification                */
    },
    {
        /* Security */
        {FsmSubsysDead,  		ROBOT_STATUS_ABNORMAL},	/* Sub-system dead                  */
        {FsmTimerExpire,  		ROBOT_STATUS_COMPUTE},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Init command response            */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Sleep command                    */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Sleep command response           */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Wakeup command                   */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Wakeup command response          */
        {FsmSecurity,  			ROBOT_STATUS_SECURITY},	/* Security command                 */
        {FsmSecurityResp,  		ROBOT_STATUS_COMPUTE},	/* Security command response        */
        {FsmExitSecurity,  		ROBOT_STATUS_SECURITY},	/* Exit security command            */
        {FsmExitSecurityResp,  		ROBOT_STATUS_COMPUTE},	/* Exit security command response   */
        {FsmExitSecurity,  		ROBOT_STATUS_SECURITY},	/* Halt command                     */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Mode command response            */
        {FsmFault,  			ROBOT_STATUS_COMPUTE},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Low power                        */
        {FsmExitSecurity,  		ROBOT_STATUS_SECURITY},	/* Power charge begin               */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Power charge end                 */
        {FsmSubsystemStateChange,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state change          */
        {FsmSubsystemStateQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state query response  */
        {FsmVersionQueryResp,  		ROBOT_STATUS_SECURITY},	/* version query response           */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Upgrade command                  */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Upgrade command response         */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Exit Upgrade command             */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Register command response        */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Restore factory command          */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Registered notification          */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Battery query command            */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Register query response          */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Login query response             */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Power on command response        */
        {FsmBatteryNoti,  		ROBOT_STATUS_SECURITY},	/* Battery notification             */
        {FsmExitSecurity,  		ROBOT_STATUS_SECURITY},	/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Power on notification            */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Battery status query response    */
        {FsmGetCPUIDResp,  		ROBOT_STATUS_SECURITY},	/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Save Robot SN command response   */
        {FsmReadSNResp,  		ROBOT_STATUS_SECURITY},	/* Read Robot SN command response   */
        {FsmDatetimeQueryResp,  	ROBOT_STATUS_SECURITY},	/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Set Datetime command response    */
        {FsmSexQueryResp,  		ROBOT_STATUS_SECURITY},	/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_SECURITY},	/* Set sex command response         */
        {FsmUptimeQueryResp,  		ROBOT_STATUS_SECURITY},	/* Uptime query response            */
        {FsmExitSecurity,  		ROBOT_STATUS_COMPUTE},	/* Wake notification                */
    },
    {
        /* Upgrade */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Sub-system dead                  */
        {FsmTimerExpire,  		ROBOT_STATUS_UPGRADE},	/* Timer expire                     */
#ifdef DEEP_SLEEP_SUPPORT
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* S_poweron change                 */
#endif
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Init command response            */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Sleep command                    */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Sleep command response           */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Wakeup command                   */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Wakeup command response          */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Security command                 */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Security command response        */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Exit security command            */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Exit security command response   */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Halt command                     */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Halt command response            */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Debug command                    */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Debug command response           */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Mode command                     */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Mode command response            */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Abnormal events occur            */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Abnormal events resolved         */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Low power                        */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Power charge begin               */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Power charge end                 */
        {FsmSubsystemStateChange,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state change          */
        {FsmSubsystemStateQueryResp,  	ROBOT_STATUS_COMPUTE},	/* Sub-system state query response  */
        {FsmVersionQueryResp,  		ROBOT_STATUS_UPGRADE},	/* version query response           */
        {FsmUpgrade,  			ROBOT_STATUS_UPGRADE},	/* Upgrade command                  */
        {FsmUpgradeResp,  		ROBOT_STATUS_COMPUTE},	/* Upgrade command response         */
        {FsmExitUpgrade,  		ROBOT_STATUS_UPGRADE},	/* Exit Upgrade command             */
        {FsmExitUpgradeResp,  		ROBOT_STATUS_COMPUTE},	/* Exit Upgrade command response    */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Not low power                    */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Register command response        */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Login command response           */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Restore factory command          */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Registered notification          */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Login notification               */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Battery query command            */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Register query response          */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Login query response             */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Power off command response       */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Power on command response        */
        {FsmBatteryNoti,  		ROBOT_STATUS_UPGRADE},	/* Battery notification             */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Halt notification                */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Power on notification            */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Battery status query response    */
        {FsmGetCPUIDResp,  		ROBOT_STATUS_UPGRADE},	/* Get CPU ID command response      */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Save Robot SN command response   */
        {FsmReadSNResp,  		ROBOT_STATUS_UPGRADE},	/* Read Robot SN command response   */
        {FsmDatetimeQueryResp,  	ROBOT_STATUS_UPGRADE},	/* Datetime query response          */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Set Datetime command response    */
        {FsmSexQueryResp,  		ROBOT_STATUS_UPGRADE},	/* Sex query response               */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Set sex command response         */
        {FsmUptimeQueryResp,  		ROBOT_STATUS_UPGRADE},	/* Uptime query response            */
        {FsmIgnore,  			ROBOT_STATUS_UPGRADE},	/* Wake notification                */
    },
};

int FsmComputeState(struct RmSysctrl *rm_sysctrl)
{
    int next_state = rm_sysctrl->state;

    switch (rm_sysctrl->state)
    {
        case ROBOT_STATUS_INIT:
        {
            if (is_all_subsys_init(rm_sysctrl))
            {
                if (rm_sysctrl->is_registered)
                    next_state = ROBOT_STATUS_LOGINNING;
                else
                    next_state = ROBOT_STATUS_REGISTER;
            }
            else if (rm_sysctrl->timer[TIMER_INIT_SUBSYS].expire_times
                     >= INIT_SUBSYS_MAX_TIMES)
            {
                RLOGE("It is timeout to init all"
                      " sub-sys into initalized state.");
                next_state = ROBOT_STATUS_ABNORMAL;
            }

            break;
        }
        case ROBOT_STATUS_REGISTER:
        {
            if (rm_sysctrl->is_halt_received)
            {
                if (is_all_subsys_halt(rm_sysctrl))
                {
                    next_state = ROBOT_STATUS_HALT;
                }
                else if (rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_HALT &&
                     !is_all_subsys_halt(rm_sysctrl))
                {
                    RLOGE("Halt command received,"
                          " but not all subsys into halt state.");
                    next_state = ROBOT_STATUS_ABNORMAL;
                    rm_sysctrl->expect_state_expired = false;
                }
            }
            else if (rm_sysctrl->is_registered)
            {
                next_state = ROBOT_STATUS_LOGINNING;
            }

            break;
        }
        case ROBOT_STATUS_LOGINNING:
        {
            if (rm_sysctrl->is_halt_received)
            {
                if (is_all_subsys_halt(rm_sysctrl))
                {
                    next_state = ROBOT_STATUS_HALT;
                }
                else if (rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_HALT &&
                     !is_all_subsys_halt(rm_sysctrl))
                {
                    RLOGE("Halt command received,"
                          " but not all subsys into halt state.");
                    next_state = ROBOT_STATUS_ABNORMAL;
                    rm_sysctrl->expect_state_expired = false;
                }
            }
            else if (!rm_sysctrl->is_login)
            {
                next_state = ROBOT_STATUS_NOT_LOGIN;
            }
            else if (is_all_subsys_work(rm_sysctrl) ||
                     may_tmp_into_work_state(rm_sysctrl))
            {
                next_state = ROBOT_STATUS_WORK;
            }
            else if (rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_WORK &&
                     rm_sysctrl->is_login &&
                     !(is_all_subsys_work(rm_sysctrl) ||
                     may_tmp_into_work_state(rm_sysctrl)))
            {
                RLOGE("It is timeout to wait all sub-sys into work state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            break;
        }
        case ROBOT_STATUS_NOT_LOGIN:
        {
            if (rm_sysctrl->is_halt_received)
            {
                if (is_all_subsys_halt(rm_sysctrl))
                {
                    next_state = ROBOT_STATUS_HALT;
                }
                else if (rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_HALT &&
                     !is_all_subsys_halt(rm_sysctrl))
                {
                    RLOGE("Halt command received,"
                          " but not all subsys into halt state.");
                    next_state = ROBOT_STATUS_ABNORMAL;
                    rm_sysctrl->expect_state_expired = false;
                }
            }
            else if (rm_sysctrl->is_login &&
                     (is_all_subsys_work(rm_sysctrl) ||
                     may_tmp_into_work_state(rm_sysctrl)))
            {
                next_state = ROBOT_STATUS_WORK;
            }
            else if (rm_sysctrl->is_need_relogin)
            {
                next_state = ROBOT_STATUS_LOGINNING;
                rm_sysctrl->is_need_relogin = false;
            }
            else if (rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_WORK &&
                     rm_sysctrl->is_login &&
                     !(is_all_subsys_work(rm_sysctrl) ||
                     may_tmp_into_work_state(rm_sysctrl)))
            {
                RLOGE("It is timeout to wait all sub-sys into work state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            break;
        }
        case ROBOT_STATUS_WORK:
        {
            if (rm_sysctrl->is_halt_received)
            {
                if (is_all_subsys_halt(rm_sysctrl))
                {
                    next_state = ROBOT_STATUS_HALT;
                }
                else if (rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_HALT &&
                     !is_all_subsys_halt(rm_sysctrl))
                {
                    RLOGE("Halt command received,"
                          " but not all subsys into halt state.");
                    next_state = ROBOT_STATUS_ABNORMAL;
                    rm_sysctrl->expect_state_expired = false;
                }
            }
            else if (rm_sysctrl->is_charging &&
                is_all_subsys_charge(rm_sysctrl))
            {
                next_state = ROBOT_STATUS_CHARGE;
            }
            else if (rm_sysctrl->is_charging &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_CHARGE &&  
                     !is_all_subsys_charge(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys into charge state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            else if (rm_sysctrl->is_security_ongoing &&
                is_all_subsys_security(rm_sysctrl))
            {
                next_state = ROBOT_STATUS_SECURITY;
            }
            else if (rm_sysctrl->is_security_ongoing &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_SECURITY &&  
                     !is_all_subsys_security(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys into security state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            else if (rm_sysctrl->is_upgrade_ongoing &&
                is_all_subsys_upgrade(rm_sysctrl))
            {
                next_state = ROBOT_STATUS_UPGRADE;
            }
            else if (rm_sysctrl->is_upgrade_ongoing &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_UPGRADE &&  
                     !is_all_subsys_upgrade(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys into upgrade state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            else if (rm_sysctrl->is_sleeping &&
                is_all_subsys_sleep(rm_sysctrl))
            {
                next_state = ROBOT_STATUS_SLEEP;
            }
#ifdef DEEP_SLEEP_SUPPORT
            else if (rm_sysctrl->is_sleeping_deep &&
                     rm_sysctrl->sub_state == SLEEP_DEEP)
            {
                unsigned int s_poweron = 0;
                unsigned int s_powerdown = rm_sysctrl->gpio->s_powerdown;
                RmSysctrlGpioGetValue(rm_sysctrl->gpio, &s_poweron);
                if (s_powerdown == 0 && s_poweron == 0)
                {
                    next_state = ROBOT_STATUS_SLEEP;
                }
                else if (rm_sysctrl->is_deep_sleep_expired)
                {
                    RLOGE("It is timeout to wait s_poweron = 0.");
                    next_state = ROBOT_STATUS_ABNORMAL;
                    rm_sysctrl->is_deep_sleep_expired = false;
                }
            }
#endif
            else if (rm_sysctrl->is_sleeping &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_SLEEP &&  
                     !is_all_subsys_sleep(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys into sleep state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            else if (!rm_sysctrl->is_charging &&
                     !rm_sysctrl->is_security_ongoing &&
                     !rm_sysctrl->is_upgrade_ongoing &&
                     !rm_sysctrl->is_sleeping &&
                     !0 &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_WORK &&  
                     !is_all_subsys_work(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys into work state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            else if (rm_sysctrl->is_debug_set_received &&
                     rm_sysctrl->is_debug_set_expired &&
                     !is_all_subsys_into_debug_set(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys debug set.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->is_debug_set_expired = false;
            }
            else if (rm_sysctrl->is_mode_set_received &&
                     rm_sysctrl->is_mode_set_expired &&
                     !is_all_subsys_into_mode_set(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys mode set.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->is_mode_set_expired = false;
            }

            break;
        }
        case ROBOT_STATUS_SLEEP:
        {
            if (!rm_sysctrl->is_sleeping &&
                (is_all_subsys_work(rm_sysctrl) ||
                may_tmp_into_work_state(rm_sysctrl))
#ifdef DEEP_SLEEP_SUPPORT
                && !rm_sysctrl->sub_state == SLEEP_DEEP
#endif
               )
            {
                next_state = ROBOT_STATUS_WORK;
            }
#ifdef DEEP_SLEEP_SUPPORT
            else if (!rm_sysctrl->is_sleeping_deep &&
                     rm_sysctrl->sub_state == SLEEP_DEEP)
            {
                unsigned int s_poweron = 0;
                unsigned int s_powerdown = rm_sysctrl->gpio->s_powerdown;
                RmSysctrlGpioGetValue(rm_sysctrl->gpio, &s_poweron);
                if (s_powerdown == 1 && s_poweron == 1)
                {
                    next_state = ROBOT_STATUS_INIT;
                }
                else if (rm_sysctrl->is_deep_wake_expired)
                {
                    RLOGE("It is timeout to wait s_poweron = 1.");
                    next_state = ROBOT_STATUS_ABNORMAL;
                    rm_sysctrl->is_deep_wake_expired = false;
                }
            }
#endif
            else if (!rm_sysctrl->is_sleeping &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_WORK &&  
                     !(is_all_subsys_work(rm_sysctrl)||
                     may_tmp_into_work_state(rm_sysctrl)) )
            {
                RLOGE("It is timeout to wait all sub-sys into work state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            else if (rm_sysctrl->is_sleeping &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_SLEEP &&  
                     !is_all_subsys_sleep(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys into sleep state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            break;
        }
        case ROBOT_STATUS_CHARGE:
        {
            if (rm_sysctrl->is_halt_received)
            {
                if (is_all_subsys_halt(rm_sysctrl))
                {
                    next_state = ROBOT_STATUS_HALT;
                }
                else if (rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_HALT &&
                     !is_all_subsys_halt(rm_sysctrl))
                {
                    RLOGE("Halt command received,"
                          " but not all subsys into halt state.");
                    next_state = ROBOT_STATUS_ABNORMAL;
                    rm_sysctrl->expect_state_expired = false;
                }
            }
            else if (!rm_sysctrl->is_charging &&
                is_all_subsys_work(rm_sysctrl))
            {
                next_state = ROBOT_STATUS_WORK;
            }
            else if (!rm_sysctrl->is_charging &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_WORK &&  
                     !is_all_subsys_work(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys into work state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            else if (rm_sysctrl->is_charging &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_CHARGE &&  
                     !is_all_subsys_charge(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys into charge state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            break;
        }
        case ROBOT_STATUS_ABNORMAL:
        {
            if (rm_sysctrl->is_halt_received)
            {
                if (is_all_subsys_halt(rm_sysctrl))
                {
                    next_state = ROBOT_STATUS_HALT;
                }
                else if (rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_HALT &&
                     !is_all_subsys_halt(rm_sysctrl))
                {
                    RLOGE("Halt command received,"
                          " but not all subsys into halt state.");
                    next_state = ROBOT_STATUS_ABNORMAL;
                    rm_sysctrl->expect_state_expired = false;
                }
            }
            else
            {
                int last_fault;
                RmSysctrlFaultGetLastFaultType(rm_sysctrl->fault, &last_fault);
                if (last_fault == FAULT_SUBSYS_DEAD)
                {
                    unsigned int subsys_idx;
                    struct RmSysctrlSubSystem *subsys;
                    RmSysctrlFaultGetFaultSubsys(rm_sysctrl->fault, &subsys_idx);
                    subsys = &rm_sysctrl->sub_system[subsys_idx];
                    if (is_all_subsys_active(rm_sysctrl))
                        next_state = rm_sysctrl->last_state;

                    else if (!RmSysctrlIsSubSystemPowerOn(subsys) ||
                             subsys_idx == RM_SYSTEM_ID)
                        next_state = ROBOT_STATUS_INIT;
                }
                else if (last_fault == FAULT_EXPECT_STATE_EXPIRE)
                {
                    if (is_all_subsys_into_expect_state(rm_sysctrl))
                        next_state = rm_sysctrl->last_state;
                }
                else if (last_fault == FAULT_INIT_SUBSYS_EXPIRE)
                {
                    if (is_all_subsys_init(rm_sysctrl))
                        next_state = rm_sysctrl->last_state;
                }
#ifdef DEEP_SLEEP_SUPPORT
                else if (last_fault == FAULT_DEEP_SLEEP_EXPIRE)
                {
                    unsigned int s_poweron = 0;
                    unsigned int s_powerdown = rm_sysctrl->gpio->s_powerdown;
                    RmSysctrlGpioGetValue(rm_sysctrl->gpio, &s_poweron);
                    if (s_powerdown == 0 && s_poweron == 0)
                        next_state = ROBOT_STATUS_SLEEP;
                }
                else if (last_fault == FAULT_DEEP_WAKE_EXPIRE)
                {
                    unsigned int s_poweron = 0;
                    unsigned int s_powerdown = rm_sysctrl->gpio->s_powerdown;
                    RmSysctrlGpioGetValue(rm_sysctrl->gpio, &s_poweron);
                    if (s_powerdown == 1 && s_poweron == 1)
                        next_state = ROBOT_STATUS_INIT;
                }
#endif
                else if (last_fault == FAULT_DEBUG_SET_EXPIRE)
                {
                    if (is_all_subsys_into_debug_set(rm_sysctrl))
                        next_state = ROBOT_STATUS_WORK;
                }
                else if (last_fault == FAULT_MODE_SET_EXPIRE)
                {
                    if (is_all_subsys_into_mode_set(rm_sysctrl))
                        next_state = ROBOT_STATUS_WORK;
                }
                else if (last_fault == FAULT_SUBSYS_FAULT)
                {
                    unsigned int subsys_fault_type = 0;
                    RmSysctrlFaultGetSubsysFaultType(rm_sysctrl->fault,
                        &subsys_fault_type);
                    if (subsys_fault_type == (unsigned int)FAULT_NONE)
                        next_state = rm_sysctrl->last_state;
                }
            }
            break;
        }
        case ROBOT_STATUS_HALT:
        {
            break;
        }
        case ROBOT_STATUS_SECURITY:
        {
            if (!rm_sysctrl->is_security_ongoing &&
                (is_all_subsys_work(rm_sysctrl) ||
                may_tmp_into_work_state(rm_sysctrl)))
            {
                next_state = ROBOT_STATUS_WORK;
            }
            else if (!rm_sysctrl->is_security_ongoing &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_WORK &&  
                     !(is_all_subsys_work(rm_sysctrl) ||
                     may_tmp_into_work_state(rm_sysctrl)))
            {
                RLOGE("It is timeout to wait all sub-sys into work state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            else if (rm_sysctrl->is_security_ongoing &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_SECURITY &&  
                     !is_all_subsys_security(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys into security state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            break;
        }
        case ROBOT_STATUS_UPGRADE:
        {
            if (!rm_sysctrl->is_upgrade_ongoing &&
                (is_all_subsys_work(rm_sysctrl) ||
                may_tmp_into_work_state(rm_sysctrl)))
            {
                next_state = ROBOT_STATUS_WORK;
            }
            else if (!rm_sysctrl->is_upgrade_ongoing &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_WORK &&  
                     !(is_all_subsys_work(rm_sysctrl) ||
                     may_tmp_into_work_state(rm_sysctrl)))
            {
                RLOGE("It is timeout to wait all sub-sys into work state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            else if (rm_sysctrl->is_upgrade_ongoing &&
                     rm_sysctrl->expect_state_expired &&
                     rm_sysctrl->expect_state == ROBOT_STATUS_UPGRADE &&  
                     !is_all_subsys_upgrade(rm_sysctrl))
            {
                RLOGE("It is timeout to wait all sub-sys into upgrade state.");
                next_state = ROBOT_STATUS_ABNORMAL;
                rm_sysctrl->expect_state_expired = false;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return next_state;
}

#define ABNORMAL_SUB_SYS_STR(CONDITION, SUB_SYS_STR) \
    do \
    {\
        int i = 0;\
        bool is_first = true;\
        struct RmSysctrlSubSystem *subsys;\
        for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)\
        {\
            subsys = &rm_sysctrl->sub_system[i];\
            if (IS_SUBSYSTEM_ENABLE(subsys))\
            {\
                if ((CONDITION))\
                {\
                    if (is_first)\
                    {\
                        strcpy((SUB_SYS_STR),\
                            RmSysctrlGetSubSystemStr(i));\
                        is_first = false;\
                    }\
                    else\
                    {\
                        strcat((SUB_SYS_STR), ",");\
                        strcat((SUB_SYS_STR),\
                            RmSysctrlGetSubSystemStr(i));\
                    }\
                }\
            }\
        }\
    }\
    while (0)

void FsmChangeState(struct RmSysctrl *rm_sysctrl, int state)
{
    int old_state;

    /* Logging change of state. */
    RLOGD ("FSM: State change %s -> %s",
	RmSysctrlGetStateStr(rm_sysctrl->state),
        RmSysctrlGetStateStr(state));

    old_state = rm_sysctrl->state;
    rm_sysctrl->state = state;
    rm_sysctrl->state_change++;
    rm_sysctrl->last_state = old_state;

    /* Notificate all sub-system the whole system state change */
    RmSysctrlBroadcastSysStateChangeNotiMsg(rm_sysctrl, state);

    if (state == ROBOT_STATUS_LOGINNING)
    {
        /* Send login query to RmScheduler */
        RLOGI("ToCan: Send Login Query to RmScheduler.");
        RmSysctrlCommSendLoginQueryToCan(rm_sysctrl->comm,
                MIDDLE, RM_SCHEDULER);

        /* Broadcast query sub-system state message into can */
        RmSysctrlBroadcastSubsysStateQueryMsg(rm_sysctrl);
        rm_sysctrl->may_retry_msg = COMMON_SYSCTRL_SUBSYS_STATE_QUERY;
    }

    if (state == ROBOT_STATUS_HALT)
    {
        /* Send Power off to Rp */
        RLOGI("ToCan: Send Power off to Rp.");
        RmSysctrlCommSendPoweroffCmdToCan(rm_sysctrl->comm,
                MIDDLE, RP_SYSCTRL, 0x80, 0x00);
    }

    if (state == ROBOT_STATUS_WORK)
    {
        int sys_mode;
        int def_mode;
        bool sys_debug;
        bool def_debug;

        rm_sysctrl->sub_state = WORK_IDLE;
        /* Update battery status
         * Send Battery Status query to Rp
         */
        RLOGI("ToCan: Send Battery Status Query to Rp.");
        RmSysctrlCommSendBatteryStatusQueryToCan(rm_sysctrl->comm,
                MIDDLE, RP_SYSCTRL);

        /* To go into charge state if ac is online */
        if (rm_sysctrl->ac_online == AC_CONNECTED)
        {
            FsmPowerCharge(rm_sysctrl);
        }
        else
        {
            /* If current config mode is different default mode,
             * then sync the mode to sub-systems.
             */
            RmSysctrlGetMode(rm_sysctrl->config, &sys_mode);
            RmSysctrlGetDefaultMode(rm_sysctrl->config, &def_mode);
            if ((old_state == ROBOT_STATUS_LOGINNING ||
                 old_state == ROBOT_STATUS_NOT_LOGIN) &&
                sys_mode != def_mode)
            {
                int ret;

                if (sys_mode == CHILDREN_MODE ||
                    sys_mode == GUARDER_MODE)
                {
                    bool debug_en = false;
                    RmSysctrlGetDebug(rm_sysctrl->config, &debug_en);
                    if (debug_en)
                    {
                        RLOGD("Close the debug as the mode to set is not"
                              " developer mode.");
                        SetDebug(rm_sysctrl, false);
                        usleep(100000);
                    }
                }

                ret = SetMode(rm_sysctrl, sys_mode);
                if (ret < 0)
                   RLOGE("Set mode failed!");
            }

            usleep(100000);
            /* If current config debug is different default debug,
             * then sync the debug to sub-systems.
             */
            RmSysctrlGetDebug(rm_sysctrl->config, &sys_debug);
            RmSysctrlGetDefaultDebug(rm_sysctrl->config, &def_debug);
            if ((old_state == ROBOT_STATUS_LOGINNING ||
                 old_state == ROBOT_STATUS_NOT_LOGIN) &&
                sys_debug != def_debug)
            {
                int ret;
                ret = SetDebug(rm_sysctrl, sys_debug);
                if (ret < 0)
                   RLOGE("Set debug failed!");
            }
        }
    }

    if (state == ROBOT_STATUS_INIT)
    {
        if (old_state == ROBOT_STATUS_ABNORMAL)
        {
            int last_fault;
            RmSysctrlFaultGetLastFaultType(rm_sysctrl->fault, &last_fault);
            if (last_fault == FAULT_SUBSYS_DEAD)
            {
                unsigned int subsys_idx;
                struct RmSysctrlSubSystem *subsys;
                RmSysctrlFaultGetFaultSubsys(rm_sysctrl->fault, &subsys_idx);
                subsys = &rm_sysctrl->sub_system[subsys_idx];
                RESET_SUBSYS_INITALIZED(subsys);
            }
        }
        else if (old_state == ROBOT_STATUS_SLEEP)
        {
             clear_all_subsys_init(rm_sysctrl);
             /* Reconfigure the init timer as 10 seconds */
             RM_SYSCTRL_CONFIG_TIMER(rm_sysctrl, TIMER_INIT_SUBSYS, 10);
        }
        else
        {
             clear_all_subsys_init(rm_sysctrl);
        }
        FsmInitSubsys(rm_sysctrl);
    }

    if (state == ROBOT_STATUS_ABNORMAL)
    {
        struct RmSysctrlFault *fault = rm_sysctrl->fault;

        if (fault->last_fault_type == FAULT_EXPECT_STATE_EXPIRE)
        {
            char sub_sys_str[50] = {0};

            ABNORMAL_SUB_SYS_STR(subsys->state != rm_sysctrl->expect_state,
                sub_sys_str);

            memset(fault->last_fault_desc, 0, sizeof(fault->last_fault_desc));
            sprintf(fault->last_fault_desc, "Subsystems not into %s %s",
                RmSysctrlGetStateStr(rm_sysctrl->expect_state),
                sub_sys_str);
        }
        else if (fault->last_fault_type == FAULT_SUBSYS_FAULT)
        {
            memset(fault->last_fault_desc, 0, sizeof(fault->last_fault_desc));
            sprintf(fault->last_fault_desc, "FaultSubsystem %s,"
                " SubsystemFaultType %d",
                RmSysctrlGetSubSystemStr(fault->fault_subsys),
                fault->subsys_fault_type);
        }
        else if (fault->last_fault_type == FAULT_SUBSYS_DEAD)
        {
            unsigned int subsys_idx;
            RmSysctrlFaultGetFaultSubsys(rm_sysctrl->fault, &subsys_idx);
            memset(fault->last_fault_desc, 0, sizeof(fault->last_fault_desc));
            sprintf(fault->last_fault_desc, "DeadSubsystem %s",
                RmSysctrlGetSubSystemStr(subsys_idx));
        }
        else if (fault->last_fault_type == FAULT_INIT_SUBSYS_EXPIRE)
        {
            char sub_sys_str[50] = {0};

            ABNORMAL_SUB_SYS_STR(!subsys->is_initalized,
                sub_sys_str);

            memset(fault->last_fault_desc, 0, sizeof(fault->last_fault_desc));
            sprintf(fault->last_fault_desc, "Unintialized Subsystem %s",
                 sub_sys_str);
        }
#ifdef DEEP_SLEEP_SUPPORT
        else if (fault->last_fault_type == FAULT_DEEP_SLEEP_EXPIRE)
        {
            memset(fault->last_fault_desc, 0, sizeof(fault->last_fault_desc));
            sprintf(fault->last_fault_desc, "The GPIO response is expired");
        }
        else if (fault->last_fault_type == FAULT_DEEP_WAKE_EXPIRE)
        {
            memset(fault->last_fault_desc, 0, sizeof(fault->last_fault_desc));
            sprintf(fault->last_fault_desc, "The GPIO response is expired");
        }
#endif
        else if (fault->last_fault_type == FAULT_DEBUG_SET_EXPIRE)
        {
            char sub_sys_str[50] = {0};
            bool debug_en = false;

            RmSysctrlGetDebug(rm_sysctrl->config, &debug_en);

            ABNORMAL_SUB_SYS_STR(subsys->debug_en != debug_en,
                sub_sys_str);

            memset(fault->last_fault_desc, 0, sizeof(fault->last_fault_desc));
            sprintf(fault->last_fault_desc, "Set debug failed Subsystem %s",
                 sub_sys_str);
        }
        else if (fault->last_fault_type == FAULT_MODE_SET_EXPIRE)
        {
            char sub_sys_str[50] = {0};
            int sys_mode;

            RmSysctrlGetMode(rm_sysctrl->config, &sys_mode);
            ABNORMAL_SUB_SYS_STR(subsys->mode != (unsigned int)sys_mode,
                sub_sys_str);

            memset(fault->last_fault_desc, 0, sizeof(fault->last_fault_desc));
            sprintf(fault->last_fault_desc, "Set mode failed Subsystem %s",
                 sub_sys_str);
        }
    }

    //The RM upgrade module needs the system state information all the time
    {
        /* Unicast the whole system state change into Rm upgrade */
        RLOGI("ToRmUpgrade: the whole system state change.");
        RmSysctrlCommSendSysStateChangeNotiToCan(rm_sysctrl->comm,
            MIDDLE, RM_UPGRADE, state);
    }

    if (old_state == ROBOT_STATUS_INIT)
    {
        /* All sub-systems initialized, update hdmi state to every sub-sys */
        RLOGD("Update HDMI state to subsys:%d", rm_sysctrl->is_hdmi_plug_in);
        /* Broadcast HDMI state change message into can */
        RmSysctrlBroadcastHDMIStateChangeNotiMsg(rm_sysctrl,
            rm_sysctrl->is_hdmi_plug_in);

        /* Unicast HDMI state change message into sub-systems to avoid that
         * any sub-systems donot receive the HDMI state change message by 
         * broadcasting.
         */
        usleep(200000);
        RmSysctrlUnicastHDMIStateChangeNotiMsg(rm_sysctrl,
            rm_sysctrl->is_hdmi_plug_in);
    }
}

int FsmTimerSet(struct RmSysctrl *rm_sysctrl)
{
    switch (rm_sysctrl->state)
    {
        case ROBOT_STATUS_INIT:
        {
            if (is_all_subsys_init(rm_sysctrl))
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_HEART_BEAT);
            }
            else if (rm_sysctrl->timer[TIMER_INIT_SUBSYS].expire_times
                     >= INIT_SUBSYS_MAX_TIMES)
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_HEART_BEAT);
            }
            else
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_INIT_SUBSYS);
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_HEART_BEAT);
            }

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
            break;
        }
        case ROBOT_STATUS_REGISTER:
        {
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
            RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_HEART_BEAT);
            if (rm_sysctrl->is_halt_received)
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_EXPECT_STATE);
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_EXPECT_STATE);
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_REGISTER);
            }
#ifdef DEEP_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_SLEEP);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_WAKE);
#endif
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEBUG_SET);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_MODE_SET);
#ifdef AUTO_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_AUTO_SLEEP);
#endif
            break;
        }
        case ROBOT_STATUS_LOGINNING:
        {
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
            RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_HEART_BEAT);
            if (rm_sysctrl->is_login || rm_sysctrl->is_halt_received)
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_EXPECT_STATE);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_EXPECT_STATE);
            }
#ifdef DEEP_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_SLEEP);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_WAKE);
#endif
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEBUG_SET);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_MODE_SET);
#ifdef AUTO_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_AUTO_SLEEP);
#endif
            break;
        }
        case ROBOT_STATUS_NOT_LOGIN:
        {
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
            if (rm_sysctrl->is_login || rm_sysctrl->is_halt_received)
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_EXPECT_STATE);
            }
            else
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_RELOGIN);
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_EXPECT_STATE);
            }
            RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_HEART_BEAT);
#ifdef DEEP_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_SLEEP);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_WAKE);
#endif
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEBUG_SET);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_MODE_SET);
#ifdef AUTO_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_AUTO_SLEEP);
#endif

            break;
        }
        case ROBOT_STATUS_WORK:
        {
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
            RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_HEART_BEAT);
            if (rm_sysctrl->is_charging ||
                rm_sysctrl->is_security_ongoing ||
                rm_sysctrl->is_upgrade_ongoing ||
                rm_sysctrl->is_sleeping ||
                rm_sysctrl->is_halt_received ||
                !is_all_subsys_into_expect_state(rm_sysctrl))
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_EXPECT_STATE);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_EXPECT_STATE);
            }
#ifdef DEEP_SLEEP_SUPPORT
            if (rm_sysctrl->is_sleeping_deep &&
                rm_sysctrl->sub_state == SLEEP_DEEP &&
                rm_sysctrl->gpio->s_powerdown == 0 &&
                rm_sysctrl->gpio->s_poweron == 1)
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_DEEP_SLEEP);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_SLEEP);
            }
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_WAKE);
#endif
            if (rm_sysctrl->is_debug_set_received &&
                !is_all_subsys_into_debug_set(rm_sysctrl))
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_DEBUG_SET);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEBUG_SET);
            }
            if (rm_sysctrl->is_mode_set_received &&
                !is_all_subsys_into_mode_set(rm_sysctrl))
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_MODE_SET);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_MODE_SET);
            }
#ifdef AUTO_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_AUTO_SLEEP);
#endif
            break;
        }
        case ROBOT_STATUS_SLEEP:
        {
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_HEART_BEAT);
            if ((!rm_sysctrl->is_sleeping ||
                !is_all_subsys_into_expect_state(rm_sysctrl)) 
#ifdef DEEP_SLEEP_SUPPORT
                && !rm_sysctrl->sub_state == SLEEP_DEEP
#endif
               )
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_EXPECT_STATE);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_EXPECT_STATE);
            }
#ifdef DEEP_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_SLEEP);
            if (!rm_sysctrl->is_sleeping_deep &&
                rm_sysctrl->sub_state == SLEEP_DEEP &&
                rm_sysctrl->gpio->s_powerdown == 1 &&
                rm_sysctrl->gpio->s_poweron == 0)
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_DEEP_WAKE);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_WAKE);
            }
#endif
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEBUG_SET);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_MODE_SET);
#ifdef AUTO_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_AUTO_SLEEP);
#endif
            break;
        }
        case ROBOT_STATUS_CHARGE:
        {
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
            RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_HEART_BEAT);
            if (!rm_sysctrl->is_charging ||
                rm_sysctrl->is_halt_received ||
                !is_all_subsys_into_expect_state(rm_sysctrl))
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_EXPECT_STATE);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_EXPECT_STATE);
            }
#ifdef DEEP_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_SLEEP);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_WAKE);
#endif
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEBUG_SET);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_MODE_SET);
#ifdef AUTO_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_AUTO_SLEEP);
#endif
            break;
        }
        case ROBOT_STATUS_ABNORMAL:
        {
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
            RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_HEART_BEAT);
            if (rm_sysctrl->is_halt_received)
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_EXPECT_STATE);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_EXPECT_STATE);
            }
#ifdef DEEP_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_SLEEP);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_WAKE);
#endif
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEBUG_SET);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_MODE_SET);
#ifdef AUTO_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_AUTO_SLEEP);
#endif
            break;
        }
        case ROBOT_STATUS_HALT:
        {
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_HEART_BEAT);
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
            break;
        }
        case ROBOT_STATUS_SECURITY:
        {
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
            RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_HEART_BEAT);
            if (!rm_sysctrl->is_security_ongoing ||
                !is_all_subsys_into_expect_state(rm_sysctrl))
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_EXPECT_STATE);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_EXPECT_STATE);
            }
#ifdef DEEP_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_SLEEP);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_WAKE);
#endif
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEBUG_SET);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_MODE_SET);
#ifdef AUTO_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_AUTO_SLEEP);
#endif
            break;
        }
        case ROBOT_STATUS_UPGRADE:
        {
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_INIT_SUBSYS);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_REGISTER);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_RELOGIN);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_HEART_BEAT);
            if (!rm_sysctrl->is_upgrade_ongoing ||
                !is_all_subsys_into_expect_state(rm_sysctrl))
            {
                RM_SYSCTRL_TIMER_ON(rm_sysctrl, TIMER_EXPECT_STATE);
            }
            else
            {
                RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_EXPECT_STATE);
            }
#ifdef DEEP_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_SLEEP);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEEP_WAKE);
#endif
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_DEBUG_SET);
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_MODE_SET);
#ifdef AUTO_SLEEP_SUPPORT
            RM_SYSCTRL_TIMER_OFF(rm_sysctrl, TIMER_AUTO_SLEEP);
#endif
            break;
        }
        default:
        {
            break;
        }
    }
    return 0;
}

/* Execute FSM event process. */
int RmSysctrlFsmEvent(struct RmSysctrl *rm_sysctrl,
                      int event)
{
    int ret;
    unsigned int next_state;
    char src_system_id;
    char src_module_id;
    struct RmSysctrlMessage *rscm;

    rscm = (struct RmSysctrlMessage *)rm_sysctrl->ibuf;
    src_system_id = rscm->src_system_id;
    src_module_id = rscm->src_module_id;

    /* Fresh sub-system */
    if (src_module_id != RM_SYSCTRL)
        RmSysctrlFreshSubSystem(&rm_sysctrl->sub_system[(int)src_system_id]);

    next_state = FSM[rm_sysctrl->state][event].next_state;

    /* Call function. */
    ret = (*(FSM[rm_sysctrl->state][event].func))(rm_sysctrl);

    if (ret < 0)
        return ret;        
    
    if (ROBOT_STATUS_COMPUTE == next_state)
        next_state = FsmComputeState(rm_sysctrl);

    /* Logging FSM */
    RLOGI("FSM: %s -> %s (%s)",
        RmSysctrlGetStateStr(rm_sysctrl->state),
        RmSysctrlGetStateStr(next_state),
        rm_sysctrl_fsm_event_str[event]);

    /* If state is changed. */
    if (next_state != rm_sysctrl->state)
        FsmChangeState(rm_sysctrl, next_state);

    /* Make sure timer is set. */
    FsmTimerSet(rm_sysctrl);

    return 0;
}



