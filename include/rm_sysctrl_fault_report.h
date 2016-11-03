/*************************************************
 Copyright (C), 2014-2016, Avatarmind. Co., Ltd.
 File name: rm_sysctrl_fault_report.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-4
 Description: This program defined the sysctrl fault report.
 Others: None
 
 Function List: 
    1. RmSysctrlFaultReportFaultDetail() to report the fault detail information
       into RmFault module.
    2. RmSysctrlFaultReportFaultGeneral() to report the general fault into
       RmFault module.
 
 History:
   V1.0    2014-9-4    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_FAULT_REPORT_H_
#define RM_SYSCTRL_FAULT_REPORT_H_

#ifdef __cplusplus
extern "C"{
#endif

#define SUB_SYSTEM_MAX SUB_SYSTEM_ID_MAX-1

enum RmSysctrlFaultReportID {
    /*General*/
    FAULT_REPORT_SUBSYS_DEAD				= 0,
    FAULT_REPORT_EXPECT_STATE_EXPIRE			= 1,
    FAULT_REPORT_INIT_SUBSYS_EXPIRE			= 2,
    FAULT_REPORT_DEEP_SLEEP_EXPIRE			= 3,
    FAULT_REPORT_DEEP_WAKE_EXPIRE			= 4,
    FAULT_REPORT_DEBUG_SET_EXPIRE			= 5,
    FAULT_REPORT_MODE_SET_EXPIRE			= 6,
    FAULT_REPORT_SUBSYS_FAULT				= 7,
    FAULT_REPORT_RESERVED1				= 8,
    FAULT_REPORT_RESERVED2				= 9,

    /*Detail*/
    FAULT_REPORT_RM_SUBSYS_DEAD				= 10,
    FAULT_REPORT_RC_SUBSYS_DEAD				= 11,
    FAULT_REPORT_RP_SUBSYS_DEAD				= 12,
    FAULT_REPORT_RF_SUBSYS_DEAD				= 13,
    FAULT_REPORT_RBR_SUBSYS_DEAD			= 14,
    FAULT_REPORT_RBL_SUBSYS_DEAD			= 15,
    FAULT_REPORT_RBB_SUBSYS_DEAD			= 16,

    FAULT_REPORT_EXPECT_COMPUTE_STATE_RM_EXPIRE		= 17,
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RC_EXPIRE		= 18,
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RP_EXPIRE		= 19,
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RF_EXPIRE		= 20,
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RBR_EXPIRE	= 21,
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RBL_EXPIRE	= 22,
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RBB_EXPIRE	= 23,

    FAULT_REPORT_EXPECT_INIT_STATE_RM_EXPIRE		= 24,
    FAULT_REPORT_EXPECT_INIT_STATE_RC_EXPIRE		= 25,
    FAULT_REPORT_EXPECT_INIT_STATE_RP_EXPIRE		= 26,
    FAULT_REPORT_EXPECT_INIT_STATE_RF_EXPIRE		= 27,
    FAULT_REPORT_EXPECT_INIT_STATE_RBR_EXPIRE		= 28,
    FAULT_REPORT_EXPECT_INIT_STATE_RBL_EXPIRE		= 29,
    FAULT_REPORT_EXPECT_INIT_STATE_RBB_EXPIRE		= 30,

    FAULT_REPORT_EXPECT_REGISTER_STATE_RM_EXPIRE	= 31,
    FAULT_REPORT_EXPECT_REGISTER_STATE_RC_EXPIRE	= 32,
    FAULT_REPORT_EXPECT_REGISTER_STATE_RP_EXPIRE	= 33,
    FAULT_REPORT_EXPECT_REGISTER_STATE_RF_EXPIRE	= 34,
    FAULT_REPORT_EXPECT_REGISTER_STATE_RBR_EXPIRE	= 35,
    FAULT_REPORT_EXPECT_REGISTER_STATE_RBL_EXPIRE	= 36,
    FAULT_REPORT_EXPECT_REGISTER_STATE_RBB_EXPIRE	= 37,

    FAULT_REPORT_EXPECT_LOGINNING_STATE_RM_EXPIRE	= 38,
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RC_EXPIRE	= 39,
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RP_EXPIRE	= 40,
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RF_EXPIRE	= 41,
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RBR_EXPIRE	= 42,
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RBL_EXPIRE	= 43,
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RBB_EXPIRE	= 44,

    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RM_EXPIRE	= 45,
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RC_EXPIRE	= 46,
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RP_EXPIRE	= 47,
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RF_EXPIRE	= 48,
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RBR_EXPIRE	= 49,
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RBL_EXPIRE	= 50,
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RBB_EXPIRE	= 51,

    FAULT_REPORT_EXPECT_WORK_STATE_RM_EXPIRE		= 52,
    FAULT_REPORT_EXPECT_WORK_STATE_RC_EXPIRE		= 53,
    FAULT_REPORT_EXPECT_WORK_STATE_RP_EXPIRE		= 54,
    FAULT_REPORT_EXPECT_WORK_STATE_RF_EXPIRE		= 55,
    FAULT_REPORT_EXPECT_WORK_STATE_RBR_EXPIRE		= 56,
    FAULT_REPORT_EXPECT_WORK_STATE_RBL_EXPIRE		= 57,
    FAULT_REPORT_EXPECT_WORK_STATE_RBB_EXPIRE		= 58,

    FAULT_REPORT_EXPECT_SLEEP_STATE_RM_EXPIRE		= 59,
    FAULT_REPORT_EXPECT_SLEEP_STATE_RC_EXPIRE		= 60,
    FAULT_REPORT_EXPECT_SLEEP_STATE_RP_EXPIRE		= 61,
    FAULT_REPORT_EXPECT_SLEEP_STATE_RF_EXPIRE		= 62,
    FAULT_REPORT_EXPECT_SLEEP_STATE_RBR_EXPIRE		= 63,
    FAULT_REPORT_EXPECT_SLEEP_STATE_RBL_EXPIRE		= 64,
    FAULT_REPORT_EXPECT_SLEEP_STATE_RBB_EXPIRE		= 65,

    FAULT_REPORT_EXPECT_CHARGE_STATE_RM_EXPIRE		= 66,
    FAULT_REPORT_EXPECT_CHARGE_STATE_RC_EXPIRE		= 67,
    FAULT_REPORT_EXPECT_CHARGE_STATE_RP_EXPIRE		= 68,
    FAULT_REPORT_EXPECT_CHARGE_STATE_RF_EXPIRE		= 69,
    FAULT_REPORT_EXPECT_CHARGE_STATE_RBR_EXPIRE		= 70,
    FAULT_REPORT_EXPECT_CHARGE_STATE_RBL_EXPIRE		= 71,
    FAULT_REPORT_EXPECT_CHARGE_STATE_RBB_EXPIRE		= 72,

    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RM_EXPIRE	= 73,
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RC_EXPIRE	= 74,
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RP_EXPIRE	= 75,
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RF_EXPIRE	= 76,
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RBR_EXPIRE	= 77,
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RBL_EXPIRE	= 78,
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RBB_EXPIRE	= 79,

    FAULT_REPORT_EXPECT_HALT_STATE_RM_EXPIRE		= 80,
    FAULT_REPORT_EXPECT_HALT_STATE_RC_EXPIRE		= 81,
    FAULT_REPORT_EXPECT_HALT_STATE_RP_EXPIRE		= 82,
    FAULT_REPORT_EXPECT_HALT_STATE_RF_EXPIRE		= 83,
    FAULT_REPORT_EXPECT_HALT_STATE_RBR_EXPIRE		= 84,
    FAULT_REPORT_EXPECT_HALT_STATE_RBL_EXPIRE		= 85,
    FAULT_REPORT_EXPECT_HALT_STATE_RBB_EXPIRE		= 86,

    FAULT_REPORT_EXPECT_SECURITY_STATE_RM_EXPIRE	= 87,
    FAULT_REPORT_EXPECT_SECURITY_STATE_RC_EXPIRE	= 88,
    FAULT_REPORT_EXPECT_SECURITY_STATE_RP_EXPIRE	= 89,
    FAULT_REPORT_EXPECT_SECURITY_STATE_RF_EXPIRE	= 90,
    FAULT_REPORT_EXPECT_SECURITY_STATE_RBR_EXPIRE	= 91,
    FAULT_REPORT_EXPECT_SECURITY_STATE_RBL_EXPIRE	= 92,
    FAULT_REPORT_EXPECT_SECURITY_STATE_RBB_EXPIRE	= 93,

    FAULT_REPORT_EXPECT_UPGRADE_STATE_RM_EXPIRE		= 94,
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RC_EXPIRE		= 95,
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RP_EXPIRE		= 96,
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RF_EXPIRE		= 97,
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RBR_EXPIRE	= 98,
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RBL_EXPIRE	= 99,
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RBB_EXPIRE	= 100,

    FAULT_REPORT_INIT_SUBSYS_RM_EXPIRE			= 101,
    FAULT_REPORT_INIT_SUBSYS_RC_EXPIRE			= 102,
    FAULT_REPORT_INIT_SUBSYS_RP_EXPIRE			= 103,
    FAULT_REPORT_INIT_SUBSYS_RF_EXPIRE			= 104,
    FAULT_REPORT_INIT_SUBSYS_RBR_EXPIRE			= 105,
    FAULT_REPORT_INIT_SUBSYS_RBL_EXPIRE			= 106,
    FAULT_REPORT_INIT_SUBSYS_RBB_EXPIRE			= 107,

    FAULT_REPORT_DEBUG_SET_RM_EXPIRE			= 108,
    FAULT_REPORT_DEBUG_SET_RC_EXPIRE			= 109,
    FAULT_REPORT_DEBUG_SET_RP_EXPIRE			= 110,
    FAULT_REPORT_DEBUG_SET_RF_EXPIRE			= 111,
    FAULT_REPORT_DEBUG_SET_RBR_EXPIRE			= 112,
    FAULT_REPORT_DEBUG_SET_RBL_EXPIRE			= 113,
    FAULT_REPORT_DEBUG_SET_RBB_EXPIRE			= 114,

    FAULT_REPORT_MODE_SET_RM_EXPIRE			= 115,
    FAULT_REPORT_MODE_SET_RC_EXPIRE			= 116,
    FAULT_REPORT_MODE_SET_RP_EXPIRE			= 117,
    FAULT_REPORT_MODE_SET_RF_EXPIRE			= 118,
    FAULT_REPORT_MODE_SET_RBR_EXPIRE			= 119,
    FAULT_REPORT_MODE_SET_RBL_EXPIRE			= 120,
    FAULT_REPORT_MODE_SET_RBB_EXPIRE			= 121,

    FAULT_REPORT_SUBSYS_RM_FAULT			= 122,
    FAULT_REPORT_SUBSYS_RC_FAULT			= 123,
    FAULT_REPORT_SUBSYS_RP_FAULT			= 124,
    FAULT_REPORT_SUBSYS_RF_FAULT			= 125,
    FAULT_REPORT_SUBSYS_RBR_FAULT			= 126,
    FAULT_REPORT_SUBSYS_RBL_FAULT			= 127,
    FAULT_REPORT_SUBSYS_RBB_FAULT			= 128,

    FAULT_REPORT_MAX
};


