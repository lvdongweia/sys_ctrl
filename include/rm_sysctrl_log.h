/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_log.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-25
 Description: This program defined the sysctrl log.
 Others: None
 
 Function List: 
   None

 
 History:
   V1.0    2014-9-25    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_LOG_H_
#define RM_SYSCTRL_LOG_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <unistd.h>
#include <android/log.h>

int RmSysctrlLog(int level, const char * tag, const char *format, ...);

#ifdef RLOGD
#undef RLOGD
#endif
#ifdef RLOGI
#undef RLOGI
#endif
#ifdef RLOGV
#undef RLOGV
#endif
#ifdef RLOGW
#undef RLOGW
#endif
#ifdef RLOGE
#undef RLOGE
#endif
#ifdef RLOGF
#undef RLOGF
#endif

#define RLOGD(fmt, ...)	RmSysctrlLog(ANDROID_LOG_DEBUG, LOG_TAG, (fmt), ##__VA_ARGS__)
#define RLOGI(fmt, ...)	RmSysctrlLog(ANDROID_LOG_INFO, LOG_TAG, (fmt), ##__VA_ARGS__)
#define RLOGV(fmt, ...)	RmSysctrlLog(ANDROID_LOG_VERBOSE, LOG_TAG, (fmt), ##__VA_ARGS__)
#define RLOGW(fmt, ...)	RmSysctrlLog(ANDROID_LOG_WARN, LOG_TAG, (fmt), ##__VA_ARGS__)
#define RLOGE(fmt, ...)	RmSysctrlLog(ANDROID_LOG_ERROR, LOG_TAG, (fmt), ##__VA_ARGS__)
#define RLOGF(fmt, ...)	RmSysctrlLog(ANDROID_LOG_FATAL, LOG_TAG, (fmt), ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_LOG_H_*/
