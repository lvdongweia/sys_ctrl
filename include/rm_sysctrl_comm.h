/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_comm.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-4
 Description: This program defined the communication interfaces.
 Others: None
 
 Function List: 
   1. RmSysctrlInitComm() to initialize the communication from the params.
   2. RmSysctrlInitComm() to initialize the communication from the params.
   3. RmSysctrlCommRecvMsg() to receive message from communication queue.
   4. RmSysctrlCommEnQueueMsgXX() to enqueue message into communication queue.
   5. RmSysctrlCommEnQueueMsgFromCan() to enqueue message into communication
      queue from can.
   6. RmSysctrlCommSendMsgToCan() to send the data onto can by rm sysctrl.
   7. RmSysctrlCommSendSubsysDead() to send the sub-system dead event into
      communication queue.
   8. RmSysctrlCommSendTimerExpire() to send the timer expired event into
      communication queue.
   9. RmSysctrlCommSendSPoweronChange() to send the S_poweron change event into
      communication queue.
   10.RmSysctrlCommSendInitCmdToCan() to send init command onto can.
   11.RmSysctrlCommSendRegQueryToCan() to send register query message onto can.
   12.RmSysctrlCommSendRegCmdToCan() to send register command message onto can.
   13.RmSysctrlCommSendLoginQueryToCan() to send login query message onto can.
   14.RmSysctrlCommSendLoginCmdToCan() to send login command message onto can.
   15.RmSysctrlCommSendChargeNotiToCan() to send charge notification message
      onto can.
   16.RmSysctrlCommSendExitChargeNotiToCan() to send exit charge notification
      message onto can.
   17.RmSysctrlCommSendLowPowerNotiToCan() to send low power notification
      message onto can.
   18.RmSysctrlCommSendBatteryQueryRespToCan() to send battery query response
      message onto can.
   19.RmSysctrlCommSendBatteryStatusQueryToCan() to send battery status query
      message onto can.
   20.RmSysctrlCommSendSecurityCmdToCan() to send security command message onto
      can.
   21.RmSysctrlCommSendExitSecurityCmdToCan() to send exit security command
      message onto can.
   22.RmSysctrlCommSendWakeCmdToCan() to send wake command message onto can.
   23.RmSysctrlCommSendSleepCmdToCan() to send sleep command message onto can.
   24.RmSysctrlCommSendHaltCmdToCan() to send halt command message onto can.
   25.RmSysctrlCommSendPoweroffCmdToCan() to send power off command message
      onto can.
   26.RmSysctrlCommSendPoweronCmdToCan() to send power on command message
      onto can.
   27.RmSysctrlCommSendSysStateChangeNotiToCan() to send system state change
      notification message onto can.
   28.RmSysctrlCommSendDebugCmdToCan() to send debug command message onto can.
   29.RmSysctrlCommSendModeCmdToCan() to send mode command message onto can.
   30.RmSysctrlCommSendRestoreFactoryCmdRespToCan() to send restore factory
      mode command response onto can.
   31.RmSysctrlCommSendSubsysStateQueryToCan() to send sub-system state query
      message onto can.
   32.RmSysctrlCommSendVersionQueryToCan() to send sub-system version query
      message onto can.
   33.RmSysctrlCommSendGetCPUIDCmdToCan() to send CPUID query message onto can.
   34.RmSysctrlCommSendReadSNCmdToCan() to send SN query message onto can.
   35.RmSysctrlCommSendDatetimeQueryToCan() to send datetime query message onto
      can.
   36.RmSysctrlCommSendSetDatetimeCmdToCan() to send set datetime message onto
      can.
   37.RmSysctrlCommSendSexQueryToCan() to send sex query message onto can.
   38.RmSysctrlCommSendSetSexCmdToCan() to send set sex message onto can.
   39.RmSysctrlCommSendUpgradeCmdToCan() to send upgrade command message onto
      can.
   40.RmSysctrlCommSendExitUpgradeCmdToCan() to send exit upgrade command
      message onto can.
   41.RmSysctrlCommSendNotLowPowerNotiToCan() to send not low power notification
      message onto can.
   42.RmSysctrlCommSendUptimeQueryToCan() to send uptime query message onto can.
 
 History:
   V1.0    2014-9-4    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_COMM_H_
#define RM_SYSCTRL_COMM_H_

