/*************************************************
 Copyright (C), 2006-2015, Avatarmind. Co., Ltd.
 File name: rm_sysctrl_custom.h
 Author: wangkun
 Version: v1.0
 Date: 2015-5-27
 Description: This program defined macro for customization.
 Others: None
 
 Function List: 
   1. none

 
 History:
   V1.0    2015-5-27    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_CUSTOM_H_
#define RM_SYSCTRL_CUSTOM_H_

#undef DEEP_SLEEP_SUPPORT
#undef AUTO_SLEEP_SUPPORT
#define SUB_SYS_STATUS_QUERY_IN_DEMAND_SUPPORT
#define GET_RM_VERSION_FROM_PROPERTY
#undef UNICAST_ALL_INSTEAD_OF_MULTICAST_SUPPORT
#define MANDATORY_ENABLE_LOW_POWER_MODE_SUPPORT
#define POWER_CHARGE_SUPPORT

#endif /*RM_SYSCTRL_CUSTOM_H_*/
