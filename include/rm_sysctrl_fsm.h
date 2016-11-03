/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_fsm.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-3
 Description: This program defined Finite Status Machine for robot system
              control.
 Others: None
 
 Function List: 
   1. RmSysctrlFsmEvent() to handle the events from sub-system or internal.
   2. RmSysctrlFsmInit() to initalize the FSM.
   3. RmSysctrlUnicastLastMsgForExpectState() to resend a message for expect
      state.
   4. RmSysctrlUnicastLastMsgForDebugSet() to resend a message for debug set.
   5. RmSysctrlUnicastLastMsgForModeSet() to resend a message for mode set.
   6. RmSysctrlIsValidState() to check if the state is valid.
   7. RmSysctrlGetStateStr() to get the string of state.
   8. RmSysctrlIsValidSubSystem() to check if the sub-system is valid.
   9. RmSysctrlGetSubSystemStr() to get the string of sub-system.
  10. RmSysctrlIsValidMode() to check if the mode is valid.
  11. RmSysctrlGetModeStr() to get the string of mode.
  12. RmSysctrlIsValidWorkSubState() to check if the work sub-state is valid.
  13. RmSysctrlGetWorkSubStateStr() to get the string of work sub-state.
  14. RmSysctrlIsValidSleepSubState() to check if the sleep sub-state is valid.
  15. RmSysctrlGetSleepSubStateStr() to get the string of sleep sub-state.
  16. SetDatetimeIntoSystem() to set the datetime into system.
 
 History:
   V1.0    2014-9-3    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_FSM_H_
#define RM_SYSCTRL_FSM_H_