#ifdef __cplusplus
extern "C"{
#endif


enum RmSysctrlCommType {
    INTERNAL = 0,
    EXTERNAL
};

/* Internal Message */
enum InternalSysctrlMsg {
    INTERNAL_SYSCTRL_MSG_BEGIN              = 0,  //0x00
    INTERNAL_SYSCTRL_SUBSYS_DEAD            = 1,  //0x01
    INTERNAL_SYSCTRL_TIMER_EXPIRE           = 2,  //0x02
#ifdef DEEP_SLEEP_SUPPORT
    INTERNAL_SYSCTRL_S_POWERON_CHANGE       = 3,  //0x03
#endif
    INTERNAL_SYSCTRL_MSG_END
};


/* External Message */
/* Common message type range from 0 to 63  */
enum CommonSysctrlMsg {
    COMMON_SYSCTRL_MSG_BEGIN                = 0,  //0x00
    COMMON_SYSCTRL_INIT_CMD                 = 1,  //0x01
    COMMON_SYSCTRL_INIT_CMD_REP             = 2,  //0x02
    COMMON_SYSCTRL_SLEEP_CMD                = 3,  //0x03
    COMMON_SYSCTRL_SLEEP_CMD_REP            = 4,  //0x04
    COMMON_SYSCTRL_WAKE_CMD                 = 5,  //0x05
    COMMON_SYSCTRL_WAKE_CMD_REP             = 6,  //0x06
    COMMON_SYSCTRL_SECURITY_CMD             = 7,  //0x07
    COMMON_SYSCTRL_SECURITY_CMD_REP         = 8,  //0x08
    COMMON_SYSCTRL_EXIT_SECURITY_CMD        = 9,  //0x09
    COMMON_SYSCTRL_EXIT_SECURITY_CMD_REP    = 10, //0x0A
    COMMON_SYSCTRL_HALT_CMD                 = 11, //0x0B
    COMMON_SYSCTRL_HALT_CMD_REP             = 12, //0x0C
    COMMON_SYSCTRL_DEBUG_CMD                = 13, //0x0D
    COMMON_SYSCTRL_DEBUG_CMD_REP            = 14, //0x0E
    COMMON_SYSCTRL_MODE_CMD                 = 15, //0x0F
    COMMON_SYSCTRL_MODE_CMD_REP             = 16, //0x10
    COMMON_SYSCTRL_ABNORMAL_OCCUR_NOTI      = 17, //0x11
    COMMON_SYSCTRL_ABNORMAL_RESOLVE_NOTI    = 18, //0x12
    COMMON_SYSCTRL_LOW_POWER_NOTI           = 19, //0x13
    COMMON_SYSCTRL_POWER_CHARGE_NOTI        = 20, //0x14
    COMMON_SYSCTRL_POWER_CHARGE_END_NOTI    = 21, //0x15
    COMMON_SYSCTRL_SUBSYS_STATE_CHANGE_NOTI = 22, //0x16
    COMMON_SYSCTRL_SYS_STATE_CHANGE_NOTI    = 23, //0x17
    COMMON_SYSCTRL_SUBSYS_STATE_QUERY       = 24, //0x18
    COMMON_SYSCTRL_SUBSYS_STATE_QUERY_REP   = 25, //0x19
    COMMON_SYSCTRL_VERSION_QUERY            = 26, //0x1A
    COMMON_SYSCTRL_VERSION_QUERY_REP        = 27, //0x1B
    COMMON_SYSCTRL_PROD_MODE_QUERY          = 28, //0x1C
    COMMON_SYSCTRL_PROD_MODE_QUERY_REP      = 29, //0x1D
    COMMON_SYSCTRL_SWITCH_PROD_MODE_CMD     = 30, //0x1E
    COMMON_SYSCTRL_SWITCH_PROD_MODE_CMD_REP = 31, //0x1F
    COMMON_SYSCTRL_UPGRADE_CMD              = 32, //0x20
    COMMON_SYSCTRL_UPGRADE_CMD_REP          = 33, //0x21
    COMMON_SYSCTRL_EXIT_UPGRADE_CMD         = 34, //0x22
    COMMON_SYSCTRL_EXIT_UPGRADE_CMD_REP     = 35, //0x23
    COMMON_SYSCTRL_NOT_LOW_POWER_NOTI       = 36, //0x24
    COMMON_SYSCTRL_HDMI_STATE_CHANGE_NOTI   = 37, //0x25
    COMMON_SYSCTRL_MSG_END
};

/* Message type only related to RM range from 64 to 255  */
enum RmSysctrlMsg {
    RM_SYSCTRL_MSG_BEGIN                    = 64, //0x40
    RM_SYSCTRL_REGISTER_CMD                 = 65, //0x41
    RM_SYSCTRL_REGISTER_CMD_REP             = 66, //0x42
    RM_SYSCTRL_LOGIN_CMD                    = 67, //0x43
    RM_SYSCTRL_LOGIN_CMD_REP                = 68, //0x44
    RM_SYSCTRL_RESTORE_FACTORY_CMD          = 69, //0x45
    RM_SYSCTRL_RESTORE_FACTORY_CMD_REP      = 70, //0x46
    RM_SYSCTRL_REGISTERED_NOTI              = 71, //0x47
    RM_SYSCTRL_LOGIN_NOTI                   = 72, //0x48
    RM_SYSCTRL_BATTERY_QUERY                = 73, //0x49
    RM_SYSCTRL_BATTERY_QUERY_REP            = 74, //0x4A
    RM_SYSCTRL_REGISTERED_QUERY             = 75, //0x4B
    RM_SYSCTRL_REGISTERED_QUERY_REP         = 76, //0x4C
    RM_SYSCTRL_LOGIN_QUERY                  = 77, //0x4D
    RM_SYSCTRL_LOGIN_QUERY_REP              = 78, //0x4E
    RM_SYSCTRL_MSG_END
};

/* Message type only related to RP range from 64 to 255 */
enum RpSysctrlMsg {
    RP_SYSCTRL_MSG_BEGIN                    = 64, //0x40
    RP_SYSCTRL_POWER_OFF_CMD                = 65, //0x41
    RP_SYSCTRL_POWER_OFF_CMD_REP            = 66, //0x42
    RP_SYSCTRL_POWER_ON_CMD                 = 67, //0x43
    RP_SYSCTRL_POWER_ON_CMD_REP             = 68, //0x44
    RP_SYSCTRL_BATTERY_NOTI                 = 69, //0x45
    RP_SYSCTRL_POWER_OFF_NOTI               = 70, //0x46
    RP_SYSCTRL_POWER_ON_NOTI                = 71, //0x47
    RP_SYSCTRL_BATTERY_STATUS_QUERY         = 72, //0x48
    RP_SYSCTRL_BATTERY_STATUS_QUERY_REP     = 73, //0x49
    RP_SYSCTRL_GET_CPU_ID_CMD               = 74, //0x4A
    RP_SYSCTRL_GET_CPU_ID_CMD_REP           = 75, //0x4B
    RP_SYSCTRL_SAVE_ROBOT_SN_CMD            = 76, //0x4C
    RP_SYSCTRL_SAVE_ROBOT_SN_CMD_REP        = 77, //0x4D
    RP_SYSCTRL_READ_ROBOT_SN_CMD            = 78, //0x4E
    RP_SYSCTRL_READ_ROBOT_SN_CMD_REP        = 79, //0x4F
    RP_SYSCTRL_DATETIME_QUERY               = 80, //0x50
    RP_SYSCTRL_DATETIME_QUERY_REP           = 81, //0x51
    RP_SYSCTRL_SET_DATETIME_CMD             = 82, //0x52
    RP_SYSCTRL_SET_DATETIME_CMD_REP         = 83, //0x53
    RP_SYSCTRL_SEX_QUERY                    = 84, //0x54
    RP_SYSCTRL_SEX_QUERY_REP                = 85, //0x55
    RP_SYSCTRL_SET_SEX_CMD                  = 86, //0x56
    RP_SYSCTRL_SET_SEX_CMD_REP              = 87, //0x57
    RP_SYSCTRL_UPTIME_QUERY                 = 88, //0x58
    RP_SYSCTRL_UPTIME_QUERY_REP             = 89, //0x59
    RP_SYSCTRL_MSG_END
};

/* Message type only related to RC range from 64 to 255 */
enum RcSysctrlMsg {
    RC_SYSCTRL_MSG_BEGIN                    = 64, //0x40
    RC_SYSCTRL_MSG_END
};

/* Message type only related to RB right arm range from 64 to 255 */
enum RbRArmSysctrlMsg {
    RB_R_ARM_SYSCTRL_MSG_BEGIN              = 64, //0x40
    RB_R_ARM_SYSCTRL_MSG_END
};

/* Message type only related to RB left arm range from 64 to 255 */
enum RbLArmSysctrlMsg {
    RB_L_ARM_SYSCTRL_MSG_BEGIN              = 64, //0x40
    RB_L_ARM_SYSCTRL_MSG_END
};

/* Message type only related to RB body range from 64 to 255 */
enum RbBodySysctrlMsg {
    RB_BODY_SYSCTRL_MSG_BEGIN               = 64, //0x40
    RB_BODY_SYSCTRL_MSG_END
};

/* Message type only related to RF range from 64 to 255*/
enum RfSysctrlMsg {
    RF_SYSCTRL_MSG_BEGIN                    = 64, //0x40
    RF_SYSCTRL_WAKE_NOTI                    = 65, //0x41
    RF_SYSCTRL_MSG_END
};

struct RmSysctrlTLV {
    char type;	//See CommonSysctrlMsg, RmSysctrlMsg, ..., RfSysctrlMsg
    char len;   //The length of the whole tlv
    char value[TEXT_SIZE];
};

/* The TLV defination is as folowing:
    TYPE							LEN	VALUE_TYPE[1]	VALUE_VALUE[1]	VALUE_TYPE[2]	VALUE_VALUE[2]
    ++++++++++++++++++++++INTERNAL++++++++++++++++++++++
    INTERNAL_SYSCTRL_SUBSYS_DEAD            = 1,  //0x01	3	uint8		Please see RmSysctrlSubSystemID
    INTERNAL_SYSCTRL_TIMER_EXPIRE           = 2,  //0x02	3	uint8		Please see RmSysctrlTimerID
    INTERNAL_SYSCTRL_S_POWERON_CHANGE       = 3,  //0x03	2	None		None
    +++++++++++++++++++++++COMMON+++++++++++++++++++++++
    COMMON_SYSCTRL_INIT_CMD                 = 1,  //0x01	2	None		None
    COMMON_SYSCTRL_INIT_CMD_REP             = 2,  //0x02	3	int8		SUCCESS 0; FAILED -1
    COMMON_SYSCTRL_SLEEP_CMD                = 3,  //0x03	3	int8		SHALLOW 0; DEEP 1
    COMMON_SYSCTRL_SLEEP_CMD_REP            = 4,  //0x04	3	int8		SUCCESS 0; FAILED -1
    COMMON_SYSCTRL_WAKE_CMD                 = 5,  //0x05	2	None		None
    COMMON_SYSCTRL_WAKE_CMD_REP             = 6,  //0x06	3	int8		SUCCESS 0; FAILED -1
    COMMON_SYSCTRL_SECURITY_CMD             = 7,  //0x07	2	None		None
    COMMON_SYSCTRL_SECURITY_CMD_REP         = 8,  //0x08	3	int8		SUCCESS 0; FAILED -1
    COMMON_SYSCTRL_EXIT_SECURITY_CMD        = 9,  //0x09	2	None		None
    COMMON_SYSCTRL_EXIT_SECURITY_CMD_REP    = 10, //0x0A	3	int8		SUCCESS 0; FAILED -1
    COMMON_SYSCTRL_HALT_CMD                 = 11, //0x0B	2	None		None
    COMMON_SYSCTRL_HALT_CMD_REP             = 12, //0x0C	3	int8		SUCCESS 0; FAILED -1
    COMMON_SYSCTRL_DEBUG_CMD                = 13, //0x0D	3	uint8		Enable 1; Disable 0
    COMMON_SYSCTRL_DEBUG_CMD_REP            = 14, //0x0E	3	int8		SUCCESS 0; FAILED -1
    COMMON_SYSCTRL_MODE_CMD                 = 15, //0x0F	3	uint8		DEVELOP_MODE 0; CHILD_MODE 1; GUARD_MODE 2
    COMMON_SYSCTRL_MODE_CMD_REP             = 16, //0x10	3	int8		SUCCESS 0; FAILED -1
    COMMON_SYSCTRL_ABNORMAL_OCCUR_NOTI      = 17, //0x11	6	uint32		Please see falt manager
    COMMON_SYSCTRL_ABNORMAL_RESOLVE_NOTI    = 18, //0x12	2	None		None
    COMMON_SYSCTRL_LOW_POWER_NOTI           = 19, //0x13	2	None		None
    COMMON_SYSCTRL_POWER_CHARGE_NOTI        = 20, //0x14	2	None		None
    COMMON_SYSCTRL_POWER_CHARGE_END_NOTI    = 21, //0x15	2	None		None
    COMMON_SYSCTRL_SUBSYS_STATE_CHANGE_NOTI = 22, //0x16	4	uint8		See RobotStatus		uint8	See SubStatus
    COMMON_SYSCTRL_SYS_STATE_CHANGE_NOTI    = 23, //0x17	3	uint8		See RobotStatus
    COMMON_SYSCTRL_SUBSYS_STATE_QUERY       = 24, //0x18	2	None		None
    COMMON_SYSCTRL_SUBSYS_STATE_QUERY_REP   = 25, //0x19	4	uint8		See RobotStatus		uint8	See SubStatus
    COMMON_SYSCTRL_VERSION_QUERY            = 26, //0x1A	2	None		None
    COMMON_SYSCTRL_VERSION_QUERY_REP        = 27, //0x1B	11    	uint8*9       	Please see following
	*send_data=COMMON_SYSCTRL_VERSION_QUERY_REP;
	*(send_data+1)=0x0B;                                	//command length
	*(send_data+2)=sub-sys-id;               	        //sub-system ID
	*(send_data+3)=major;                	                //major version number
	*(send_data+4)=minor;              	                //minor version number
	*(send_data+5)=revision; 	                        //revision number
	*(send_data+6)=year>>8;
	*(send_data+7)=year;                                    //building year
	*(send_data+8)=month;         	                        //building month
	*(send_data+9)=day;                                     //building day
	*(send_data+10)=rom-id;            	                //rom id
    COMMON_SYSCTRL_PROD_MODE_QUERY          = 28, //0x1C	2	None
    COMMON_SYSCTRL_PROD_MODE_QUERY_REP      = 29, //0x1D	3	uint8		0: FACTORY_MODE 1: NORMAL_MODE
    COMMON_SYSCTRL_SWITCH_PROD_MODE_CMD     = 30, //0x1E	2	None
    COMMON_SYSCTRL_SWITCH_PROD_MODE_CMD_REP = 31, //0x1F	3	uint8		0: Successful -1: failed  
    COMMON_SYSCTRL_UPGRADE_CMD              = 32, //0x20	2	None		None
    COMMON_SYSCTRL_UPGRADE_CMD_REP          = 33, //0x21	3	int8		SUCCESS 0; FAILED -1
    COMMON_SYSCTRL_EXIT_UPGRADE_CMD         = 34, //0x22	2	None		None
    COMMON_SYSCTRL_EXIT_UPGRADE_CMD_REP     = 35, //0x23	3	int8		SUCCESS 0; FAILED -1
    COMMON_SYSCTRL_NOT_LOW_POWER_NOTI       = 36, //0x24	2	None		None
    COMMON_SYSCTRL_HDMI_STATE_CHANGE_NOTI   = 37, //0x25	3	uint8		0: PLUG_OUT 1: PLUG_IN
    +++++++++++++++++++++++RM+++++++++++++++++++++++++++
    RM_SYSCTRL_REGISTER_CMD                 = 65, //0x41	2	None		None
    RM_SYSCTRL_REGISTER_CMD_REP             = 66, //0x42	3	int8		SUCCESS 0; FAILED -1
    RM_SYSCTRL_LOGIN_CMD                    = 67, //0x43	2	None		None
    RM_SYSCTRL_LOGIN_CMD_REP                = 68, //0x44	3	int8		SUCCESS 0; FAILED -1
    RM_SYSCTRL_RESTORE_FACTORY_CMD          = 69, //0x45	2	None		None
    RM_SYSCTRL_RESTORE_FACTORY_CMD_REP      = 70, //0x46	3	int8		SUCCESS 0; FAILED -1
    RM_SYSCTRL_REGISTERED_NOTI              = 71, //0x47	3	uint8		YES 1; NO 0
    RM_SYSCTRL_LOGIN_NOTI                   = 72, //0x48	3	uint8		YES 1; NO 0
    RM_SYSCTRL_BATTERY_QUERY                = 73, //0x49	2	None		None
    RM_SYSCTRL_BATTERY_QUERY_REP            = 74, //0x4A	3	uint8		Percentage: 0~100
    RM_SYSCTRL_REGISTERED_QUERY             = 75, //0x4B	2	None		None
    RM_SYSCTRL_REGISTERED_QUERY_REP         = 76, //0x4C	3	uint8		YES 1; NO 0
    RM_SYSCTRL_LOGIN_QUERY                  = 77, //0x4D	2	None		None
    RM_SYSCTRL_LOGIN_QUERY_REP              = 78, //0x4E	3	uint8		YES 1; NO 0
    +++++++++++++++++++++++RP+++++++++++++++++++++++++++
    RP_SYSCTRL_POWER_OFF_CMD                = 65, //0x41    	4  	uint16         	ALL: 1xxxxxxx,xxxxxxxx;
											NOT ALL: 0xxxxxxx,xxxxxxxx
 											one bit one sub-system from right-hand
    RP_SYSCTRL_POWER_OFF_CMD_REP            = 66, //0x42    	3    	int8        	SUCCESS 0; FAILED -1
    RP_SYSCTRL_POWER_ON_CMD                 = 67, //0x43    	4  	uint16         	ALL: 1xxxxxxx,xxxxxxxx;
											NOT ALL: 0xxxxxxx,xxxxxxxx
											one bit one sub-system from right-hand
    RP_SYSCTRL_POWER_ON_CMD_REP             = 68, //0x44    	3    	int8        	SUCCESS 0; FAILED -1
    RP_SYSCTRL_BATTERY_NOTI                 = 69, //0x45	10	uint8*8       	Please see following
	*send_data=RP_SYSCTRL_BATTERY_NOTI;
	*(send_data+1)=0x0A;                                	//command length
	*(send_data+2)=RpData.ChargingStatus;               	//power charging status
	*(send_data+3)=RpData.BatteryStatus;                	//battery status
	*(send_data+4)=RpData.BatteryQuantity;              	//battery quantity
	*(send_data+5)=RpBatData.BatSingle_Avg_Vol_Val>>8; 	//battery voltage (mV)
	*(send_data+6)=RpBatData.BatSingle_Avg_Vol_Val;
	*(send_data+7)=RpBatData.Bat_Temp_Val1>>8;         	//battery temperature
	*(send_data+8)=RpBatData.Bat_Temp_Val1;
	*(send_data+9)=RpData.BatteryTechnology;            	//battery technology
    RP_SYSCTRL_POWER_OFF_NOTI               = 70, //0x46    	4  	uint16         	ALL: 1xxxxxxx,xxxxxxxx;
											NOT ALL: 0xxxxxxx,xxxxxxxx
											one bit one sub-system from right-hand
    RP_SYSCTRL_POWER_ON_NOTI                = 71, //0x47    	4  	uint16         	ALL: 1xxxxxxx,xxxxxxxx;
											NOT ALL: 0xxxxxxx,xxxxxxxx
											one bit one sub-system from right-hand
    RP_SYSCTRL_BATTERY_STATUS_QUERY         = 72, //0x48    	2  	None        	None
    RP_SYSCTRL_BATTERY_STATUS_QUERY_REP     = 73, //0x49    	10    	uint8*8       	Please see following
	*send_data=RP_SYSCTRL_BATTERY_STATUS_QUERY_REP;
	*(send_data+1)=0x0A;                                	//command length
	*(send_data+2)=RpData.ChargingStatus;               	//power charging status
	*(send_data+3)=RpData.BatteryStatus;                	//battery status
	*(send_data+4)=RpData.BatteryQuantity;              	//battery quantity
	*(send_data+5)=RpBatData.BatSingle_Avg_Vol_Val>>8; 	//battery voltage (mV)
	*(send_data+6)=RpBatData.BatSingle_Avg_Vol_Val;
	*(send_data+7)=RpBatData.Bat_Temp_Val1>>8;         	//battery temperature
	*(send_data+8)=RpBatData.Bat_Temp_Val1;
	*(send_data+9)=RpData.BatteryTechnology;            	//battery technology
    RP_SYSCTRL_GET_CPU_ID_CMD               = 74, //0x4A    	2   	None        	None
    RP_SYSCTRL_GET_CPU_ID_CMD_REP           = 75, //0x4B    	14  	uint8*12	CPU_ID:96 bit,12 byte
    RP_SYSCTRL_SAVE_ROBOT_SN_CMD            = 76, //0x4C    	20  	uint8*18	SN:   18 byte
    RP_SYSCTRL_SAVE_ROBOT_SN_CMD_REP        = 77, //0x4D    	3   	int8            SUCCESS 0; FAILED -1
    RP_SYSCTRL_READ_ROBOT_SN_CMD            = 78, //0x4E    	2   	None        	None
    RP_SYSCTRL_READ_ROBOT_SN_CMD_REP        = 79, //0x4F    	20  	uint8*18	SN:   18 byte 
    RP_SYSCTRL_DATETIME_QUERY               = 80, //0x50    	2   	None        	None
    RP_SYSCTRL_DATETIME_QUERY_REP           = 81, //0x51    	9   	uint8*7		year(1),month(1),day(1),week(1),hour(1),minute(1),second(1),
    RP_SYSCTRL_SET_DATETIME_CMD             = 82, //0x52    	9   	uint8*7		year(1),month(1),day(1),week(1),hour(1),minute(1),second(1),
    RP_SYSCTRL_SET_DATETIME_CMD_REP         = 83, //0x53    	3   	int8            SUCCESS 0; FAILED -1
    RP_SYSCTRL_SEX_QUERY                    = 84, //0x54    	2   	None        	None
    RP_SYSCTRL_SEX_QUERY_REP                = 85, //0x55    	3   	uint8		Male 1; Female 2; Unknown 0xFF 
    RP_SYSCTRL_SET_SEX_CMD                  = 86, //0x56    	3   	uint8		Male 1; Female 2; Unknown 0xFF
    RP_SYSCTRL_SET_SEX_CMD_REP              = 87, //0x57    	3   	int8            SUCCESS 0; FAILED -1
    RP_SYSCTRL_UPTIME_QUERY                 = 88, //0x58    	2   	None        	None
    RP_SYSCTRL_UPTIME_QUERY_REP             = 89, //0x59    	6   	uint8*4		seconds(4) 
    +++++++++++++++++++++++RC+++++++++++++++++++++++++++
    +++++++++++++++++++++++RB+++++++++++++++++++++++++++
    +++++++++++++++++++++++RF+++++++++++++++++++++++++++
    RF_SYSCTRL_WAKE_NOTI                    = 65, //0x41	2	None		None
*/

enum RmSysctrlMsgs {
    INTERNAL_SYSCTRL_SUBSYS_DEAD_MSG        = 0,
    INTERNAL_SYSCTRL_TIMER_EXPIRE_MSG,
#ifdef DEEP_SLEEP_SUPPORT
    INTERNAL_SYSCTRL_S_POWERON_CHANGE_MSG,
#endif
    COMMON_SYSCTRL_INIT_CMD_REP_MSG,
    COMMON_SYSCTRL_SLEEP_CMD_MSG,
    COMMON_SYSCTRL_SLEEP_CMD_REP_MSG,
    COMMON_SYSCTRL_WAKE_CMD_MSG,
    COMMON_SYSCTRL_WAKE_CMD_REP_MSG,
    COMMON_SYSCTRL_SECURITY_CMD_MSG,
    COMMON_SYSCTRL_SECURITY_CMD_REP_MSG,
    COMMON_SYSCTRL_EXIT_SECURITY_CMD_MSG,
    COMMON_SYSCTRL_EXIT_SECURITY_CMD_REP_MSG,
    COMMON_SYSCTRL_HALT_CMD_MSG,
    COMMON_SYSCTRL_HALT_CMD_REP_MSG,
    COMMON_SYSCTRL_DEBUG_CMD_MSG,
    COMMON_SYSCTRL_DEBUG_CMD_REP_MSG,
    COMMON_SYSCTRL_MODE_CMD_MSG,
    COMMON_SYSCTRL_MODE_CMD_REP_MSG,
    COMMON_SYSCTRL_ABNORMAL_OCCUR_NOTI_MSG,
    COMMON_SYSCTRL_ABNORMAL_RESOLVE_NOTI_MSG,
    COMMON_SYSCTRL_LOW_POWER_NOTI_MSG,
    COMMON_SYSCTRL_POWER_CHARGE_NOTI_MSG,
    COMMON_SYSCTRL_POWER_CHARGE_END_NOTI_MSG,
    COMMON_SYSCTRL_SUBSYS_STATE_CHANGE_NOTI_MSG,
    COMMON_SYSCTRL_SUBSYS_STATE_QUERY_REP_MSG,
    COMMON_SYSCTRL_VERSION_QUERY_REP_MSG,
    COMMON_SYSCTRL_UPGRADE_CMD_MSG,
    COMMON_SYSCTRL_UPGRADE_CMD_REP_MSG,
    COMMON_SYSCTRL_EXIT_UPGRADE_CMD_MSG,
    COMMON_SYSCTRL_EXIT_UPGRADE_CMD_REP_MSG,
    COMMON_SYSCTRL_NOT_LOW_POWER_NOTI_MSG,
    RM_SYSCTRL_REGISTER_CMD_REP_MSG,
    RM_SYSCTRL_LOGIN_CMD_REP_MSG,
    RM_SYSCTRL_RESTORE_FACTORY_CMD_MSG,
    RM_SYSCTRL_REGISTERED_NOTI_MSG,
    RM_SYSCTRL_LOGIN_NOTI_MSG,
    RM_SYSCTRL_BATTERY_QUERY_MSG,
    RM_SYSCTRL_REGISTERED_QUERY_REP_MSG,
    RM_SYSCTRL_LOGIN_QUERY_REP_MSG,
    RP_SYSCTRL_POWER_OFF_CMD_REP_MSG,
    RP_SYSCTRL_POWER_ON_CMD_REP_MSG,
    RP_SYSCTRL_BATTERY_NOTI_MSG,
    RP_SYSCTRL_POWER_OFF_NOTI_MSG,
    RP_SYSCTRL_POWER_ON_NOTI_MSG,
    RP_SYSCTRL_BATTERY_STATUS_QUERY_REP_MSG,
    RP_SYSCTRL_GET_CPU_ID_CMD_REP_MSG,
    RP_SYSCTRL_SAVE_ROBOT_SN_CMD_REP_MSG,
    RP_SYSCTRL_READ_ROBOT_SN_CMD_REP_MSG,
    RP_SYSCTRL_DATETIME_QUERY_REP_MSG,
    RP_SYSCTRL_SET_DATETIME_CMD_REP_MSG,
    RP_SYSCTRL_SEX_QUERY_REP_MSG,
    RP_SYSCTRL_SET_SEX_CMD_REP_MSG,
    RP_SYSCTRL_UPTIME_QUERY_REP_MSG,
    RF_SYSCTRL_WAKE_NOTI_MSG,
    RM_SYSCTRL_MSGS_MAX
};

struct RmSysctrlMessage {
    char mtype;	//See RmSysctrlMsgs match from TLV type
    char src_module_id;
    char src_system_id;
    struct RmSysctrlTLV tlv;
};

struct RmSysctrlMsgQueue {
    struct RmSysctrlMsgQueue *next;
    struct RmSysctrlMsgQueue *prev;
    struct RmSysctrlMessage rscm;
};

struct RmSysctrlComm {
    struct RmSysctrlMsgQueue *msg_queue;
    pthread_mutex_t msg_queue_mutex;
    sem_t msg_queue_count;
};

/****************************************************************************
 Name: RmSysctrlInitComm
 Function: This function is to initialize the communication from the params.
 Params: comm - The point of point of a communication.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlInitComm(<POINT of POINT of COMMUNICATION>);
******************************************************************************/
int RmSysctrlInitComm(struct RmSysctrlComm **comm);

/****************************************************************************
 Name: RmSysctrlCommRecvMsg
 Function: This function is to receive message from communication queue.
 Params: comm - The point of a communication.
         data - The buffer received from communication queue.
         len  - The data buffer length.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommRecvMsg(<POINT of COMMUNICATION>,
                                        <POINT of data buffer>,
                                        <LENGTH to WANT to RECEIVE>);
******************************************************************************/
int RmSysctrlCommRecvMsg(struct RmSysctrlComm *comm,
                         void *data,
                         unsigned int len);

/****************************************************************************
 Name: RmSysctrlCommEnQueueMsgXX
 Function: This function is to enqueue message into communication queue.
 Params: comm - The point of a communication.
         src_module_id - The soure module ID.
         data - The buffer to enqueue into communication queue.
         len  - The data buffer length.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommEnQueueMsgXX(<POINT of COMMUNICATION>,
                                             <SRC MODULE ID>,
                                             <POINT of data buffer>,
                                             <LENGTH to WANT to RECEIVE>);
******************************************************************************/
int RmSysctrlCommEnQueueMsgXX(struct RmSysctrlComm *comm,
                         char src_module_id,
                         void *data,
                         unsigned int len);

/****************************************************************************
 Name: RmSysctrlCommEnQueueMsgFromCan
 Function: This function is to enqueue message into communication queue from can.
 Params: comm - The point of a communication.
         can_packet - The packet to enqueue into communication queue from can.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommEnQueueMsgXX(<POINT of COMMUNICATION>,
                                             <POINT of CAN PACKET>);
******************************************************************************/
int RmSysctrlCommEnQueueMsgFromCan(struct RmSysctrlComm *comm,
                         void *can_packet);

/****************************************************************************
 Name: RmSysctrlCommSendMsgToCan
 Function: This function is to send the data onto can by rm sysctrl.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         data - The data.
         len - The data len.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendMsgToCan(<POINT of COMMUNICATION>,
                                             <PRIORITY>, <DST>, <DATA>, <LEN>);
******************************************************************************/
int RmSysctrlCommSendMsgToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         void *data,
                         unsigned int len);

