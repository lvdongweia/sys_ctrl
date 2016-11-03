#include <binder/IServiceManager.h>
#include <binder/IBinder.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <private/binder/binder_module.h>
#include <semaphore.h>

#include "rm_sysctrl_service.h"
#include "robot_common_data.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_fsm.h"
#include "rm_sysctrl_conf.h"
#include "rm_sysctrl_client.h"

using namespace android;
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RM_SYSCTRL_SRV_CLT"

#define GET_IBINDER \
    sp<IServiceManager> sm = defaultServiceManager(); \
    sp<IBinder> b = sm->getService(String16(SERVICE)); \
    if (b == NULL) \
    { \
	ALOGW("Can't find binder service \"%s\"", SERVICE); \
	return -1; \
    }

int RmSysctrlGetSubsysVersion(int subsys_id, char *ver)
{
    char tmp_str[200] = {0};

    GET_IBINDER;

    //Get sub-system version
    Parcel in1,out1;
    in1.writeInterfaceToken(String16(SERVICE));
    in1.writeInt32(subsys_id);
    int ret = b->transact(SUB_SYSTEM_VERSION, in1, &out1, 0);
    ALOGD("transact(%d) return %d", SUB_SYSTEM_VERSION, ret);
    int result = out1.readInt32();
    if (result != 0)
    {
        ALOGD("Get sub system version failed.");
        return -1;
    }
    const String8 version8(out1.readString8());
    sprintf(tmp_str, "%s", version8.string());
    ALOGD("The RC sub-system version:%s", tmp_str);

    strcpy(ver, tmp_str);

    return 0;
}


int RmSysctrlGetSerialNumber(char *sn, bool *is_valid)
{
    char tmp_str[200] = {0};

    GET_IBINDER;

    //Get serial number
    Parcel in2,out2;
    in2.writeInterfaceToken(String16(SERVICE));
    int ret = b->transact(SERIAL_NUMBER, in2, &out2, 0);
    ALOGD("transact(%d) return %d", SERIAL_NUMBER, ret);
    int result = out2.readInt32();
    if (result != 0)
    {
        ALOGD("Get serial number failed.");
        return -1;
    }
    const String8 sn8(out2.readString8());
    sprintf(tmp_str, "%s", sn8.string());
    ALOGD("The serial number:%s", tmp_str);
    int is_sn_valid = out2.readInt32();
    if (is_sn_valid)
        ALOGD("The serial number is valid.");
    else
        ALOGD("The serial number is NOT valid.");

    strcpy(sn, tmp_str);
    *is_valid = is_sn_valid;

    return 0;
}


int RmSysctrlCheckRobotMotion(bool *is_robot_in_motion)
{
    GET_IBINDER;

    //Check if the robot is in motion
    Parcel in3,out3;
    in3.writeInterfaceToken(String16(SERVICE));
    int ret = b->transact(IS_IN_MOTION, in3, &out3, 0);
    ALOGD("transact(%d) return %d", IS_IN_MOTION, ret);
    int result = out3.readInt32();
    if (result != 0)
    {
        ALOGD("Check if the robot is in motion failed.");
        return -1;
    }
    int is_in_motion = out3.readInt32();
    if (is_in_motion)
        ALOGD("The robot is in motion.");
    else
        ALOGD("The robot is NOT in motion.");

    *is_robot_in_motion = is_in_motion;

    return 0;
}


