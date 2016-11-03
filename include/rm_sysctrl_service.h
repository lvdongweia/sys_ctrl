/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_service.h
 Author: wangkun
 Version: v1.0
 Date: 2015-5-19
 Description: This program defined the service interfaces.
 Others: None
 
 Function List: 
   1. RmSysctrlSrvThread() to implement the service thread.

 
 History:
   V1.0    2015-5-19    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_SERVICE_H_
#define RM_SYSCTRL_SERVICE_H_

#ifdef __cplusplus
extern "C"{
#endif

enum RmSysctrlService {
    SUB_SYSTEM_VERSION	= 0,
    SERIAL_NUMBER	= 1,
    IS_IN_MOTION	= 2,
    SUB_SYSTEM_STATE	= 3,
    GET_SEX		= 4,
    SET_SEX		= 5,
    GET_DATETIME	= 6,
    SET_DATETIME	= 7,
    GET_UPTIME		= 8,
};

#define SERVICE "rm.sysctrl.srv"

/****************************************************************************
 Name: RmSysctrlSrvThread
 Function: This function is to implement the service thread.
 Params: arg - The parameter of thread like rm_sysctrl.

 Return:
   None

 Systax:
   ret = pthread_create(&rm_sysctrl->ntid_cli,
                        NULL,
                        RmSysctrlSrvThread,
                        rm_sysctrl);
******************************************************************************/
void *RmSysctrlSrvThread(void *arg);

#ifdef SUB_SYS_STATUS_QUERY_IN_DEMAND_SUPPORT
/****************************************************************************
 Name: RmSysctrlDecreaseNeedUpdateSubSysStatus
 Function: This function is to implement the decrease the number need update
           the sub-system status.
 Params: None

 Return:
   None

 Systax:
   RmSysctrlDecreaseNeedUpdateSubSysStatus();
******************************************************************************/
void RmSysctrlDecreaseNeedUpdateSubSysStatus();
#endif


#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_SERVICE_H_*/
