/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_subsys.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-3
 Description: This program defined a structure for sub-system, in which
              the status of sub-system will be recored. And you can get
              if the sub-system is active. If the sub-system occurs faults,
              you can query the fault by interfaces.
 Others: None
 
 Function List: 
   1. RmSysctrlInitSubSystem() to initialize the sub-system.
   2. RmSysctrlSetSubSystemStatus() to set the sub-system status.   
   3. RmSysctrlActiveSubSystem() to active the sub-system.
   4. RmSysctrlInctiveSubSystem() to inactive the sub-system.
   5. RmSysctrlIsSubSystemDead() to check if the sub-system is active.
   6. RmSysctrlSetPowerOnSubSystem() to set the sub-system poweron.
   7. RmSysctrlUnsetPowerOnSubSystem() to unset the sub-system poweron.
   8. RmSysctrlIsSubSystemPowerOn() to check if the sub-system is poweron.
   9. RmSysctrlFreshSubSystem() to reset the dead_time for sub-system.
  10. RmSysctrlConfSubSystemExpire to config the dead_time for sub-system.
  11. RmSysctrlUpdateSubSystemLastFault() to update the last fault for 
      sub-system.
  12. RmSysctrlEnableSubSystem() to active the sub-system.
  13. RmSysctrlDisableSubSystem() to inactive the sub-system.
  14. RmSysctrlSetVersion() to set the sub-system version.
 
 History:
   V1.0    2014-9-3    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_SUBSYS_H_
#define RM_SYSCTRL_SUBSYS_H_