int RmSysctrlGetSubsysState(int subsys_id, bool *is_enable,
    bool *is_poweron, bool *is_active, int *curr_state,
    int *curr_sub_state, bool *is_init, bool *is_debug,
    int *curr_mode)
{
    char tmp_str[200] = {0};

    GET_IBINDER;

    //Get sub-system state
    Parcel in4,out4;
    in4.writeInterfaceToken(String16(SERVICE));
    in4.writeInt32(subsys_id);
    int ret = b->transact(SUB_SYSTEM_STATE, in4, &out4, 0);
    ALOGD("transact(%d) return %d", SUB_SYSTEM_STATE, ret);
    int result = out4.readInt32();
    if (result != 0)
    {
        ALOGD("Get sub system state failed.");
        return -1;
    }
    int enable = out4.readInt32();
    if (enable)
        ALOGD("The sub-system is enable.");
    else
        ALOGD("The sub-system is NOT enable.");
    int poweron = out4.readInt32();
    if (poweron)
        ALOGD("The sub-system is power on.");
    else
        ALOGD("The sub-system is NOT power on.");
    int active = out4.readInt32();
    if (active)
        ALOGD("The sub-system is active.");
    else
        ALOGD("The sub-system is NOT active.");
    int state = out4.readInt32();
    switch (state)
    {
    case ROBOT_STATUS_INIT:
        ALOGD("The sub-system state is: Init");
        break;
    case ROBOT_STATUS_REGISTER:
        ALOGD("The sub-system state is: Register");
        break;
    case ROBOT_STATUS_LOGINNING:
        ALOGD("The sub-system state is: Loginning");
        break;
    case ROBOT_STATUS_NOT_LOGIN:
        ALOGD("The sub-system state is: Not Login");
        break;
    case ROBOT_STATUS_WORK:
        ALOGD("The sub-system state is: Work");
        break;
    case ROBOT_STATUS_SLEEP:
        ALOGD("The sub-system state is: Sleep");
        break;
    case ROBOT_STATUS_CHARGE:
        ALOGD("The sub-system state is: Charge");
        break;
    case ROBOT_STATUS_ABNORMAL:
        ALOGD("The sub-system state is: Abnormal");
        break;
    case ROBOT_STATUS_HALT:
        ALOGD("The sub-system state is: Halt");
        break;
    case ROBOT_STATUS_SECURITY:
        ALOGD("The sub-system state is: Security");
        break;
    case ROBOT_STATUS_UPGRADE:
        ALOGD("The sub-system state is: upgrade");
        break;
    default:
        ALOGD("Invalid state.");
    }
    int sub_state = out4.readInt32();
    if (ROBOT_STATUS_WORK == state)
    {
        if (sub_state == WORK_IDLE)
            ALOGD("The sub-system sub-state is: idle");
        else if (sub_state == WORK_BUSY)
            ALOGD("The sub-system sub-state is: busy");
        else
            ALOGD("Invalid sub-state");
    }
    else if (ROBOT_STATUS_SLEEP == state)
    {
        if (sub_state == SLEEP_SHALLOW)
            ALOGD("The sub-system sub-state is: sleep shallow");
#ifdef DEEP_SLEEP_SUPPORT
        else if (sub_state == SLEEP_DEEP)
            ALOGD("The sub-system sub-state is: sleep deep");
#endif
        else
            ALOGD("Invalid sub-state");
    }
    else
        ALOGD("It is no sense to sub state.");
    int is_initalized = out4.readInt32();
    if (is_initalized)
        ALOGD("The sub-system is initalized.");
    else
        ALOGD("The sub-system is NOT initalized.");
    int debug_en = out4.readInt32();
    if (debug_en)
        ALOGD("The sub-system is enable debug.");
    else
        ALOGD("The sub-system is NOT enable debug.");
    int mode = out4.readInt32();
    if (mode == DEVELOPER_MODE)
        ALOGD("The sub-system mode is: Developer mode");
    else if (mode == CHILDREN_MODE)
        ALOGD("The sub-system mode is: Children mode");
    else if (mode == GUARDER_MODE)
        ALOGD("The sub-system mode is: Guarder mode");
    else
        ALOGD("Invalid mode");

    *is_enable = enable;
    *is_poweron = poweron;
    *is_active = active;
    *curr_state = state;
    *curr_sub_state = sub_state;
    *is_init = is_initalized;
    *is_debug = debug_en;
    *curr_mode = mode;

    return 0;
}


