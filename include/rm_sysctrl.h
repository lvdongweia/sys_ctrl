/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-4
 Description: This program defined the sysctrl main structure.
 Others: None
 
 Function List: 
   1. RmSysctrlSelf() to get the global Rm Sysctrl.
   2. RmSysctrlIsValidTimer() to check if the timer is valid.
   3. RmSysctrlGetTimerStr() to get the string of timer.
 
 History:
   V1.0    2014-9-4    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_H_
#define RM_SYSCTRL_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "robot_common_data.h"
#include "rm_sysctrl_custom.h"

#define SUCCESS		0
#define FAILED		-1

#define TEXT_SIZE     3072
#define BUF_MAX_SIZE  (TEXT_SIZE + 4)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define SET_STATE(a, b)		((a)->state = (b))
#define CHECK_STATE(a, b)	((a)->state == (b))
#define SET_SUBSTATE(a, b)	((a)->sub_state = (b))
#define CHECK_SUBSTATE(a, b)	((a)->sub_state == (b))

#define SUB_SYSTEM_ID_MAX  8

#define SN_LEN 		18
#define CPUID_LEN  	12
#define MAX_VER_LEN	30

struct RmSysctrlDatetime {
    char year;
    char month;
    char day;
    char weekday;
    char hour;
    char minute;
    char second;
};

struct RmSysctrlVersion {
    char sub_sys_id;
    char major;
    char minor;
    char revision;
    unsigned short building_year;
    char building_month;
    char building_day;
    char rom_id;
};

struct RmSysctrlSubSystem {
    bool enable;
    bool poweron;
    bool active;
    unsigned dead_times;
    unsigned int state;
    unsigned int sub_state;
    int expire_time;
    int expire_conf;
    bool is_initalized;
    struct RmSysctrlFault *last_fault;
    bool debug_en;
    unsigned int mode;
    bool ver_str_used;
    union {
        struct RmSysctrlVersion ver;
        char ver_str[MAX_VER_LEN + 1];
    }u;
};

enum RmSysctrlTimerID {
    TIMER_HEART_BEAT	= 0,
    TIMER_INIT_SUBSYS	= 1,
    TIMER_REGISTER	= 2,
    TIMER_RELOGIN	= 3,
    TIMER_EXPECT_STATE	= 4,
#ifdef DEEP_SLEEP_SUPPORT
    TIMER_DEEP_SLEEP	= 5,
    TIMER_DEEP_WAKE	= 6,
#endif
    TIMER_DEBUG_SET	= 7,
    TIMER_MODE_SET	= 8,
#ifdef AUTO_SLEEP_SUPPORT
    TIMER_AUTO_SLEEP	= 9,
#endif
    TIMER_MAX
};

struct RmSysctrlTimer {
    unsigned int expire_time;
    unsigned int expire_conf;
    unsigned int expire_times;
};

#define HEART_BEAT_EXPIRE_TIME			10
#define INIT_SUBSYS_EXPIRE_TIME			20
#define INIT_SUBSYS_MAX_TIMES			8
#define REGISTER_EXPIRE_TIME			120
#define RELOGIN_EXPIRE_TIME			120
#define EXPECT_STATE_EXPIRE_TIME		10
#ifdef DEEP_SLEEP_SUPPORT
#define DEEP_SLEEP_EXPIRE_TIME			60
#define DEEP_WAKE_EXPIRE_TIME			60
#endif
#define DEBUG_SET_EXPIRE_TIME			10
#define MODE_SET_EXPIRE_TIME			10
#define AUTO_SLEEP_EXPIRE_TIME			300

#define RM_SYSCTRL_CONFIG_TIMER(rsc, tid, t)	\
	((&(rsc)->timer[tid])->expire_conf = t)

#define RM_SYSCTRL_RESET_TIMER(rsc, tid)	\
	((&(rsc)->timer[tid])->expire_time = 0)
#define RM_SYSCTRL_INC_TIMER(rsc, tid)		\
	((&(rsc)->timer[tid])->expire_time++)
#define RM_SYSCTRL_IS_TIMER_EXPIRE(rsc, tid)	\
	((&(rsc)->timer[tid])->expire_time >= (&(rsc)->timer[tid])->expire_conf)