/* Sub-system dead */
/****************************************************************************
 Name: RmSysctrlCommSendSubsysDead
 Function: This function is to send the sub-system dead event into
           communication queue.
 Params: comm - The point of a communication.
         subsys_id - The dead sub-system ID.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendSubsysDead(<POINT of COMMUNICATION>,
                                               <SUBSYS ID>);
******************************************************************************/
int RmSysctrlCommSendSubsysDead(struct RmSysctrlComm *comm,
                         unsigned int subsys_id);

/* Timer expired */
/****************************************************************************
 Name: RmSysctrlCommSendTimerExpire
 Function: This function is to send the timer expired event into
           communication queue.
 Params: comm - The point of a communication.
         timer_id - The expired timer ID.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendTimerExpire(<POINT of COMMUNICATION>,
                                               <TIMER ID>);
******************************************************************************/
int RmSysctrlCommSendTimerExpire(struct RmSysctrlComm *comm,
                         unsigned int timer_id);

#ifdef DEEP_SLEEP_SUPPORT
/* S_poweron change */
/****************************************************************************
 Name: RmSysctrlCommSendSPoweronChange
 Function: This function is to send the S_poweron change event into
           communication queue.
 Params: comm - The point of a communication.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendSPoweronChange(<POINT of COMMUNICATION>);
******************************************************************************/
int RmSysctrlCommSendSPoweronChange(struct RmSysctrlComm *comm);
#endif