int RmSysctrlGetSex(int *robot_sex)
{
    GET_IBINDER;

    //Get sex
    Parcel in5,out5;
    in5.writeInterfaceToken(String16(SERVICE));
    int ret = b->transact(GET_SEX, in5, &out5, 0);
    ALOGD("transact(%d) return %d", GET_SEX, ret);
    int result = out5.readInt32();
    if (result != 0)
    {
        ALOGD("Get sex failed.");
        return -1;
    }
    int sex = out5.readInt32();
    if (sex == MALE)
        ALOGD("Male.");
    else if (sex == FEMALE)
        ALOGD("Female.");
    else if (sex == UNKNOWN_SEX)
        ALOGD("Unknown sex.");
    else
        ALOGD("Invalid sex.");

    *robot_sex = sex;

    return 0;
}


int RmSysctrlSetSex(int robot_sex)
{
    GET_IBINDER;

    //Set sex
    Parcel in6,out6;
    in6.writeInterfaceToken(String16(SERVICE));
    in6.writeInt32(robot_sex);
    int ret = b->transact(SET_SEX, in6, &out6, 0);
    ALOGD("transact(%d) return %d", SET_SEX, ret);
    int result = out6.readInt32();
    if (result != 0)
    {
        ALOGD("Set sex failed.");
        return -1;
    }

    return 0;
}


int RmSysctrlGetDatetime(int *o_year, int *o_month,
    int *o_day, int *o_weekday, int *o_hour, int *o_minute,
    int *o_second)
{
    GET_IBINDER;

    //Get datetime
    Parcel in7,out7;
    in7.writeInterfaceToken(String16(SERVICE));
    int ret = b->transact(GET_DATETIME, in7, &out7, 0);
    ALOGD("transact(%d) return %d", GET_DATETIME, ret);
    int result = out7.readInt32();
    if (result != 0)
    {
        ALOGD("Get datetime failed.");
        return -1;
    }
    int year = out7.readInt32();
    int month = out7.readInt32();
    int day = out7.readInt32();
    int weekday = out7.readInt32();
    int hour = out7.readInt32();
    int minute = out7.readInt32();
    int second = out7.readInt32();
    ALOGD("Datetime: %d-%02d-%02d %d %02d:%02d:%02d\n",
          year,
          month,
          day,
          weekday,
          hour,
          minute,
          second);

    *o_year = year;
    *o_month = month;
    *o_day = day;
    *o_weekday = weekday;
    *o_hour = hour;
    *o_minute = minute;
    *o_second = second;

    return 0;
}


int RmSysctrlSetDatetime(int year, int month,
    int day, int weekday, int hour, int minute,
    int second)
{
    GET_IBINDER;

    //Set datetime
    Parcel in8,out8;
    in8.writeInterfaceToken(String16(SERVICE));
    in8.writeInt32(year);
    in8.writeInt32(month);
    in8.writeInt32(day);
    in8.writeInt32(weekday);
    in8.writeInt32(hour);
    in8.writeInt32(minute);
    in8.writeInt32(second);
    int ret = b->transact(SET_DATETIME, in8, &out8, 0);
    ALOGD("transact(%d) return %d", SET_DATETIME, ret);
    int result = out8.readInt32();
    if (result != 0)
    {
        ALOGD("Set datetime failed.");
        return -1;
    }

    return 0;
}

int RmSysctrlGetUptime(int *robot_uptime)
{
    GET_IBINDER;

    //Get uptime
    Parcel in5,out5;
    in5.writeInterfaceToken(String16(SERVICE));
    int ret = b->transact(GET_UPTIME, in5, &out5, 0);
    ALOGD("transact(%d) return %d", GET_UPTIME, ret);
    int result = out5.readInt32();
    if (result != 0)
    {
        ALOGD("Get uptime failed.");
        return -1;
    }
    int uptime = out5.readInt32();

    ALOGD("%d seconds", uptime);

    *robot_uptime = uptime;

    return 0;
}