#define RM_SYSCTRL_IS_TIMER_NEARLY_EXPIRE(rsc, tid)	\
	((&(rsc)->timer[tid])->expire_time == (&(rsc)->timer[tid])->expire_conf - 3)
#define RM_SYSCTRL_IS_TIMER_TOO_LONG(rsc, tid)	\
	((&(rsc)->timer[tid])->expire_time == (4))
#define RM_SYSCTRL_IS_TIMER_NOT_IMMEDIATELY(rsc, tid)	\
	((&(rsc)->timer[tid])->expire_time == (2))
#define RM_SYSCTRL_RESET_EXPIRE_TIMES(rsc, tid)	\
	((&(rsc)->timer[tid])->expire_times = 0)
#define RM_SYSCTRL_INC_EXPIRE_TIMES(rsc, tid)	\
	((&(rsc)->timer[tid])->expire_times++)

#define RM_SYSCTRL_TIMER_OFF(rsc, tid)					\
	do								\
	{								\
		((rsc)->timer_flag = (rsc)->timer_flag & ~(1 << (tid)));\
		((&(rsc)->timer[tid])->expire_time = 0);		\
		((&(rsc)->timer[tid])->expire_times = 0);		\
	} while(0)
#define RM_SYSCTRL_TIMER_ON(rsc, tid)	\
	((rsc)->timer_flag = (rsc)->timer_flag | (1 << (tid)))
#define RM_SYSCTRL_TIMER_IS_ON(rsc, tid)\
	((rsc)->timer_flag & (1 << (tid)))

#define RM_SYSCTRL_DEFAULT_CAN_PRIVORITY

#define MAX_UNCHANGED_DATA_UPDATE_TIMES  10

enum RmSysctrlSex {
    INVALID_SEX 	= 0,
    MALE 		= 1,
    FEMALE 		= 2,
    UNKNOWN_SEX 	= 0xFF
};

struct RmSysctrl {
    /* Status */
    unsigned int state;
    unsigned int sub_state;
    int state_change;
    unsigned int expect_state;
    bool expect_state_expired;
    unsigned int last_state;

    /* Events */
    char ibuf[BUF_MAX_SIZE];
    char obuf[BUF_MAX_SIZE];
    struct RmSysctrlComm *comm;

    /* Sub-system */
    struct RmSysctrlSubSystem sub_system[SUB_SYSTEM_ID_MAX];
    bool is_registered;
    bool is_login;
    bool is_need_relogin;

    /* Fault */
    struct RmSysctrlFault *fault;

    /* Can */
    struct RmSysctrlCan *can;

    /* Cli */
    pthread_t ntid_cli;

    /* Timer */
    pthread_t ntid_timer;
    struct RmSysctrlTimer timer[TIMER_MAX];
    long timer_flag;

    /* Others */
    bool is_halt_received;
    bool is_charging;
    bool is_security_ongoing;
    bool is_upgrade_ongoing;
    bool is_sleeping;
#ifdef DEEP_SLEEP_SUPPORT
    bool is_sleeping_deep;
    bool is_deep_sleep_expired;
    bool is_deep_wake_expired;
#endif
    bool is_debug_set_received;
    bool is_debug_set_expired;
    bool is_mode_set_received;
    bool is_mode_set_expired;
    unsigned short unchanged_data_updated_times;

#ifdef DEEP_SLEEP_SUPPORT
    /* GPIO */
    struct RmSysctrlGpio *gpio;
#endif

