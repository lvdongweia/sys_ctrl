/*************************************************
 Copyright (C), 2014-2016, Avatarmind. Co., Ltd.
 File name: rm_sysctrl_uevent.h
 Author: wangkun
 Version: v1.0
 Date: 2016-8-31
 Description: This program receives uevent from kerner and
              broadcast to sub-system.
 Others: None
 
 Function List: 
   1. RmSysctrlUeventThread() to implement the uevent receiver
      thread.
 
 History:
   V1.0    2016-8-31    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_UEVENT_H_
#define RM_SYSCTRL_UEVENT_H_

#ifdef __cplusplus
extern "C"{
#endif

enum RmSysctrlUevent {
    HDMI	= 0,
};

/****************************************************************************
 Name: RmSysctrlUeventThread
 Function: This function is to implement the uevent receiver thread.
 Params: arg - The parameter of thread like rm_sysctrl.

 Return:
   None

 Systax:
   ret = pthread_create(&rm_sysctrl->ntid_uevent,
                        NULL,
                        RmSysctrlUeventThread,
                        rm_sysctrl);
******************************************************************************/
void *RmSysctrlUeventThread(void *arg);


#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_UEVENT_H_*/