/* Init */
/****************************************************************************
 Name: RmSysctrlCommSendInitCmdToCan
 Function: This function is to send init command onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendInitCmdToCan(<POINT of COMMUNICATION>,
                                                 <PRIORITY>,
                                                 <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendInitCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/* Register */
/****************************************************************************
 Name: RmSysctrlCommSendRegQueryToCan
 Function: This function is to send register query message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendRegQueryToCan(<POINT of COMMUNICATION>,
                                                  <PRIORITY>,
                                                  <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendRegQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/****************************************************************************
 Name: RmSysctrlCommSendRegCmdToCan
 Function: This function is to send register command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendRegCmdToCan(<POINT of COMMUNICATION>,
                                                <PRIORITY>,
                                                <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendRegCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/* Login */
/****************************************************************************
 Name: RmSysctrlCommSendLoginQueryToCan
 Function: This function is to send login query message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendLoginQueryToCan(<POINT of COMMUNICATION>,
                                                    <PRIORITY>,
                                                    <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendLoginQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/****************************************************************************
 Name: RmSysctrlCommSendLoginCmdToCan
 Function: This function is to send login command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendLoginCmdToCan(<POINT of COMMUNICATION>,
                                                  <PRIORITY>,
                                                  <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendLoginCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/* Charge */