    /* Power */
    bool is_low_power;
    enum {
        SYS_POWER_SUPPLY_TYPE_UNKNOWN 		= 0,
        SYS_POWER_SUPPLY_TYPE_BATTERY 		= 1,
        SYS_POWER_SUPPLY_TYPE_UPS 		= 2,
        SYS_POWER_SUPPLY_TYPE_MAINS 		= 3,
        SYS_POWER_SUPPLY_TYPE_USB 		= 4,
        SYS_POWER_SUPPLY_TYPE_USB_DCP 		= 5,
        SYS_POWER_SUPPLY_TYPE_USB_CDP 		= 6,
        SYS_POWER_SUPPLY_TYPE_USB_ACA 		= 7,
        SYS_POWER_SUPPLY_TYPE_WIRELESS 		= 8,
    } power_supply_type;
    enum {
        AC_DISCONNECTED = 0,
        AC_CONNECTED    = 1,
    } ac_online;
    enum {
        SYS_BATTERY_STATUS_UNKNOWN 		= 0,
        SYS_BATTERY_STATUS_CHARGING 		= 1,
        SYS_BATTERY_STATUS_DISCHARGING 		= 2,
        SYS_BATTERY_STATUS_NOT_CHARGING 	= 3,
        SYS_BATTERY_STATUS_FULL 		= 4,
    } battery_status;
    enum {
        SYS_BATTERY_CHARGE_UNKNOWN 		= 0,
        SYS_BATTERY_CHARGE_NA 			= 1,
        SYS_BATTERY_CHARGE_TRICKLE 		= 2,
        SYS_BATTERY_CHARGE_FAST 		= 3,
    } battery_charge_type;
    enum {
        SYS_BATTERY_HEALTH_UNKNOWN 		= 0,
        SYS_BATTERY_HEALTH_GOOD 		= 1,
        SYS_BATTERY_HEALTH_OVERHEAT 		= 2,
        SYS_BATTERY_HEALTH_DEAD 		= 3,
        SYS_BATTERY_HEALTH_OVER_VOLTAGE 	= 4,
        SYS_BATTERY_HEALTH_UNSPECIFIED_FAILURE 	= 5,
        SYS_BATTERY_HEALTH_COLD 		= 6,
    } health_status;
    enum {
        SYS_BATTERY_IN_USE = 1,
        SYS_BATTERY_UN_USE = 0,
    } battery_present;
    unsigned int battery_capacity; //percentage 0-100
    enum {
        SYS_BATTERY_CAPACITY_LEVEL_UNKNOWN	= 0,
        SYS_BATTERY_CAPACITY_LEVEL_CRITICAL 	= 1,
        SYS_BATTERY_CAPACITY_LEVEL_LOW 		= 2,
        SYS_BATTERY_CAPACITY_LEVEL_NORMAL 	= 3,
        SYS_BATTERY_CAPACITY_LEVEL_HIGH 	= 4,
        SYS_BATTERY_CAPACITY_LEVEL_FULL 	= 5,
    } battery_capacity_level;
    unsigned long battery_voltage; //uV
    unsigned short battery_temp;   // oC/100
    enum {
        SYS_BATTERY_TECHNOLOGY_UNKNOWN 		= 0,
        SYS_BATTERY_TECHNOLOGY_NIMH 		= 1,
        SYS_BATTERY_TECHNOLOGY_LI_ION 		= 2,
        SYS_BATTERY_TECHNOLOGY_LI_POLY 		= 3,
        SYS_BATTERY_TECHNOLOGY_LIFE 		= 4,
        SYS_BATTERY_TECHNOLOGY_NICD 		= 5,
        SYS_BATTERY_TECHNOLOGY_LIMN 		= 6,
    } battery_technology;

    /* Resend message for reliability */
    unsigned int may_retry_msg;

    /* Config */
    struct RmSysctrlCnf *config;

    /* SN */
    char cpuid[CPUID_LEN + 1];
    char sn[SN_LEN + 1];
    bool is_valid_sn;

    /* Datetime */
    struct RmSysctrlDatetime datetime;

    /* Sex */
    enum RmSysctrlSex sex;

    /* Uptime*/
    unsigned int uptime;

    /* Srv */
    pthread_t ntid_srv;

    /* Uevent */
    pthread_t ntid_uevent;

    /* HDMI */
    bool is_hdmi_plug_in;
};

/****************************************************************************
 Name: RmSysctrlSelf
 Function: This function is to get the global rm sysctrl.
 Params: None

 Return:
   The pointer of global rm sysctrl

 Systax:
  [RETURN VALUE] = RmSysctrlSelf();
******************************************************************************/
struct RmSysctrl *RmSysctrlSelf();

/****************************************************************************
 Name: RmSysctrlIsValidTimer
 Function: This function is to check if the timer is valid.
 Params: timer - robot timer

 Return:
   ture  - the timer is valid
   false - the timer is not valid
 Systax:
  [RETURN VALUE] = RmSysctrlIsValidTimer(<TIMER>);
******************************************************************************/
bool RmSysctrlIsValidTimer(int timer);