#define FAULT_REPORT_DESCIPTION_STR                                                                              \
    "The sub-system is dead.",                                /*FAULT_REPORT_SUBSYS_DEAD                      */ \
    "The synchronization of state to sub-system is timeout.", /*FAULT_REPORT_EXPECT_STATE_EXPIRE              */ \
    "The init subsys is timeout.",                            /*FAULT_REPORT_INIT_SUBSYS_EXPIRE               */ \
    "The deep sleep is timeout.",                             /*FAULT_REPORT_DEEP_SLEEP_EXPIRE                */ \
    "The deep wake is timeout.",                              /*FAULT_REPORT_DEEP_WAKE_EXPIRE                 */ \
    "The debug setting is timeout.",                          /*FAULT_REPORT_DEBUG_SET_EXPIRE                 */ \
    "The mode setting is timeout.",                           /*FAULT_REPORT_MODE_SET_EXPIRE                  */ \
    "The subsys fault occurs.",                               /*FAULT_REPORT_SUBSYS_FAULT                     */ \
    "Reserved",                                               /*FAULT_REPORT_RESERVED1                        */ \
    "Reserved",                                               /*FAULT_REPORT_RESERVED2                        */ \
    "The sub-system RM is dead.",                             /*FAULT_REPORT_RM_SUBSYS_DEAD                   */ \
    "The sub-system RC is dead.",                             /*FAULT_REPORT_RC_SUBSYS_DEAD                   */ \
    "The sub-system RP is dead.",                             /*FAULT_REPORT_RP_SUBSYS_DEAD                   */ \
    "The sub-system RF is dead.",                             /*FAULT_REPORT_RF_SUBSYS_DEAD                   */ \
    "The sub-system RBR is dead.",                            /*FAULT_REPORT_RBR_SUBSYS_DEAD                  */ \
    "The sub-system RRL is dead.",                            /*FAULT_REPORT_RBL_SUBSYS_DEAD                  */ \
    "The sub-system RRB is dead.",                            /*FAULT_REPORT_RBB_SUBSYS_DEAD                  */ \
    "The sub-system RM is not into compute state in time.",   /*FAULT_REPORT_EXPECT_COMPUTE_STATE_RM_EXPIRE   */ \
    "The sub-system RC is not into compute state in time.",   /*FAULT_REPORT_EXPECT_COMPUTE_STATE_RC_EXPIRE   */ \
    "The sub-system RP is not into compute state in time.",   /*FAULT_REPORT_EXPECT_COMPUTE_STATE_RP_EXPIRE   */ \
    "The sub-system RF is not into compute state in time.",   /*FAULT_REPORT_EXPECT_COMPUTE_STATE_RF_EXPIRE   */ \
    "The sub-system RBR is not into compute state in time.",  /*FAULT_REPORT_EXPECT_COMPUTE_STATE_RBR_EXPIRE  */ \
    "The sub-system RBL is not into compute state in time.",  /*FAULT_REPORT_EXPECT_COMPUTE_STATE_RBL_EXPIRE  */ \
    "The sub-system RBB is not into compute state in time.",  /*FAULT_REPORT_EXPECT_COMPUTE_STATE_RBB_EXPIRE  */ \
    "The sub-system RM is not into init state in time.",      /*FAULT_REPORT_EXPECT_INIT_STATE_RM_EXPIRE      */ \
    "The sub-system RC is not into init state in time.",      /*FAULT_REPORT_EXPECT_INIT_STATE_RC_EXPIRE      */ \
    "The sub-system RP is not into init state in time.",      /*FAULT_REPORT_EXPECT_INIT_STATE_RP_EXPIRE      */ \
    "The sub-system RF is not into init state in time.",      /*FAULT_REPORT_EXPECT_INIT_STATE_RF_EXPIRE      */ \
    "The sub-system RBR is not into init state in time.",     /*FAULT_REPORT_EXPECT_INIT_STATE_RBR_EXPIRE     */ \
    "The sub-system RBL is not into init state in time.",     /*FAULT_REPORT_EXPECT_INIT_STATE_RBL_EXPIRE     */ \
    "The sub-system RBB is not into init state in time.",     /*FAULT_REPORT_EXPECT_INIT_STATE_RBB_EXPIRE     */ \
    "The sub-system RM is not into register state in time.",  /*FAULT_REPORT_EXPECT_REGISTER_STATE_RM_EXPIRE  */ \
    "The sub-system RC is not into register state in time.",  /*FAULT_REPORT_EXPECT_REGISTER_STATE_RC_EXPIRE  */ \
    "The sub-system RP is not into register state in time.",  /*FAULT_REPORT_EXPECT_REGISTER_STATE_RP_EXPIRE  */ \
    "The sub-system RF is not into register state in time.",  /*FAULT_REPORT_EXPECT_REGISTER_STATE_RF_EXPIRE  */ \
    "The sub-system RBR is not into register state in time.", /*FAULT_REPORT_EXPECT_REGISTER_STATE_RBR_EXPIRE */ \
    "The sub-system RBL is not into register state in time.", /*FAULT_REPORT_EXPECT_REGISTER_STATE_RBL_EXPIRE */ \
    "The sub-system RBB is not into register state in time.", /*FAULT_REPORT_EXPECT_REGISTER_STATE_RBB_EXPIRE */ \
    "The sub-system RM is not into loginning state in time.", /*FAULT_REPORT_EXPECT_LOGINNING_STATE_RM_EXPIRE */ \
    "The sub-system RC is not into loginning state in time.", /*FAULT_REPORT_EXPECT_LOGINNING_STATE_RC_EXPIRE */ \
    "The sub-system RP is not into loginning state in time.", /*FAULT_REPORT_EXPECT_LOGINNING_STATE_RP_EXPIRE */ \
    "The sub-system RF is not into loginning state in time.", /*FAULT_REPORT_EXPECT_LOGINNING_STATE_RF_EXPIRE */ \
    "The sub-system RBR is not into loginning state in time.",/*FAULT_REPORT_EXPECT_LOGINNING_STATE_RBR_EXPIRE*/ \
    "The sub-system RBL is not into loginning state in time.",/*FAULT_REPORT_EXPECT_LOGINNING_STATE_RBL_EXPIRE*/ \
    "The sub-system RBB is not into loginning state in time.",/*FAULT_REPORT_EXPECT_LOGINNING_STATE_RBB_EXPIRE*/ \
    "The sub-system RM is not into unlogin state in time.",   /*FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RM_EXPIRE */ \
    "The sub-system RC is not into unlogin state in time.",   /*FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RC_EXPIRE */ \
    "The sub-system RP is not into unlogin state in time.",   /*FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RP_EXPIRE */ \
    "The sub-system RF is not into unlogin state in time.",   /*FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RF_EXPIRE */ \
    "The sub-system RBR is not into unlogin state in time.",  /*FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RBR_EXPIRE*/ \
    "The sub-system RBL is not into unlogin state in time.",  /*FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RBL_EXPIRE*/ \
    "The sub-system RBB is not into unlogin state in time.",  /*FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RBB_EXPIRE*/ \
    "The sub-system RM is not into work state in time.",      /*FAULT_REPORT_EXPECT_WORK_STATE_RM_EXPIRE      */ \
    "The sub-system RC is not into work state in time.",      /*FAULT_REPORT_EXPECT_WORK_STATE_RC_EXPIRE      */ \
    "The sub-system RP is not into work state in time.",      /*FAULT_REPORT_EXPECT_WORK_STATE_RP_EXPIRE      */ \
    "The sub-system RF is not into work state in time.",      /*FAULT_REPORT_EXPECT_WORK_STATE_RF_EXPIRE      */ \
    "The sub-system RBR is not into work state in time.",     /*FAULT_REPORT_EXPECT_WORK_STATE_RBR_EXPIRE     */ \
    "The sub-system RBL is not into work state in time.",     /*FAULT_REPORT_EXPECT_WORK_STATE_RBL_EXPIRE     */ \
    "The sub-system RBB is not into work state in time.",     /*FAULT_REPORT_EXPECT_WORK_STATE_RBB_EXPIRE     */ \
    "The sub-system RM is not into sleep state in time.",     /*FAULT_REPORT_EXPECT_SLEEP_STATE_RM_EXPIRE     */ \
    "The sub-system RC is not into sleep state in time.",     /*FAULT_REPORT_EXPECT_SLEEP_STATE_RC_EXPIRE     */ \
    "The sub-system RP is not into sleep state in time.",     /*FAULT_REPORT_EXPECT_SLEEP_STATE_RP_EXPIRE     */ \
    "The sub-system RF is not into sleep state in time.",     /*FAULT_REPORT_EXPECT_SLEEP_STATE_RF_EXPIRE     */ \
    "The sub-system RBR is not into sleep state in time.",    /*FAULT_REPORT_EXPECT_SLEEP_STATE_RBR_EXPIRE    */ \
    "The sub-system RBL is not into sleep state in time.",    /*FAULT_REPORT_EXPECT_SLEEP_STATE_RBL_EXPIRE    */ \
    "The sub-system RBB is not into sleep state in time.",    /*FAULT_REPORT_EXPECT_SLEEP_STATE_RBB_EXPIRE    */ \
    "The sub-system RM is not into charge state in time.",    /*FAULT_REPORT_EXPECT_CHARGE_STATE_RM_EXPIRE    */ \
    "The sub-system RC is not into charge state in time.",    /*FAULT_REPORT_EXPECT_CHARGE_STATE_RC_EXPIRE    */ \
    "The sub-system RP is not into charge state in time.",    /*FAULT_REPORT_EXPECT_CHARGE_STATE_RP_EXPIRE    */ \
    "The sub-system RF is not into charge state in time.",    /*FAULT_REPORT_EXPECT_CHARGE_STATE_RF_EXPIRE    */ \
    "The sub-system RBR is not into charge state in time.",   /*FAULT_REPORT_EXPECT_CHARGE_STATE_RBR_EXPIRE   */ \
    "The sub-system RBL is not into charge state in time.",   /*FAULT_REPORT_EXPECT_CHARGE_STATE_RBL_EXPIRE   */ \
    "The sub-system RBB is not into charge state in time.",   /*FAULT_REPORT_EXPECT_CHARGE_STATE_RBB_EXPIRE   */ \
    "The sub-system RM is not into abnormal state in time.",  /*FAULT_REPORT_EXPECT_ABNORMAL_STATE_RM_EXPIRE  */ \
    "The sub-system RC is not into abnormal state in time.",  /*FAULT_REPORT_EXPECT_ABNORMAL_STATE_RC_EXPIRE  */ \
    "The sub-system RP is not into abnormal state in time.",  /*FAULT_REPORT_EXPECT_ABNORMAL_STATE_RP_EXPIRE  */ \
    "The sub-system RF is not into abnormal state in time.",  /*FAULT_REPORT_EXPECT_ABNORMAL_STATE_RF_EXPIRE  */ \
    "The sub-system RBR is not into abnormal state in time.", /*FAULT_REPORT_EXPECT_ABNORMAL_STATE_RBR_EXPIRE */ \
    "The sub-system RBL is not into abnormal state in time.", /*FAULT_REPORT_EXPECT_ABNORMAL_STATE_RBL_EXPIRE */ \
    "The sub-system RBB is not into abnormal state in time.", /*FAULT_REPORT_EXPECT_ABNORMAL_STATE_RBB_EXPIRE */ \
    "The sub-system RM is not into halt state in time.",      /*FAULT_REPORT_EXPECT_HALT_STATE_RM_EXPIRE      */ \
    "The sub-system RC is not into halt state in time.",      /*FAULT_REPORT_EXPECT_HALT_STATE_RC_EXPIRE      */ \
    "The sub-system RP is not into halt state in time.",      /*FAULT_REPORT_EXPECT_HALT_STATE_RP_EXPIRE      */ \
    "The sub-system RF is not into halt state in time.",      /*FAULT_REPORT_EXPECT_HALT_STATE_RF_EXPIRE      */ \
    "The sub-system RBR is not into halt stat in timee.",     /*FAULT_REPORT_EXPECT_HALT_STATE_RBR_EXPIRE     */ \
    "The sub-system RBL is not into halt state in time.",     /*FAULT_REPORT_EXPECT_HALT_STATE_RBL_EXPIRE     */ \
    "The sub-system RBB is not into halt state in time.",     /*FAULT_REPORT_EXPECT_HALT_STATE_RBB_EXPIRE     */ \
    "The sub-system RM is not into security state in time.",  /*FAULT_REPORT_EXPECT_SECURITY_STATE_RM_EXPIRE  */ \
    "The sub-system RC is not into security state in time.",  /*FAULT_REPORT_EXPECT_SECURITY_STATE_RC_EXPIRE  */ \
    "The sub-system RP is not into security state in time.",  /*FAULT_REPORT_EXPECT_SECURITY_STATE_RP_EXPIRE  */ \
    "The sub-system RF is not into security state in time.",  /*FAULT_REPORT_EXPECT_SECURITY_STATE_RF_EXPIRE  */ \
    "The sub-system RBR is not into security stat in timee.", /*FAULT_REPORT_EXPECT_SECURITY_STATE_RBR_EXPIRE */ \
    "The sub-system RBL is not into security state in time.", /*FAULT_REPORT_EXPECT_SECURITY_STATE_RBL_EXPIRE */ \
    "The sub-system RBB is not into security state in time.", /*FAULT_REPORT_EXPECT_SECURITY_STATE_RBB_EXPIRE */ \
    "The sub-system RM is not into upgrade state in time.",   /*FAULT_REPORT_EXPECT_UPGRADE_STATE_RM_EXPIRE   */ \
    "The sub-system RC is not into upgrade state in time.",   /*FAULT_REPORT_EXPECT_UPGRADE_STATE_RC_EXPIRE   */ \
    "The sub-system RP is not into upgrade state in time.",   /*FAULT_REPORT_EXPECT_UPGRADE_STATE_RP_EXPIRE   */ \
    "The sub-system RF is not into upgrade state in time.",   /*FAULT_REPORT_EXPECT_UPGRADE_STATE_RF_EXPIRE   */ \
    "The sub-system RBR is not into upgrade stat in timee.",  /*FAULT_REPORT_EXPECT_UPGRADE_STATE_RBR_EXPIRE  */ \
    "The sub-system RBL is not into upgrade state in time.",  /*FAULT_REPORT_EXPECT_UPGRADE_STATE_RBL_EXPIRE  */ \
    "The sub-system RBB is not into upgrade state in time.",  /*FAULT_REPORT_EXPECT_UPGRADE_STATE_RBB_EXPIRE  */ \
    "The sub-system RM is not initalized in time.",           /*FAULT_REPORT_INIT_SUBSYS_RM_EXPIRE            */ \
    "The sub-system RC is not initalized in time.",           /*FAULT_REPORT_INIT_SUBSYS_RC_EXPIRE            */ \
    "The sub-system RP is not initalized in time.",           /*FAULT_REPORT_INIT_SUBSYS_RP_EXPIRE            */ \
    "The sub-system RF is not initalized in time.",           /*FAULT_REPORT_INIT_SUBSYS_RF_EXPIRE            */ \
    "The sub-system RBR is not initalized in time.",          /*FAULT_REPORT_INIT_SUBSYS_RBR_EXPIRE           */ \
    "The sub-system RBL is not initalized in time.",          /*FAULT_REPORT_INIT_SUBSYS_RBL_EXPIRE           */ \
    "The sub-system RBB is not initalized in time.",          /*FAULT_REPORT_INIT_SUBSYS_RBB_EXPIRE           */ \
    "The sub-system RM debug is not opened in time.",         /*FAULT_REPORT_DEBUG_SET_RM_EXPIRE              */ \
    "The sub-system RC debug is not opened in time.",         /*FAULT_REPORT_DEBUG_SET_RC_EXPIRE              */ \
    "The sub-system RP debug is not opened in time.",         /*FAULT_REPORT_DEBUG_SET_RP_EXPIRE              */ \
    "The sub-system RF debug is not opened in time.",         /*FAULT_REPORT_DEBUG_SET_RF_EXPIRE              */ \
    "The sub-system RBR debug is not opened in time.",        /*FAULT_REPORT_DEBUG_SET_RBR_EXPIRE             */ \
    "The sub-system RBL debug is not opened in time.",        /*FAULT_REPORT_DEBUG_SET_RBL_EXPIRE             */ \
    "The sub-system RBB debug is not opened in time.",        /*FAULT_REPORT_DEBUG_SET_RBB_EXPIRE             */ \
    "The sub-system RM mode is not set in time.",             /*FAULT_REPORT_MODE_SET_RM_EXPIRE               */ \
    "The sub-system RC mode is not set in time.",             /*FAULT_REPORT_MODE_SET_RC_EXPIRE               */ \
    "The sub-system RP mode is not set in time.",             /*FAULT_REPORT_MODE_SET_RP_EXPIRE               */ \
    "The sub-system RF mode is not set in time.",             /*FAULT_REPORT_MODE_SET_RF_EXPIRE               */ \
    "The sub-system RBR mode is not set in time.",            /*FAULT_REPORT_MODE_SET_RBR_EXPIRE              */ \
    "The sub-system RBL mode is not set in time.",            /*FAULT_REPORT_MODE_SET_RBL_EXPIRE              */ \
    "The sub-system RBB mode is not set in time.",            /*FAULT_REPORT_MODE_SET_RBB_EXPIRE              */ \
    "The sub-system RM fault occured.",                       /*FAULT_REPORT_SUBSYS_RM_FAULT                  */ \
    "The sub-system RC fault occured.",                       /*FAULT_REPORT_SUBSYS_RC_FAULT                  */ \
    "The sub-system RP fault occured.",                       /*FAULT_REPORT_SUBSYS_RP_FAULT                  */ \
    "The sub-system RF fault occured.",                       /*FAULT_REPORT_SUBSYS_RF_FAULT                  */ \
    "The sub-system RBR fault occured.",                      /*FAULT_REPORT_SUBSYS_RBR_FAULT                 */ \
    "The sub-system RBL fault occured.",                      /*FAULT_REPORT_SUBSYS_RBL_FAULT                 */ \
    "The sub-system RBB fault occured.",                      /*FAULT_REPORT_SUBSYS_RBB_FAULT                 */ \