/****************************************************************************
 Name: RmSysctrlCommSendChargeNotiToCan
 Function: This function is to send charge notification message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendChargeNotiToCan(<POINT of COMMUNICATION>,
                                                    <PRIORITY>,
                                                    <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendChargeNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/****************************************************************************
 Name: RmSysctrlCommSendExitChargeNotiToCan
 Function: This function is to send exit charge notification message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendExitChargeNotiToCan(<POINT of COMMUNICATION>,
                                                        <PRIORITY>,
                                                        <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendExitChargeNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/* Power */
/****************************************************************************
 Name: RmSysctrlCommSendLowPowerNotiToCan
 Function: This function is to send low power notification message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendLowPowerNotiToCan(<POINT of COMMUNICATION>,
                                                      <PRIORITY>,
                                                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendLowPowerNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/****************************************************************************
 Name: RmSysctrlCommSendNotLowPowerNotiToCan
 Function: This function is to send not low power notification message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendNotLowPowerNotiToCan(<POINT of COMMUNICATION>,
                                                      <PRIORITY>,
                                                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendNotLowPowerNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/****************************************************************************
 Name: RmSysctrlCommSendBatteryQueryRespToCan
 Function: This function is to send battery query response message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendBatteryQueryRespToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>,
                      <BATTERY CAPICITY>);
******************************************************************************/
int RmSysctrlCommSendBatteryQueryRespToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         unsigned int battery_power);