/****************************************************************************
 Name: RmSysctrlGetTimerStr
 Function: This function is to get the string of timer.
 Params: timer - robot timer

 Return:
   The point of timer string
 Systax:
  [RETURN VALUE] = RmSysctrlGetTimerStr(<TIMER>);
******************************************************************************/
char* RmSysctrlGetTimerStr(int timer);

#define UNICAST_MSG_TO_SUBSYS_DECLARE(T) \
	void RmSysctrlUnicast##T##Msg(struct RmSysctrl *rm_sysctrl);
#define UNICAST_MSG_TO_SUBSYS2_DECLARE(T, TYPE) \
	void RmSysctrlUnicast##T##Msg(struct RmSysctrl *rm_sysctrl, TYPE v);
#define BROADCAST_MSG_TO_SUBSYS_DECLARE(T) \
	void RmSysctrlBroadcast##T##Msg(struct RmSysctrl *rm_sysctrl);
#define BROADCAST_MSG_TO_SUBSYS2_DECLARE(T, TYPE) \
	void RmSysctrlBroadcast##T##Msg(struct RmSysctrl *rm_sysctrl, TYPE v);

#define UNICAST_MSG_TO_SUBSYS(T) \
	void RmSysctrlUnicast##T##Msg(struct RmSysctrl *rm_sysctrl) \
	{ \
    		int i = 0; \
				\
    		for (i = 0; i < SUB_SYSTEM_ID_MAX; i++) \
    		{ \
        		struct RmSysctrlSubSystem *subsys = \
            			&rm_sysctrl->sub_system[i]; \
								\
        		if (IS_SUBSYSTEM_ENABLE(subsys)) \
        		{ \
            			char dst_module_id; \
            			if (i == 0) \
                			dst_module_id = RM_SCHEDULER; \
            			else \
                			dst_module_id = i << 4; \
									\
           			RLOGI("ToCan: Unicast %s message.", #T); \
            			RmSysctrlCommSend##T##ToCan(rm_sysctrl->comm, \
                			MIDDLE, dst_module_id); \
									\
            			usleep(250000); \
        		} \
    		} \
	}

#define UNICAST_MSG_TO_SUBSYS2(T, TYPE) \
	void RmSysctrlUnicast##T##Msg(struct RmSysctrl *rm_sysctrl, TYPE v) \
	{ \
    		int i = 0; \
				\
    		for (i = 0; i < SUB_SYSTEM_ID_MAX; i++) \
    		{ \
        		struct RmSysctrlSubSystem *subsys = \
            			&rm_sysctrl->sub_system[i]; \
								\
        		if (IS_SUBSYSTEM_ENABLE(subsys)) \
        		{ \
            			char dst_module_id; \
            			if (i == 0) \
                			dst_module_id = RM_SCHEDULER; \
            			else \
                			dst_module_id = i << 4; \
									\
           			RLOGI("ToCan: Unicast %s message.", #T); \
            			RmSysctrlCommSend##T##ToCan(rm_sysctrl->comm, \
                			MIDDLE, dst_module_id, v); \
									\
            			usleep(250000); \
        		} \
    		} \
	}

#define BROADCAST_MSG_TO_SUBSYS(T) \
	void RmSysctrlBroadcast##T##Msg(struct RmSysctrl *rm_sysctrl) \
	{ \
        	RLOGI("ToCan: Broadcast %s message.", #T); \
        	RmSysctrlCommSend##T##ToCan(rm_sysctrl->comm, \
            		MIDDLE, ROBOT_RADIOCAST); \
							\
        	if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[RM_SYSTEM_ID])) \
        	{ \
            		RLOGI("ToRmSchedule: %s message.", #T); \
            		RmSysctrlCommSend##T##ToCan(rm_sysctrl->comm, \
                		MIDDLE, RM_SCHEDULER); \
        	} \
	}

#define BROADCAST_MSG_TO_SUBSYS2(T, TYPE) \
	void RmSysctrlBroadcast##T##Msg(struct RmSysctrl *rm_sysctrl, TYPE v) \
	{ \
        	RLOGI("ToCan: Broadcast %s message.", #T); \
        	RmSysctrlCommSend##T##ToCan(rm_sysctrl->comm, \
            		MIDDLE, ROBOT_RADIOCAST, v); \
							\
        	if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[RM_SYSTEM_ID])) \
        	{ \
            		RLOGI("ToRmSchedule: %s message.", #T); \
            		RmSysctrlCommSend##T##ToCan(rm_sysctrl->comm, \
                		MIDDLE, RM_SCHEDULER, v); \
        	} \
	}