#ifdef __cplusplus
extern "C"{
#endif

#define ENABLE_SUBSYSTEM(a)		((a)->enable = true)
#define DISABLE_SUBSYSTEM(a)		((a)->enable = false)
#define IS_SUBSYSTEM_ENABLE(a)  	((a)->enable)

#define SET_POWERON_SUBSYSTEM(a)	((a)->poweron = true)
#define UNSET_POWERON_SUBSYSTEM(a)	((a)->poweron = false)
#define IS_SUBSYSTEM_POWERON(a)  	((a)->poweron)

#define ACTIVE_SUBSYSTEM(a)		((a)->active = true)
#define INACTIVE_SUBSYSTEM(a)		((a)->active = false)
#define IS_SUBSYSTEM_ACTIVE(a)  	((a)->active)

#define SUBSYSTEM_EXPIRE_TIME		40
#define CONFIG_SUBSYS_EXPIRE_TIME(a, t)	((a)->expire_conf = (t))

#define RESET_SUBSYSTEM_TIMER(a)	((a)->expire_time = 0)
#define INC_SUBSYSTEM_TIMER(a)		((a)->expire_time++)
#define IS_SUBSYSTEM_EXPIRE(a)		((a)->expire_time >= (a)->expire_conf)
#define IS_SUBSYSTEM_NEARLY_EXPIRE(a)	((a)->expire_time >= ((a)->expire_conf - 3))

#define SET_SUBSYS_INITALIZED(a)	((a)->is_initalized = true)
#define RESET_SUBSYS_INITALIZED(a)	((a)->is_initalized = false)
#define IS_SUBSYS_INITALIZED(a)  	((a)->is_initalized)

#define SET_SUBSYS_DEBUG(a, b)		((a)->debug_en = b)
#define SET_SUBSYS_MODE(a, b)		((a)->mode = b)

#define SUBSYSTEM_MAX_DEAD_TIMES	255
#define INC_SUBSYSTEM_DEAD_TIMES(a)	((a)->dead_times++)
#define IS_SUBSYS_DEAD_TIMES_REACH_MAX(a)	\
	((a)->dead_times >= SUBSYSTEM_MAX_DEAD_TIMES)

/****************************************************************************
 Name: RmSysctrlInitSubSystem
 Function: This function is to initialize the sub-system from the params.
 Params: subsys - The point of a subsys.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlInitSubSystem(<POINT of SUB-SYSTEM>);
******************************************************************************/
int RmSysctrlInitSubSystem(struct RmSysctrlSubSystem *subsys);

/****************************************************************************
 Name: RmSysctrlSetSubSystemStatus
 Function: This function is to set the sub-system status and sub-status.
 Params: subsys - The point of a subsys.
         status - The status of sub-system to set.
         sub_status - the sub-status of sub-system to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetSubSystemStatus(<POINT of SUB-SYSTEM>,
                                               <STATUS of SUB-SYSTEM>,
                                               <SUB-STATUS of SUB-SYSTEM>);
******************************************************************************/
int RmSysctrlSetSubSystemStatus(struct RmSysctrlSubSystem *subsys,
    unsigned int state,
    unsigned int sub_state);

/****************************************************************************
 Name: RmSysctrlEnableSubSystem
 Function: This function is to enalbe the sub-system from the params.
 Params: subsys - The point of a subsys.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlEnableSubSystem(<POINT of SUB-SYSTEM>);
******************************************************************************/
int RmSysctrlEnableSubSystem(struct RmSysctrlSubSystem *subsys);

/****************************************************************************
 Name: RmSysctrlDisableSubSystem
 Function: This function is to disable the sub-system from the params.
 Params: subsys - The point of a subsys.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlDisableSubSystem(<POINT of SUB-SYSTEM>);
******************************************************************************/
int RmSysctrlDisableSubSystem(struct RmSysctrlSubSystem *subsys);

/****************************************************************************
 Name: RmSysctrlSetPowerOnSubSystem
 Function: This function is to set the sub-system poweron from the params.
 Params: subsys - The point of a subsys.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetPowerOnSubSystem(<POINT of SUB-SYSTEM>);
******************************************************************************/
int RmSysctrlSetPowerOnSubSystem(struct RmSysctrlSubSystem *subsys);

/****************************************************************************
 Name: RmSysctrlUnsetPowerOnSubSystem
 Function: This function is to unset the sub-system poweron from the params.
 Params: subsys - The point of a subsys.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlUnsetPowerOnSubSystem(<POINT of SUB-SYSTEM>);
******************************************************************************/
int RmSysctrlUnsetPowerOnSubSystem(struct RmSysctrlSubSystem *subsys);

/****************************************************************************
 Name: RmSysctrlIsSubSystemPowerOn
 Function: This function is to check if the sub-system from the params if poweron.
 Params: subsys - The point of a subsys.

 Return:
  true  - the sub-system is poweron
  false - the sub-system is not poweron
 Systax:
  [RETURN VALUE] = RmSysctrlIsSubSystemPowerOn(<POINT of SUB-SYSTEM>);
******************************************************************************/
bool RmSysctrlIsSubSystemPowerOn(struct RmSysctrlSubSystem *subsys);

/****************************************************************************
 Name: RmSysctrlActiveSubSystem
 Function: This function is to active the sub-system from the params.
 Params: subsys - The point of a subsys.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlActiveSubSystem(<POINT of SUB-SYSTEM>);
******************************************************************************/
int RmSysctrlActiveSubSystem(struct RmSysctrlSubSystem *subsys);

/****************************************************************************
 Name: RmSysctrlInactiveSubSystem
 Function: This function is to inactive the sub-system from the params.
 Params: subsys - The point of a subsys.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlInactiveSubSystem(<POINT of SUB-SYSTEM>);
******************************************************************************/
int RmSysctrlInactiveSubSystem(struct RmSysctrlSubSystem *subsys);

/****************************************************************************
 Name: RmSysctrlIsSubSystemDead
 Function: This function is to check if the sub-system from the params if dead.
 Params: subsys - The point of a subsys.

 Return:
  true  - the sub-system is dead
  false - the sub-system is not dead
 Systax:
  [RETURN VALUE] = RmSysctrlIsSubSystemDead(<POINT of SUB-SYSTEM>);
******************************************************************************/
bool RmSysctrlIsSubSystemDead(struct RmSysctrlSubSystem *subsys);

/****************************************************************************
 Name: RmSysctrlFreshSubSystem
 Function: This function is to fresh the sub-system by reset the expire_time as
           0.
 Params: subsys - The point of a subsys.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFreshSubSystem(<POINT of SUB-SYSTEM>);
******************************************************************************/
int RmSysctrlFreshSubSystem(struct RmSysctrlSubSystem *subsys);

/****************************************************************************
 Name: RmSysctrlConfSubSystemExpire
 Function: This function is to config the sub-system expire or dead time.
 Params: subsys - The point of a subsys.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlConfSubSystemExpire(<POINT of SUB-SYSTEM>,
                                                <EXPIRE TIME to CONFIG>);
******************************************************************************/
int RmSysctrlConfSubSystemExpire(struct RmSysctrlSubSystem *subsys,
    int expire_conf);

/****************************************************************************
 Name: RmSysctrlConfSubSystemExpire
 Function: This function is to update the sub-system last fault.
 Params: subsys - The point of a subsys.
         last_fault - the latest fault info of the sub-system.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlConfSubSystemExpire(<POINT of SUB-SYSTEM>,
                                                <EXPIRE TIME to CONFIG>);
******************************************************************************/
int RmSysctrlUpdateSubSystemLastFault(struct RmSysctrlSubSystem *subsys,
    struct RmSysctrlFault *last_fault);


/****************************************************************************
 Name: RmSysctrlSetVersion
 Function: This function is to set the sub-system version.
 Params: sub_sys_id - The id of a subsys.
         major - The major version number of sub-system to set.
         minor - the minor version number of sub-system to set.
         revision - the revision number of sub-system to set.
         building_year - the building year of sub-system to set.
         building_month - the building month of sub-system to set.
         building_day - the building day of sub-system to set.
         rom_id - the rom id of sub-system to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetVersion(<POINT of SUB-SYSTEM>,
                                       <MAJOR VERSION NUMBER of SUB-SYSTEM>,
                                       <MINOR VERSION NUMBER of SUB-SYSTEM>,
                                       <REVISION NUMBER of SUB-SYSTEM>,
                                       <BUILDING YEAR of SUB-SYSTEM>,
                                       <BUILDING MONTH of SUB-SYSTEM>,
                                       <BUILDING DAY of SUB-SYSTEM>,
                                       <ROM ID of SUB-SYSTEM>);
******************************************************************************/
int RmSysctrlSetVersion(struct RmSysctrlSubSystem *subsys,
    char sub_sys_id,
    char major,
    char minor,
    char revision,
    unsigned short building_year,
    char building_month,
    char building_day,
    char rom_id);

#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_SUBSYS_H_*/