#define FAULT_SUBSYS_DEAD_ID_PER_SUBSYSTEM         \
    FAULT_REPORT_RM_SUBSYS_DEAD, /*RM_SYSTEM_ID*/  \
    FAULT_REPORT_RC_SUBSYS_DEAD, /*RC_SYSTEM_ID*/  \
    FAULT_REPORT_RP_SUBSYS_DEAD, /*RP_SYSTEM_ID*/  \
    FAULT_REPORT_RF_SUBSYS_DEAD, /*RF_SYSTEM_ID*/  \
    FAULT_REPORT_RBR_SUBSYS_DEAD,/*RB_R_ARM_ID */  \
    FAULT_REPORT_RBL_SUBSYS_DEAD,/*RB_L_ARM_ID */  \
    FAULT_REPORT_RBB_SUBSYS_DEAD,/*RB_BODY_ID  */

#define FAULT_EXPECT_STATE_EXPIRE_ID_PER_STATUS_SUBSYS                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RM_EXPIRE,   /*ROBOT_STATUS_COMPUTE|RM_SYSTEM_ID     */ \
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RC_EXPIRE,   /*ROBOT_STATUS_COMPUTE|RC_SYSTEM_ID     */ \
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RP_EXPIRE,   /*ROBOT_STATUS_COMPUTE|RP_SYSTEM_ID     */ \
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RF_EXPIRE,   /*ROBOT_STATUS_COMPUTE|RF_SYSTEM_ID     */ \
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RBR_EXPIRE,  /*ROBOT_STATUS_COMPUTE|RB_R_ARM_ID      */ \
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RBL_EXPIRE,  /*ROBOT_STATUS_COMPUTE|RB_L_ARM_ID      */ \
    FAULT_REPORT_EXPECT_COMPUTE_STATE_RBB_EXPIRE,  /*ROBOT_STATUS_COMPUTE|RB_BODY_ID       */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_INIT_STATE_RM_EXPIRE,      /*ROBOT_STATUS_INIT|RM_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_INIT_STATE_RC_EXPIRE,      /*ROBOT_STATUS_INIT|RC_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_INIT_STATE_RP_EXPIRE,      /*ROBOT_STATUS_INIT|RP_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_INIT_STATE_RF_EXPIRE,      /*ROBOT_STATUS_INIT|RF_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_INIT_STATE_RBR_EXPIRE,     /*ROBOT_STATUS_INIT|RB_R_ARM_ID         */ \
    FAULT_REPORT_EXPECT_INIT_STATE_RBL_EXPIRE,     /*ROBOT_STATUS_INIT|RB_L_ARM_ID         */ \
    FAULT_REPORT_EXPECT_INIT_STATE_RBB_EXPIRE,     /*ROBOT_STATUS_INIT|RB_BODY_ID          */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_REGISTER_STATE_RM_EXPIRE,  /*ROBOT_STATUS_REGISTER|RM_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_REGISTER_STATE_RC_EXPIRE,  /*ROBOT_STATUS_REGISTER|RC_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_REGISTER_STATE_RP_EXPIRE,  /*ROBOT_STATUS_REGISTER|RP_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_REGISTER_STATE_RF_EXPIRE,  /*ROBOT_STATUS_REGISTER|RF_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_REGISTER_STATE_RBR_EXPIRE, /*ROBOT_STATUS_REGISTER|RB_R_ARM_ID     */ \
    FAULT_REPORT_EXPECT_REGISTER_STATE_RBL_EXPIRE, /*ROBOT_STATUS_REGISTER|RB_L_ARM_ID     */ \
    FAULT_REPORT_EXPECT_REGISTER_STATE_RBB_EXPIRE, /*ROBOT_STATUS_REGISTER|RB_BODY_ID      */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RM_EXPIRE, /*ROBOT_STATUS_LOGINNING|RM_SYSTEM_ID   */ \
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RC_EXPIRE, /*ROBOT_STATUS_LOGINNING|RC_SYSTEM_ID   */ \
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RP_EXPIRE, /*ROBOT_STATUS_LOGINNING|RP_SYSTEM_ID   */ \
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RF_EXPIRE, /*ROBOT_STATUS_LOGINNING|RF_SYSTEM_ID   */ \
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RBR_EXPIRE,/*ROBOT_STATUS_LOGINNING|RB_R_ARM_ID    */ \
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RBL_EXPIRE,/*ROBOT_STATUS_LOGINNING|RB_L_ARM_ID    */ \
    FAULT_REPORT_EXPECT_LOGINNING_STATE_RBB_EXPIRE,/*ROBOT_STATUS_LOGINNING|RB_BODY_ID     */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RM_EXPIRE, /*ROBOT_STATUS_NOT_LOGIN|RM_SYSTEM_ID   */ \
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RC_EXPIRE, /*ROBOT_STATUS_NOT_LOGIN|RC_SYSTEM_ID   */ \
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RP_EXPIRE, /*ROBOT_STATUS_NOT_LOGIN|RP_SYSTEM_ID   */ \
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RF_EXPIRE, /*ROBOT_STATUS_NOT_LOGIN|RF_SYSTEM_ID   */ \
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RBR_EXPIRE,/*ROBOT_STATUS_NOT_LOGIN|RB_R_ARM_ID    */ \
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RBL_EXPIRE,/*ROBOT_STATUS_NOT_LOGIN|RB_L_ARM_ID    */ \
    FAULT_REPORT_EXPECT_NOT_LOGIN_STATE_RBB_EXPIRE,/*ROBOT_STATUS_NOT_LOGIN|RB_BODY_ID     */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_WORK_STATE_RM_EXPIRE,      /*ROBOT_STATUS_WORK|RM_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_WORK_STATE_RC_EXPIRE,      /*ROBOT_STATUS_WORK|RC_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_WORK_STATE_RP_EXPIRE,      /*ROBOT_STATUS_WORK|RP_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_WORK_STATE_RF_EXPIRE,      /*ROBOT_STATUS_WORK|RF_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_WORK_STATE_RBR_EXPIRE,     /*ROBOT_STATUS_WORK|RB_R_ARM_ID         */ \
    FAULT_REPORT_EXPECT_WORK_STATE_RBL_EXPIRE,     /*ROBOT_STATUS_WORK|RB_L_ARM_ID         */ \
    FAULT_REPORT_EXPECT_WORK_STATE_RBB_EXPIRE,     /*ROBOT_STATUS_WORK|RB_BODY_ID          */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_SLEEP_STATE_RM_EXPIRE,     /*ROBOT_STATUS_SLEEP|RM_SYSTEM_ID       */ \
    FAULT_REPORT_EXPECT_SLEEP_STATE_RC_EXPIRE,     /*ROBOT_STATUS_SLEEP|RC_SYSTEM_ID       */ \
    FAULT_REPORT_EXPECT_SLEEP_STATE_RP_EXPIRE,     /*ROBOT_STATUS_SLEEP|RP_SYSTEM_ID       */ \
    FAULT_REPORT_EXPECT_SLEEP_STATE_RF_EXPIRE,     /*ROBOT_STATUS_SLEEP|RF_SYSTEM_ID       */ \
    FAULT_REPORT_EXPECT_SLEEP_STATE_RBR_EXPIRE,    /*ROBOT_STATUS_SLEEP|RB_R_ARM_ID        */ \
    FAULT_REPORT_EXPECT_SLEEP_STATE_RBL_EXPIRE,    /*ROBOT_STATUS_SLEEP|RB_L_ARM_ID        */ \
    FAULT_REPORT_EXPECT_SLEEP_STATE_RBB_EXPIRE,    /*ROBOT_STATUS_SLEEP|RB_BODY_ID         */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_CHARGE_STATE_RM_EXPIRE,    /*ROBOT_STATUS_CHARGE|RM_SYSTEM_ID      */ \
    FAULT_REPORT_EXPECT_CHARGE_STATE_RC_EXPIRE,    /*ROBOT_STATUS_CHARGE|RC_SYSTEM_ID      */ \
    FAULT_REPORT_EXPECT_CHARGE_STATE_RP_EXPIRE,    /*ROBOT_STATUS_CHARGE|RP_SYSTEM_ID      */ \
    FAULT_REPORT_EXPECT_CHARGE_STATE_RF_EXPIRE,    /*ROBOT_STATUS_CHARGE|RF_SYSTEM_ID      */ \
    FAULT_REPORT_EXPECT_CHARGE_STATE_RBR_EXPIRE,   /*ROBOT_STATUS_CHARGE|RB_R_ARM_ID       */ \
    FAULT_REPORT_EXPECT_CHARGE_STATE_RBL_EXPIRE,   /*ROBOT_STATUS_CHARGE|RB_L_ARM_ID       */ \
    FAULT_REPORT_EXPECT_CHARGE_STATE_RBB_EXPIRE,   /*ROBOT_STATUS_CHARGE|RB_BODY_ID        */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RM_EXPIRE,  /*ROBOT_STATUS_ABNORMAL|RM_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RC_EXPIRE,  /*ROBOT_STATUS_ABNORMAL|RC_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RP_EXPIRE,  /*ROBOT_STATUS_ABNORMAL|RP_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RF_EXPIRE,  /*ROBOT_STATUS_ABNORMAL|RF_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RBR_EXPIRE, /*ROBOT_STATUS_ABNORMAL|RB_R_ARM_ID     */ \
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RBL_EXPIRE, /*ROBOT_STATUS_ABNORMAL|RB_L_ARM_ID     */ \
    FAULT_REPORT_EXPECT_ABNORMAL_STATE_RBB_EXPIRE, /*ROBOT_STATUS_ABNORMAL|RB_BODY_ID      */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_HALT_STATE_RM_EXPIRE,      /*ROBOT_STATUS_HALT|RM_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_HALT_STATE_RC_EXPIRE,      /*ROBOT_STATUS_HALT|RC_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_HALT_STATE_RP_EXPIRE,      /*ROBOT_STATUS_HALT|RP_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_HALT_STATE_RF_EXPIRE,      /*ROBOT_STATUS_HALT|RF_SYSTEM_ID        */ \
    FAULT_REPORT_EXPECT_HALT_STATE_RBR_EXPIRE,     /*ROBOT_STATUS_HALT|RB_R_ARM_ID         */ \
    FAULT_REPORT_EXPECT_HALT_STATE_RBL_EXPIRE,     /*ROBOT_STATUS_HALT|RB_L_ARM_ID         */ \
    FAULT_REPORT_EXPECT_HALT_STATE_RBB_EXPIRE,     /*ROBOT_STATUS_HALT|RB_BODY_ID          */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_SECURITY_STATE_RM_EXPIRE,  /*ROBOT_STATUS_SECURITY|RM_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_SECURITY_STATE_RC_EXPIRE,  /*ROBOT_STATUS_SECURITY|RC_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_SECURITY_STATE_RP_EXPIRE,  /*ROBOT_STATUS_SECURITY|RP_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_SECURITY_STATE_RF_EXPIRE,  /*ROBOT_STATUS_SECURITY|RF_SYSTEM_ID    */ \
    FAULT_REPORT_EXPECT_SECURITY_STATE_RBR_EXPIRE, /*ROBOT_STATUS_SECURITY|RB_R_ARM_ID     */ \
    FAULT_REPORT_EXPECT_SECURITY_STATE_RBL_EXPIRE, /*ROBOT_STATUS_SECURITY|RB_L_ARM_ID     */ \
    FAULT_REPORT_EXPECT_SECURITY_STATE_RBB_EXPIRE, /*ROBOT_STATUS_SECURITY|RB_BODY_ID      */ \
    },                                                                                        \
    {                                                                                         \
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RM_EXPIRE,   /*ROBOT_STATUS_UPGRADE|RM_SYSTEM_ID     */ \
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RC_EXPIRE,   /*ROBOT_STATUS_UPGRADE|RC_SYSTEM_ID     */ \
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RP_EXPIRE,   /*ROBOT_STATUS_UPGRADE|RP_SYSTEM_ID     */ \
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RF_EXPIRE,   /*ROBOT_STATUS_UPGRADE|RF_SYSTEM_ID     */ \
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RBR_EXPIRE,  /*ROBOT_STATUS_UPGRADE|RB_R_ARM_ID      */ \
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RBL_EXPIRE,  /*ROBOT_STATUS_UPGRADE|RB_L_ARM_ID      */ \
    FAULT_REPORT_EXPECT_UPGRADE_STATE_RBB_EXPIRE,  /*ROBOT_STATUS_UPGRADE|RB_BODY_ID       */ \
    }