/****************************************************************************
 Name: RmSysctrlCommSendBatteryStatusQueryToCan
 Function: This function is to send battery status query message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendBatteryStatusQueryToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendBatteryStatusQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/* Security */
/****************************************************************************
 Name: RmSysctrlCommSendSecurityCmdToCan
 Function: This function is to send security command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendSecurityCmdToCan(<POINT of COMMUNICATION>,
                                                     <PRIORITY>,
                                                     <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendSecurityCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/****************************************************************************
 Name: RmSysctrlCommSendExitSecurityCmdToCan
 Function: This function is to send exit security command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendExitSecurityCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendExitSecurityCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/* Wake */
/****************************************************************************
 Name: RmSysctrlCommSendWakeCmdToCan
 Function: This function is to send wake command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendWakeCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendWakeCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/* Sleep */
/****************************************************************************
 Name: RmSysctrlCommSendSleepCmdToCan
 Function: This function is to send sleep command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         sleep_level - The level to sleep like SLEEP_SHALLOW

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendSleepCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>,
                      <SLEEP_LEVEL>);
******************************************************************************/
int RmSysctrlCommSendSleepCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         char sleep_level);

/* Halt */
/****************************************************************************
 Name: RmSysctrlCommSendHaltCmdToCan
 Function: This function is to send halt command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendHaltCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendHaltCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/****************************************************************************
 Name: RmSysctrlCommSendPoweroffCmdToCan
 Function: This function is to send power off command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         high - the high byte to control sub-system power off.
         low - the low byte to control sub-system power off.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendPoweroffCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>,
                      <HIGH>,
                      <LOW>);
******************************************************************************/
int RmSysctrlCommSendPoweroffCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         char high,
                         char low);