#define BROADCAST_MSG_TO_SUBSYS_VIA_UNICAST_ALL(T) \
	void RmSysctrlBroadcast##T##Msg(struct RmSysctrl *rm_sysctrl) \
	{ \
        	RmSysctrlUnicast##T##Msg(rm_sysctrl); \
	}

#define BROADCAST_MSG_TO_SUBSYS2_VIA_UNICAST_ALL(T, TYPE) \
	void RmSysctrlBroadcast##T##Msg(struct RmSysctrl *rm_sysctrl, TYPE v) \
	{ \
        	RmSysctrlUnicast##T##Msg(rm_sysctrl, v); \
	}

UNICAST_MSG_TO_SUBSYS_DECLARE(InitCmd)
UNICAST_MSG_TO_SUBSYS2_DECLARE(SleepCmd, int)
UNICAST_MSG_TO_SUBSYS_DECLARE(WakeCmd)
UNICAST_MSG_TO_SUBSYS_DECLARE(ChargeNoti)
UNICAST_MSG_TO_SUBSYS_DECLARE(ExitChargeNoti)
UNICAST_MSG_TO_SUBSYS_DECLARE(SecurityCmd)
UNICAST_MSG_TO_SUBSYS_DECLARE(ExitSecurityCmd)
UNICAST_MSG_TO_SUBSYS_DECLARE(UpgradeCmd)
UNICAST_MSG_TO_SUBSYS_DECLARE(ExitUpgradeCmd)
UNICAST_MSG_TO_SUBSYS_DECLARE(LowPowerNoti)
UNICAST_MSG_TO_SUBSYS_DECLARE(SubsysStateQuery)
UNICAST_MSG_TO_SUBSYS_DECLARE(HaltCmd)
UNICAST_MSG_TO_SUBSYS2_DECLARE(DebugCmd, unsigned int)
UNICAST_MSG_TO_SUBSYS2_DECLARE(ModeCmd, unsigned int)
UNICAST_MSG_TO_SUBSYS2_DECLARE(SysStateChangeNoti, unsigned int)
UNICAST_MSG_TO_SUBSYS_DECLARE(VersionQuery)
UNICAST_MSG_TO_SUBSYS2_DECLARE(HDMIStateChangeNoti, bool)

BROADCAST_MSG_TO_SUBSYS_DECLARE(InitCmd)
BROADCAST_MSG_TO_SUBSYS2_DECLARE(SleepCmd, char)
BROADCAST_MSG_TO_SUBSYS_DECLARE(WakeCmd)
BROADCAST_MSG_TO_SUBSYS_DECLARE(ChargeNoti)
BROADCAST_MSG_TO_SUBSYS_DECLARE(ExitChargeNoti)
BROADCAST_MSG_TO_SUBSYS_DECLARE(SecurityCmd)
BROADCAST_MSG_TO_SUBSYS_DECLARE(ExitSecurityCmd)
BROADCAST_MSG_TO_SUBSYS_DECLARE(UpgradeCmd)
BROADCAST_MSG_TO_SUBSYS_DECLARE(ExitUpgradeCmd)
BROADCAST_MSG_TO_SUBSYS_DECLARE(LowPowerNoti)
BROADCAST_MSG_TO_SUBSYS_DECLARE(SubsysStateQuery)
BROADCAST_MSG_TO_SUBSYS_DECLARE(HaltCmd)
BROADCAST_MSG_TO_SUBSYS2_DECLARE(DebugCmd, unsigned int)
BROADCAST_MSG_TO_SUBSYS2_DECLARE(ModeCmd, unsigned int)
BROADCAST_MSG_TO_SUBSYS2_DECLARE(SysStateChangeNoti, unsigned int)
BROADCAST_MSG_TO_SUBSYS_DECLARE(VersionQuery)
BROADCAST_MSG_TO_SUBSYS2_DECLARE(HDMIStateChangeNoti, bool)

#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_H_*/