#define FAULT_INIT_SUBSYS_EXPIRE_ID_PER_SUBSYSTEM         \
    FAULT_REPORT_INIT_SUBSYS_RM_EXPIRE, /*RM_SYSTEM_ID*/  \
    FAULT_REPORT_INIT_SUBSYS_RC_EXPIRE, /*RC_SYSTEM_ID*/  \
    FAULT_REPORT_INIT_SUBSYS_RP_EXPIRE, /*RP_SYSTEM_ID*/  \
    FAULT_REPORT_INIT_SUBSYS_RF_EXPIRE, /*RF_SYSTEM_ID*/  \
    FAULT_REPORT_INIT_SUBSYS_RBR_EXPIRE,/*RB_R_ARM_ID */  \
    FAULT_REPORT_INIT_SUBSYS_RBL_EXPIRE,/*RB_L_ARM_ID */  \
    FAULT_REPORT_INIT_SUBSYS_RBB_EXPIRE,/*RB_BODY_ID  */

#define FAULT_DEBUG_SET_EXPIRE_ID_PER_SUBSYSTEM         \
    FAULT_REPORT_DEBUG_SET_RM_EXPIRE, /*RM_SYSTEM_ID*/  \
    FAULT_REPORT_DEBUG_SET_RC_EXPIRE, /*RC_SYSTEM_ID*/  \
    FAULT_REPORT_DEBUG_SET_RP_EXPIRE, /*RP_SYSTEM_ID*/  \
    FAULT_REPORT_DEBUG_SET_RF_EXPIRE, /*RF_SYSTEM_ID*/  \
    FAULT_REPORT_DEBUG_SET_RBR_EXPIRE,/*RB_R_ARM_ID */  \
    FAULT_REPORT_DEBUG_SET_RBL_EXPIRE,/*RB_L_ARM_ID */  \
    FAULT_REPORT_DEBUG_SET_RBB_EXPIRE,/*RB_BODY_ID  */