#ifdef __cplusplus
extern "C"{
#endif

/*Status*/
enum RobotStatus {
    ROBOT_STATUS_COMPUTE 	= 0x00,
    ROBOT_STATUS_INIT 		= 0x01,
    ROBOT_STATUS_REGISTER 	= 0x02,
    ROBOT_STATUS_LOGINNING 	= 0x03,
    ROBOT_STATUS_NOT_LOGIN 	= 0x04,
    ROBOT_STATUS_WORK 		= 0x05,
    ROBOT_STATUS_SLEEP 		= 0x06,
    ROBOT_STATUS_CHARGE 	= 0x07,
    ROBOT_STATUS_ABNORMAL 	= 0x08,
    ROBOT_STATUS_HALT 		= 0x09,
    ROBOT_STATUS_SECURITY 	= 0x0A,
    ROBOT_STATUS_UPGRADE 	= 0x0B,
    ROBOT_STATUS_MAX
};

#define ROBOT_STATUS_NONE	-1

/* The priority of state to go into at work state is:
   HALT > CHARGE & SECURITY &  SLEEP
*/

enum RobotWorkSubStatus
{
    WORK_IDLE = 0,
    WORK_BUSY
};

enum RobotSleepSubStatus
{
    SLEEP_SHALLOW = 0,
#ifdef DEEP_SLEEP_SUPPORT
    SLEEP_DEEP
#endif
};

typedef enum RmSysctrlMsgs RmSysctrlEvents;
#define RM_SYSCTRL_EVENTS_MAX RM_SYSCTRL_MSGS_MAX

/****************************************************************************
 Name: RmSysctrlFsmInit
 Function: This function is to initalize the FSM.
 Params: rm_sysctrl - a structure pointer to manager the sysctrl.
         event - the sysctrl event received from sub-system or internal.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFsmInit(<POINT of RM SYSCTRL>);
******************************************************************************/
int RmSysctrlFsmInit(struct RmSysctrl *rm_sysctrl);

/****************************************************************************
 Name: RmSysctrlFsmEvent
 Function: This function is to handle sysctrl events in the fsm.
 Params: rm_sysctrl - a structure pointer to manager the sysctrl.
         event - the sysctrl event received from sub-system or internal.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFsmEvent(<POINT of RM SYSCTRL>, <EVENT>);
******************************************************************************/
int RmSysctrlFsmEvent(struct RmSysctrl *rm_sysctrl, int event);

/****************************************************************************
 Name: RmSysctrlUnicastLastMsgForExpectState
 Function: This function is to resend a message for expect state.
 Params: rm_sysctrl - a structure pointer to manager the sysctrl.

 Return:
   None
 Systax:
  [RETURN VALUE] = RmSysctrlUnicastLastMsgForExpectState(<POINT of RM SYSCTRL>);
******************************************************************************/
void RmSysctrlUnicastLastMsgForExpectState(struct RmSysctrl *rm_sysctrl);

/****************************************************************************
 Name: RmSysctrlUnicastLastMsgForDebugSet
 Function: This function is to resend a message for debug set.
 Params: rm_sysctrl - a structure pointer to manager the sysctrl.

 Return:
   None
 Systax:
  [RETURN VALUE] = RmSysctrlUnicastLastMsgForDebugSet(<POINT of RM SYSCTRL>);
******************************************************************************/
void RmSysctrlUnicastLastMsgForDebugSet(struct RmSysctrl *rm_sysctrl);

/****************************************************************************
 Name: RmSysctrlUnicastLastMsgForModeSet
 Function: This function is to resend a message for mode set.
 Params: rm_sysctrl - a structure pointer to manager the sysctrl.

 Return:
   None
 Systax:
  [RETURN VALUE] = RmSysctrlUnicastLastMsgForModeSet(<POINT of RM SYSCTRL>);
******************************************************************************/
void RmSysctrlUnicastLastMsgForModeSet(struct RmSysctrl *rm_sysctrl);

/****************************************************************************
 Name: RmSysctrlIsValidState
 Function: This function is to check if the state is valid.
 Params: state - robot state

 Return:
   ture  - the state is valid
   false - the state is not valid
 Systax:
  [RETURN VALUE] = RmSysctrlIsValidState(<STATE>);
******************************************************************************/
bool RmSysctrlIsValidState(int state);

/****************************************************************************
 Name: RmSysctrlGetStateStr
 Function: This function is to get the string of state.
 Params: state - robot state

 Return:
   The point of state string
 Systax:
  [RETURN VALUE] = RmSysctrlGetStateStr(<STATE>);
******************************************************************************/
char* RmSysctrlGetStateStr(int state);

/****************************************************************************
 Name: RmSysctrlIsValidSubSystem
 Function: This function is to check if the sub-system id is valid.
 Params: sub_sys_id - robot sub-system id

 Return:
   ture  - the system id is valid
   false - the system id is not valid
 Systax:
  [RETURN VALUE] = RmSysctrlIsValidSubSystem(<SUB_SYS_ID>);
******************************************************************************/
bool RmSysctrlIsValidSubSystem(int sub_sys_id);

/****************************************************************************
 Name: RmSysctrlGetSubSystemStr
 Function: This function is to get the string of sub-system.
 Params: sub_sys_id - robot sub-system id

 Return:
   The point of sub-system string
 Systax:
  [RETURN VALUE] = RmSysctrlGetSubSystemStr(<SUB_SYS_ID>);
******************************************************************************/
char* RmSysctrlGetSubSystemStr(int sub_sys_id);

/****************************************************************************
 Name: RmSysctrlIsValidMode
 Function: This function is to check if the mode is valid.
 Params: mode - robot mode

 Return:
   ture  - the mode is valid
   false - the mode is not valid
 Systax:
  [RETURN VALUE] = RmSysctrlIsValidMode(<MODE>);
******************************************************************************/
bool RmSysctrlIsValidMode(int mode);

/****************************************************************************
 Name: RmSysctrlGetModeStr
 Function: This function is to get the string of mode.
 Params: mode - robot mode

 Return:
   The point of mode string
 Systax:
  [RETURN VALUE] = RmSysctrlGetModeStr(<MODE>);
******************************************************************************/
char* RmSysctrlGetModeStr(int mode);

/****************************************************************************
 Name: RmSysctrlIsValidWorkSubState
 Function: This function is to check if the work sub-state is valid.
 Params: sub_state - robot work sub-state

 Return:
   ture  - the sub-state is valid
   false - the sub-state is not valid
 Systax:
  [RETURN VALUE] = RmSysctrlIsValidWorkSubState(<SUB_STATE>);
******************************************************************************/
bool RmSysctrlIsValidWorkSubState(int sub_state);

/****************************************************************************
 Name: RmSysctrlGetWorkSubStateStr
 Function: This function is to get the string of work sub-state.
 Params: sub-state - robot work sub-state

 Return:
   The point of sub-state string
 Systax:
  [RETURN VALUE] = RmSysctrlGetWorkSubStateStr(<SUB_STATE>);
******************************************************************************/
char* RmSysctrlGetWorkSubStateStr(int sub_state);

/****************************************************************************
 Name: RmSysctrlIsValidSleepSubState
 Function: This function is to check if the sleep sub-state is valid.
 Params: sub_state - robot sleep sub-state

 Return:
   ture  - the sub-state is valid
   false - the sub-state is not valid
 Systax:
  [RETURN VALUE] = RmSysctrlIsValidSleepSubState(<SUB_STATE>);
******************************************************************************/
bool RmSysctrlIsValidSleepSubState(int sub_state);

/****************************************************************************
 Name: RmSysctrlGetSleepSubStateStr
 Function: This function is to get the string of sleep sub-state.
 Params: sub-state - robot sleep sub-state

 Return:
   The point of sub-state string
 Systax:
  [RETURN VALUE] = RmSysctrlGetSleepSubStateStr(<SUB_STATE>);
******************************************************************************/
char* RmSysctrlGetSleepSubStateStr(int sub_state);

/****************************************************************************
 Name: SetDatetimeIntoSystem
 Function: This function is to set the datetime into system.
 Params: dt - datetime structure

 Return:
   0  - successful
  -1  - failed
 Systax:
  [RETURN VALUE] = RmSysctrlGetSleepSubStateStr(<SUB_STATE>);
******************************************************************************/
int SetDatetimeIntoSystem(struct RmSysctrlDatetime *dt);

#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_FSM_H_*/