/****************************************************************************
 Name: RmSysctrlCommSendPoweronCmdToCan
 Function: This function is to send power on command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         high - the high byte to control sub-system power on.
         low - the low byte to control sub-system power on.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendPoweronCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>,
                      <HIGH>,
                      <LOW>);
******************************************************************************/
int RmSysctrlCommSendPoweronCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         char high,
                         char low);


/* System status */
/****************************************************************************
 Name: RmSysctrlCommSendSysStateChangeNotiToCan
 Function: This function is to send system state change notification message
           onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         state - The system state

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendSysStateChangeNotiToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>,
                      <SYSTEM STATE>);
******************************************************************************/
int RmSysctrlCommSendSysStateChangeNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         unsigned int state);

/* Debug */
/****************************************************************************
 Name: RmSysctrlCommSendDebugCmdToCan
 Function: This function is to send debug command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         debug_en - the option of debug

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendDebugCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>,
                      <OPTION of DEBUG>);
******************************************************************************/
int RmSysctrlCommSendDebugCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         unsigned int debug_en);

/* Mode */
/****************************************************************************
 Name: RmSysctrlCommSendModeCmdToCan
 Function: This function is to send mode command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         mode - the mode to set

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendModeCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>,
                      <MODE>);
******************************************************************************/
int RmSysctrlCommSendModeCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         unsigned int mode);