#define FAULT_MODE_SET_EXPIRE_ID_PER_SUBSYSTEM         \
    FAULT_REPORT_MODE_SET_RM_EXPIRE, /*RM_SYSTEM_ID*/  \
    FAULT_REPORT_MODE_SET_RC_EXPIRE, /*RC_SYSTEM_ID*/  \
    FAULT_REPORT_MODE_SET_RP_EXPIRE, /*RP_SYSTEM_ID*/  \
    FAULT_REPORT_MODE_SET_RF_EXPIRE, /*RF_SYSTEM_ID*/  \
    FAULT_REPORT_MODE_SET_RBR_EXPIRE,/*RB_R_ARM_ID */  \
    FAULT_REPORT_MODE_SET_RBL_EXPIRE,/*RB_L_ARM_ID */  \
    FAULT_REPORT_MODE_SET_RBB_EXPIRE,/*RB_BODY_ID  */

#define FAULT_SUBSYS_FAULT_ID_PER_SUBSYSTEM         \
    FAULT_REPORT_SUBSYS_RM_FAULT, /*RM_SYSTEM_ID*/  \
    FAULT_REPORT_SUBSYS_RC_FAULT, /*RC_SYSTEM_ID*/  \
    FAULT_REPORT_SUBSYS_RP_FAULT, /*RP_SYSTEM_ID*/  \
    FAULT_REPORT_SUBSYS_RF_FAULT, /*RF_SYSTEM_ID*/  \
    FAULT_REPORT_SUBSYS_RBR_FAULT,/*RB_R_ARM_ID */  \
    FAULT_REPORT_SUBSYS_RBL_FAULT,/*RB_L_ARM_ID */  \
    FAULT_REPORT_SUBSYS_RBB_FAULT,/*RB_BODY_ID  */

/****************************************************************************
 Name: RmSysctrlFaultReportFaultGeneral
 Function: This function is to report the general fault into RmFault module.
 Params: fault - The point of a fault.
         fault_id - The fault ID to report.
         level - The level of fault to report.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFaultReportFaultGeneral(<POINT of FAULT>,
                                             <FAULT ID>,
                                             <FAULT LEVEL>);
******************************************************************************/
int RmSysctrlFaultReportFaultGeneral(char src_id, char fault_id, char level);

/****************************************************************************
 Name: RmSysctrlFaultReportFaultDetail
 Function: This function is to report the fault detail information into
           RmFault module.
 Params: fault - The point of a fault.
         fault_id - The fault ID to report.
         level - The level of fault to report.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFaultReportFaultDetail(<POINT of FAULT>,
                                             <FAULT ID>,
                                             <FAULT LEVEL>);
******************************************************************************/
int RmSysctrlFaultReportFaultDetail(char src_id, char fault_id, char level);


#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_FAULT_REPORT_H_*/
