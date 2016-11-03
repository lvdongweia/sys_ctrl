/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_cli.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-4
 Description: This program defined the sysctrl cli.
 Others: None
 
 Function List: 
   1. RmSysctrlGetSubsysVersion()
   2. RmSysctrlGetSerialNumber()
   3. RmSysctrlCheckRobotMotion()
   4. RmSysctrlGetSex()
   5. RmSysctrlSetSex()
   6. RmSysctrlGetDatetime()
   7. RmSysctrlSetDatetime()
   8. RmSysctrlGetUptime()
 
 History:
   V1.0    2014-9-4    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_CLIENT_H_
#define RM_SYSCTRL_CLIENT_H_

#ifdef __cplusplus
extern "C"{
#endif

int RmSysctrlGetSubsysVersion(int subsys_id, char *ver);
int RmSysctrlGetSerialNumber(char *sn, bool *is_valid);
int RmSysctrlCheckRobotMotion(bool *is_robot_in_motion);
int RmSysctrlGetSubsysState(int subsys_id, bool *is_enable,
    bool *is_poweron, bool *is_active, int *curr_state,
    int *curr_sub_state, bool *is_init, bool *is_debug,
    int *curr_mode);
int RmSysctrlGetSex(int *robot_sex);
int RmSysctrlSetSex(int robot_sex);
int RmSysctrlGetDatetime(int *o_year, int *o_month,
    int *o_day, int *o_weekday, int *o_hour, int *o_minute,
    int *o_second);
int RmSysctrlSetDatetime(int year, int month,
    int day, int weekday, int hour, int minute,
    int second);
int RmSysctrlGetUptime(unsigned int *robot_uptime);

#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_CLIENT_H_*/