/* Restory Factory mode */
/****************************************************************************
 Name: RmSysctrlCommSendRestoreFactoryCmdRespToCan
 Function: This function is to send restore factory mode command response
           message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         result - the result of restoring factory

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendRestoreFactoryCmdRespToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>,
                      <RESULT>);
******************************************************************************/
int RmSysctrlCommSendRestoreFactoryCmdRespToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         int result);

/* Sub-system state */
/****************************************************************************
 Name: RmSysctrlCommSendSubsysStateQueryToCan
 Function: This function is to send sub-system state query message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendSubsysStateQueryToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendSubsysStateQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/* version */
/****************************************************************************
 Name: RmSysctrlCommSendVersionQueryToCan
 Function: This function is to send version query message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendVersionQueryToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendVersionQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);
/* SN */
/****************************************************************************
 Name: RmSysctrlCommSendGetCPUIDCmdToCan
 Function: This function is to send CPUID query message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendGetCPUIDCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendGetCPUIDCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/****************************************************************************
 Name: RmSysctrlCommSendReadSNCmdToCan
 Function: This function is to send SN query message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendReadSNCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendReadSNCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);
/* datetime */
/****************************************************************************
 Name: RmSysctrlCommSendDatetimeQueryToCan
 Function: This function is to send datetime query message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendDatetimeQueryToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendDatetimeQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);
/****************************************************************************
 Name: RmSysctrlCommSendSetDatetimeCmdToCan
 Function: This function is to send set datetime message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         datetime - The datetime to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendSetDatetimeCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>,
                      <DATETIME>);
******************************************************************************/
int RmSysctrlCommSendSetDatetimeCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         void *datetime);
/* sex */
/****************************************************************************
 Name: RmSysctrlCommSendSexQueryToCan
 Function: This function is to send sex query message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendSexQueryToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendSexQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);
/****************************************************************************
 Name: RmSysctrlCommSendSetSexCmdToCan
 Function: This function is to send set sex message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         sex - The sex to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendSetSexCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>,
                      <SEX>);
******************************************************************************/
int RmSysctrlCommSendSetSexCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         unsigned short sex);

/* Upgrade */
/****************************************************************************
 Name: RmSysctrlCommSendUpgradeCmdToCan
 Function: This function is to send upgrade command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendUpgradeCmdToCan(<POINT of COMMUNICATION>,
                                                     <PRIORITY>,
                                                     <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendUpgradeCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/****************************************************************************
 Name: RmSysctrlCommSendExitUpgradeCmdToCan
 Function: This function is to send exit upgrade command message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendExitUpgradeCmdToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendExitUpgradeCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);
/* uptime */
/****************************************************************************
 Name: RmSysctrlCommSendUptimeQueryToCan
 Function: This function is to send uptime query message onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendUptimeQueryToCan(
                      <POINT of COMMUNICATION>,
                      <PRIORITY>,
                      <DESTINATION MODULE ID>);
******************************************************************************/
int RmSysctrlCommSendUptimeQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id);

/* HDMI */
/****************************************************************************
 Name: RmSysctrlCommSendHDMIStateChangeNotiToCan
 Function: This function is to send hdmi state change notification message
           onto can.
 Params: comm - The point of a communication.
         priority - The priority.
         dst_module_id - The destination module.
         is_plug_in - The hdmi state, true: plug in, false: plug out

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlCommSendHDMIStateChangeNotiToCan(
                                                      <POINT of COMMUNICATION>,
                                                      <PRIORITY>,
                                                      <DESTINATION MODULE ID>,
                                                      <IS_PLUG_IN>);
******************************************************************************/
int RmSysctrlCommSendHDMIStateChangeNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         bool is_plug_in);
#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_COMM_H_*/
