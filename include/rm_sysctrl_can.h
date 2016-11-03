/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_can.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-4
 Description: This program defined the can interfaces.
 Others: None
 
 Function List: 
   1. RmSysctrlInitCan() to initialize the can.
   2. RmSysctrlCanDataSend() to send the data onto can.

 
 History:
   V1.0    2014-9-4    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_CAN_H_
#define RM_SYSCTRL_CAN_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "robot_common_data.h"

struct RmSysctrlCan {
    struct can_client_callback *callback;
};

struct RmSysctrlCanPacket {
    int priority;  	/* Priority, divided into class 0-6,
			 * the default is 3. 0 represents the
			 * highest priority.
			 */
    int s;      	/* Source module number */
    int d;      	/* Destination module number */
    void *pData;	/* Packet buffer, different modules
			 * defined by own for interaction
			 */
    int len;		/* Packet buffer length */
};

enum RmSysctrlRobotModuleID {
    /* Human and machine
     * interaction center
     */
    RM_SYSCTRL		= RM_SYSCTRL_ID,
    RM_MOTOR		= RM_MOTOR_ID,
    RM_FM	 	= RM_FAULT_ID,
    RM_DEBUG		= RM_DEBUG_ID,
    RM_FACE		= RM_FACE_ID,
    RM_SENSOR		= RM_SENSOR_ID,
    RM_SCHEDULER	= RM_SCHEDULER_ID,
    RM_POWER		= RM_POWER_ID,
    RM_DEBUG_CTRL	= RM_DEBUG_CTRL_ID,
    RM_FACTORY_TEST	= RM_FACTORY_TEST_ID,
    RM_UPGRADE		= RM_UPGRADE_ID,

    /* Robot control center */
    RC_SYSCTRL		= RC_SYS_CTRL_ID,
    RC_CONTROL		= RC_BASE_CTRL_ID,
    RC_TRAJECTORY	= RC_TRAJ_CTRL_ID,
    RC_TASK		= RC_TASK_CTRL_ID,
    RC_SENSOR		= RC_SENSOR_ID,
    RC_FAULT		= RC_FAULT_ID,
    RC_DEBUG		= RC_DEBUG_ID,
    RC_EVENT_DETECT	= RC_EVENT_DETECT_ID,
    RC_UPGRADE		= RC_UPGRADE_ID,

    /* Power management */
    RP_SYSCTRL		= RP_SYSCTRL_ID,
    RP_CHARGE		= RP_CHARGE_ID,
    RP_CONTROL		= RP_CONTROL_ID, 
    RP_FAULT		= RP_FAULT_ID,
    RP_DEBUG		= RP_DEBUG_ID,
    RP_UPGRADE		= RP_UPGRADE_ID,

    /* Face control */
    RF_SYSCTRL		= RF_SYSCTRL_ID,
    RF_SENSOR		= RF_SENSOR_ID,
    RF_DISPLAY		= RF_DISPLAY_ID,
    RF_FAULT		= RF_FAULT_ID,
    RF_DEBUG		= RF_DEBUG_ID,
    RF_UPGRADE		= RF_UPGRADE_ID,

    /* The right arm motor
     * servo control
     */
    RB_R_ARM_SYSCTRL	= RB_R_ARM_SYS_CTRL_ID,
    RB_R_ARM_CONTROL	= RB_R_ARM_MOTOR_CTRL_ID,
    RB_R_ARM_SENSOR	= RB_R_ARM_SENSOR_ID,
    RB_R_ARM_DEBUG	= RB_R_ARM_DEBUG_ID,
    RB_R_ARM_FAULT	= RB_R_ARM_FAULT_ID,
    RB_R_ARM_EVENT	= RB_R_ARM_EVENT_DETECT_ID,
    RB_R_ARM_UPGRADE	= RB_R_ARM_UPGRADE_ID,

    /* The left arm motor
     * servo control
     */
    RB_L_ARM_SYSCTRL	= RB_L_ARM_SYS_CTRL_ID,
    RB_L_ARM_CONTROL	= RB_L_ARM_MOTOR_CTRL_ID,
    RB_L_ARM_SENSOR	= RB_L_ARM_SENSOR_ID,
    RB_L_ARM_DEBUG	= RB_L_ARM_DEBUG_ID,
    RB_L_ARM_FAULT	= RB_L_ARM_FAULT_ID,
    RB_L_ARM_EVENT	= RB_L_ARM_EVENT_DETECT_ID,
    RB_L_ARM_UPGRADE	= RB_L_ARM_UPGRADE_ID,

    /* Head waist bottom
     * wheel motor servo
     * control
     */
    RB_BODY_SYSCTRL	= RB_BODY_SYS_CTRL_ID,
    RB_BODY_CONTROL	= RB_BODY_MOTOR_CTRL_ID,
    RB_BODY_SENSOR	= RB_BODY_SENSOR_ID,
    RB_BODY_DEBUG	= RB_BODY_DEBUG_ID,
    RB_BODY_FAULT	= RB_BODY_FAULT_ID,
    RB_BODY_EVENT	= RB_BODY_EVENT_DETECT_ID,
    RB_BODY_UPGRADE	= RB_BODY_UPGRADE_ID,

    /* Broadcast */
    ROBOT_RADIOCAST 	= ROBOT_RADIOCAST_ID,
};

enum RmSysctrlCanPriority {
    HIGH	= ROBOT_CAN_LEVEL0,
    MIDDLE	= ROBOT_CAN_LEVEL3,
    LOW		= ROBOT_CAN_LEVEL6
};

/* Init */
/****************************************************************************
 Name: RmSysctrlInitCan
 Function: This function is to initialize the can from the params.
 Params: can_addr - The point of point of the can.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlInitCan(<POINT of POINT of CAN>);
******************************************************************************/
int RmSysctrlInitCan(struct RmSysctrlCan **can_addr);

/* Send */
/****************************************************************************
 Name: RmSysctrlCanDataSend
 Function: This function is to send the data onto can.
 Params: p - The priority.
         s - The source module.
         d - The destination module.
         pdata - The data.
         len - The data len.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCanDataSend(<PRIORITY>, <SRC>, <DST>, <DATA>, <LEN>);
******************************************************************************/
int  RmSysctrlCanDataSend(unsigned int p, int s, int d, void *pdata, int len);

#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_CAN_H_*/
